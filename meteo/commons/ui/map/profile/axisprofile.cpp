#include "axisprofile.h"

#include <qpainter.h>
#include <qimage.h>
#include <qline.h>

#include <commons/mathtools/mnmath.h>
#include <commons/geobasis/generalproj.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/puanson.h>

#include "layerprofile.h"

namespace meteo {
namespace map {

static int drawDebug = false;

const int kPenWidth = 1;
const int kPuansonWidth = 100;
const int kPuansonHeight = 100;

AxisProfile::AxisProfile(LayerProfile* layer, const QSizeF& mapSize)
  : AxisSimple(mapSize)
{
  setLayer(layer);
  pixCache_.setMaxCost(1024*1024*2/4); // 2 Mb (делим на 4, т.к. считаем только количество пикселей)
}

void AxisProfile::setPuanson(const puanson::proto::Puanson& puanson)
{
  puanson_ = puanson;
  pixCache_.clear();
}

void AxisProfile::updateData(const GeoPoint& coord, map::Document* doc)
{
  LayerProfile* l = profileLayer();
  if ( nullptr == l ) {
    error_log << QObject::tr("Необходимо установить слой LayerProfile.");
    return;
  }

  updateDocScale(doc->scale());

  dataPos_ = coord;
  data_ = l->getDataByCoords(mkProfile(coord, dataByLevel()[generalizationLevel()].ticks_));
}

void AxisProfile::paint(QPainter* painter, const QPoint& beg, const QPoint& end, Document* doc) const
{
  if ( nullptr == doc ) { return; }

  updateDocScale(doc->scale());

  int w = qAbs(beg.x() - end.x());
  int h = qAbs(beg.y() - end.y());

  QRect geom;
  switch ( axisType() ) {
    case kLeft:
      geom = QRect(end - QPoint(tickSize(),0), QSize(tickSize() + kPenWidth,h));
      break;
    case kRight:
      geom = QRect(end, QSize(tickSize() + kPenWidth,h));
      break;
    case kCenter:
      geom = QRect(end - QPoint(kPuansonWidth/2. + tickSize(),0), QSize(tickSize()*2 + kPenWidth,h));
      break;
    case kTop:
      geom = QRect(beg - QPoint(0,tickSize()), QSize(w,tickSize() + kPenWidth));
      break;
    case kBottom:
      geom = QRect(beg, QSize(w,tickSize() + kPenWidth));
      break;
    case kMiddle:
      not_impl;
      break;
  }

  painter->save();
  painter->setFont(labelFont());

  QPen pen = pen2qpen(property().pen());

  // DEBUG
  if ( drawDebug ) {
    switch ( axisType() ) {
      case kLeft:   pen.setColor(Qt::darkGreen);  break;
      case kRight:  pen.setColor(Qt::darkRed);    break;
      case kCenter: pen.setColor(Qt::darkCyan);   break;
      case kTop:    pen.setColor(Qt::darkBlue);   break;
      case kBottom: pen.setColor(Qt::darkYellow); break;
      case kMiddle: pen.setColor(Qt::darkMagenta);break;
    }
  }
  // END DEBUG

  pen.setWidth(kPenWidth);
  painter->setPen(pen);

  // рисуем ось
  // note: будет работать некорректно при толщине оси больше 1
  QLine axisLine(beg,end);
  if ( axisType() == kCenter ) {
    QPoint p1(beg.x() - kPuansonWidth/2., beg.y());
    QPoint p2(end.x() - kPuansonWidth/2., end.y());
    axisLine.setPoints(p1,p2);
  }
  painter->drawLine(axisLine);

  // рисуем крайние насечки
  // note: будет работать некорректно при толщине оси больше 1
  int dx = tickSize()*tickXDirection();
  int dy = tickSize()*tickYDirection();
  QLine begTick(axisLine.p1(), axisLine.p1() + QPoint(dx,dy));
  QLine endTick(axisLine.p2(), axisLine.p2() + QPoint(dx,dy));
  if ( axisType() == kCenter ) {
    begTick = QLine(axisLine.p1() - QPoint(dx,dy), axisLine.p1() + QPoint(dx,dy));
    endTick = QLine(axisLine.p2() - QPoint(dx,dy), axisLine.p2() + QPoint(dx,dy));
  }
  painter->drawLine(begTick);
  painter->drawLine(endTick);

  // рисуем шкалу
  QList<QPoint> list;

  QPoint scenePos = doc->coord2screen(geoPos());

  if ( data_.size() != dataByLevel()[generalizationLevel()].ticks_.size() ) {
    painter->restore();
    return;
  }

  for ( int i = 0, isz = dataByLevel()[generalizationLevel()].ticks_.size(); i < isz; ++i )
  {
    const float pLevel = dataByLevel()[generalizationLevel()].ticks_.at(i);

    GeoPoint gp = geoPos();
    if ( isHorizontal() ) { gp.setLat(scale2coord(pLevel)); }
    else                  { gp.setLon(scale2coord(pLevel)); }

    QPoint tickBeg = doc->coord2screen(gp) - scenePos + beg;
    if ( drawOnRamka() ) {
      tickBeg.ry() += QPoint(scenePos - beg).y();
    }

    if ( axisType() == kCenter ) {
      tickBeg.setX(tickBeg.x() - kPuansonWidth/2.0 + tickSize());
    }

    QPoint tickEnd = tickBeg;
    tickEnd.rx() += tickSize()*tickXDirection();
    tickEnd.ry() += tickSize()*tickYDirection();

    if ( axisType() == kCenter ) {
      tickEnd.rx() += tickSize()*tickXDirection();
    }

    if ( geom.contains(tickBeg) ) {
      pen.setColor(Qt::black);
      painter->setPen(pen);
      painter->drawLine(tickBeg, tickEnd);

      PixItem* pix = nullptr;

      QString sKey = key(dataPos_, generalizationLevel(), pLevel);
      if ( !pixCache_.contains(sKey) ) {        
        const TMeteoData& md = data_.at(i);
        if ( md.count() == 0 ) {
          continue;
        }

        pix = renderTickImage(md);
        if ( nullptr == pix ) {
          continue;
        }

        pixCache_.insert(sKey,pix,pix->pixmap.width()*pix->pixmap.height());
      }
      else {
        pix = pixCache_.object(sKey);
      }

      QPoint p = tickBeg;
      if ( kLeft == axisType() || kRight == axisType() ) {
        int dx = (tickSize() + kPuansonWidth/2.0)*tickXDirection();
        QTransform tr;
        tr.translate(p.x() + dx, p.y());
        p = tr.map(pix->rect.topLeft());
      }
      else if ( kCenter == axisType() ) {
        int dx = tickSize()*tickXDirection();
        QTransform tr;
        tr.translate(p.x() + dx, p.y());
        p = tr.map(pix->rect.topLeft());
      }

      painter->drawPixmap(p,pix->pixmap);

      if ( drawDebug ) {
        // прямоугольник пуансона
        QRect r = pix->rect;
        r.moveTopLeft(p);
        painter->save();
        QPen pen;
        pen.setColor(Qt::red);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        r.setWidth(r.width() - 1);
        r.setHeight(r.height() - 1);
        painter->drawRect(r);
        painter->restore();
      }
    }
  }

  if ( 0 != subTickCount() ) {
    for ( int i = 1, isz = list.size(); i < isz; ++i ) {
      const QPoint& first  = list.at(i - 1);
      const QPoint& second = list.at(i);

      QPointF b = first;
      QPointF e = first;
      e.rx() += tickSize()/2. * tickXDirection();
      e.ry() += tickSize()/2. * tickYDirection();

      float step;
      if ( isHorizontal() ) {
        step = (second.x() - first.x()) / float(subTickCount());
      }
      else {
        step = (second.y() - first.y()) / float(subTickCount());
      }

      for ( int n = 1; n < subTickCount(); ++n ) {
        if ( isHorizontal() ) {
          b.rx() += step;
          e.rx() += step;
        }
        else {
          b.ry() += step;
          e.ry() += step;
        }

        if ( geom.contains(b.toPoint()) ) {
          painter->drawLine(b, e);
        }
      }
    }
  }

  painter->restore();
}

QRect AxisProfile::boundingRect(map::Document* doc) const
{
  if ( nullptr == doc ) { return QRect(); }

  const GeoPoint gpBeg = begGeoPos();
  const GeoPoint gpEnd = endGeoPos();

  const QPoint beg = doc->coord2screen(gpBeg);
  const QPoint end = doc->coord2screen(gpEnd);

  const int w = qAbs(beg.x() - end.x());
  const int h = qAbs(beg.y() - end.y());

  QSize size;
  QPoint pos;
  switch ( axisType() ) {
    case kLeft:
      pos  = QPoint(end - QPoint(tickSize() + kPuansonWidth,0));
      size = QSize(tickSize() + kPenWidth + kPuansonWidth,h);
      break;
    case kRight:
      pos  = end;
      size = QSize(tickSize() + kPenWidth + kPuansonWidth,h);
      break;
    case kCenter:
      pos  = QPoint(end - QPoint(kPuansonWidth/2,0));
      size = QSize(kPuansonWidth,h);
      break;
    case kTop:
      pos  = QPoint(beg - QPoint(0,tickSize() + kPuansonHeight));
      size = QSize(w,tickSize() + kPenWidth + kPuansonHeight);
      break;
    case kBottom:
      pos  = beg;
      size = QSize(w,tickSize() + kPenWidth + kPuansonHeight);
      break;
    case kMiddle:
      pos  = QPoint(beg - QPoint(0,kPuansonHeight/2));
      size = QSize(w,kPuansonHeight);
      break;
  }

  const QRect rect(pos - beg,size);
  return rect;
}

int32_t AxisProfile::dataSize() const
{
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(QDataStream::Qt_4_0);

  std::string str;
  puanson_.SerializePartialToString(&str);
  ds << QByteArray::fromRawData(str.data(), str.size());

  int32_t size = 0;
  size += sizeof(size);
  size += ba.size();

  return AxisSimple::dataSize() + size;
}

int32_t AxisProfile::serializeToArray(char* data) const
{
  int32_t pos = AxisSimple::serializeToArray(data);

  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(QDataStream::Qt_4_0);

  std::string str;
  puanson_.SerializePartialToString(&str);
  ds << QByteArray::fromRawData(str.data(), str.size());

  int32_t sz = ba.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  ::memcpy( data + pos, ba.data(), ba.size() );
  pos += ba.size();

  return pos;
}

int32_t AxisProfile::parseFromArray(const char* data)
{
  int32_t pos = AxisSimple::parseFromArray(data);

  int32_t sz = 0;
  global::fromByteArray(data + pos, &sz);
  pos += sizeof(sz);

  QByteArray ba = QByteArray::fromRawData(data + pos, sz);

  QDataStream ds(&ba, QIODevice::ReadOnly);
  ds.setVersion(QDataStream::Qt_4_0);

  QByteArray puan;
  ds >> puan;
  puanson_.ParsePartialFromArray(puan.data(), puan.size());

  pos += sz;

  return pos;
}

QString AxisProfile::key(const GeoPoint& pos, int generalizationLevel, int tickNumber) const
{
  return QString("%1_%2_%3").arg(generalizationLevel).arg(pos.toString()).arg(tickNumber);
}

PixItem* AxisProfile::renderTickImage(const TMeteoData& md) const
{
  Puanson puan;
  puan.setScreenPos(QPoint(0,0));
  puan.setPunch(puanson_);
  puan.setMeteodata(md);
  puan.windCorrection(angleWindCorrection_);

  QList<QRect> rr = puan.boundingRect(QTransform());
  if ( rr.isEmpty() ) {
    return nullptr;
  }

  QRect r  = rr.first();

  PixItem* item = new PixItem;
  item->rect = r;
  item->pixmap = QPixmap(r.size());
  item->pixmap.fill(Qt::transparent);

  QTransform tr;
  tr.translate(r.left(),r.top());
  tr = tr.inverted();

  QPainter p(&item->pixmap);
  p.setTransform(tr);
  if ( !puan.render(&p, r, QTransform()) ) {
    return nullptr;
  }

  return item;
}

QVector<GeoPoint> AxisProfile::mkProfile(const GeoPoint& beg, const QVector<float>& ticks) const
{
  QVector<GeoPoint> vec;
  vec.fill(beg,ticks.size());

  bool horiz = isHorizontal();
  for ( int i = 0, isz= ticks.size(); i < isz; ++i ) {
    if ( horiz ) {
      vec[i].setLat(ticks.at(i));
    }
    else {
      vec[i].setLon(ticks.at(i));
    }
  }

  return vec;
}

LayerProfile* AxisProfile::profileLayer() const
{
  if ( nullptr == layer() ) { return nullptr; }

  return maplayer_cast<LayerProfile*>(layer());
}

} // map
} // meteo

//QDataStream& operator>>(QDataStream& in, meteo::AxisData& data)
//{
//  in >> data.level;
//  in >> data.format_;
//  in >> data.ticks_;
//  in >> data.lastDrawed_;
//  in >> data.cacheTickLabelSize_;
//  in >> data.cacheTickLabels_;

//  return in;
//}

//QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data)
//{
//  out << data.level;
//  out << data.format_;
//  out << data.ticks_;
//  out << data.lastDrawed_;
//  out << data.cacheTickLabelSize_;
//  out << data.cacheTickLabels_;

//  return out;
//}
