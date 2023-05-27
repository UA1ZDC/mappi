#include "channel.h"

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

  if (_header.instr == mappi::conf::kInstrUnk) {
    error_log << QObject::tr("В файле записан неизвестный прибор");
    file.close();
    return false;
  }

  if (-1 == _header.channel){
    error_log << QObject::tr("В файле записан неизвестный канал");
    file.close();
    return false;
  }

  if (_header.type_hash == typeid(float).hash_code()) ds >> *this;
  else if (_header.type_hash == typeid(short).hash_code()) putValues<short>(ds);
  else if (_header.type_hash == typeid(ushort).hash_code()) putValues<ushort>(ds);
  else if (_header.type_hash == typeid(uint8_t).hash_code()) putValues<uint8_t>(ds);
  else if (_header.type_hash == typeid(uint16_t).hash_code()) putValues<uint16_t>(ds);
  else error_log << QObject::tr("hash_code не соответствует ни одному поддерживаемому типу");
  file.close();

  if (0 == size()) {
    error_log << QObject::tr("Размер данных в файле равен 0");
    return false;
  }

  calculateMaxMin();
  
  return true;
}

template<typename T>
bool to::Channel::putValues(QDataStream &ds) {
  QVector <T> shdata;
  ds >> shdata;
  reserve(shdata.size());
  std::copy(shdata.cbegin(), shdata.cend(), std::back_inserter(*this));
  squeeze(); //free unused memory because vector will not expand anymore

  //TODO: Использовать 0 или -1 для беззнаковых типов?
  if(std::is_unsigned<T>::value){
    for (int idx = 0; idx < size(); idx++) {
      if(at(idx) == 0) operator[](idx) = INVALID_VALUE;
    }
  }
  return true;
}

void to::Channel::scale(int new_size){
  if(size() == new_size) return;
  debug_log << QObject::tr("Изменяем размер массива (%1) -> (%2)").arg(size()).arg(new_size);

  double scaleFactor = 1.0 * size() / new_size;

  QVector<float> oldData(*this);
  resize(new_size);
  _header.lines = static_cast<int>(rows() * scaleFactor);
  _header.samples = static_cast<int>(columns() * scaleFactor);

  for (int i = 0; i < new_size; i++) {
    int oldIndex = static_cast<int>(i * scaleFactor);
    operator[](i) = oldData.value(oldIndex, INVALID_VALUE);
  }
};
void to::Channel::scale(int new_rows, int new_columns){
  if(rows() == new_rows && columns() == new_columns) return;
  debug_log << QObject::tr("Изменяем размер массива (%1х%2) -> (%3х%4)")
                  .arg(columns())
                  .arg(rows())
                  .arg(new_columns)
                  .arg(new_rows);

  double scaleFactorRow = 1.0 * rows() / new_rows;
  double scaleFactorCol = 1.0 * columns() / new_columns;
  if(scaleFactorRow == scaleFactorCol) return scale(new_rows * new_columns);

  QVector<float> oldData(*this);
  resize(new_rows*new_columns);
  _header.lines = new_rows;
  _header.samples = new_columns;

  for(int row=0; row < new_rows; row++){
    for(int col=0; col < new_columns; col++){
      int linearIndex = row * new_columns + col;
      int scaledRowIdx = scaleFactorRow == 1.0 ? row : static_cast<int>(row * scaleFactorRow);
      int scaledColIdx = scaleFactorCol == 1.0 ? col : static_cast<int>(col * scaleFactorCol);
      int oldIndex = scaledRowIdx * new_columns + scaledColIdx;
      operator[](linearIndex) = oldData.value(oldIndex, INVALID_VALUE);
    }
  }
};

void to::Channel::calculateMaxMin(){
  _min = std::numeric_limits<float>::max();
  _max = std::numeric_limits<float>::min();
  for (int idx = 0; idx < size(); idx++) {
    float val = at(idx);
    if (qFuzzyCompare(val, INVALID_VALUE)) continue;
    if (val > _max) _max = val;
    if (val < _min) _min = val;
  }
}