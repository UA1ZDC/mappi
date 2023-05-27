#include "filters.h"

#include <cross-commons/debug/tlog.h>

#include <commons/mathtools/mnmath.h>

namespace meteo {

//Сортировка вставками
template<class T> void insertionSort(T arr[], int size)
{
  int idx, j;
  T key;
  
  for (idx = 1; idx < size; idx++) {
    key = arr[idx];
    j = idx - 1;
    
    /* Move elements of arr[0..i-1], that are  greater than key, to one position ahead of their current position */
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    
    arr[j + 1] = key;
  }
}


//медианный фильтр размером 3x3
template<class T> void median3Templ(uint rows, uint cols, const QVector<T>& data, QVector<T>* dst)
{
  T window[9] = {0};
  
  int lastRow = (rows-1) * cols;
  for(uint col = 0; col < cols; ++col) {
    (*dst)[col] = data.at(col);
    (*dst)[lastRow + col] =  data.at(lastRow + col);
  }
  

  for (uint row = 1; row < rows - 1; ++row) {
    for (uint col = 0; col < cols; ++col) {
      if (col == 0 || col == cols - 1) {
        (*dst)[row * cols + col] = data.at(row * cols + col);
        continue;
      }
      
      for (int wr = 0; wr < 3; wr++) {
        int idx = (row + wr - 1)* cols + col;
        //пиксели попадающие в окно
        window[wr*3 + 0] = data.at(idx-1);
        window[wr*3 + 1] = data.at(idx);
        window[wr*3 + 2] = data.at(idx+1);
      }
      
      insertionSort(window, 9);
      (*dst)[row * cols + col] = window[4];
    }
  }
  
}


//медианный фильтр размером 5x5
template<class T> void median5Templ(uint rows, uint cols, const QVector<T>& data, QVector<T>* dst)
{
  const int kernel = 5;
  
  T window[kernel*kernel] = {0};

  int lastRow = (rows-2) * cols;
  for(uint col = 0; col < cols*2; ++col) {
    (*dst)[col] = data.at(col);
    (*dst)[lastRow + col] =  data.at(lastRow  + col);
  }

  
  for (uint row = 2; row < rows - 2; ++row) {
    for (uint col = 0; col < cols; ++col) {
      if (col <= 1 || col >= cols - 2) {
        (*dst)[row * cols + col] = data.at(row * cols + col);
        continue;
      }
      
      for (int wr = 0; wr < kernel; wr++) {
        int idx = (row + wr - 2)* cols + col;
        //пиксели попадающие в окно
        window[wr*3 + 0] = data.at(idx-2);
        window[wr*3 + 1] = data.at(idx-1);
        window[wr*3 + 2] = data.at(idx);
        window[wr*3 + 3] = data.at(idx+1);
        window[wr*3 + 4] = data.at(idx+2);
      }
      
      insertionSort(window, kernel*kernel);
      (*dst)[row * cols + col] = window[13];

    }
  }
  
}


template<class T> bool nagaoMacuyamaMask(const T window[], int size, int direct, float* mean, float* std)
{
  if (size < 25) {
    return false;
  }

  T subwindow[9];
  int num = 0;

  switch (direct) {
    case 0: //центр
      subwindow[0] = window[6];  subwindow[1] = window[7];  subwindow[2] = window[8];
      subwindow[3] = window[11]; subwindow[4] = window[12]; subwindow[5] = window[13];
      subwindow[6] = window[16]; subwindow[1] = window[17]; subwindow[8] = window[18];
      num = 9;
    break;
      
    case 1: //верх
      subwindow[0] = window[1];  subwindow[1] = window[2];  subwindow[2] = window[3];
      subwindow[3] = window[6];  subwindow[4] = window[7];  subwindow[5] = window[8];
      subwindow[6] = window[12];
      num = 7;
    break;
      
    case 2: //право
      subwindow[0] = window[8];  subwindow[1] = window[9];
      subwindow[2] = window[12]; subwindow[3] = window[13]; subwindow[4] = window[14];
      subwindow[5] = window[18]; subwindow[6] = window[19];
      num = 7;
    break;
      
    case 3: //низ
      subwindow[0] = window[12];
      subwindow[1] = window[16]; subwindow[2] = window[17]; subwindow[3] = window[18];
      subwindow[4] = window[21]; subwindow[5] = window[22]; subwindow[6] = window[23];
      num = 7;
    break;
      
    case 4: //лево
      subwindow[0] = window[5];  subwindow[1] = window[6];
      subwindow[2] = window[10]; subwindow[3] = window[11]; subwindow[4] = window[12];
      subwindow[5] = window[15]; subwindow[6] = window[16];
      num = 7;
    break;
      
    case 5: //лево-верх
      subwindow[0] = window[0];  subwindow[1] = window[1];
      subwindow[2] = window[5];  subwindow[3] = window[6]; subwindow[4] = window[7];
      subwindow[5] = window[11]; subwindow[6] = window[12];
      num = 7;
    break;
      
    case 6: //право-верх
      subwindow[0] = window[3];  subwindow[1] = window[4];
      subwindow[2] = window[7];  subwindow[3] = window[8]; subwindow[4] = window[9];
      subwindow[5] = window[12]; subwindow[6] = window[13];
      num = 7;
    break;
      
    case 7: //право-низ
      subwindow[0] = window[12]; subwindow[1] = window[13];
      subwindow[2] = window[17]; subwindow[3] = window[18]; subwindow[4] = window[19];
      subwindow[5] = window[23]; subwindow[6] = window[24];
      num = 7;
    break;
      
    case 8: //лево-низ
      subwindow[0] = window[11]; subwindow[1] = window[12];
      subwindow[2] = window[15]; subwindow[3] = window[16]; subwindow[4] = window[17];
      subwindow[5] = window[20]; subwindow[6] = window[21];
      num = 7;
    break;
    default: {
      return false;
    }
  }

  *std = MnMath::standartDeviation(subwindow, num, mean);
  float limit = 2*(*std);
  uint numTmp = 0;
  T tmp[num];

  for (int j = 0; j < num; j++) {
    if (subwindow[j] <= (*mean + limit) &&
        subwindow[j] >= (*mean - limit)) {
      tmp[numTmp++] = subwindow[j];
    }
  }

  insertionSort(tmp, numTmp);

  if (numTmp % 2 == 0) {
    *mean = (tmp[numTmp/2] + tmp[numTmp/2 + 1])/2;
  } else {
    *mean = tmp[numTmp/2];
  }
  
  return true;
}


//Фильтр Нагао-Мацуямы размером 5x5
template<class T> void nagaoMacuyamaTempl(float stdLim, uint rows, uint cols, const QVector<T>& data, QVector<T>* dst)
{
  const int kernel = 5;

  int lastRow = (rows-2) * cols;
  for (uint col = 0; col < cols*2; ++col) {
    (*dst)[col] = data.at(col);
    (*dst)[lastRow + col] =  data.at(lastRow  + col);
  }

  for (uint row = 2; row < rows - 2; ++row) {
    for (uint col = 0; col < cols; ++col) {
      if (col <= 1 || col >= cols - 2) {
        (*dst)[row * cols + col] = data.at(row * cols + col);
        continue;
      }
      
      float mean = 0;
      float std = 9999;
      T window[kernel*kernel];
      //пиксели попадающие в окно
      for (int wr = 0; wr < kernel; wr++) {
        int idx = (row + wr - 2)* cols + col - 2;
        for (int wc = 0; wc < 5; wc++) {
          window[wr*kernel + wc] = data.at(idx + wc);
        }
      }
      
      int stdCnt = 0;
      for (uint direct = 0; direct < 9; direct++) {
        float curMean, curStd;
        bool ok = nagaoMacuyamaMask<T>(window, kernel*kernel, direct, &curMean, &curStd);

        if ((direct == 4 || direct == 2) && std < stdLim) {
          stdCnt++;
        }
        if (ok && std > curStd) {
          std = curStd;
          mean = curMean;
        }
        //	debug_log << std << curStd << stdCnt;
      }

      if (stdCnt == 2) {
        (*dst)[row * cols + col] = data.at(row * cols + col);
      } else {
        (*dst)[row * cols + col] = round(mean);
      }

    }
  }
  
}


}


void meteo::median3(uint rows, uint cols, QVector<uchar>* data)
{
  QVector<uchar> dst(data->size());
  median3Templ<uchar>(rows, cols, *data, &dst);
  data->swap(dst);
}


void meteo::median5(uint rows, uint cols, QVector<uchar>* data)
{
  QVector<uchar> dst(data->size());
  median5Templ<uchar>(rows, cols, *data, &dst);
  data->swap(dst);
}

void meteo::nagaoMacuyamaModify(uint rows, uint cols, QVector<uchar>* data)
{
  int stdLim = 6;
  QVector<uchar> dst(data->size());
  nagaoMacuyamaTempl<uchar>(stdLim, rows, cols, *data, &dst);
  data->swap(dst);
}
