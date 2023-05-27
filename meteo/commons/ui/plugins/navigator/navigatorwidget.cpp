#include "navigatorwidget.h"

#include <commons/geobasis/geovector.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/view/mapscene.h>

#include <QApplication>
#include <QColor>
#include <QCursor>
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRegExp>
#include <QRect>
#include <QSize>
#include <QString>
#include <QTransform>
#include <qlayout.h>

namespace
{
const QString widgetTitleName() { return QString::fromUtf8("Навигатор"); }

const QColor backgroundColor(const QColor* next = nullptr)
{
  static QColor color = QColor(Qt::white);
  if (next != nullptr) {
    QColor prev = color;
    color = *next;
    return prev;
  }
  return color;
}

const QColor pointColor(const QColor* next = nullptr)
{
  static QColor color = QColor(Qt::red);
  if (next != nullptr) {
    QColor prev = color;
    color = *next;
    return prev;
  }
  return color;
}

const QColor markerColor(const QColor* next = nullptr)
{
  static QColor color = QColor(Qt::red);
  if (next != nullptr) {
    QColor prev = color;
    color = *next;
    return prev;
  }
  return color;
}

int pointRadius(int* next = nullptr)
{
  static int radius = 1;
  if (next != nullptr) {
    int prev = radius;
    radius = *next;
    return prev;
  }
  return radius;
}

int markerWidth(int* next = nullptr)
{
  static int width = 2;
  if (next != nullptr) {
    int prev = width;
    width = *next;
    return prev;
  }
  return width;
}

int markerRadius(int* next = nullptr)
{
  static int radius = 3;
  if (next != nullptr) {
    int prev = radius;
    radius = *next;
    return prev;
  }
  return radius;
}

QBrush displayRectBrush()
{
  const int opaquePercent = 50;
  QColor color(Qt::gray);
  color.setAlpha(255-opaquePercent*0.01*255);

  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(color);

  return brush;
}

QPen displayRectPen()
{
  QColor color(Qt::black);
  QPen pen;
  pen.setColor(color);

  return pen;
}

double defaultScale()
{
  return 20.5;
}

QSize defaultSize()
{
  return QSize(500,500);
}

const meteo::GeoPoint invalidGeoPoint()
{
  return meteo::GeoPoint::Invalid;
}

}

namespace meteo {
namespace map {

NavigatorWidget::NavigatorWidget(MapWindow* window) :
  MapWidget(window),
  paintArea_(new QLabel(this)),
  currentDoc_(nullptr),
  markerPosition_(::invalidGeoPoint()),
  needReDraw_(true)
{
  setTitle(::widgetTitleName());

  if (layout() == nullptr) {
    setLayout(new QHBoxLayout(this));
  }

  layout()->addWidget(paintArea_);
  layout()->setSizeConstraint(QLayout::SetFixedSize);
}

NavigatorWidget::~NavigatorWidget()
{
}

void NavigatorWidget::turnSelectingPosition(bool active)
{
  if (active == true) {
    paintArea_->installEventFilter(this);
  }
  else {
    paintArea_->removeEventFilter(this);
  }
}

void NavigatorWidget::initFromDocument(Document* doc)
{
  if (doc == nullptr) {
    return;
  }

  if (   currentDoc_ == nullptr
      || !qFuzzyCompare(currentDoc_->rotateAngle(),
                        doc->rotateAngle())) {
    delete currentDoc_;

    GeoPoint docCenter;
    switch (doc->projection()->type()) {
      case meteo::STEREO:
        docCenter = GeoPoint(doc->projection()->hemisphere()*M_PI/2.0, doc->center().lon());
      break;
      case meteo::CONICH:
      case meteo::MERCAT:
      default:
        docCenter = GeoPoint(0.0, doc->center().lon());
      break;
    }
    QColor docBackground = doc->backgroundColor();
    ::backgroundColor(&docBackground);
    currentDoc_ = doc->stub(docCenter, ::defaultSize());
    makeOriginalPixmap();
    makePointsPixmap();
  }
  needReDraw_ = true;
  reDraw();
}

void NavigatorWidget::makeOriginalPixmap()
{
  if (currentDoc_ != nullptr) {
    currentDoc_->setScale(::defaultScale());
    currentDoc_->setRamkaWidth(0);

    QRegExp re(QString("(?:%1|%2|%3)")
               .arg(QString::fromUtf8("координатная"))
               .arg(QString::fromUtf8("гидрография"))
               .arg(QString::fromUtf8("береговая")));
    QList<QString> drawedLayers;
    QList<Layer*> hiddenLayers;
    foreach (Layer* each, currentDoc_->layers()) {
      if (each != nullptr) {
        if (re.indexIn(each->name().toLower()) != -1) {
          drawedLayers.append(each->uuid());
          if (!each->visible()) {
            hiddenLayers.append(each);
            each->show();
          }
        }
      }
    }

    QPixmap tmp = QPixmap(::defaultSize());
    tmp.fill(::backgroundColor());
    QScopedPointer<QPainter> painter(new QPainter(&tmp));
    currentDoc_->drawDocument(painter.data(),
                              drawedLayers,
                              tmp.rect(),
                              currentDoc_->documentRect());
    painter.reset();
    QListIterator<Layer*> it(hiddenLayers);
    while (it.hasNext()) {
      it.next()->hide();
    }

    originalMap_ = tmp.copy(currentDoc_->mapramka().boundingRect());
    QPolygon one(currentDoc_->mapramka().boundingRect());
    QPolygon two(originalMap_.rect());
    if (!QTransform::quadToQuad(QPolygonF(one), QPolygonF(two), doc2pixmap_)) {
      doc2pixmap_ = QTransform();
    }
    if (!QTransform::quadToQuad(QPolygonF(two), QPolygonF(one), pixmap2doc_)) {
      pixmap2doc_ = QTransform();
    }
  }
  else {
    error_log << QString::fromUtf8("Ошибка инициализации документа");
    originalMap_ = QPixmap();
  }
}

void NavigatorWidget::makePointsPixmap()
{
  pointsMap_ = QPixmap(originalMap_.size());
  QColor transparent(Qt::white);
  transparent.setAlpha(0);
  pointsMap_.fill(transparent);

  if (!visiblePoints_.empty()) {
    GeoVector points;
    points.reserve(visiblePoints_.size());
    for (std::set<GeoPoint>::const_iterator it = visiblePoints_.begin(), end = visiblePoints_.end(); it != end; ++it) {
      points.append(*it);
    }
    changePointsPixmap(points, true);
  }
}

void NavigatorWidget::reDraw()
{
  if (needReDraw_ == true) {
    QPixmap pix(originalMap_);
    if (currentDoc_ != nullptr) {
      drawVisiblePoints(&pix);
      drawPositionMarker(&pix);
      drawDisplayRamka(&pix);
    }
    paintArea_->setPixmap(pix);
    needReDraw_ = false;
  }
}

bool NavigatorWidget::checkGeoPoint(const GeoPoint& point) const
{
  return visiblePoints_.find(point) != visiblePoints_.end();
}

void NavigatorWidget::clearAllPoints()
{
  visiblePoints_.clear();
  makePointsPixmap();
  needReDraw_ = true;
  reDraw();
}

void NavigatorWidget::setGeoPoint(const GeoPoint& point)
{
  if (setGeoPoint(point, true) == true) {
    changePointsPixmap(GeoVector(QVector<GeoPoint>() << point), true);
    needReDraw_ = true;
    reDraw();
  }
}

void NavigatorWidget::clearGeoPoint(const GeoPoint& point)
{
  if (setGeoPoint(point, false) == true) {
    changePointsPixmap(GeoVector(QVector<GeoPoint>() << point), false);
    needReDraw_ = true;
    reDraw();
  }
}

bool NavigatorWidget::setGeoPoint(const GeoPoint& point, bool visible)
{
  bool changed = false;
  if (visible == true) {
    if (isValidGeoPoint(point) == true && checkGeoPoint(point) == false) {
      visiblePoints_.insert(point);
      changed = true;
    }
  }
  else {
    if (checkGeoPoint(point) == true) {
      visiblePoints_.erase(visiblePoints_.find(point));
      changed = true;
    }
  }
  return changed;
}

void NavigatorWidget::setGeoVector(const GeoVector& points)
{
  setGeoVector(points, true);
}

void NavigatorWidget::clearGeoVector(const GeoVector& points)
{
  setGeoVector(points, false);
}

void NavigatorWidget::setGeoVector(const GeoVector& points, bool visible)
{
  bool changed = false;
  for (int i = 0, sz = points.size(); i < sz; ++i) {
    changed = setGeoPoint(points[i], visible) || changed;
  }

  if (changed == true) {
    changePointsPixmap(points, visible);
    needReDraw_ = true;
    reDraw();
  }
}

void NavigatorWidget::changePointsPixmap(const GeoVector& points, bool visible)
{
  QPolygon pos;
  pos.reserve(points.size());
  QVectorIterator<GeoPoint> it(points);
  while (it.hasNext()) {
    const GeoPoint& each = it.next();
    if (isValidGeoPoint(each) == true) {
      pos.append(doc2pixmap_.map(currentDoc_->coord2screen(each)));
    }
  }

  QPen pen;
  if (visible == true) {
    pen.setColor(::pointColor());
  }
  else {
    QColor color(Qt::white);
    color.setAlpha(0);
    pen.setColor(color);
  }
  pen.setWidth(::pointRadius());

  QPainter painter(&pointsMap_);
  painter.setPen(pen);
  painter.drawPoints(pos);
}

void NavigatorWidget::drawVisiblePoints(QPixmap* target)
{
  Q_CHECK_PTR(target);

  QPainter painter(target);
  painter.drawPixmap(target->rect(), pointsMap_);
}

void NavigatorWidget::drawPositionMarker(QPixmap* target)
{
  Q_CHECK_PTR(target);

  if (isValidGeoPoint(markerPosition_) == true) {
    QPainter pnt(target);
    QPen pen;
    pen.setColor(::markerColor());
    pen.setWidth(::markerWidth());
    pnt.setPen(pen);

    int r = ::markerRadius();
    QPointF pos = doc2pixmap_.map(currentDoc_->coord2screenf(markerPosition_));

    pnt.drawLine(QLineF(QPointF(pos.x()-r, pos.y()),
                        QPointF(pos.x()+r, pos.y())));
    pnt.drawLine(QLineF(QPointF(pos.x(), pos.y()-r),
                        QPointF(pos.x(), pos.y()+r)));
  }
}

void NavigatorWidget::drawDisplayRamka(QPixmap* target)
{
  Q_CHECK_PTR(target);
  QPainter pnt(target);
  pnt.setPen(::displayRectPen());
  pnt.setBrush(::displayRectBrush());

  QPainterPath path;
  path.addPolygon(doc2pixmap_.map(currentDoc_->coord2screen(displayRamka_)));
  path.closeSubpath();
  pnt.drawPath(path);
}

bool NavigatorWidget::isValidGeoPoint(const GeoPoint& pos) const
{
  if (currentDoc_ == nullptr || currentDoc_->projection() == nullptr) {
    return false;
  }
  Projection* p = currentDoc_->projection();
  switch (p->type()) {
    case MERCAT:
      return (p->endFi() <= pos.fi() && pos.fi() <= p->startFi());
    case STEREO:
      return (qMin(p->startFi(), p->endFi()) <= pos.fi() && pos.fi() <= qMax(p->startFi(), p->endFi()));
    case CONICH:
      return (qMin(p->startFi(), p->endFi()) <= pos.fi() && pos.fi() <= qMax(p->startFi(), p->endFi()));
    default:
      break;
  }
  return false;
}

const GeoPoint NavigatorWidget::currentPosition() const
{
  GeoPoint result(markerPosition_);
  if (result.lon() < -M_PI) {
    result.setLon(result.lon() + 2.0*M_PI);
  }
  else if (result.lon() > M_PI) {
    result.setLon(result.lonDeg() - 2.0*M_PI);
  }
  return result;
}

void NavigatorWidget::clearCurrentPosition()
{
  setCurrentPosition(::invalidGeoPoint());
}

void NavigatorWidget::setCurrentPosition(const GeoPoint& pos)
{
  needReDraw_ = markerPosition_ != pos;
  markerPosition_ = (isValidGeoPoint(pos) ? pos : ::invalidGeoPoint());
  reDraw();
}

void NavigatorWidget::setDisplayRamka(const GeoVector& ramka)
{
  needReDraw_ = displayRamka_ != ramka;
  if (needReDraw_) {
    displayRamka_ = ramka;
  }
  reDraw();
}

bool NavigatorWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == paintArea_) {
    switch (event->type()) {
      case QEvent::Enter:
          QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
        return true;
      case QEvent::Leave:
          QApplication::restoreOverrideCursor();
        return true;
      case QEvent::MouseButtonPress:
          if (currentDoc_ != nullptr) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
              GeoPoint gp = currentDoc_->screen2coord(pixmap2doc_.map(me->pos()));
              emit selectedPosition(gp);
            }
          }
        return true;
      default:
        break;
    }
  }
  return MapWidget::eventFilter(obj, event);
}

QColor NavigatorWidget::setBackgroundColor(const QColor& color)
{
  return ::backgroundColor(&color);
}

QColor NavigatorWidget::setPointsColor(const QColor& color)
{
  return ::pointColor(&color);
}

QColor NavigatorWidget::setMarkerColor(const QColor& color)
{
  return ::markerColor(&color);
}

int NavigatorWidget::setPointsPenWidth(int pixel)
{
  return ::pointRadius(&pixel);
}

int NavigatorWidget::setMarkerPenWidth(int pixel)
{
  return ::markerWidth(&pixel);
}

int NavigatorWidget::setMarkerRadius(int pixel)
{
  return ::markerRadius(&pixel);
}

} // map
} // meteo
