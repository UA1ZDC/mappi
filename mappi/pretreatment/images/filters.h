#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_FILTERS_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_FILTERS_H

#include <qvector.h>

namespace meteo {
  void median3(uint rows, uint cols, QVector<uchar>* data);
  void median5(uint rows, uint cols, QVector<uchar>* data);
  void nagaoMacuyamaModify(uint rows, uint cols, QVector<uchar>* data);
  
  // template<class T> void insertionSort(T arr[], int size);
  // template<class T> void median3(uint rows, int ucols, const QVector<T>& data, QVector<T>* dst);
}



#endif
