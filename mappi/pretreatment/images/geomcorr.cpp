#include "geomcorr.h"

#include <commons/mathtools/mnmath.h>

#include <cross-commons/debug/tlog.h>

#include <QImage>
#include <QDebug>

using namespace MnMath;
using namespace mappi;

#define KM_IN_RAD  (rad2deg(1.0)*111.111)  //!< количество км в радиане


namespace mappi {
namespace po {
//TODO это, наверное, надо в математику
float bilinInterp(int x1, int x2, int y1, int y2,
									float c11, float c21, float c12, float c22,
									int x, int y);
float linInterp(int x1, int x2, float c1, float c2, int x);

float bilinInterpF(float x1, float x2, float y1, float y2,
									 float c11, float c21, float c12, float c22,
									 float x, float y);
float linInterpF(float x1, float x2, float c1, float c2, float x);
}
}

/*! 
 * \brief Билинейная интерполяция
 * \param x1 Положение на оси абсцисс двух точек слева от искомой
 * \param x2 Положение на оси абсцисс двух точек справа от искомой
 * \param y1 Положение на оси ординат двух точек снизу от искомой
 * \param y2 Положение на оси ординат двух точек сверху от искомой
 * \param c11 Значение нижней левой
 * \param c21 Значение нижней правой
 * \param c12 Значение верхней левой
 * \param c22 Значение верхней правой
 * \param x   Положение на оси абсцисс искомой точки
 * \param y   Положение на оси ординат искомой точки
 * \return Значение искомой точки
 */
float mappi::po::bilinInterp(int x1, int x2, int y1, int y2,
														 float c11, float c21, float c12, float c22,
														 int x, int y)
{
  return c11*(x2-x)*(y2-y)/(x2-x1)/(y2-y1) +
      c21*(x-x1)*(y2-y)/(x2-x1)/(y2-y1) +
      c12*(x2-x)*(y-y1)/(x2-x1)/(y2-y1) +
      c22*(x-x1)*(y-y1)/(x2-x1)/(y2-y1);
}

float mappi::po::bilinInterpF(float x1, float x2, float y1, float y2,
															float c11, float c21, float c12, float c22,
															float x, float y)
{
  return c11*(x2-x)*(y2-y)/(x2-x1)/(y2-y1) +
      c21*(x-x1)*(y2-y)/(x2-x1)/(y2-y1) +
      c12*(x2-x)*(y-y1)/(x2-x1)/(y2-y1) +
      c22*(x-x1)*(y-y1)/(x2-x1)/(y2-y1);
}

/*! 
 * \brief Линейная интерполяция
 * \param x1 Положение на оси точки слева от искомой
 * \param x2 Положение на оси точки справа от искомой
 * \param c1 Цвет левой точки
 * \param c2 Цвет правой точки
 * \param x Положение искомой точки
 * \return  Цвет искомой точки
 */
float mappi::po::linInterp(int x1, int x2, float c1, float c2, int x)
{
  return c1 + (c2-c1)*(x-x1)/(x2-x1);
}

float mappi::po::linInterpF(float x1, float x2, float c1, float c2, float x)
{
  return c1 + (c2-c1)*(x-x1)/(x2-x1);
}



po::GeomCorrection::GeomCorrection(const ::SatViewPoint& sat) :
  _sat(sat)
{
}

po::GeomCorrection::~GeomCorrection()
{
}

//в min, max долгота +/-Pi
//расчет исходной сетки координат, формирование вспомогательного массива координат для перехода к регулярной сетке
void po::GeomCorrection::createCoords(uint rows, float gridStep, float scanAngle, float fovStep)
{
  //debug_log << "create coord" << rows << gridStep << scanAngle << fovStep;
  
  if (_sorted.size() != 0 && _fovStep == fovStep) {
    //подходящая сетка уже создана
    return;
  }
  
  _fovStep = fovStep;
  _gridStep = gridStep;
  float vstep = _sat.dtStart().msecsTo(_sat.dtEnd()) / float(rows);
  Coords::GeoCoord min, max;
  _sat.countMinMaxBorders(fovStep, vstep, scanAngle, &min, &max);
  _leftUp.lon = min.lon;
  _leftUp.lat = max.lat;
    
  QVector<Coords::GeoCoord> points;
  
  _sat.countGrid(fovStep, vstep, scanAngle, points);

  sortPoints(points, gridStep, min, max);
  
  //var(ok);
}



//сортировка долгот и широт для последующей трансформации изображения
void po::GeomCorrection::sortPoints(const QVector<Coords::GeoCoord>& points, float step,
                                    const Coords::GeoCoord& min, const Coords::GeoCoord& max)
{
  //определяется номер строки и столбца пикселя для имеющихся координат при их наложении на сетку с заданным шагом.
  //значения будут с плавающей запятой.
  //_sorted - заготовка для будущей сетки. Вектор, где индекс idx будет указывать, что внутри только широты, попавшие в строку idx (с номерами [idx, idx+1))
  //внутри QMap с ключом по долготам, имеющимся для этой строки.
  //Значением QMap - пара с значением строки и индексом координаты в массива points
  // При таком способе пересчета координат на новой сетке будут узлы, куда попало несколько точек со старыми координатами
  // и пустые узлы. Поэтому требуется интерполяция.

  float latmin = min.lat, latmax = max.lat;
  float lonmin = min.lon, lonmax = max.lon;
  //debug_log << latmin*180/3.14 << latmax*180/3.14 << lonmin*180/3.14 << lonmax*180/3.14;

  // latmin = 20; //42.6169;
  // latmax = 90;

  // lonmin = -30;
  // lonmax = 90;
  
  //float step = 0.045; //0.01; //0.005 : 21590 x 14001, тяжело открывается; 0.045 - норм
  _rows = (latmax - latmin) / step + 1;
  _cols = (lonmax - lonmin) / step + 1;
  _sorted.resize(_rows);

  for (int64_t idx = 0; idx < points.size(); idx++) {
    
    float lat =  points.at(idx).lat;
    float lon = points.at(idx).lon;

    float col = (lon - lonmin) / step;
    float row = _rows - (lat - latmin) / step; //TODO восходящ/нисходящ, вроде работает
    if (row < 0) {
      row = 0;
    }
    int rowRound = floor(row);
    if (rowRound >= _rows) {
      //TODO
      rowRound = _rows - 1;
    }
    //debug_log << row << rowRound;

    _sorted[rowRound][col] = QPair<float, int64_t>(row, idx);
  }

}

//преобразование изображения в соответствии с новой сеткой координат
QVector<uchar> po::GeomCorrection::transformImage(const QVector<uchar>& imData) const
{
  QVector<uchar> resImg;

  if (_sorted.size() == 0 || _cols <= 0 || _rows <= 0) {
    error_log << QObject::tr("Преобразование изображения невозможно: объект не инициализирован");
    //debug_log << _sorted.size() << _cols << _rows;
    return resImg;
  }
  
  //для интерполяции берутся все значения попадающие +/-1 номер пикселя по строки и по столбцу, сохраняется медиана
  
  resImg.resize(_cols*_rows/**2*/);
  
  int64_t idx = 0;
  for (int row = 0; row < _rows; row++) {
    bool start = false;
    
    for (int col = 0; col < _cols; col++) {
      int64_t val = -9999;
      uchar colorByte = 0;
      //      uchar maskByte = 0;
      
      if (!_sorted.at(row).isEmpty()) {
        auto more = _sorted.at(row).lowerBound(col);
        auto less = _sorted.at(row).lowerBound(col);
        if (less == _sorted.at(row).begin()) {
          less = _sorted.at(row).end();
        } else {
          less -= 1;
        }

        QList<uchar> vals;
        for (int r = qMax(row - 1, 0); r <= qMin(row + 1, _rows-1); r++) {
          auto it = _sorted.at(r).lowerBound(col - 1);
          //	    debug_log << row << r << col << it.key();
          while (it != _sorted.at(r).end() && col + 1 >= it.key() &&
                 it.value().second >=0 && it.value().second < imData.size()
                 ) {
            //debug_log << row << r << col << it.key() << it.value();
            vals.append(imData[it.value().second]);
            it++;
          }
        }
        if (vals.size() != 0) {
          colorByte = vals.at(vals.size() / 2);
          //	  maskByte = 255;
          start = true;
        }
        else {
          // if (less != _sorted.at(row).end() && start/*col - 5 <= less.key()*/) { //правые полосы
          //   val = less.value().second;
          // }
          if (more != _sorted.at(row).end() && start /*col + 5 >= more.key()*/) {
            val = more.value().second;
          }
          if (val >= 0 && val < imData.size()) {
            colorByte = imData[val];
            //	    maskByte = 255;
          }
        }
        //debug_log << row << col << less.key() << more.key() << int64_t(val);
      }

      // if (col == 2352 && row == 13) {
      // 	debug_log << "err" << row << col << val << imData.size() << _sorted.at(row);
      // }

      resImg[idx++] = colorByte;
      // (*resImg)[idx++] = maskByte;

    }
  }

  return resImg;

}

//в min, max долгота +/-Pi
//расчет только исходной сетки (координаты пикселей изображения)
// void Instrument::createCoords(uint rows, SatViewPoint* sat, float fovStep, QVector<Coords::GeoCoord>* points) const
// {
//   if (nullptr == sat || nullptr == points) {
//     return;
//   }

//   float vstep = sat->dtStart().msecsTo(sat->dtEnd()) / float(rows);


//   bool ok = sat->countGrid(MnMath::deg2rad(fovStep), vstep, MnMath::deg2rad(_conf.scan_angle()), *points); //TODO
//   var(ok);

//   var(points->size());

// }
