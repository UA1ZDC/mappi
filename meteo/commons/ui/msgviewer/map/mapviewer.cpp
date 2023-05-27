#include "mapviewer.h"

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/selfchecked/selfcheckedcontroller.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/navigator/navigatorwidget.h> //fix
#include <meteo/novost/global/global.h>
#include <meteo/novost/ui/msgviewer/decode/tlgdecode.h>
#include <sql/nspgbase/ns_pgbase.h>
#include <sql/nspgbase/tsqlquery.h>

#include <QCoreApplication>
#include <QList>
#include <QMessageBox>
#include <QString>

namespace {
  const meteo::GeoPoint mapCenterPoint() { return meteo::GeoPoint::fromDegree(60.0, 30.0); }
  const QString querySelectDecodedOnly() { return QString("SELECT id FROM telegrams WHERE decode = TRUE AND id IN (%1)"); }
}

namespace meteo {

MapViewer::MapViewer(QWidget* parent) :
  QObject(parent),
  map_(0),
  navigator_(new map::NavigatorWidget(parent)),
  serviceDecodeCtrl_(new rpc::SelfCheckedController( meteo::global::serviceAddress( meteo::settings::proto::kSrcData ) ))

{
  map_ = new map::Map(STEREO);
  map_->projection()->setMapCenter(::mapCenterPoint());
  QSharedPointer<map::Document> doc(map_->createDocument(QSize(500, 500)));
  navigator_->setCurrentDocument(*doc);
  navigator_->setMarkerRadius(2);
  navigator_->setMarkerPenWidth(2);
  navigator_->setPointsPenWidth(2);
  connect(navigator_, SIGNAL(finished(int)), this, SIGNAL(finished()));
}

MapViewer::~MapViewer()
{
  delete map_;
  map_ = 0;
}

void MapViewer::setVisible(bool visible)
{
  if (navigator_ != 0) {
    navigator_->setVisible(visible);
    moveNavigatorWidget();
  }
}

void MapViewer::clearPoints()
{
  if( 0 != navigator_ ){
    navigator_->clearAllPoints();
  }
}

void MapViewer::moveNavigatorWidget()
{
  if (navigator_ != 0) {
    QPoint pos(qobject_cast<QWidget*>(parent())->rect().center() - QPoint(navigator_->width()/2, navigator_->height()/2));
    navigator_->move(pos);
  }
}

void MapViewer::slotReloadContent(const QList<u_int64_t>& messagesId)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (navigator_ == 0) {
        return;
    }

    navigator_->clearAllPoints();

    if (messagesId.isEmpty() == true) {
        return;
    }

    GeoVector stations;
    stations.reserve(messagesId.size());

    meteo::surf::TlgCoordRequest req;
    foreach (u_int64_t id, selectDecodedOnly(messagesId)) {
        req.add_id(id);

//    foreach (const TMeteoData& entry, meteo::internal::getDecodedContent(serviceDecodeCtrl_.data(), id)) {
//      GeoPoint gp;
//      if (TMeteoDescriptor::instance()->getCoord(entry, &gp) == true) {
//        debug_log << gp.la() << gp.lon() << gp.laDeg() << gp.lonDeg();
//        if (stations.contains(gp) == false) {
//          stations.append(gp);
//        }
//      }
//    }
        QCoreApplication::processEvents();
    }

    surf::CoordReply* res = serviceDecodeCtrl_->remoteCall(&surf::SurfaceService::GetTlgCoord, req, 50000);

    if (res != 0) {
        for( int i = 0; i < res->point().size(); ++i ){
          GeoPoint gp;
          gp.setLatDeg(res->point(i).la());
          gp.setLonDeg(res->point(i).lo());
          stations.append(gp);
        }
    }
    else {
        error_log << QString::fromUtf8("Ошибка. Не удалось получить ответ от сервиса");
    }
    delete res;

    debug_log << QString::fromUtf8("Раскодированные данные доступны с %1 станции(-ий)").arg(stations.size());
    navigator_->setGeoVector(stations);
    QApplication::restoreOverrideCursor();
}

const QList<u_int64_t> MapViewer::selectDecodedOnly(const QList<u_int64_t>& messagesId) const
{
  QList<u_int64_t> result;
  if (messagesId.isEmpty() == false) {
    QStringList strId;
    foreach (u_int64_t id, messagesId) {
      strId.append(QString::number(id));
    }
    TSqlQuery query(global::dbTelegram());
    if (query.exec(::querySelectDecodedOnly().arg(strId.join(","))) == true) {
      for (int i = 0, sz = query.size(); i < sz; ++i) {
        result.append(query.value(i,"id").toULongLong());
      }
    }
  }
  return result;
}

} // meteo
