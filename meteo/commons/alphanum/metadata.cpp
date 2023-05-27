#include "metadata.h"
#include "tmeteoelement.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/proto/alphanum.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/primarydb/dbmeta.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/nosql/document.h>

#include <qvector.h>

#include <math.h>

#define DECODERS_DIR  QString(MnCommon::projectPath() + "/var/meteo/decoders/")
#define CODEFORM_FILE_XML  QString(MnCommon::projectPath() + "/etc/meteo/decoders/codeforms.xml")

template<> meteo::anc::TMetaData* meteo::anc::MetaData::_instance = 0;

using namespace meteo;
using namespace anc;



TMetaData::TMetaData() :
  _isLoaded(false)
{
  _dataElements = new MeteoElement;
}

TMetaData::~TMetaData()
{
  if (_dataElements) delete _dataElements;
}

int TMetaData::load(const QString& confFile, const QList<int>& decodeTypes)
{
  if (_isLoaded) return 0;

  QFile file(confFile);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(confFile);
    return -1;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &_conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла конфигурации");
    return -1;
  }

  QStringList forms;
  for (int idx = 0; idx < _conf.report_size(); idx++) {
    forms << QString::fromStdString(_conf.report(idx));
  }

  return load(forms, decodeTypes);
}

int TMetaData::loadConf(const QStringList& confList, const QList<int>& decodeTypes)
{
  if (_isLoaded) return 0;

  QStringList forms;

  for (int idx = 0; idx < confList.size(); idx++) {
    QString confFile = confList.at(idx);

    QFile file(confFile);
    if ( !file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(confFile);
      continue;
    }

    QString text = QString::fromUtf8(file.readAll());
    file.close();

    meteo::anc::DecodingTypes conf;
    if ( !TProtoText::fillProto(text, &conf) ) {
      error_log << QObject::tr("Ошибка в структуре файла конфигурации");
      continue;
    }
    _conf.MergeFrom(conf);

    for (int idx = 0; idx < conf.report_size(); idx++) {
      forms << QString::fromStdString(conf.report(idx));
    }
  }

  return load(forms, decodeTypes);
}

//! Загрузка xml для раскодирования
/*!
  \param usedForms Список типов используемых кодовых форм (для них будут згаружены шаблоны)
  \return 0 - если всё норм
*/
int TMetaData::load(const QList<QString>& usedForms, const QList<int>& decodeTypes)
{
  if (_isLoaded) return 0;

  if ( false == loadXmlCodeforms(CODEFORM_FILE_XML, decodeTypes) ) { //загружаем список кодовых форм
    return -1;
  }
  // var(decodeTypes);
  // var(usedForms);

  if ( false == loadXmlDecoders(usedForms) ) { //загружаем список шаблонов для кодовых форм
    return -1;
  }

  _isLoaded = true;

  return 0;
}

bool TMetaData::isCodeLoaded(int num)
{
  return _domDecoders.contains(num);
}

QDomDocument TMetaData::domDecoder(int num)
{
  return _domDecoders.value(num);
}

DecoderFormat TMetaData::format(const QString& type)
{
  int num = _forms.value(type, -1);

  if (! _format.contains(num)) {
    error_log << QObject::tr("Неизвестный формат %1").arg(type);
    return kUnkFormat;
  }

  return _format.value(num);
}

DecoderFormat TMetaData::format(int num)
{
  if (! _format.contains(num)) {
    error_log << QObject::tr("Неизвестный формат. номер = %1").arg(num);
    return kUnkFormat;
  }

  return _format.value(num);
}


bool TMetaData::loadXmlDecoders(const QList<QString>& usedForms)
{
  if ( _forms.count() < 1 ) {
    //error_log << QObject::tr("Ошибка. Не загружена ни одна кодовая форма.");
    return false;
  }

  QString dirname = DECODERS_DIR;
  QString filename;

  for (int idx = 0; idx < usedForms.count(); idx++) {
    if (! _forms.contains(usedForms.at(idx))) {
      error_log << QObject::tr("Неизвестная кодовая форма '%1'").arg(usedForms.at(idx));
      continue;
    }
    filename = dirname +"/"+ usedForms.at(idx) + ".xml" ;
    //debug_log << "filename=" << filename << _forms.value(usedForms.at(idx));
    QDomDocument domDecoder;
    QFile fl(filename);
    if(!domDecoder.setContent(&fl)) {
      error_log << QObject::tr("Ошибка загрузки шаблона кодовой формы '%1'. Файл '%2'").arg(usedForms.at(idx)).arg(filename);
      continue;
    }
    _domDecoders[_forms.value(usedForms.at(idx))] = domDecoder;

  }

 return true;
}


bool TMetaData::loadXmlCodeforms(const QString& filename, const QList<int>& decodeTypes)
{
  QDomDocument codeforms;
  QString typeAttribute, activeAttribute;
  bool ok;
  int decodetype;

  QFile fl(filename);
  if(!codeforms.setContent(&fl)) {
    error_log << QObject::tr("Ошибка загрузки кодовых форма из xml-файлов");
    return false;
  }

  QDomNodeList codes = codeforms.documentElement().childNodes();

  int num = 0;
  for (int idx = 0; idx < codes.count(); idx++) {
    if (!codes.item(idx).isElement()) { continue; }
    typeAttribute = codes.item(idx).toElement().attribute("type");
    if (typeAttribute.isEmpty()) { continue; }
    decodetype = codes.item(idx).toElement().attribute("decodetype").toInt(&ok);
    if (!ok) {
      error_log << QObject::tr("Ошибка считывания значения типа раскодировки decodetype=%1").
        arg(codes.item(idx).toElement().attribute("decodetype"));
      continue;
    }

    if (!decodeTypes.contains(decodetype)) {
      continue;
    }

#ifdef PARSE_DEBUG_LOG
    //debug_log << i << typeAttribute << decodetype;
#endif

    _forms.insert(typeAttribute, num);
    _format.insert(num, DecoderFormat(decodetype));

    int prevnum = num;
    ++num;
    QDomNodeList childs = codes.item(idx).childNodes();
    for(int j = 0; j< childs.count(); j++) {
      QDomElement el = childs.item(j).toElement();
      if (el.tagName() == "group") {
        _headers.insert(prevnum, QRegExp(el.text()));	
      } else if (el.tagName() == "code") { //для составных, типа гелио
        _forms.insert(el.attribute("type"), num);
        QStringList gr = el.attribute("group").split(",");
        for (int gridx = 0; gridx < gr.size(); gridx++) {
          _subgroups.insert(num, gr.at(gridx).trimmed());
        }
        _format.insert(num, DecoderFormat(decodetype));
        ++num;
      }
    }
  }

  // var(_headers);
  // var(_forms);
  // var(_format);
  // var(_subgroups);

  return true;
}

void TMetaData::setMeteoData(TMeteoData* data)
{
  _dataElements->setMeteoData(data);
}


//! возвращает код для текстового значения (явления w_w_)
int TMetaData::bufrcode(const QString& text)
{
  if (nullptr == _dbmeta) {
    return BAD_METEO_ELEMENT_VAL;
  }

  return _dbmeta->bufrcode(text);
}

QString TMetaData::currentCountry(int category, const QString& station)
{
  if (nullptr == _dbmeta) {
    return QString();
  }

  const StationInfo* meta = _dbmeta->stationInfo(category, station);
  if (nullptr == meta) {
    warning_log << QObject::tr("Нет данных по станции %1(%2)").arg(station).arg(category);
    return QString();
  }

  return QString::number(meta->country);
}


