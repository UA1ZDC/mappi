#include "posgrid.h"

#include <qmath.h>
#include <qtransform.h>
#include <qelapsedtimer.h>

#include "pos.h"

#include <meteo/commons/ui/map/loader.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

bool fakeLoader( Document* map ) { Q_UNUSED( map ); return true; }

static bool res = Loader::instance()->registerDataHandler("null.loader", &fakeLoader );

} // map
} // meteo


namespace meteo {

static const qint32 kGridNoData = std::numeric_limits<qint32>::max();
static const qint16 kNoDataX2F = std::numeric_limits<qint16>::max();

static const double kScale = 13.8;

PosGrid::PosGrid()
  : POSproj()
{
}

PosGrid::~PosGrid()
{
  delete doc_;
  doc_ = nullptr;
}

bool PosGrid::buildGridX2F(int samples, int lines, int step)
{
  if ( step < 1 ) { return false; }

  lines_2_ = lines*0.5;
  samples_2_ = samples*0.5;

  x2fInitialized_ = false;
  x2fGridStep_ = step;
  latVec_.clear();
  lonVec_.clear();

  nLat_ = lines/x2fGridStep_;
  nLon_ = samples/x2fGridStep_;

  latVec_.resize(nLat_*nLon_);
  latVec_.fill(kGridNoData);

  lonVec_.resize(nLat_*nLon_);
  lonVec_.fill(kGridNoData);

  pLat_ = latVec_.data();
  pLon_ = lonVec_.data();

  meteo::GeoPoint gp;
  for ( int j = 0; j < nLat_; ++j ) {
    for ( int i = 0; i < nLon_; ++i ) {
      if ( POSproj::X2F_one(QPointF(i*x2fGridStep_-samples_2_,j*x2fGridStep_-lines_2_), &gp) ) {
        pLat_[j*nLon_ + i] = gp.lat();
        pLon_[j*nLon_ + i] = gp.lon();
      }
    }
  }

  x2fInitialized_ = true;

  qint64 x2fmem = nLat_*nLon_*sizeof(double)*2;
  debug_log << "X2F mem:" << x2fmem << "bytes";

  return true;
}

bool PosGrid::buildGridF2X(int gridWidth, int gridHeight)
{
  QElapsedTimer t;
  t.start();

  f2xInitialized_ = false;

  if ( !buildGridX2F(gridWidth + 512, gridHeight + 512, 32) ) {
    return false;
  }
  info_log << "init X2F grid:" << t.restart() << "msec.";

  GeoPoint center;
  X2F_one(QPoint(0,0), &center);

  map::proto::Document docParam;
  docParam.set_projection(meteo::kStereo);
  docParam.set_geoloader("null.loader");
  docParam.set_scale(kScale);
  *docParam.mutable_doc_center() = meteo::geopoint2pbgeopoint( center );
  *docParam.mutable_map_center() = meteo::geopoint2pbgeopoint( center );

  delete doc_;
  doc_ = new map::Document;
  if ( !doc_->init(docParam) ) {
    return false;
  }

  samples_ = gridWidth;
  lines_  = gridHeight;
  f2xHalfHeight = qCeil(gridHeight*0.5);
  f2xHalfWidth  = qCeil(gridWidth*0.5);

  // min-max
  int minX = 99999;
  int maxX = -99999;
  int minY = 99999;
  int maxY = -99999;

  for ( auto j : {-f2xHalfHeight, f2xHalfHeight} ) {
    for ( int i = -f2xHalfWidth; i < f2xHalfWidth; ++i ) {
      GeoPoint gp;
      bool res = X2F_one(QPointF(i,j), &gp);
      if ( false == res ) {
        continue;
      }
      bool ok;
      QPoint mapPoint = doc_->coord2screen(gp,&ok);
      if ( !ok ) { continue; }

      minX = qMin(minX, mapPoint.x());
      maxX = qMax(maxX, mapPoint.x());
      minY = qMin(minY, mapPoint.y());
      maxY = qMax(maxY, mapPoint.y());
    }
  }

  for ( auto i : {-f2xHalfWidth, f2xHalfWidth} ) {
    for ( int j = -f2xHalfHeight; j < f2xHalfHeight; ++j ) {
      GeoPoint gp;
      bool res = X2F_one(QPointF(i,j), &gp);
      if ( false == res ) {
        continue;
      }
      bool ok;
      QPoint mapPoint = doc_->coord2screen(gp,&ok);
      if ( !ok ) { continue; }

      minX = qMin(minX, mapPoint.x());
      maxX = qMax(maxX, mapPoint.x());
      minY = qMin(minY, mapPoint.y());
      maxY = qMax(maxY, mapPoint.y());
    }
  }

  QPoint topLeft(minX, minY);
  QPoint bottomRight(maxX, maxY);

  boundRect_ = QRect(topLeft, bottomRight);
  debug_log << QObject::tr("%1x%2").arg(boundRect_.width()).arg(boundRect_.height()) << boundRect_.bottomLeft() << boundRect_.topRight();

  gridSize_ = 1000;

  info_log << "calc rect:" << t.restart() << "msec.";

  GeoPoint gp;
  if ( !X2F_one(QPointF(0,0),&gp) ) {
    return false;
  }
  bool ok;
  QPoint p1 = doc_->coord2screen(gp,&ok);
  if ( !ok ) { return false; }

  if ( !X2F_one(QPointF(0,1000),&gp) ) {
    return false;
  }
  QPoint p2 = doc_->coord2screen(gp,&ok);
  if ( !ok ) { return false; }

  QLineF midLine(QPointF(p1.x(),p1.y()+1000),p1);
  QLineF line(p1,p2);
  double angle = midLine.angleTo(line);

  xGridStep_ = boundRect_.width() / double(gridSize_);
  yGridStep_ = boundRect_.height() / double(gridSize_);

  xVec_.resize(gridSize_*gridSize_);
  xVec_.fill(kNoDataX2F);

  yVec_.resize(gridSize_*gridSize_);
  yVec_.fill(kNoDataX2F);

  pX_ = xVec_.data();
  pY_ = yVec_.data();

  gridRect_ = QRect(QPoint(0,0),QSize(gridSize_,gridSize_));

  transform_.reset();
  transform_.translate(-p1.x(),-p1.y());
  transform_ *= QTransform().rotate(angle);
  transform_ *= QTransform::fromTranslate(boundRect_.width()*0.5,boundRect_.height()*0.5);
  transform_ *= QTransform::fromScale(1./xGridStep_,1./yGridStep_);

  mask_.resize(gridSize_*gridSize_);
  mask_.fill(kEmptyNode);
  char* pMask = mask_.data();

  for ( int y = -f2xHalfHeight; y < f2xHalfHeight; y += 3 ) {
    for ( int x = -f2xHalfWidth; x < f2xHalfWidth; x += 2 ) {
      if ( !X2F_one(QPointF(x,y), &gp) ) {
        continue;
      }

      bool ok;
      QPoint pos = doc_->coord2screen(gp,&ok);
      if ( !ok ) { continue; }

      int jj = (pos.y());
      int ii = (pos.x());
      transform_.map(ii,jj,&ii,&jj);

      if ( ii < 0 || jj < 0 || ii >= gridSize_ || jj >= gridSize_ ) {
        continue;
      }

      int idx = jj*gridSize_ + ii;

      pX_[idx] = x;
      pY_[idx] = y;
      pMask[idx] = kDataNode;
    }
  }

  info_log << "X2F:" << t.restart() << "msec.";

  edgeVec.clear();
  edgeVec.reserve((f2xHalfHeight*f2xHalfWidth)*4);

  QVector<QPoint> points = { QPoint(-f2xHalfWidth, f2xHalfHeight),
                             QPoint( f2xHalfWidth, f2xHalfHeight),
                             QPoint( f2xHalfWidth,-f2xHalfHeight),
                             QPoint(-f2xHalfWidth,-f2xHalfHeight),
                             QPoint(-f2xHalfWidth, f2xHalfHeight)
                           };
  for ( int i = 1, isz = points.size(); i < isz; ++i ) {
    const QPoint& p0 = points.at(i - 1);
    const QPoint& p1 = points.at(i);

    int x1 = p0.x();
    int x2 = p1.x();
    int y1 = p0.y();
    int y2 = p1.y();

    const int deltaX = qAbs(x2 - x1);
    const int deltaY = qAbs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    int error = deltaX - deltaY;

    while( x1 != x2 || y1 != y2 )
    {
      if ( X2F_one(QPointF(x1,y1), &gp) ) {
        bool ok;
        QPoint pos = doc_->coord2screen(gp,&ok);
        if ( !ok ) { break; }

        pos = transform_.map(pos);
        if ( gridRect_.contains(pos) ) {
          edgeVec.append(pos);
        }
      }

      const int error2 = error * 2;

      if ( error2 > -deltaY ) {
        error -= deltaY;
        x1 += signX;
      }
      if ( error2 < deltaX ) {
        error += deltaX;
        y1 += signY;
      }
    }
  }

  info_log << "calc edges:" << t.restart() << "msec.";

  // маркируем граничные узлы
  for ( int i = 1, isz = edgeVec.size(); i < isz; ++i ) {
    const QPoint& p0 = edgeVec.at(i - 1);
    const QPoint& p1 = edgeVec.at(i);
    int x1 = p0.x();
    int x2 = p1.x();
    int y1 = p0.y();
    int y2 = p1.y();

    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;

    int error = deltaX - deltaY;

    pMask[y2*gridSize_ + x2] = kEdgeNode;
    while ( x1 != x2 || y1 != y2 )
    {
      pMask[y1*gridSize_ + x1] = kEdgeNode;
      const int error2 = error * 2;

      if ( error2 > -deltaY ) {
        error -= deltaY;
        x1 += signX;
      }
      if ( error2 < deltaX ) {
        error += deltaX;
        y1 += signY;
      }
    }
  }

  info_log << "mark edge nodes:" << t.restart() << "msec.";

  // заполняем пропуски
  int j = 0;
  int l = 0;
  int r = 0;
  int i = -1;
  while ( j < gridSize_ )
  {
    if ( i >= gridSize_ ) {
      ++j;
      l = 0;
      r = 0;
      i = -1;
    }

    // поиск начала левой границы
    for ( i += 1; i < gridSize_; ++i ) {
      if ( kEdgeNode == pMask[j*gridSize_ + i] ) {
        l = i;
        break;
      }
    }
    // поиск конца левой границы
    for ( i += 1; i < gridSize_; ++i ) {
      if ( kEdgeNode != pMask[j*gridSize_ + i] ) {
        break;
      }
    }
    // поиск начала правой границы
    for ( i += 1; i < gridSize_; ++i ) {
      if ( kEdgeNode == pMask[j*gridSize_ + i] ) {
        r = i;
        break;
      }
    }
    // поиск конца правой границы
    for ( i += 1; i < gridSize_; ++i ) {
      if ( kEdgeNode != pMask[j*gridSize_ + i] ) {
        break;
      }
      r = i;
    }

    // билинейная интерполяция
    for ( int ii = l; ii <= r; ++ii ) {
      if ( kEmptyNode != pMask[j*gridSize_ + ii] ) {
        continue;
      }

      int idx00 = kNoDataX2F;
      int idx01 = kNoDataX2F;
      int idx11 = kNoDataX2F;
      int idx10 = kNoDataX2F;

      int i00 = kNoDataX2F;
      int i01 = kNoDataX2F;
      int j00 = kNoDataX2F;
      int j10 = kNoDataX2F;

      bool found = false;
      for ( int incU = 1; incU < 5 && !found; ++incU ) {
        for ( int incD = 1; incD < 5 && !found; ++incD ) {
          for ( int incL = 1; incL < 5 && !found; ++incL ) {
            for ( int incR = 1; incR < 5 && !found; ++incR ) {

              int jU = j-incU;
              int jD = j+incD;
              int iL = ii-incL;
              int iR = ii+incR;

              if ( jU < 0 || jD >= gridSize_ || iL < 0 || iR >= gridSize_ ) {
                continue;
              }

              if (    kDataNode != pMask[(jU)*gridSize_ + (iL)]
                      || kDataNode != pMask[(jU)*gridSize_ + (iR)]
                      || kDataNode != pMask[(jD)*gridSize_ + (iR)]
                      || kDataNode != pMask[(jD)*gridSize_ + (iL)]
                      )
              {
                continue;
              }

              found = true;
              idx00 = (jU)*gridSize_ + (iL);
              idx01 = (jU)*gridSize_ + (iR);
              idx11 = (jD)*gridSize_ + (iR);
              idx10 = (jD)*gridSize_ + (iL);

              i00 = iL;
              i01 = iR;

              j00 = jU;
              j10 = jD;
              break;
            }
          }
        }

        if ( false == found ) { continue; }

        double gridStepX = qAbs(i00 - i01);
        double gridStepY = qAbs(j00 - j10);

        double dx = (j - j00) / gridStepY;
        double dy = (ii - i00) / gridStepX;

        qint16 d00 = pX_[idx00];
        qint16 d01 = pX_[idx01];
        qint16 d10 = pX_[idx10];
        qint16 d11 = pX_[idx11];

        double vs_1=(1.-dy)*d00 + dy*d01;
        double vs_2=(1.-dy)*d10 + dy*d11;
        double x = (1.-dx)*vs_1+dx*vs_2;

        d00 = pY_[idx00];
        d01 = pY_[idx01];
        d10 = pY_[idx10];
        d11 = pY_[idx11];

        vs_1=(1.-dy)*d00 + dy*d01;
        vs_2=(1.-dy)*d10 + dy*d11;
        double y = (1.-dx)*vs_1+dx*vs_2;

        if ( kNoDataX2F == pX_[j*gridSize_ + ii] ) {
          pX_[j*gridSize_ + ii] = x;
          pY_[j*gridSize_ + ii] = y;
          pMask[j*gridSize_ + ii] = kBilInterpolDataNode;
        }
        else {
          trc << "BAD!";
        }
      }
    }

    // линейная интерполяция
    for ( int ii = l; ii <= r; ++ii ) {
      if ( kEmptyNode != pMask[j*gridSize_ + ii] ) {
        continue;
      }

      int idx00 = kNoDataX2F;
      int idx01 = kNoDataX2F;

      int i00 = kNoDataX2F;
      int i01 = kNoDataX2F;

      bool found = false;
      for ( int incL = 1; incL < 10 && !found; ++incL ) {
        for ( int incR = 1; incR < 10 && !found; ++incR ) {

          int iL = ii-incL;
          int iR = ii+incR;

          if ( iL < 0 || iR >= gridSize_ ) {
            continue;
          }

          if ( kEmptyNode == pMask[j*gridSize_ + (ii-incL)] || kEmptyNode == pMask[j*gridSize_ + (ii+incR)] ) {
            continue;
          }

          found = true;
          idx00 = j*gridSize_ + (ii-incL);
          idx01 = j*gridSize_ + (ii+incR);

          i00 = ii - incL;
          i01 = ii + incR;
          break;
        }
      }

      if ( false == found ) { continue; }

      double gridStepX = qAbs(i00 - i01);

      double dy = (ii - i00) / gridStepX;

      qint16 d00 = pX_[idx00];
      qint16 d01 = pX_[idx01];

      double x =(1.-dy)*d00 + dy*d01;

      d00 = pY_[idx00];
      d01 = pY_[idx01];
      double y =(1.-dy)*d00 + dy*d01;

      if ( kNoDataX2F == pX_[j*gridSize_ + ii] ) {
        pX_[j*gridSize_ + ii] = x;
        pY_[j*gridSize_ + ii] = y;
        pMask[j*gridSize_ + ii] = kLinInterpolDataNode;
      }
      else {
        trc << "BAD!";
      }
    }
  }

  info_log << "interpol:" << t.restart() << "msec.";


  f2xInitialized_ = true;

  qint64 f2xmem = gridSize_*gridSize_*sizeof(qint16)*2;
  debug_log << "F2X mem:" << f2xmem << "bytes";

  f2xmem = gridSize_*gridSize_*sizeof(qint16)*2 + edgeVec.size()*sizeof(QPoint) + mask_.size();
  debug_log << "F2X mem:" << f2xmem << "bytes";

  return true;
}

bool PosGrid::F2X(const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed) const
{
  return POSproj::F2X(geoPoints, scrPoints, isClosed);
}

bool PosGrid::F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord) const
{
  QPointF pnt(*meterCoord);
  if ( false == F2X_one( geoCoord, &pnt ) ) {
    return false;
  }
  *meterCoord = pnt.toPoint();
  return true;
}

bool PosGrid::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord) const
{
  return X2F_one( QPointF(meterCoord), geoCoord );
}

bool PosGrid::F2X_one(const GeoPoint& geoCoord, QPointF* screenCoord) const
{
  if ( !f2xInitialized_ ) {
    return POSproj::F2X_one(geoCoord,screenCoord);
  }

  bool ok;
  QPointF pos = doc_->coord2screenf(geoCoord,&ok);
  if ( !ok ) { return false; };

  double x = pos.x();
  double y = pos.y();

  transform_.map(x, y, &x, &y);

  int j = int(y);
  int i = int(x);

  if ( j < 0 || i < 0 || j >= gridSize_-1 || i >= gridSize_-1 ) { return false; }

  double dx = (y - j);
  double dy = (x - i);

  int idx00 = j*gridSize_+i;
  int idx01 = j*gridSize_+i+1;
  int idx10 = (j+1)*gridSize_+i;
  int idx11 = (j+1)*gridSize_+i+1;

  qint16 d00 = pX_[idx00];
  qint16 d01 = pX_[idx01];
  qint16 d10 = pX_[idx10];
  qint16 d11 = pX_[idx11];

  if ( kNoDataX2F == d00 || kNoDataX2F == d01 || kNoDataX2F == d10 || kNoDataX2F == d11 ) {
    return false;
  }

  double vs_1=(1.-dy)*d00 + dy*d01;
  double vs_2=(1.-dy)*d10 + dy*d11;
  x = (1.-dx)*vs_1+dx*vs_2;

  d00 = pY_[idx00];
  d01 = pY_[idx01];
  d10 = pY_[idx10];
  d11 = pY_[idx11];

  // нет надобности в проверке, т.к. если есть X то есть и Y
  //  if ( kNoDataX2F == d00 || kNoDataX2F == d01 || kNoDataX2F == d10 || kNoDataX2F == d11 ) {
  //    return false;
  //  }

  vs_1=(1.-dy)*d00 + dy*d01;
  vs_2=(1.-dy)*d10 + dy*d11;
  y = (1.-dx)*vs_1+dx*vs_2;

  screenCoord->setX(x);
  screenCoord->setY(y);

  return true;
}

bool PosGrid::X2F_one(const QPointF& screenCoord, GeoPoint* geoCoord) const
{
  if ( !x2fInitialized_ ) {
    return POSproj::X2F_one(screenCoord, geoCoord);
  }

  double x = screenCoord.x();
  double y = screenCoord.y();
  x += samples_2_;
  y += lines_2_;

  int j = int(y/x2fGridStep_);
  int i = int(x/x2fGridStep_);

  if ( j < 0 || i < 0 || j >= nLat_-1 || i >= nLon_-1 ) {
    return false;
  }

  double dx = (y - j*x2fGridStep_)/x2fGridStep_;
  double dy = (x - i*x2fGridStep_)/x2fGridStep_;

  double d00 = pLon_[j*nLon_+i];
  double d01 = pLon_[j*nLon_+i+1];
  double d10 = pLon_[(j+1)*nLon_+i];
  double d11 = pLon_[(j+1)*nLon_+i+1];

  // Для корректной интерполяции
  bool correction = false;
  if ( d00*RAD2DEG > 170 || d00*RAD2DEG < -170 ) {
    correction = true;
    d00 = MnMath::PiToPi(d00 + 180*DEG2RAD);
    d01 = MnMath::PiToPi(d01 + 180*DEG2RAD);
    d10 = MnMath::PiToPi(d10 + 180*DEG2RAD);
    d11 = MnMath::PiToPi(d11 + 180*DEG2RAD);
  }

  double vs_1=(1.-dy)*d00 + dy*d01;
  double vs_2=(1.-dy)*d10 + dy*d11;
  double lon = (1.-dx)*vs_1+dx*vs_2;

  if ( correction ) {
    lon = MnMath::PiToPi(lon - 180*DEG2RAD);
  }

  d00 = pLat_[j*nLon_+i];
  d01 = pLat_[j*nLon_+i+1];
  d10 = pLat_[(j+1)*nLon_+i];
  d11 = pLat_[(j+1)*nLon_+i+1];

  correction = false;
  if ( d00*RAD2DEG > 75 || d00*RAD2DEG < -75 ) {
    correction = true;
    d00 = MnMath::PiToPi(d00 + 90*DEG2RAD);
    d01 = MnMath::PiToPi(d01 + 90*DEG2RAD);
    d10 = MnMath::PiToPi(d10 + 90*DEG2RAD);
    d11 = MnMath::PiToPi(d11 + 90*DEG2RAD);
  }

  vs_1 = (1.-dy)*d00 + dy*d01;
  vs_2 = (1.-dy)*d10 + dy*d11;
  double lat = (1.-dx)*vs_1+dx*vs_2;

  if ( correction ) {
    lat = MnMath::PiToPi(lat - 90*DEG2RAD);
  }

  *geoCoord = GeoPoint(lat,lon);

  return true;
}

bool PosGrid::cartesian2X(const QVector<QPoint>& coords, const GeoPoint& mapCenter, int* ax, int* ay) const
{
  if ( !f2xInitialized_ ) {
    return false;
  }

  double angel = doc_->projection()->getMapCenter().lon() - mapCenter.lon();

  QTransform trans;
  trans.rotateRadians(angel);

  QVector<QPoint> vec = trans.map(coords);
  vec = doc_->cartesianToScreen(vec);

  QPoint* pVec = vec.data();
  for ( int i = 0, isz = coords.size(); i < isz; ++i ) {
    double xx = pVec[i].x();
    double yy = pVec[i].y();

    transform_.map(xx, yy, &xx, &yy);

    int jj = int(yy);
    int ii = int(xx);

    if ( jj < 0 || ii < 0 || jj >= gridSize_-1 || ii >= gridSize_-1 ) {
      continue;
    }

    double dx = (yy - jj);
    double dy = (xx - ii);

    int idx00 = jj*gridSize_+ii;
    int idx01 = jj*gridSize_+ii+1;
    int idx10 = (jj+1)*gridSize_+ii;
    int idx11 = (jj+1)*gridSize_+ii+1;

    qint16 d00 = pX_[idx00];
    qint16 d01 = pX_[idx01];
    qint16 d10 = pX_[idx10];
    qint16 d11 = pX_[idx11];

    if ( kNoDataX2F == d00 || kNoDataX2F == d01 || kNoDataX2F == d10 || kNoDataX2F == d11 ) {
      continue;
    }

    double vs_1=(1.-dy)*d00 + dy*d01;
    double vs_2=(1.-dy)*d10 + dy*d11;
    xx = (1.-dx)*vs_1+dx*vs_2;

    d00 = pY_[idx00];
    d01 = pY_[idx01];
    d10 = pY_[idx10];
    d11 = pY_[idx11];

    vs_1=(1.-dy)*d00 + dy*d01;
    vs_2=(1.-dy)*d10 + dy*d11;
    yy = (1.-dx)*vs_1+dx*vs_2;

    ax[i] = xx;
    ay[i] = yy;
  }

  return true;
}

bool PosGrid::calcMinMax(int width, int height, GeoPoint* minPoint, GeoPoint* maxPoint) const
{
  int halfWidth  = qRound(width*0.5);
  int halfHeight = qRound(height*0.5);

  float minLat = 9999;
  float maxLat = -9999;
  float minLon = 9999;
  float maxLon = -9999;

  for ( auto j : { -halfHeight, halfHeight } ) {
    GeoPoint gp;
    for ( int i = -halfWidth; i < halfWidth; ++i ) {
      if ( POSproj::X2F_one(QPointF(i,j), &gp) ) {
        minLat = qMin(minLat, gp.latDeg());
        maxLat = qMax(maxLat, gp.latDeg());
        minLon = qMin(minLon, gp.lonDeg());
        maxLon = qMax(maxLon, gp.lonDeg());
      }
    }
  }
  for ( auto i : { -halfWidth, halfWidth } ) {
    for ( int j = -halfHeight; j < halfHeight; ++j ) {
      GeoPoint gp;
      if ( POSproj::X2F_one(QPointF(i,j), &gp) ) {
        minLat = qMin(minLat, gp.latDeg());
        maxLat = qMax(maxLat, gp.latDeg());
        minLon = qMin(minLon, gp.lonDeg());
        maxLon = qMax(maxLon, gp.lonDeg());
      }
    }
  }

  if ( std::isnan(minLat) || std::isnan(minLon) || std::isnan(maxLat) || std::isnan(maxLon) ) {
    return false;
  }

  *minPoint = GeoPoint(minLat*DEG2RAD,minLon*DEG2RAD);
  *maxPoint = GeoPoint(maxLat*DEG2RAD,maxLon*DEG2RAD);

  return true;
}

} // meteo
