#include "layerspectr.h"
#include "axisspectr.h"

#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/puanson.h>

#include <commons/geom/geom.h>


namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerSpectr(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerSpectr::Type, createLayer );
}

LayerSpectr::LayerSpectr(Document* d, const QString& n )
  : Layer(d)
{
  setName(n);
  line_ = new meteo::map::GeoPolygon(this);
  setBase(true);
  if ( 0 != document() ) {
    Projection* proj = document_->projection();
    const GeoVector& rmk = proj->ramka();
    QPolygonF poly;
    for ( int i=0,isz=rmk.size(); i<isz; ++i ) {
      poly << QPointF(rmk[i].lat(),rmk[i].lon());
    }
    mapRect_ = poly.boundingRect().toRect();
    x_.setMapSize(mapRect_.size());
    y_.setMapSize(mapRect_.size());
    y_.setPosition(meteo::AxisSpectr::kLeft);
    QString name= "A,дБ";
    y_.setLabel(name.toUtf8());
    x_.setPosition(meteo::AxisSpectr::kBottom);
    name = "f,МГц";
    x_.setLabel(name.toUtf8());
  }
}



void LayerSpectr::setGraphProperty(const meteo::Property& prop)
{
  prop_ = prop;

  Object* obj = objectByUuid(graphUuid_);
  if ( 0 != obj ) {
    obj->setProperty(prop);
  }

  meteo::Property xProp = x_.property();
  xProp.mutable_pen()->set_color(prop_.pen().color());
  x_.setProperty(xProp);

  meteo::Property yProp = y_.property();
  yProp.mutable_pen()->set_color(prop_.pen().color());
  y_.setProperty(yProp);
}


QList<GeoPoint> LayerSpectr::crossByX(double x) const
{
  if ( 0 ==line_ ) { return QList<GeoPoint>(); }

  QTransform tr;
  tr.scale(100000,100000);

  QLineF line(QPointF(x,mapRect_.top()),QPointF(x,mapRect_.bottom()));
  line = tr.map(line);

  QList<QPoint> crosses;

  QPointF p1;
  QPointF p2;
  GeoVector skelet = line_->skelet();
  for ( int i=1,isz=skelet.size(); i<isz; ++i ) {
    p1.rx() = skelet.at(i).lat();
    p1.ry() = skelet.at(i).lon();
    p2.rx() = skelet.at(i-1).lat();
    p2.ry() = skelet.at(i-1).lon();

    QLine l = tr.map(QLineF(p1,p2)).toLine();

    QPoint p;
    if ( meteo::geom::crossPoint(line.toLine(), l, &p) ) {
      // проверяем на повтор, чтобы исключить дубликаты когда точка пересечения располагается на границе двух секторов
      if ( crosses.isEmpty() || crosses.last() != p ) {
        crosses << p;
      }
    }
  }

  QList<GeoPoint> list;
  foreach ( const QPoint& p, crosses ) {
    p1.rx() = p.x();
    p1.ry() = p.y();

    p1 = tr.inverted().map(p1);
    list << GeoPoint(p1.x(), p1.y(), 0, LA_GENERAL);
  }
  return list;
}






QVector<double> LayerSpectr::getValuesX(QMap<double, double> coord)
{
  QVector<double> data(coord.size());
  QMap<double,double>::iterator itMap;
  itMap=coord.begin();
  for (int i=0; i<coord.size(); i++) {
    data[i]=itMap.key();
    itMap++;
  }
  return data;
}

QVector<double> LayerSpectr::getValuesY(QMap<double, double> coord)
{
  QVector<double> data(coord.size());
  QMap<double,double>::iterator itMap;
  itMap=coord.begin();
  for (int i=0; i<coord.size(); i++) {
    data[i]=itMap.value();
    itMap++;
  }
  return data;
}


void LayerSpectr::calcXAxisRange(double lower, double upper)
{
  x_.setRange(lower, upper);
}

void LayerSpectr::calcYAxisRange()
{
  if ( 0 != yVals_.size()) {
    double minY = minElement(yVals_)-10;
    double maxY = maxElement(yVals_)+10;
    if ( !std::isnan(minY) && !std::isnan(maxY) ) {
      y_.setRange(minY, maxY);
    }
  }
  else {
    y_.setRange(-40,20);
  }
}

void LayerSpectr::printLayerInfo() const
{
  debug_log << "name:" << name();
  debug_log << "graph UUID:" << graphUuid_;
  debug_log << "prop:" << prop_.Utf8DebugString();
  debug_log << "axis X:\n"
            << var(x_.lower()) << var(x_.upper()) << "\n";
  debug_log << "axis Y:\n"
            << var(y_.lower()) << var(y_.upper()) << "\n";

}

double LayerSpectr::minElement(const QVector<double>& v) const
{
  double min=v[0];
  for (int i=0; i<v.size();i++) {
    if(v[i]<min) {
      min=v[i];
    }
  }
  return min;
}

double LayerSpectr::maxElement(const QVector<double>& v) const
{
  double max=v[0];
  for (int i=0; i<v.size();i++) {
    if(v[i]>max) {
      max=v[i];
    }
  }
  return max;
}

double LayerSpectr::minAmpl()
{
  if(0 != yVals_.size()) {
    return minElement(yVals_)-5;
  }
  return -40.00;
}

double LayerSpectr::maxAmpl()
{
  if(0 != yVals_.size()) {
    return maxElement(yVals_)+5;
  }
  return 20.00;
}

void LayerSpectr::setAmplitude (QMap<double,double>& coord )
{
  meteo::GeoVector skelet;
  QMap<double,double>::iterator itMap;
  for (itMap=coord.begin(); itMap!=coord.end(); itMap++) {
    meteo::GeoPoint gp( (itMap.key())*1e-6, itMap.value(), 0, meteo::LA_GENERAL);
    skelet << gp;
  }
  xVals_ = getValuesX(coord);
  yVals_ = getValuesY(coord);
  updateSkelet(skelet);
}

void LayerSpectr::updateSkelet(GeoVector &skelet)
{
  line_->setSkelet(skelet);
  line_->setVisible(true);
  graphUuid_ = line_->uuid();
}

const meteo::Property &LayerSpectr::grProperty() const
{
  return prop_;
}

} // map
} // meteo
