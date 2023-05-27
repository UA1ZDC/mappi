#ifndef MAPPI_UI_POS_POSGRID_H
#define MAPPI_UI_POS_POSGRID_H

#include <qtransform.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/document.h>
#include <mappi/projection/pos.h>

namespace meteo {

class PosGrid : public POSproj
{
private:
  enum NodeMaskType {
    kEmptyNode,
    kEdgeNode,
    kDataNode,
    kBilInterpolDataNode,
    kLinInterpolDataNode,
  };

public:
  PosGrid();
  virtual ~PosGrid();

  enum {
    Type = POLARORBSAT + 1024
  };
  int type() const { return Type; }

  map::Document* document() const { return doc_; }

  //! Описывающий прямоугольник области, для которой рассчитана сетка (в экранных координатах).
  const QRect& boundingRect() const { return boundRect_; }

  bool buildGridX2F(int samples, int lines, int step = 16);
  bool buildGridF2X(int gridWidth, int gridHeight);

  virtual bool F2X(const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed) const;

  virtual bool F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord) const;
  virtual bool X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord) const;

  virtual bool F2X_one(const GeoPoint& geoCoord, QPointF* screenCoord) const;
  virtual bool X2F_one(const QPointF& screenCoord, GeoPoint* geoCoord) const;

  bool cartesian2X(const QVector<QPoint>& coords, const GeoPoint& mapCenter, int* ax, int* ay) const;


private:
  bool calcMinMax(int width, int height, GeoPoint* minPoint, GeoPoint* maxPoint) const;
  bool nearestF2X(const QPoint& screenCoord, QPointF* pos) const;

private:
  QRect gridRect_;
  QRect boundRect_;

  map::Document* doc_ = nullptr;
  QTransform transform_;

  QVector<qint16> xVec_;
  QVector<qint16> yVec_;
  qint16* pX_ = nullptr;
  qint16* pY_ = nullptr;

  QVector<QPoint> edgeVec;
  QByteArray mask_;

  int gridSize_ = 1000;
  double xGridStep_ = 1;
  double yGridStep_ = 1;

  double deltaAngle_ = 0;

  const int kMultX2F = 10000;

  int f2xHalfHeight = 0;
  int f2xHalfWidth  = 0;

  int lines_2_ = -1;
  int samples_2_ = -1;

  double x2fGridStep_ = -1;

  int nLat_ = 0;
  int nLon_ = 0;

  QVector<double> latVec_;
  QVector<double> lonVec_;
  double* pLat_ = 0;
  double* pLon_ = 0;

  bool f2xInitialized_ = false;
  bool x2fInitialized_ = false;
};

} // meteo

#endif // MAPPI_UI_POS_POSGRID_H
