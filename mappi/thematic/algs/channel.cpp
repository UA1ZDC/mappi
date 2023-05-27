#include "channel.h"

#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>

#include <QFile>

using namespace mappi;


bool to::Channel::readHeader(const QString& fileName, meteo::global::PoHeader* header)
{
  if (nullptr == header) {
    return false;
  }
  
  QFile file(fileName);
  bool ok = file.open(QIODevice::ReadOnly);
  if (!ok) {
    error_log << meteo::msglog::kFileOpenError.arg(fileName).arg("");
    return false;
  }

  ok = false;
  QDataStream ds(&file);
  meteo::global::PreHeader preHeader;
  ds >> preHeader;
  if (preHeader.type == meteo::global::kPretrFile) {
    ds >> *header;
    ok = true;
  } else {
    error_log << QObject::tr("Формат файла не соответствует данным предварительной обработки");
  }    
  
  file.close();
  return ok;
  
}

bool to::Channel::readData(const QString& fileName)
{
  QFile file(fileName);
  bool ok = file.open(QIODevice::ReadOnly);
  if (!ok) {
    error_log << meteo::msglog::kFileOpenError.arg(fileName).arg("");
    return false;
  }


  QDataStream ds(&file);
  meteo::global::PreHeader preHeader;
  ds >> preHeader;
  if (preHeader.type != meteo::global::kPretrFile) {
    error_log << QObject::tr("Формат файла не соответствует данным предварительной обработки");
    file.close();
    return false;
  }

  ds >> _header;  

  if (_header.instr == mappi::conf::kInstrUnk || 
      -1 == _header.channel) {
    var(_header.instr);
    var(_header.channel);
    var(size());
    file.close();
    return false;
  }
  
  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();

  //TODO это надо сделать шаблонной функцией. Или реализовать оператор >> из qdatastream
  if (_header.type_hash == typeid(float).hash_code()) {
    QVector<float> shdata;
    ds >> shdata;
    resize(shdata.size());
    for (int idx = 0; idx < shdata.size(); idx++) {
      operator[](idx) = shdata.at(idx);
      if (at(idx) == -9999) continue; //TODO
      if (at(idx) > max) max = at(idx);
      if (at(idx) < min) min = at(idx);
      
    }
  }
  else if (_header.type_hash == typeid(short).hash_code()) {
    QVector<short> shdata;
    ds >> shdata;
    resize(shdata.size());
    for (int idx = 0; idx < shdata.size(); idx++) { 
      operator[](idx) = shdata.at(idx);
      if (at(idx) == -9999) continue; //TODO
      if (at(idx) > max) max = at(idx);
      if (at(idx) < min) min = at(idx);      
    }
  }
  else if (_header.type_hash == typeid(ushort).hash_code()) {
    QVector<ushort> shdata;
    ds >> shdata;
    resize(shdata.size());
    for (int idx = 0; idx < shdata.size(); idx++) { 
      operator[](idx) = shdata.at(idx);
      if (at(idx) == -9999) continue; //TODO
      if (at(idx) > max) max = at(idx);
      if (at(idx) < min) min = at(idx);
    }
  }  
  

  file.close();

  if (min != std::numeric_limits<float>::max()) {
    _min = min;
  }
  if (max != std::numeric_limits<float>::min()) {
    _max = max;
  }

  if (0 == size()) {
    return false;
  }
  
  return true;
}
