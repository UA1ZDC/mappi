#ifndef METEO_NOVOST_UI_MSGVIEWER_MAP_MAPVIEWER_H
#define METEO_NOVOST_UI_MSGVIEWER_MAP_MAPVIEWER_H

#include <QObject>
#include <QScopedPointer>
#include <meteo/commons/proto/surface_service.pb.h>

template<typename T> class QList;
class QString;

namespace rpc {
  class SelfCheckedController;
} // rpc

namespace meteo {
namespace map {
  class Map;
  class NavigatorWidget;
} // map

class MapViewer : public QObject
{
  Q_OBJECT

public:
  explicit MapViewer(QWidget* parent);
  ~MapViewer();

  void setVisible(bool visible);
  void clearPoints();

signals:
  void finished();

public slots:
  void slotReloadContent(const QList<u_int64_t>& messagesId);

private:
  const QList<u_int64_t> selectDecodedOnly(const QList<u_int64_t>& messagesId) const;
  void moveNavigatorWidget();

private:
  map::Map* map_;
  map::NavigatorWidget* navigator_;
  QScopedPointer<rpc::SelfCheckedController> serviceDecodeCtrl_;

};

} // meteo

#endif // METEO_NOVOST_UI_MSGVIEWER_MAP_MAPVIEWER_H
