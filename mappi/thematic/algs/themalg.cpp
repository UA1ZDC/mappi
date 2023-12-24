#include "themalg.h"
#include "channel.h"
#include "datastore.h"
#include <commons/textproto/tprototext.h>

#include <commons/mathtools/mnmath.h>

#include <mappi/settings/mappisettings.h>

#include <qsharedpointer.h>
#include <QFile>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

using namespace mappi;
using namespace to;

ThemAlg::ThemAlg(QSharedPointer<to::DataStore>& ds): _ds(ds)
{
}


ThemAlg::~ThemAlg()
{
  _ch.clear();
}


bool ThemAlg::readConfig(const QString& filename, ::google::protobuf::Message* conf, QList<conf::InstrumentType>* valid)
{
  if (nullptr == valid) {
    return false;
  }
    
  QFile file(filename);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке параметров файла '%1'").arg(filename);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла параметров '%1'").arg(filename);
    return false;
  }

  const google::protobuf::Reflection * refl = conf->GetReflection();
  const google::protobuf::FieldDescriptor * field = conf->GetDescriptor()->FindFieldByName("them");
  int size = refl->FieldSize(*conf, field);
 
  for (int idx = 0; idx < size; idx++) {
    auto them = refl->MutableRepeatedMessage(conf, field, idx);
    const google::protobuf::Reflection * refl_them = them->GetReflection();
    const google::protobuf::FieldDescriptor * field_instr = them->GetDescriptor()->FindFieldByName("instr");
    conf::InstrumentType itype = conf::InstrumentType(refl_them->GetEnumValue(*them, field_instr));
    if (!valid->contains(itype)) {
      valid->append(itype);
    }
  }
  
  return true;
}

//! Обработка данных спутника
bool ThemAlg::process(const QDateTime& start, const QString& satname, QList<conf::InstrumentType> instruments)
{
  bool ok = false;

  if (_validInstr.isEmpty()) {
    ok = readConfig(&_validInstr);
    if (!ok) {
      return false;
    }
  }
  
  for (auto instr : instruments) {
    if (_validInstr.contains(instr)) {
      ok |= process(start, satname, instr);
    }
  }

  return ok;
}


//! Обработка данных прибора спутника
bool ThemAlg::process(const QDateTime& start, const QString& satname, conf::InstrumentType instrument)
{
  if (_ds.isNull()) {
    return false;
  }

  QList<uint8_t> chNum;
  bool ok = channelsList(instrument, &chNum);
  var(chNum);

  QMap<uint8_t, QString> chfiles; //номер канала, название файла
  _ds->getFiles(start, satname, instrument, chNum, &chfiles);

  // chfiles.insert(1, "/home/maria/satdata/pretreatment/20210720_0847/_GOMS-3_9_01.po");
  // chfiles.insert(2, "/home/maria/satdata/pretreatment/20210720_0847/_GOMS-3_9_02.po");
  // chfiles.insert(3, "/home/maria/satdata/pretreatment/20210720_0847/_GOMS-3_9_03.po");
     

  if (chNum.size() != chfiles.size()) { 
    error_log << QObject::tr("Нет данных всех необходимых каналов. Требуется: ") << chNum
  	      << QObject::tr("В наличии: ") << chfiles;
    return false;
  }
  
  for (auto number : qAsConst(chNum)) {
    ok = readChannelData(number, chfiles.value(number));
    if (!ok) {
      _ch.clear();
      return false;
    }
  }

  ok = process(instrument);
  
  return ok;
}


//! Чтение данных каналов из файлов
bool ThemAlg::readChannelData(uint8_t number, const QString& fileName)
{
    
  QSharedPointer<Channel> sch = QSharedPointer<Channel>(new Channel());
  bool ok = sch->readData(fileName);
  if(ok) {
    _ch.insert(number, sch);
  }
  return ok;
}


void ThemAlg::normalize(int min, int max)
{
  for(auto it : qAsConst(_ch)) {
    if (it->number() >= 4 && it->header().satellite == "METEOR-M 2") {  //TODO пока так для макета, и-за ИК.  Должно быть в настройках, зависит от калибровки
      MnMath::normalize(it->data(), it->size(), it->min(), it->max(), max, min);
    } else {
      MnMath::normalize(it->data(), it->size(), it->min(), it->max(), min, max);
    }
  }
}

//! путь для сохранения и шаблон имени (его начало)
void ThemAlg::getSavePath(const QDateTime& start, const QString& satname, QString* path, QString* templName)
{
  if (nullptr == path || nullptr == templName) {
    return;
  }
  
  *path = MnCommon::varPath() + "/thematics/" + start.toString("yyyy-MM-dd/");
  QDir dir;
  dir.mkpath(*path);
  
  *templName = start.toString("yyyyMMddmmhh_") + satname;
  templName->remove(' ');
}

bool ThemAlg::saveData(const QString& baseName)
{
  if(_data.isEmpty() || channels().isEmpty()) {
    return false;
  }

  QFile file(baseName + ".to");
  if(!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  auto src = channels().first();
  if (nullptr == src) {
    return false;
  }

  QDataStream stream(&file);
  stream << src->header();
  stream.writeRawData((const char*)(_data.data()), _data.length());

  file.close();

  store()->save(src->header(), type(), file.fileName());

  return true;
}

bool ThemAlg::saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format)
{
  if(_data.isEmpty() || channel.isNull()) {
    return false;
  }
  QImage im;
  switch (format) {
    case QImage::Format_RGB32:
      im = QImage(_data.data(), channel->columns(), channel->rows(), channel->columns()*4, QImage::Format_RGB32);
    break;
    default:
      im = QImage(_data.data(), channel->columns(), channel->rows(), channel->columns(), format);
      im.setColorCount(256);
      im.setColorTable(_palette);
    break;
  }

  bool ok = im.save(name + ".png", "PNG");
  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения файла %1").arg(name);
  }

  debug_log << "write" << name + ".png";

  return ok;
}

