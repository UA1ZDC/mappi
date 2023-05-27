#ifndef MAPPI_UI_PLUGINS_MONTAGEACTION_THEMATICLISTWIDGET_H
#define MAPPI_UI_PLUGINS_MONTAGEACTION_THEMATICLISTWIDGET_H

#include <qwidget.h>
#include <qbrush.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include "areascene.h"

namespace Ui {
class ThematicListWidget;
}

namespace mappi {

namespace proto {

class SatelliteImage;

} // proto
} // mappi

namespace meteo {
namespace map {

class SatLayer;
class SessionViewerWidget;

class ThematicListWidget : public MapWidget
{
  Q_OBJECT

  enum Column {
    kIconColumn,
    kInstrColumn,
    kProductColumn,
    kSatelliteColumn,
    kDateColumn,
    kDurationColumn,
    kLayerUuidColumn,
  };

  enum Role {
    kSessionParamRole = Qt::UserRole + 1,
    kVisibleRole,
    kTitleRole,
  };

public:
  explicit ThematicListWidget(MapWindow* window);
  virtual ~ThematicListWidget();

  void init();

private slots:
  void slotShowTableContextMenu();
  void slotTypeChanged();
  void slotHideImage();
  void slotAddLayer();
  void slotRemoveLayer();
  void slotShowMapInfo();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);
  void resizeEvent(QResizeEvent* e);
  void showEvent(QShowEvent* e);

private:
  bool addImageObject(SatLayer* layer, const QString& name);
  QString mkTitle(const ::mappi::proto::SatelliteImage& params) const;
  //  mappi::proto::SatelliteImage loadSessionParam(int sessionId, const QString& thematicName, const QString& channelName) const;

private:
  QByteArray getTLE(int sessionID) const;
  QString getFile(const QString& fileName, bool* ok = 0);
  //  bool initTypes();
  bool loadSessions();
  bool grayscaleChannels();

  QPixmap getEyeIcon(bool active);

  void resizeMap(const QSize& size);                  //!< Задать размер сцене

  QBrush defaultItemBg_;
  QBrush activeItemBg_;

  AreaScene* scene_;
  bool inited_ = false;

  Ui::ThematicListWidget *ui_;
};

} //map
} //meteo

#endif // MAPPI_UI_PLUGINS_MONTAGEACTION_THEMATICLISTWIDGET_H
