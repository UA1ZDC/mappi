#include "axispuanson.h"

#include <qpainter.h>
#include <qimage.h>
#include <qline.h>

#include <commons/mathtools/mnmath.h>
#include <commons/geobasis/generalproj.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/axis/layeraxis.h>

namespace meteo {
namespace map {

static int drawDebug = false;

const float kPuanScale = 0.75;
const float kTextScale = 0.85;
const int kPenWidth = 1;
const int kPuansonWidth = 96*kPuanScale;
const int kPuansonHeight = 96*kPuanScale;

AxisPuanson::AxisPuanson(LayerAxis* layer, const QSizeF& mapSize)
  : AxisSimple(mapSize)
{
  setLayer(layer);
  puanCache_.setMaxCost(1024*1024*2/4); // 2 Mb (делим на 4, т.к. считаем только количество пикселей)
  textCache_.setMaxCost(1024*1024*2/4); // 2 Mb (делим на 4, т.к. считаем только количество пикселей)
}

void AxisPuanson::setPuanson(const puanson::proto::Puanson& puanson)
{
  puanson_ = puanson;
  puanCache_.clear();
  textCache_.clear();
}

void AxisPuanson::updateData(const GeoPoint& coord, map::Document* doc)
{
  Q_UNUSED( coord );

  updateDocScale(doc->scale());

  data_.clear();

  for ( map::Layer* layer : doc->layers() ) {
    map::LayerAxis* l = maplayer_cast<map::LayerAxis*>(layer);
    if ( nullptr == l ) {
      continue;
    }

    if ( !l->visible() ) { continue; }

    for ( Puanson* o : l->objectsByType<Puanson*>() ) {
      PuanDataItem d;
      d.data = o->meteodata();
      d.pos = o->skelet().first();

      data_ << d;
    }
  }
}

void AxisPuanson::paint(QPainter* painter, const QPoint& beg, const QPoint& end, Document* doc) const
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

  // рисуем подпись оси
  if ( ktpBegin == axisLabelPosition() ) {
    QPoint lblPoint = beg;
    lblPoint.rx() += tickSize()*tickXDirection();
    lblPoint.ry() += tickSize()*tickYDirection();

    QFontMetrics fm = painter->fontMetrics();
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, label());
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor() + tickLabelXOffset();
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor() + tickLabelYOffset();
    QRect r(lblPoint, axisLabelRect.size());

    painter->drawText(r, tickLabelAlignFlags(), label());
  }
  else if ( ktpEnd == axisLabelPosition() ) {
    QPoint lblPoint = end;
    lblPoint.rx() += tickSize()*tickXDirection();
    lblPoint.ry() += tickSize()*tickYDirection();

    QFontMetrics fm = painter->fontMetrics();
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, label());
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor() + tickLabelXOffset();
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor() + tickLabelYOffset();
    QRect r(lblPoint, axisLabelRect.size());

    painter->drawText(r, tickLabelAlignFlags(), label());
  }

  // рисуем шкалу
  QList<QPoint> list;

  QPoint scenePos = doc->coord2screen(geoPos());

  if ( data_.size() == 0 ) {
    painter->restore();
    return;
  }

  for ( int i = 0, isz = data_.size(); i < isz; ++i )
  {
    const TMeteoData& md = data_.at(i).data;
    if ( md.count() == 0 ) {
      continue;
    }

    GeoPoint gp = data_.at(i).pos;

    if ( !isHorizontal() ) { gp.setLat(geoPos().lat()); }
    else                   { gp.setLon(geoPos().lon()); }

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

      PuanPixItem* puanPix = nullptr;
      PuanPixItem* textPix = nullptr;

      QString sKey = key(data_.at(i).pos, generalizationLevel(), gp.lat());
      if ( !puanCache_.contains(sKey) ) {
        TMeteoData md0;
        md0.set("dd", md.meteoParam("dd"));
        md0.set("ff", md.meteoParam("ff"));

        puanPix = renderTickImage(md0, QColor(Qt::transparent), kPuanScale);
        if ( nullptr == puanPix ) {
          continue;
        }

        puanCache_.insert(sKey,puanPix,puanPix->pixmap.width()*puanPix->pixmap.height());
      }
      else {
        puanPix = puanCache_.object(sKey);
      }

      if ( !textCache_.contains(sKey) ) {
        TMeteoData md1;
        md1.set("fx", md.meteoParam("fx"));

        textPix = renderTickImage(md1, QColor(Qt::white), kTextScale);
        if ( nullptr == textPix ) {
          continue;
        }
        textCache_.insert(sKey,textPix,textPix->pixmap.width()*textPix->pixmap.height());
      }
      else {
        textPix = textCache_.object(sKey);
      }

      QPoint puanPoint = tickBeg;
      QPoint textPoint = tickBeg;
      if ( kLeft == axisType() || kRight == axisType() ) {
        int dx = (tickSize() + kPuansonWidth/2.0)*tickXDirection();
        QTransform tr;
        tr.translate(puanPoint.x() + dx, puanPoint.y());
        puanPoint = tr.map(puanPix->rect.topLeft());
        textPoint = tr.map(textPix->rect.topLeft());
      }
      else if ( kCenter == axisType() ) {
        int dx = tickSize()*tickXDirection();
        QTransform tr;
        tr.translate(puanPoint.x() + dx, puanPoint.y());
        puanPoint = tr.map(puanPix->rect.topLeft());
        textPoint = tr.map(textPix->rect.topLeft());
      }

      painter->drawPixmap(puanPoint,puanPix->pixmap);
      painter->drawPixmap(textPoint,textPix->pixmap);

      if ( drawDebug ) {
        // прямоугольник пуансона
        QRect r0 = puanPix->rect;
        QRect r1 = textPix->rect;
        r0.moveTopLeft(puanPoint);
        r1.moveTopLeft(textPoint);
        painter->save();
        QPen pen;
        pen.setColor(Qt::red);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        r0.setWidth(r0.width() - 1);
        r0.setHeight(r0.height() - 1);
        r1.setWidth(r1.width() - 1);
        r1.setHeight(r1.height() - 1);
        painter->drawRect(r0);
        painter->drawRect(r1);
        painter->restore();
      }
    }
  }

  painter->restore();
}

QRect AxisPuanson::boundingRect(map::Document* doc) const
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

bool AxisPuanson::isEqual(const AxisSimple& a) const
{
  bool b = AxisSimple::isEqual(a);

  if ( a.type() != AxisPuanson::Type ) {
    return b;
  }

  const AxisPuanson& axis = static_cast<const AxisPuanson&>(a);

  std::string s0;
  std::string s1;
  puanson_.SerializeToString(&s0);
  axis.puanson_.SerializeToString(&s1);

  b = b && (s0 == s1)
      && (data_ == axis.data_);

  return b;
}

int32_t AxisPuanson::dataSize() const
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

int32_t AxisPuanson::serializeToArray(char* data) const
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

int32_t AxisPuanson::parseFromArray(const char* data)
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

QString AxisPuanson::key(const GeoPoint& pos, int generalizationLevel, int tickNumber) const
{
  return QString("%1_%2_%3").arg(generalizationLevel).arg(pos.toString()).arg(tickNumber);
}

PuanPixItem* AxisPuanson::renderTickImage(const TMeteoData& md, const QColor& bg, float scale) const
{
  Puanson puan;
  puan.setScreenPos(QPoint(0,0));
  puan.setMeteodata(md);
  auto pp = puanson_;
  pp.set_scale(scale);
  puan.setPunch(pp);

  QList<QRect> rr = puan.boundingRect(QTransform());
  if ( rr.isEmpty() ) {
    return nullptr;
  }

  QRect r  = rr.first();

  PuanPixItem* item = new PuanPixItem;
  item->rect = r;
  item->pixmap = QPixmap(r.size());
  item->pixmap.fill(bg);

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

QVector<GeoPoint> AxisPuanson::mkProfile(const GeoPoint& beg, const QVector<float>& ticks) const
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

LayerAxis* AxisPuanson::axisLayer() const
{
  if ( nullptr == layer() ) { return nullptr; }

  return maplayer_cast<LayerAxis*>(layer());
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
