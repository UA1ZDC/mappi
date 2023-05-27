#include "cliwaremsg.h"
#include "metadata.h"
#include "tmeteoelement.h"
#include "xmllinedecode.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/surface.pb.h>

#include <QJsonArray>
#include <QFile>
#include <QDomDocument>

//"abrev" похоже что повторяется в разных типах сводок
#define KEY_XML_FIELD "ClN"
#define KEY_JSON_FIELD "Column"

using namespace meteo;
using namespace anc;

#define CONF_FILE  QString(MnCommon::etcPath("meteo") + "/decoders/clidescriptors.xml")

CliwareMsg::CliwareMsg()
{
  readConfig();
}


CliwareMsg::~CliwareMsg()
{
  delete _data;
  _data = nullptr;
  delete _lineDecoder;
  _lineDecoder = nullptr;

}

void CliwareMsg::readConfig()
{
  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(CONF_FILE);
    return;
  }
  
  QDomDocument dom;
  if(!dom.setContent(&file)) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(CONF_FILE);
    file.close();
    return;
  }

  QDomNode node = dom.documentElement().firstChild();
   while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "clitype") {
  	readCliType(el.firstChild());
      } else if (el.tagName() == "descriptors") {
  	readDescriptors(el.firstChild());
  	break;
      }
    }
    node = node.nextSibling();
  }

   file.close();
}

void CliwareMsg::readCliType(QDomNode node)
{
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (!el.hasAttribute("id") || !el.hasAttribute("datatype")) {
	error_log << QObject::tr("Ошибка описания типа данных id=%1 datatype=%2").
	  arg(el.attribute("id")).arg(el.attribute("datatype"));
	
      } else if (_types.contains(el.attribute("id"))) {
	error_log << QObject::tr("Ошибка. Повторение дескриптора с идентификатором '%1'").arg(el.attribute("id"));
	
      } else {
	bool ok;
	int datatype = el.attribute("datatype").toUInt(&ok);
	if (!ok) {
	  error_log << QObject::tr("Ошибка типа данных id=%1 type=%2").
	    arg(el.attribute("id")).arg(el.attribute("datatype"));
	} else {
	  _types.insert(el.attribute("id"), datatype);
	}
      }
    } 
      
    node = node.nextSibling();
  }
}


void CliwareMsg::readDescriptors(QDomNode node)
{
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (!el.hasAttribute(KEY_XML_FIELD) || (!el.hasAttribute("outDescr") && !el.hasAttribute("descrName"))) {
	error_log << QObject::tr("Ошибка описания дескриптора %1=%2 outDescr=%3 descrName=%4")
	  .arg(KEY_XML_FIELD).arg(el.attribute(KEY_XML_FIELD))
	  .arg(el.attribute("outDescr")).arg(el.attribute("descrName"));
	
      } else if (_descr.contains(el.attribute(KEY_XML_FIELD))) {
	error_log << QObject::tr("Ошибка. Повторение дескриптора '%1'").arg(el.attribute(KEY_XML_FIELD));
	
      } else {
	_descr.insert(el.attribute(KEY_XML_FIELD).toUpper(), DescrDefinition(el.attribute("outDescr"), el.attribute("descrName"), node));
	  
      }
    } 
    node = node.nextSibling();
  }
}

int CliwareMsg::parse(const QMap<QString, QString>& type, const QJsonDocument& json)
{
  Q_UNUSED(type);
  //_idPtkpp = type.value("id");

  
  delete _data;
  _data = nullptr;
  _cliNames.clear();
  _msgList = QJsonArray();
  _msgIdx = 0;
  delete _lineDecoder;
  _lineDecoder = nullptr;


  
  
  _code = json["Id"].toString();
  _category = getCategory(_code);
  //  debug_log << "code" << _code << _category;
  if (_category == meteo::surf::kUnknownDataType) {
    return 0;
  }
    
  _cliNames = json["Def_aaData"][KEY_JSON_FIELD].toArray().toVariantList();
  _msgList = json["aaData"].toArray();

  if (_cliNames.isEmpty() || _msgList.isEmpty()) {
    return 0;
  }

  
  return _msgList.size();
}

bool CliwareMsg::hasNext()
{
  return _msgIdx < _msgList.size();
}

bool CliwareMsg::decodeNext()
{
  if (_msgIdx >= _msgList.size()) {
    return false;
  }

  bool ok = decode(_cliNames, _msgList.at(_msgIdx).toArray().toVariantList());
  ++_msgIdx;

  return ok;  
}

bool CliwareMsg::decode(const QVariantList& cliNames, const QVariantList& msg)
{
  if (cliNames.size() != msg.size()) {
    error_log << QObject::trUtf8("Не совпадает размер массива названий и значений %1, %2").arg(cliNames.size()).arg(msg.size());
    return false;
  }

  _lineDecoder = new XmlLineDecoder;
  _data = new TMeteoData;
  MetaData::instance()->setMeteoData(_data);

  
  // _data->set(TMeteoDescriptor::instance()->descriptor("level_type"),
  // 	     QString::number(meteodescr::kSurfaceLevel),
  // 	     meteodescr::kSurfaceLevel, control::RIGHT);
  _data->set(TMeteoDescriptor::instance()->descriptor("category"),
	     QString::number(_category),
	     _category, control::RIGHT);
  
  for (int idx = 0; idx < msg.size(); idx++) {
    addCliValue(cliNames.at(idx).toString().trimmed().toUpper(), msg.at(idx).toString().trimmed());
  }

  //  _data->printData();
  
  return true;
}


int CliwareMsg::getCategory(const QString& cliId)
{
  if (!_types.contains(cliId)) {
    error_log << QObject::tr("Неизвестный тип данных %1").arg(cliId);
    return meteo::surf::kUnknownDataType;
  }
  
  return _types.value(cliId);
}

bool CliwareMsg::addCliValue(const QString& abrev, const QString& cliVal)
{ 
  if (!_descr.contains(abrev)) {
    error_log << QObject::tr("Неизвестное название параметра %1").arg(abrev);
    return false;
  }

  bool ok = false;
  QString descrname;
  float value = BAD_METEO_ELEMENT_VAL;

  QString code = cliVal;
  
  if (!_descr.value(abrev).descrname.isEmpty()) {
    descrname = _descr.value(abrev).descrname;
    value = cliVal.toFloat(&ok);
    if (!ok) {
      value = BAD_METEO_ELEMENT_VAL;
    }
    _data->add(descrname, TMeteoParam(code, value, control::NO_CONTROL));
    return true;    
  }
  

  if (!_descr.value(abrev).andescr.isEmpty()) {
    bool ok = _lineDecoder->setMeteoDataValue(_code,  _descr.value(abrev).node, cliVal, _data);
    if (!ok) return false;

    
    // QString andescr = _descr.value(name).andescr;
    // control::QualityControl quality = control::NO_CONTROL;

    
    // if (MetaData::instance()->dataElements()->contains(andescr)) {
    //   value = MetaData::instance()->dataElements()->getValue(andescr, code, &quality);
    //   _data->add(descrname, TMeteoParam(code, value, control::NO_CONTROL));
    // } else {
    //   error_log << QObject::tr("Неизвестный дескриптор %1").arg(andescr);
    // }
    
  }

  
  return true;
}
