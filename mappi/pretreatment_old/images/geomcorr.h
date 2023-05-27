#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_SATGEOM_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_SATGEOM_H

#include <qvector.h>
#include <qdatetime.h>
#include <sat-commons/satellite/satviewpoint.h>


namespace Coords {
  struct GeoCoord;
}

namespace mappi {
  namespace po {

    
    //! геометричечкая коррекция данных спутникового прибора
    class GeomCorrection {
    public:
      GeomCorrection(const ::SatViewPoint& sat);
      ~GeomCorrection();

      int rows() const { return _rows; }
      int cols() const { return _cols; }
      float step() const { return _gridStep; }
      const Coords::GeoCoord& leftUp() const { return _leftUp; }
      
      void createCoords(uint rows, float gridStep, float scanAngle, float fovStep);
      QVector<uchar> transformImage(const QVector<uchar>& imData) const;
      
    private:

      void sortPoints(const QVector<Coords::GeoCoord>& points, float step,
		      const Coords::GeoCoord& min, const Coords::GeoCoord& max);
            
    private:
      const ::SatViewPoint& _sat;

      float _fovStep = 0; //<! шаг сканирования по горизонтали, радиан
      float _gridStep = 0; //!< шаг сетки, радиан
      Coords::GeoCoord _leftUp; //!< левый верхний угол после преобразования, радиан
      
      QVector<QMap<float, QPair<float, int64_t>>> _sorted; //см. в коде
      int _cols = 0; //!< число колонок после преобразования
      int _rows = 0; //!< число строк после преобразования
      	
    };

  }
}

#endif
