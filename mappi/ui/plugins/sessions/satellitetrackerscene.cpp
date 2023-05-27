#include "satellitetrackerscene.h"

#include <qgraphicsitem.h>

#include <sat-commons/satellite/satellite.h>
#include <sat-commons/satellite/satviewpoint.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopixmap.h>

#include <mappi/global/global.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/schedule/schedule.h>
#include <mappi/schedule/schedulehelper.hpp>

static meteo::GeoPoint kCenter(0.0, 0.0);                         //!< Центровка карты
static float           kOrbitalScale                 = 19.75;     //!< Масштаб для режима отображения траектории спутника
static float           kReceiverScale                = 18.0;      //!< Масштаб для режима отображения станции приема
static int             kVerticalStepForSatrlliteArea = 1000;      //!< Шаг для расчета зоны снимка спутника

namespace mappi {

SatelliteTrackerScene::SatelliteTrackerScene(QGraphicsView *parent)
  : QGraphicsScene(parent)
  , updateTimer_(new QTimer(parent))
  , document_(new meteo::map::Document())
  , layer_(new meteo::map::Layer(document_))
  , satellite_(new Satellite())
  , nadirAngle_(55.37)
  , satelliteTrajectory_(10000)
  , satelliteAreaCoord_(4000)
  , receiverAreas_(nullptr)
  , satelliteLabel_(nullptr)
  , satelliteIcon_(nullptr)
  , satelliteText_(nullptr)
  , satelliteDirection_(nullptr)
{
  updateTimer_->setInterval(1000);
  connect(updateTimer_, &QTimer::timeout, this, &SatelliteTrackerScene::slotTimeout);
}

SatelliteTrackerScene::~SatelliteTrackerScene()
{
  delete document_;
  delete satellite_;
}

bool SatelliteTrackerScene::setupMap(MapType mode)
{
  mapMode_ = mode;

  meteo::map::proto::Document doc;

  if(document_->documentsize().width() == 0 || document_->documentsize().height() == 0) {
    doc.mutable_mapsize()->CopyFrom(meteo::qsize2size(QSize(2000, 2000)));
  }
  else {
    doc.mutable_mapsize()->CopyFrom(meteo::qsize2size(document_->mapsize()));
  }

  doc.set_geoloader("geo.old");
  //doc.set_geoloader("ptkpp");
  doc.set_cache(true);

  switch (mode) {
  case MapType::kOrbita:
    doc.set_scale(kOrbitalScale);
    doc.set_projection(meteo::kMercat);
    doc.mutable_doc_center()->CopyFrom(meteo::geopoint2pbgeopoint(kCenter));
    doc.mutable_map_center()->CopyFrom(meteo::geopoint2pbgeopoint(kCenter));
    break;
  case MapType::kReceiver:
    doc.set_scale(kReceiverScale);
    doc.set_projection(meteo::kStereo);
    doc.mutable_doc_center()->CopyFrom(meteo::geopoint2pbgeopoint(receiverCoords_));
    doc.mutable_map_center()->CopyFrom(meteo::geopoint2pbgeopoint(receiverCoords_));
    break;
  default:
    doc.set_scale(kOrbitalScale);
    doc.mutable_doc_center()->CopyFrom(meteo::geopoint2pbgeopoint(kCenter));
    doc.mutable_map_center()->CopyFrom(meteo::geopoint2pbgeopoint(kCenter));
    doc.set_projection(meteo::kMercat);
    break;
  }
  if(false == document_->init(doc)) {
    error_log << QObject::tr("Не удалось создать документ с параметрами = %1")
                 .arg(QString::fromStdString(doc.Utf8DebugString()));
    return false;
  }
  if(mode == MapType::kReceiver) {
    document_->projection()->setMapCenter(receiverCoords_);
  }
  layer_ = new meteo::map::Layer(document_);

  meteo::map::Loader::instance()->handleGrid("grid.common", document_);
  QGraphicsScene::setBackgroundBrush(QBrush(meteo::kMAP_ISLAND_COLOR));


  return true;
}

void SatelliteTrackerScene::resizeDocument(const QSize& size)
{
  document_->resizeDocument(size);
  document_->resizeMap(size);
  document_->setScreenCenter(document_->screenCenter());
  redraw();
}

void SatelliteTrackerScene::resize(const QSize& size)
{
  setSceneRect(QRect(QPoint(0, 0), size));
  resizeDocument(size);
}

void SatelliteTrackerScene::changeMapMode(MapType mode)
{
  if(mode == mapMode_) return;
  setupMap(mode);
  resizeDocument(document_->documentsize());
}

void SatelliteTrackerScene::setConf(const mappi::conf::Reception& conf)
{
  if(true == conf.has_site())
  {
    if(true == conf.site().has_name()) {
      receiverName_ = QString::fromStdString(conf.site().name());
    }
    else {
      receiverName_ = QObject::tr("Неизвестная станция");
    }
    
    receiverCoords_ =  meteo::GeoPoint(conf.site().point().lat_radian(),
				       conf.site().point().lon_radian(),
				       conf.site().point().height_meters());

   createReceiverLabel(receiverName_, receiverCoords_);
  }
  setupMap(mapMode_);
}

void SatelliteTrackerScene::setSession(std::shared_ptr<schedule::Session> session, const conf::Instrument& instrument,
               int timeOffset, const MnSat::STLEParams &stle )
{
  timeOffset_ = timeOffset;
  nadirAngle_ = instrument.has_scan_angle() ? instrument.scan_angle() : 55.37;

  if(false == stle.satName.isEmpty() && satellite_->readTLE(stle)) {
    createSatelliteInformation(session);
    if(false == updateTimer_->isActive()) {
      updateTimer_->start();
    }
  }
}

void SatelliteTrackerScene::slotTimeout()
{
  createSatelliteLabel(satellite_->name());
  invalidate();
}

void SatelliteTrackerScene::redraw()
{
  drawReceiverArea();
  drawSatelliteTrajectory();
  drawSatelliteArea();
  createReceiverLabel(receiverName_, receiverCoords_);
  createSatelliteLabel(satellite_->name());

  invalidate();
}

void SatelliteTrackerScene::drawBackground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawBackground(painter, rect);
  document_->drawDocument(painter);
}
void SatelliteTrackerScene::drawForeground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawForeground(painter, rect);
  document_->drawRamka(painter);
}


void SatelliteTrackerScene::createReceiverLabel(const QString& receiverName, const meteo::GeoPoint& pos)
{
  meteo::GeoVector position;
  position.append(pos);
  // Название станции приема
  {
    meteo::map::GeoText* label = mapobject_cast<meteo::map::GeoText*>(layer_->objectByUuid(receiverLabelUuid_));
    if(nullptr == label) {
      label = new meteo::map::GeoText(layer_);
      receiverLabelUuid_ = label->uuid();

      meteo::Property prop;
      meteo::Font* font;
      font = prop.mutable_font();
      font->set_family("Carlito");
      font->set_pointsize(12);
      prop.set_pos(meteo::kTopRight);

      label->setProperty(prop);
    }
    label->setSkelet(position);
    label->setText(receiverName);
  }
  // Иконка станции приема
  {
    meteo::map::GeoPixmap* pix = mapobject_cast<meteo::map::GeoPixmap*>(layer_->objectByUuid(receiverPixUuid_));
    if(nullptr == pix) {
      pix = new meteo::map::GeoPixmap(layer_);
      receiverPixUuid_ = pix->uuid();

      float ratio = 0.25;
      meteo::Property prop;
      meteo::PointF* scale;
      scale = prop.mutable_scalexy();
      scale->set_x(ratio);
      scale->set_y(ratio);

      pix->setProperty(prop);
      pix->setImage(QImage(":/mappi/icons/radar.png"));
      pix->setVisible(true);
    }
    pix->setSkelet(position);
  }
}
void SatelliteTrackerScene::createSatelliteLabel(const QString& satelliteName)
{
  QPointF satPosition;
  {
    Coords::GeoCoord geoPos;
    satellite_->getPosition(QDateTime::currentDateTimeUtc().addSecs(timeOffset_), &geoPos);
    satPosition = document_->coord2screenf(geoPos.getGeoPoint());
  }
  // Название спутника
  {
    if(nullptr == satelliteText_) {
      QFont font;
      font.setFamily("Carlito");
      font.setPixelSize(12);

      satelliteText_ = addText(satelliteName, font);
    }
    else {
      satelliteText_->setPlainText(satelliteName);
    }
  }
  // Иконка спутника
  {
    if(nullptr == satelliteIcon_)
    {
      float scaleFactor = 0.20;
      QPixmap pix(":/mappi/icons/satellite.png");

      int iconHeight = pix.height();
      int iconWidth = pix.width();
      int offsetX = scaleFactor * iconWidth / 2;
      int offsetY = scaleFactor * iconHeight;

      satelliteIcon_ = addPixmap(pix);
      satelliteIcon_->setScale(scaleFactor);
      satelliteIcon_->setPos(-offsetX, -offsetY / 4.0);
      satelliteText_->setPos(offsetX, -offsetY);
    }
  }
  // Направление движения спутника
  {
    //координаты спутника спусти i минут
    QVector<QPointF> pts;
    pts.append(satPosition);
    QPointF prevPos = satPosition, curPos;
    for(int i = 1; i < 6; ++i)
    {
      Coords::GeoCoord gpt;
      satellite_->getPosition(QDateTime::currentDateTimeUtc().addSecs(timeOffset_).addSecs(i * 60), &gpt);
      curPos = document_->coord2screenf(gpt.getGeoPoint());
      if(std::abs(prevPos.x() - curPos.x()) > 100) {
        break;
      }
      pts.append(curPos);
      prevPos = curPos;
    }

    //Стрелка
    QPoint arrow;
    int endPtIndex = pts.size() - 1;
    if(endPtIndex > 0)
    {
      QPointF dist = pts.at(endPtIndex) - pts.at(endPtIndex - 1);
      double angle = std::atan2(dist.x(), dist.y()) + M_PI*0.5;
      arrow.setX(MnMath::ftoi_norm(std::sin(angle) * 4));
      arrow.setY(MnMath::ftoi_norm(std::cos(angle) * 4));
    }

    //Полигон
    QPolygonF direction;
    for(int i = 0; i < endPtIndex; ++i)
    {
      direction.append(pts.at(i));
      if(i > 0 && i == endPtIndex - 1)
      {
        direction.append(pts.at(i) - arrow);
        direction.append(pts.at(endPtIndex));
        direction.append(pts.at(i) + arrow);
        for(int j = i; j > 0; --j) {
          direction.append(pts.at(j));
        }
        direction.append(pts.at(0));
      }
    }

    if(nullptr == satelliteDirection_) {
      satelliteDirection_ = addPolygon(direction, QPen(QBrush(Qt::red), 2.0));
      satelliteDirection_->setBrush(QBrush(Qt::red));
      satelliteDirection_->setZValue(19.0);
    }
    else {
      satelliteDirection_->setPolygon(direction);
    }
  }

  if(nullptr == satelliteLabel_) {
    satelliteLabel_ = createItemGroup({ satelliteIcon_, satelliteText_ });
    satelliteLabel_->setZValue(20.0);
  }
  satelliteLabel_->setPos(satPosition);
}
  void SatelliteTrackerScene::createSatelliteInformation(std::shared_ptr<schedule::Session> session)
{
  // Троекотория [-64; 82] минут от окна приема (Можно более точно подобрать границы времени для уменьшения вичислений лишних точек
  {
    QVector<Coords::GeoCoord> geoVec;
    satellite_->trajectory(session->data().aos.addSecs(-64 * 60).toUTC(), session->data().los.addSecs(82 * 60).toUTC(), 1.0, geoVec);
    Coords::GeoCoord satPos;
    satellite_->getPosition(session->data().aos.addMSecs(session->data().los.toMSecsSinceEpoch() - session->data().aos.toMSecsSinceEpoch() / 2.0), &satPos);
    satelliteHeight_ = satPos.alt;
    if(geoVec.isEmpty()) {
      debug_log << QObject::tr("Для спутника %1 не найдено точек траектории.").arg(session->data().satellite);
    }
    else
    {
      int size = geoVec.size();
      int  left, right, mid = size / 2;

      //Ищем левую границу отображаемой траектории
      bool isInvert = true;
      for(left = mid; left > 0; --left)
      {
        if(isInvert) {
          isInvert = geoVec.at(left).lon > M_PI;
          continue;
        }
        if(geoVec.at(left).lon > M_PI) {
          break;
        }
      }
      //Ищем правую границу отображаемой траектории
      isInvert = true;
      for(right = mid; right < size; ++right)
      {
        if(isInvert) {
          isInvert = geoVec.at(right).lon < M_PI;
          continue;
        }
        if(geoVec.at(right).lon < M_PI) {
          break;
        }
      }

      satelliteTrajectory_.clear();
      for(int i = left; i < right; ++i) {
        satelliteTrajectory_.append(geoVec[i].getGeoPoint());
      }

      drawSatelliteTrajectory();
    }
  }
  // Границы зоны снимка
  SatViewPoint satViewPoint(session->data().aos, session->data().los);
  {
    satViewPoint.setTLEParams(satellite_->getTLEParams());
    satelliteAreaCoord_.clear();
    if(satViewPoint.countBorder(kVerticalStepForSatrlliteArea, meteo::DEG2RAD * nadirAngle_, &satelliteAreaCoord_)) {
      drawSatelliteArea();
    }
  }
  // Зоны приёма станции
  {
    QVector<float> receiverPointsSat;
    receiverPointsSat.append(-1);
    for(int i = 0; i < 3; ++i)
    {
      float angle = 30.0f * i;
      receiverPointsSat.append(angle);
    }

    Coords::GeoCoord borderSatPt;
    satViewPoint.getPosition(session->data().aos, &borderSatPt);
    RcvWndInf* inf;

    for(const auto& it : receiverPointsSat) {
      float dist;
      if(nullptr == receiverAreaInfo_.value(it)) {
        inf = new RcvWndInf();
      }
      else {
        inf = receiverAreaInfo_.value(it);
      }

      if(-1.0 == it) { //Зона для орбитальной карты
        dist = receiverCoords_.radianDistance(borderSatPt.getGeoPoint());
      }
      else { //Зоны для карты приёмника
        dist = calcRadius(meteo::DEG2RAD * it, satelliteHeight_);
        meteo::GeoPoint secondPt = receiverCoords_.findSecondCoord(dist, M_PI);
        dist = receiverCoords_.radianDistance(secondPt);
      }

      inf->circle = getCircle(receiverCoords_, dist);
      receiverAreaInfo_.insert(it, inf);
    }
    drawReceiverArea();
  }
}


/*! Рисуем зоны приёма станции */
void SatelliteTrackerScene::drawReceiverArea()
{
  for (auto it = receiverAreaInfo_.begin(); it != receiverAreaInfo_.end(); ++it) {
    meteo::map::GeoPolygon* area = mapobject_cast<meteo::map::GeoPolygon*>(layer_->objectByUuid(it.value()->areaUuid_));
    if(nullptr == area)
    {
      area = new meteo::map::GeoPolygon(layer_);
      it.value()->areaUuid_ = area->uuid();


      meteo::Font font;
      font.set_family("Carlito");
      font.set_pointsize(12);

      if(-1.0 == it.key()) {
        area->setPen(meteo::qpen2pen(QPen(QBrush(Qt::darkGreen), 1)));
      }
      else {
        area->setPen(meteo::qpen2pen(QPen(QBrush(Qt::darkGreen), 1)));
      }

      area->setFont(font);
    }
    if(MapType::kOrbita == mapMode_)
    {
      if(-1.0 != it.key()) {
        area->setSkelet(meteo::GeoVector());
        continue;
      }
      else {
        area->setSkelet(it.value()->circle);
      }
    }
    else
    {
      if(-1.0 == it.key()) {
        area->setSkelet(meteo::GeoVector());
        continue;
      }
      else {
        area->setSkelet(it.value()->circle);
        area->setValue(it.key());
      }
    }
  }
}
/*! Рисуем траекторию спутника */
void SatelliteTrackerScene::drawSatelliteTrajectory()
{
  if(satelliteTrajectory_.isEmpty()) return;

  meteo::map::GeoPolygon* trajectory = mapobject_cast<meteo::map::GeoPolygon*>(layer_->objectByUuid(satelliteTrajectoryUuid_));
  if(nullptr == trajectory) {
    trajectory = new meteo::map::GeoPolygon(layer_);
    satelliteTrajectoryUuid_ = trajectory->uuid();
    trajectory->setPen(meteo::qpen2pen(QPen(QBrush(Qt::black), 1)));
  }
  trajectory->setSkelet(satelliteTrajectory_);
}
/*! Рисуем зону снимка */
void SatelliteTrackerScene::drawSatelliteArea()
{
  if(satelliteAreaCoord_.isEmpty()) return;

  meteo::map::GeoPolygon* area = mapobject_cast<meteo::map::GeoPolygon*>(layer_->objectByUuid(satelliteAreaUuid_));
  if(nullptr == area) {
    area = new meteo::map::GeoPolygon(layer_);
    satelliteAreaUuid_ = area->uuid();
    area->setPen(meteo::qpen2pen(QPen(QBrush(Qt::darkRed), 2.0)));
    area->setClosed(true);
  }
  area->setSkelet(satelliteAreaCoord_);
  area->setVisible(MapType::kReceiver == mapMode_);
}


float SatelliteTrackerScene::calcRadius(float angle, float height) // static
{
  double radius;
  double R = height + R_E;

  if(0.0 == angle) {
    radius = std::sqrt((R * R) - (R_E * R_E));
  }
  else {
    double hi = height * (1 - cos(M_PI_2 - angle));
    double h = height - hi;
    double r = h + R_E;
    radius = std::sqrt((R * R) - (r * r));
  }
  radius /= R_E;
  return radius;
}
meteo::GeoVector SatelliteTrackerScene::getCircle(const meteo::GeoPoint &center, float radius) // static
{
  meteo::GeoVector coords;
  for(int i = 0; i < 360; ++i) {
      coords.append(center.findSecondCoord(radius, meteo::DEG2RAD * i));
  }
  return coords;
}

} // mappi
