#include "axisspectr.h"

#include <qpainter.h>

#include <commons/mathtools/mnmath.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/document.h>

namespace meteo {

AxisSpectr::AxisSpectr(const QSizeF& mapSize)
{
  curLevel_ = 20;
  dataByLevel_[20] = AxisData(20);

  mapSize_ = mapSize;

  autoTicks_ = true;
  position_ = kLeft;

  rangeLower_ = -10;
  rangeUpper_ = 10;
  factor_ = 1;
  tickSize_ = 6;
  nSteps_ = 100;
  subTickCount_ = 5;

  scale_ = 0.0;
  inverted_ = false;
  invertedVal_ = mapSize_.height();

  dataByLevel_[curLevel_].format_ = "f,2,";
  labelType_ = kltNumber;

  prop_.mutable_pen()->set_width(1);
  prop_.mutable_pen()->set_color(qRgba(0,0,0,255));
}

void AxisSpectr::setMapSize(const QSizeF& size)
{
  mapSize_ = size;
  updateFactor();
}

void AxisSpectr::setRange(double lower, double upper)
{
  rangeLower_ = lower;
  rangeUpper_ = upper;
  updateFactor();
  if ( autoTicks_ ) { calcTicks(); }
}

void AxisSpectr::setPosition(AxisSpectr::Position position)
{
  position_ = position;

  switch ( position_ ) {
    case kLeft:
      tickXDirection_ = -1;
      tickYDirection_ = 0;
      tickLabelAlignFlags_ = Qt::AlignVCenter|Qt::AlignRight;
      tickLabelXOffsetFactor_ = -1;
      tickLabelYOffsetFactor_ = -0.5;
      tickLabelXOffset_ = -3;
      tickLabelYOffset_ = 0;
      margins_ = QMargins(-5,0,0,0);
      break;
    case kRight:
      tickXDirection_ = 1;
      tickYDirection_ = 0;
      tickLabelAlignFlags_ = Qt::AlignVCenter|Qt::AlignLeft;
      tickLabelXOffsetFactor_ = 0;
      tickLabelYOffsetFactor_ = -0.5;
      tickLabelXOffset_ = 3;
      tickLabelYOffset_ = 0;
      margins_ = QMargins(0,0,5,0);
      break;
    case kTop:
      tickXDirection_ = 0;
      tickYDirection_ = -1;
      tickLabelAlignFlags_ = Qt::AlignBottom|Qt::AlignHCenter;
      tickLabelXOffsetFactor_ = -0.5;
      tickLabelYOffsetFactor_ = -1;
      tickLabelXOffset_ = 0;
      tickLabelYOffset_ = 0;
      margins_ = QMargins(-7,0,7,0);
      break;
    case kBottom:
      tickXDirection_ = 0;
      tickYDirection_ = 1;
      tickLabelAlignFlags_ = Qt::AlignTop|Qt::AlignHCenter;
      tickLabelXOffsetFactor_ = -0.5;
      tickLabelYOffsetFactor_ = 0;
      tickLabelXOffset_ = 0;
      tickLabelYOffset_ = 0;
      margins_ = QMargins(-7,0,7,0);
      break;
  }

  invertedVal_ = isHorizontal() ? mapSize_.width() : mapSize_.height();
}

QString AxisSpectr::tickLabel(double scaleValue, const QString& def) const
{
  if ( std::isnan(scaleValue) ) {
    return def;
  }

  if ( kltDateTime == labelType_ ) {
    return QDateTime::fromTime_t(static_cast<uint>(scaleValue)).toString(dataByLevel_[curLevel_].format_);
  }
  else {
    char c = 'f';
    QString t = dataByLevel_[curLevel_].format_.section(",", 0, 0);
    if ( t.size() > 0 ) {
      c = dataByLevel_[curLevel_].format_.at(0).toLatin1();
    }
    int prec = dataByLevel_[curLevel_].format_.section(",", 1, 1).toInt();
    QString unit = dataByLevel_[curLevel_].format_.section(",", 2, 2);
    return QString::number(scaleValue, c, prec) + unit;
  }
}

void AxisSpectr::setTickLabelFormat(const QString& format, int level)
{
  if ( !dataByLevel_.contains(level) ) {
    dataByLevel_[level] = AxisData(level);
  }
  dataByLevel_[level].format_ = format;
  dataByLevel_[level].cacheTickLabels_.clear();
  dataByLevel_[level].cacheTickLabelSize_ = QSize(0,0);
}

QVector<double> AxisSpectr::ticks(int l) const
{
  QList<int> levels = dataByLevel_.keys();

  if ( levels.isEmpty() ) {
    return QVector<double>();
  }

  if ( levels.size() == 1 && l < levels.first()  ) {
    return dataByLevel_[levels.first()].ticks_;
  }

  if ( levels.contains(l) ) {
    return dataByLevel_[l].ticks_;
  }

  for ( int i=levels.size()-1; i>0; --i ) {
    int lvl = levels[i-1];

    if ( l < lvl ) { continue; }

    return dataByLevel_[lvl].ticks_;
  }

  return dataByLevel_[levels.first()].ticks_;
}

void AxisSpectr::setTickVector(const QVector<double>& ticks, int level)
{
  if ( !dataByLevel_.contains(level) ) {
    dataByLevel_[level] = AxisData(level);
  }
  dataByLevel_[level].ticks_ = ticks;
  dataByLevel_[level].lastDrawed_ = QBitArray(ticks.size(), false);
  dataByLevel_[level].cacheTickLabels_.clear();
  dataByLevel_[level].cacheTickLabelSize_ = QSize(0,0);
  autoTicks_ = false;
//    clearCache();
}


void AxisSpectr::setAutoTicks(bool b)
{
  autoTicks_ = b;

  if ( autoTicks_ ) {
    calcTicks();
  }
  else {
    dataByLevel_[curLevel_].ticks_.clear();
    clearCache();
  }
}

void AxisSpectr::paint(QPainter* painter, map::Document* doc, const QPoint& beg, const QPoint& end) const
{
  updateDocScale(doc->scale());

  updateTicksCache();

  painter->save();
  painter->setFont(labelFont_);

  QPen pen = meteo::pen2qpen(prop_.pen());
  painter->setPen(pen);

  QPoint tl = beg;
  QPoint br = end;
  tl.rx() += tickSize_ * tickXDirection_;
  tl.ry() += tickSize_ * tickYDirection_;

  QRect scaleRect;
  scaleRect.setTopLeft(tl);
  scaleRect.setBottomRight(br);

  // рисуем ось
  painter->drawLine(beg, end);
  painter->drawLine(scaleRect.topLeft(), beg);
  painter->drawLine(scaleRect.bottomLeft(), end);

  // рисуем шкалу
  GeoPoint gpBeg = doc->screen2coord(beg);

  QList<QRect> drawed;

  // рисуем подпись оси
  if ( ktpBegin == axisLabelPosition_ ) {
    QPoint lblPoint = beg;
    lblPoint.rx() += tickSize_ * tickXDirection_;
    lblPoint.ry() += tickSize_ * tickYDirection_;

    QFontMetrics fm = painter->fontMetrics();
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, axisLabel_);
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;
    QRect r(lblPoint, axisLabelRect.size());

    drawed << r;

    painter->drawText(r, tickLabelAlignFlags_, axisLabel_);
  }
  else if ( ktpEnd == axisLabelPosition_ ) {
    QPoint lblPoint = end;
    lblPoint.rx() += tickSize_ * tickXDirection_;
    lblPoint.ry() += tickSize_ * tickYDirection_;

    QFontMetrics fm = painter->fontMetrics();
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, axisLabel_);
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;
    QRect r(lblPoint, axisLabelRect.size());

    drawed << r;

    painter->drawText(r, tickLabelAlignFlags_, axisLabel_);
  }

  QList<QPoint> list;

  dataByLevel_[curLevel_].lastDrawed_ = QBitArray(dataByLevel_[curLevel_].ticks_.size(), false);
  for ( int i=0,isz=dataByLevel_[curLevel_].ticks_.size(); i<isz; ++i ) {
    double val = dataByLevel_[curLevel_].ticks_.at(i);

    GeoPoint gp = gpBeg;
    if ( isHorizontal() ) { gp.setLat(val); }
    else                  { gp.setLon(val); }
    QPoint tickBeg = doc->coord2screen(gp);
    QPoint tickEnd = tickBeg;
    tickEnd.rx() += tickSize_ * tickXDirection_;
    tickEnd.ry() += tickSize_ * tickYDirection_;

    // подписи для шкалы
    QString lbl = dataByLevel_[curLevel_].cacheTickLabels_.at(i);

    QPoint lblPoint = tickEnd;
    lblPoint.rx() += dataByLevel_[curLevel_].cacheTickLabelSize_.width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    lblPoint.ry() += dataByLevel_[curLevel_].cacheTickLabelSize_.height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;

    QRect lblRect(lblPoint, dataByLevel_[curLevel_].cacheTickLabelSize_);

    bool draw = true;
    foreach ( const QRect& rect, drawed ) {
      if ( rect.intersects(lblRect) ) {
        draw = false;
        break;
      }
    }

    if ( draw ) {
      drawed << lblRect;
      if ( scaleRect.contains(tickBeg) ) {
        painter->drawText(lblRect, tickLabelAlignFlags_, lbl);
        dataByLevel_[curLevel_].lastDrawed_[i] = true;
      }
    }

    if ( scaleRect.contains(tickBeg) ) {
      painter->drawLine(tickBeg, tickEnd);
    }
    list << tickBeg;
  }

  if ( 0 != subTickCount_ ) {
    for ( int i=1,isz=list.size(); i<isz; ++i ) {
      QPoint first  = list.at(i - 1);
      QPoint second = list.at(i);

      QPointF b = first;
      QPointF e = first;
      e.rx() += tickSize_/2. * tickXDirection_;
      e.ry() += tickSize_/2. * tickYDirection_;

      double step;
      if ( isHorizontal() ) {
        step = qAbs(first.x() - second.x()) / double(subTickCount_);
      }
      else {
        step = qAbs(first.y() - second.y()) / double(subTickCount_);
      }

      for ( int n=1; n<subTickCount_; ++n ) {
        if ( isHorizontal() ) {
          b.rx() += step;
          e.rx() += step;
        }
        else {
          b.ry() -= step;
          e.ry() -= step;
        }

        if ( scaleRect.contains(b.toPoint()) ) {
          painter->drawLine(b, e);
        }
      }
    }
  }

  painter->restore();
}

QRect AxisSpectr::geometry(map::Document* doc, const QPoint& beg, const QPoint& end) const
{
  updateDocScale(doc->scale());

  updateTicksCache();

  GeoPoint gpBeg = doc->screen2coord(beg);
  GeoPoint gpEnd = doc->screen2coord(end);

  double minTick = ( isHorizontal() ? gpBeg.lat() : gpBeg.lon() );
  double maxTick = ( isHorizontal() ? gpEnd.lat() : gpEnd.lon() );

  if ( minTick > maxTick ) {
    qSwap(minTick, maxTick);
  }

  QPoint tl = beg;
  QPoint br = end;
  tl.rx() += tickSize_ * tickXDirection_;
  tl.ry() += tickSize_ * tickYDirection_;

  QRect scaleRect;
  scaleRect.setTopLeft(tl);
  scaleRect.setBottomRight(br);

  QList<QRect> drawed;
  for ( int i=0,isz=dataByLevel_[curLevel_].ticks_.size(); i<isz; ++i ) {
    double val = dataByLevel_[curLevel_].ticks_.at(i);

    GeoPoint gp = gpBeg;
    if ( isHorizontal() ) { gp.setLat(val); }
    else                  { gp.setLon(val); }


    QPoint tickBeg = doc->coord2screen(gp);
    QPoint tickEnd = tickBeg;
    tickEnd.rx() += tickSize_ * tickXDirection_;
    tickEnd.ry() += tickSize_ * tickYDirection_;

    // подписи для шкалы
    tickEnd.rx() += dataByLevel_[curLevel_].cacheTickLabelSize_.width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    tickEnd.ry() += dataByLevel_[curLevel_].cacheTickLabelSize_.height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;

    QRect lblRect(tickEnd, dataByLevel_[curLevel_].cacheTickLabelSize_);

    bool draw = true;
    foreach ( const QRect& rect, drawed ) {
      if ( rect.intersects(lblRect) ) {
        draw = false;
        break;
      }
    }

    if ( draw ) {
      if ( (val > minTick || qAbs(val - minTick) <= 0.01) && (val < maxTick || qAbs(val - maxTick) <= 0.01) ) {
        drawed << lblRect;
      }
    }
  }

  if ( drawed.size() > 0 ) { scaleRect = scaleRect.united(drawed.first()); }
  if ( drawed.size() > 1 ) { scaleRect = scaleRect.united(drawed.last());  }

  if ( ktpBegin == axisLabelPosition_ ) {
    QPoint lblPoint = beg;
    lblPoint.rx() += tickSize_ * tickXDirection_;
    lblPoint.ry() += tickSize_ * tickYDirection_;

    QFontMetrics fm(labelFont_);
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, axisLabel_);
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;
    QRect r(lblPoint, axisLabelRect.size());

    scaleRect = scaleRect.united(r);
  }
  else if ( ktpEnd == axisLabelPosition_ ) {
    QPoint lblPoint = end;
    lblPoint.rx() += tickSize_ * tickXDirection_;
    lblPoint.ry() += tickSize_ * tickYDirection_;

    QFontMetrics fm(labelFont_);
    QRect axisLabelRect = fm.boundingRect(0,0,0,0, Qt::TextDontClip, axisLabel_);
    lblPoint.rx() += axisLabelRect.size().width() * tickLabelXOffsetFactor_ + tickLabelXOffset_;
    lblPoint.ry() += axisLabelRect.size().height()* tickLabelYOffsetFactor_ + tickLabelYOffset_;
    QRect r(lblPoint, axisLabelRect.size());

    scaleRect = scaleRect.united(r);
  }

  QRect r = scaleRect;
  r.setLeft(r.left() + margins_.left());
  r.setRight(r.right() + margins_.right());
  r.setTop(r.top() + margins_.top());
  r.setBottom(r.bottom() + margins_.bottom());

  return r;
}

void AxisSpectr::setScale(double scale) const
{
  scale_ = scale;
}

void AxisSpectr::updateDocScale(double scale) const
{
  int s = qRound(scale);
//  var(s);

  QList<int> levels = dataByLevel_.keys();

  // FIXME:
  if ( levels.isEmpty() ) {
    curLevel_ = 20;
    return;
  }

  if ( levels.size() == 1 && s < levels.first()  ) {
    curLevel_ = levels.first();
    return;
  }

  if ( levels.contains(s) ) {
    curLevel_ = s;
    return;
  }

  for ( int i=levels.size()-1; i>0; --i ) {
    int l = levels[i-1];

    if ( s < l ) { continue; }

    curLevel_ = levels[i];
    return;
  }

  curLevel_ = levels.first();
}

QList<meteo::Generalization> AxisSpectr::dataLevels() const
{
  QList<meteo::Generalization> list;

  QList<int> levels = dataByLevel_.keys();

  if ( !levels.contains(0) ) {
    levels.prepend(0);
  }

  // FIXME
  if ( levels.size() < 2 ) {
    meteo::Generalization g;
    g.setLimits(0,20);
    list << g;
    return list;
  }

  for ( int i=1,isz=levels.size(); i<isz; ++i ) {
    meteo::Generalization g;
    g.setLimits(levels[i-1],levels[i]);
    list << g;
  }
  return list;
}

int32_t AxisSpectr::dataSize() const
{
  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(QDataStream::Qt_4_0);
  ds << autoTicks_;
  ds << rangeLower_;
  ds << rangeUpper_;
  ds << factor_;

  ds << (int32_t)position_;
  ds << (int32_t)tickSize_;
  ds << (int32_t)nSteps_;
  ds << (int32_t)subTickCount_;
  ds << labelFont_;
  ds << (int32_t)labelType_;
  ds << axisLabel_;
  ds << (int32_t)axisLabelPosition_;

  std::string str;
  prop_.SerializePartialToString(&str);
  ds << QByteArray::fromRawData(str.data(), str.size());

  ds << mapSize_;

  ds << inverted_;
  ds << (int32_t)invertedVal_;

  ds << (int32_t)tickXDirection_;
  ds << (int32_t)tickYDirection_;
  ds << (int32_t)tickLabelAlignFlags_;
  ds << (int32_t)tickLabelXOffset_;
  ds << (int32_t)tickLabelYOffset_;
  ds << tickLabelXOffsetFactor_;
  ds << tickLabelYOffsetFactor_;

  QRect rect;
  rect.setLeft(margins_.left());
  rect.setTop(margins_.top());
  rect.setRight(margins_.right());
  rect.setBottom(margins_.bottom());
  ds << rect;

  ds << (int32_t)curLevel_;
  ds << dataByLevel_;

  int32_t size = 0;
  size += sizeof(int32_t);
  size += ba.size();

  return size;
}

int32_t AxisSpectr::serializeToArray(char* data) const
{
  int32_t pos = 0;

  QByteArray ba;
  QDataStream ds(&ba, QIODevice::WriteOnly);
  ds.setVersion(QDataStream::Qt_4_0);
  ds << autoTicks_;
  ds << rangeLower_;
  ds << rangeUpper_;
  ds << factor_;

  ds << (int32_t)position_;
  ds << (int32_t)tickSize_;
  ds << (int32_t)nSteps_;
  ds << (int32_t)subTickCount_;
  ds << labelFont_;
  ds << (int32_t)labelType_;
  ds << axisLabel_;
  ds << (int32_t)axisLabelPosition_;

  std::string str;
  prop_.SerializePartialToString(&str);
  ds << QByteArray::fromRawData(str.data(), str.size());

  ds << mapSize_;

  ds << inverted_;
  ds << (int32_t)invertedVal_;

  ds << (int32_t)tickXDirection_;
  ds << (int32_t)tickYDirection_;
  ds << (int32_t)tickLabelAlignFlags_;
  ds << (int32_t)tickLabelXOffset_;
  ds << (int32_t)tickLabelYOffset_;
  ds << tickLabelXOffsetFactor_;
  ds << tickLabelYOffsetFactor_;

  QRect rect;
  rect.setLeft(margins_.left());
  rect.setTop(margins_.top());
  rect.setRight(margins_.right());
  rect.setBottom(margins_.bottom());
  ds << rect;

  ds << (int32_t)curLevel_;
  ds << dataByLevel_;

  int32_t sz = ba.size();
  ::memcpy( data + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  ::memcpy( data + pos, ba.data(), ba.size() );
  pos += ba.size();

  return pos;
}

int32_t AxisSpectr::parseFromArray(const char* data)
{
  int32_t pos = 0;

  int32_t sz = 0;
  global::fromByteArray( data + pos, &sz );
  pos += sizeof(sz);

  QByteArray ba = QByteArray::fromRawData(data + pos, sz);

  QDataStream ds(&ba, QIODevice::ReadOnly);
  ds.setVersion(QDataStream::Qt_4_0);
  ds >> autoTicks_;
  ds >> rangeLower_;
  ds >> rangeUpper_;
  ds >> factor_;

  int32_t p;
  ds >> p;
  position_ = static_cast<Position>(p);
  ds >> tickSize_;
  ds >> nSteps_;
  ds >> subTickCount_;
  ds >> labelFont_;
  ds >> p;
  labelType_ = static_cast<LabelType>(p);
  ds >> axisLabel_;
  ds >> p;
  axisLabelPosition_ = static_cast<TitlePosition>(p);

  QByteArray prop;
  ds >> prop;
  prop_.ParsePartialFromArray(prop.data(), prop.size());

  ds >> mapSize_;

  ds >> inverted_;
  ds >> invertedVal_;

  ds >> tickXDirection_;
  ds >> tickYDirection_;
  ds >> tickLabelAlignFlags_;
  ds >> tickLabelXOffset_;
  ds >> tickLabelYOffset_;
  ds >> tickLabelXOffsetFactor_;
  ds >> tickLabelYOffsetFactor_;

  QRect rect;
  ds >> rect;
  margins_.setLeft(rect.left());
  margins_.setTop(rect.top());
  margins_.setRight(rect.right());
  margins_.setBottom(rect.bottom());

  ds >> curLevel_;
  ds >> dataByLevel_;

  pos += sz;

  return pos;
}

void AxisSpectr::updateFactor()
{
  double len = qAbs(rangeUpper_ - rangeLower_);
  if ( len > 0 ) {
    if ( isHorizontal() ) {
      factor_ = mapSize_.width() / len;
    }
    else {
      factor_ = mapSize_.height() / len;
    }
  }
  else {
    factor_ = 1;
  }
}

void AxisSpectr::calcTicks() const
{
  clearCache();
//TODO решить вопрос с генерализацией тиков
//  if (0 != scale_) {
//    if ( scale_< 10 ) {
//      nSteps_ = 100;
//    }
//    else if ( scale_ < 11 ) {
//      nSteps_ = 80;
//    }
//    else if ( scale_ < 11.5 ) {
//      nSteps_ = 50;
//    }
//    else if ( scale_ < 12) {
//      nSteps_ = 20;
//    }
//    else {
//      nSteps_ = 10;
//    }
//  }
  dataByLevel_[curLevel_].ticks_.resize(nSteps_ + 1);

  double step = qAbs(rangeUpper_ - rangeLower_) / nSteps_;
  for ( int i=0; i<=nSteps_; ++i ) {
    dataByLevel_[curLevel_].ticks_[i] = (rangeLower_ + step*i);
  }
}

void AxisSpectr::clearCache() const
{
  dataByLevel_[curLevel_].cacheTickLabels_.clear();
  dataByLevel_[curLevel_].cacheTickLabelSize_ = QSize(0,0);
}

void AxisSpectr::updateTicksCache() const
{
  if ( !dataByLevel_[curLevel_].cacheTickLabels_.isEmpty() ) { return; }

  dataByLevel_[curLevel_].cacheTickLabels_.resize(dataByLevel_[curLevel_].ticks_.size());

  QSize size;
  QString text;
  QFontMetrics fm(labelFont_);
  for ( int i=0,isz=dataByLevel_[curLevel_].ticks_.size(); i<isz; ++i ) {
    text = tickLabel(dataByLevel_[curLevel_].ticks_.at(i));
    dataByLevel_[curLevel_].cacheTickLabels_[i] = text;
    dataByLevel_[curLevel_].cacheTickLabelSize_ = dataByLevel_[curLevel_].cacheTickLabelSize_.expandedTo(fm.boundingRect(0,0,0,0, Qt::TextDontClip, text).size());
  }
}

bool AxisSpectr::compareTicks(const AxisSpectr& a) const
{
  if ( dataByLevel_[curLevel_].ticks_.size() != a.dataByLevel_[curLevel_].ticks_.size() ) { return false; }

  for ( int i=0,isz=dataByLevel_[curLevel_].ticks_.size(); i<isz; ++i ) {
    if ( !MnMath::isEqual(dataByLevel_[curLevel_].ticks_.at(i), a.dataByLevel_[curLevel_].ticks_.at(i)) ) {
      return false;
    }
  }

  return true;
}

bool AxisSpectr::isEqual(const AxisSpectr& a) const
{
  bool b = true;
  b = b && (position_ == a.position_)
        && MnMath::isEqual(rangeLower_, a.rangeLower_)
        && MnMath::isEqual(rangeUpper_, a.rangeUpper_)
        && (dataByLevel_[curLevel_].format_ == a.dataByLevel_[curLevel_].format_)
        && (tickSize_ == a.tickSize_)
        && (nSteps_ == a.nSteps_)
        && (labelFont_ == a.labelFont_)
        && (autoTicks_ == a.autoTicks_)
        && (subTickCount_ == a.subTickCount_)
        && (inverted_ == a.inverted_)
        && (prop_.SerializeAsString() == a.prop_.SerializeAsString())
        && compareTicks(a)
      ;
  return b;
}

} // meteo

QDataStream& operator>>(QDataStream& in, meteo::AxisData& data)
{
  in >> data.level;
  in >> data.format_;
  in >> data.ticks_;
  in >> data.lastDrawed_;
  in >> data.cacheTickLabelSize_;
  in >> data.cacheTickLabels_;

  return in;
}

QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data)
{
  out << data.level;
  out << data.format_;
  out << data.ticks_;
  out << data.lastDrawed_;
  out << data.cacheTickLabelSize_;
  out << data.cacheTickLabels_;

  return out;
}
