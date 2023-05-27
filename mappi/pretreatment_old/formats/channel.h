#ifndef MAPPI_PRETREATMENT_FORMATS_SATCHANNEL_H
#define MAPPI_PRETREATMENT_FORMATS_SATCHANNEL_H

#include <qvector.h>
#include <qimage.h>
#include <qfile.h>
#include <qdir.h>

#include <cross-commons/debug/tlog.h>
#include <mappi/global/streamheader.h>


#include <limits>

namespace mappi {
namespace po {

struct ChannelProperty {
  uint rows = -1; //!< количество строк
  uint cols =  0; //!< количество колонок
  int num   = -1; //!< номер канала
  QString name;
  //тип данных TODO
};

//! данные канала прибора спутника с построчным сканированием
template<class T> class Channel {

  //если добавлять тип, надо в ТО сделать его чтение
  static_assert(std::is_same<T, short>::value ||
  std::is_same<T, ushort>::value ||
  std::is_same<T, float>::value,
  "Not valid type. Allowed only short, ushort, float");

public:

  Channel() = default;
  ~Channel() = default;

  void setup(uint rows, uint cols, uint num);

  void clear();
  //      void resetRT(int rows);
  void setNum(uint num) { _prop.num = num; }
  void setName(const QString& name) { _prop.name = name; }
  void setData(const QVector<T>& data, uint rows, uint cols);
  void setLimits(int min, int max) { _min = min; _max = max; }
  void setMask(uint mask) { _mask = mask; }
  void resize(uint64_t size);

  const ChannelProperty& property() const { return _prop; }
  uint number() const  { return _prop.num; }
  QString name() const  {
    if (_prop.name.isEmpty()) return QString::number(_prop.num).rightJustified(2, '0');
    return _prop.name;
  }
  uint columns() const { return _prop.cols; }
  uint rows() const { return _prop.rows; }
  uint size() const { return _data.size(); }
  T minLimit() const { return _min; }
  T maxLimit() const { return _max; }
  const QVector<T>& data() const { return _data; }
  QVector<T>& data() { return _data; }
  QByteArray imgRawData(int thinning, int* resRows, int* resCols) const;

  void set(uint idx, T val);
  void update(uint idx, T val) { _data[idx] = val; }

  T get(uint idx) { return _data[idx]; }
  T get(uint col, uint row) { return _data[_prop.cols * row + col]; }

  QVector<uchar> toImageData() const;

  QString saveData(const QString& baseName, meteo::global::PoHeader& pohead) const;

private:
  void reset(uint rows, uint cols);

private:
  QVector<T> _data;
  T _min = std::numeric_limits<T>::min(); //!< Минимально возможное значение в канале (напр., некалиброванный AVHRR3 - 11 бит, калиброванный 1 канал 0-100%)
  T _max = std::numeric_limits<T>::max(); //!< Максимально возможное значение в канале
  uint _mask = 0xffffffff;

  ChannelProperty _prop;

  int _prevRow = -1; //!< Количество строк сканирования с учётом предыдущих порций данных (для обработки данных в реальном времени)

  bool _isEmpty = true;
};


template<class T> class ChannelVector : public QVector<Channel<T>> {
public:
  ChannelVector() {}
  ~ChannelVector() {}

  // void resetRT() {
  // 	for (int idx = 0; idx < QVector<Channel<T>>::size(); idx++) {
  // 	  QVector<Channel<T>>::operator[](idx).resetRT(0);
  // 	}
  // }
};




/*!
      \param rows количество строк
      \param cols количество столбцов
      \param num номер канала
    */
template<class T> void Channel<T>::setup(uint rows, uint cols, uint num)
{
  _prop.num = num;
  _prop.name = QString::number(num);
  reset(rows, cols);
}

template<class T> void Channel<T>::clear()
{
  _prevRow = _prop.rows;
  _data.clear();
  _data.resize(_prop.rows*_prop.cols);
  _isEmpty = true;
}

//! Сброс данных, установка нового размера данных
/*!
      \param rows количество строк
      \param cols количество колонок
    */
template<class T> void Channel<T>::reset(uint rows, uint cols)
{
  _prop.rows = rows;
  _prop.cols = cols;
  clear();
}

//! Очистка, установка нового количества строк
// template<class T> void Channel<T>::resetRT(int rows)
// {
//   _prop.rows = rows;
//   _data.clear();
//   _data.resize(_prop.rows*_prop.cols);
//   _prevRow += _prop.rows;
// }

//! Установка данных
/*!
      \param data данные
      \param rows количество строк
      \param cols количество столбцов
    */
template<class T> void Channel<T>::setData(const QVector<T>& data, uint rows, uint cols)
{
  _data = data;
  _prop.rows = rows;
  _prop.cols = cols;
  _prevRow = rows;
  _isEmpty = false;
  //  _idx = _data.size() - 1;
}

template<class T> void Channel<T>::resize(uint64_t size)
{
  _prop.rows = size / _prop.cols;
  _data.resize(size);
}

//! Установить значение
template<class T> void Channel<T>::set(uint idx, T val)
{
	if (idx >= (uint)_data.size()) {
		if (idx >= _data.size() + _prop.cols) {
			error_log << QObject::tr("Указанный индекс выходит за пределы допустимого размера данных. "
				   "Индекс = %1. Размер = %2. Канал = %3").arg(idx).arg(_data.size() + _prop.cols).arg(_prop.num);
			return;
		}
		++_prop.rows;
		_data.resize(_data.size() + _prop.cols);
	}

	_data[idx] = val;
	_isEmpty = false;
	// if (idx > _idx) {
	//   _idx = idx;
	// }
}

//! Установить значение в следующую пустую ячейку
// void Channel::add(float val)
// {
//   ++_idx;
//   if (_idx >= _data.size()) {
//     ++_rows;
//     _data.resize(_data.size() + _cols);
//   }

//   _data[_idx] = val;
// }


//! К 0-255
template<class T> QVector<uchar> Channel<T>::toImageData() const
{
  trc;
  if (_data.size() == 0 || _isEmpty) return QVector<uchar>();

  QVector<uchar> imData(_data.size());

  uint8_t add = 0; //чтоб 0 значение не было, исп как не валидное
  for(int idx = 0; idx < _data.size(); idx++) {
    T value = _data.at(idx) & _mask;
    if (value > _max) {
      //debug_log << "max" << _prop.name << idx << _data.at(idx) << _min << _max;
      imData[idx] = _max;
    } else if (value < _min + add) {
      //debug_log << "min" << _prop.name << idx << _data.at(idx) << _min << _max;
      imData[idx] = _min + add;
    }

    imData[idx] = uchar((value - _min + add) * 255 / (_max - _min + add));

    // if (_num == 5) {
    //   debug_log << idx << _data.at(idx) << imData[idx];
    // }
  }
  return imData;
}



//! Сохранение данных
template<class T> QString Channel<T>::saveData(const QString& baseName, meteo::global::PoHeader& pohead) const
{
  trc;
  debug_log << _data.size() << _isEmpty;

  if (_data.size() == 0 || _isEmpty) return QString();

  pohead.channel = _prop.num;
  pohead.lines = _prop.rows;
  pohead.samples = _prop.cols;
  pohead.type_hash = typeid(T).hash_code();
  pohead.min = _min;
  pohead.max = _max;
  pohead.undef = -9999;

  QDir dir;
  dir.mkpath(baseName.left(baseName.lastIndexOf("/")));

  QByteArray bah;
  pohead >> bah;

  QDate cur = QDateTime::currentDateTimeUtc().date();
  meteo::global::PreHeader pre;
  pre.type = meteo::global::kPretrFile;
  pre.year = cur.year();
  pre.month = cur.month();
  pre.day  = cur.day();
  pre.offset = bah.size();

  // for (uint idx = 1000*_prop.cols+1000; idx < 1000*_prop.cols+1000 + 30; idx++) {
  // 	debug_log << _data[idx];
  // }

  QFile file(baseName + QString::number(_prop.num).rightJustified(2, '0') + ".po");
  file.open(QIODevice::WriteOnly);
  QDataStream ds(&file);
  ds << pre;
  ds << pohead;
  ds << _data;
  file.close();

  return file.fileName();
}

//! Исходные данные спутника, приведённые к 1 байту, прореженные в соответствии с коэффициентом thinning
template<class T> QByteArray Channel<T>::imgRawData(int thinning, int* resRows, int* resCols) const
{
  QByteArray ba;
  ba.reserve(_data.size() / thinning / thinning);

  int shift = 2;
  if (_max == 0xff) {
    shift = 0;
  }

  int cnt_row = 0;
  int cnt_col = 0;
  int thCols = _prop.cols - (_prop.cols % thinning*4); //для выравнивания
  //for (int kk = 0; kk < _data.size(); kk++) {
  for (uint row = 0; row < _prop.rows; ++row) {
    if ((_prevRow - _prop.rows + row) % thinning == 0) {
      ++cnt_row;
      cnt_col = 0;
      for (int col = 0; col < thCols; col += thinning) {
        ++cnt_col;
        ba.append(char((int)_data[row*_prop.cols + col] >> shift));
      }
      // if (cnt_col % 4 != 0) { //выравнивание изображения
      // 	ba.append(4 - (cnt_col % 4), char(0));
      // 	cnt_col += 4 - (cnt_col % 4);
      // }
    }
  }

  // var(ba.size());
  // var(cnt_row);
  // var(cnt_col);
  if (0 != resRows) {
    *resRows = cnt_row;
  }
  if (0 != resCols) {
    *resCols = cnt_col;
  }

  // var(*resRows);
  // var(*resCols);

  return ba;
}



}
}

#endif
