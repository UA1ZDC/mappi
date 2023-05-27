#ifndef METEO_COMMONS_UI_PLUGINS_AEROACTION_AEROACTION_H
#define METEO_COMMONS_UI_PLUGINS_AEROACTION_AEROACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/axis/axispuanson.h>
#include <meteo/commons/proto/usersettings.pb.h>
#include <meteo/commons/ui/map/view/actionbutton.h>

namespace meteo {
namespace map {

class MapView;

} // map
} // meteo
namespace meteo {
  class StationList;
  namespace adiag {
    class CreateAeroDiagWidget;
  }
}
namespace meteo {
namespace map {

class AeroAction : public Action
{
  Q_OBJECT

  enum PropertyType {
    kRemoveUuid,
  };

public:
  static const QString kName;

  explicit AeroAction(adiag::CreateAeroDiagWidget *ad, MapScene* scene);
  ~AeroAction();

  virtual void addActionsToMenu(Menu* am) const;
  virtual bool eventFilter(QObject* obj, QEvent* e);

private slots:
  void slotActionClicked();
  void slotShow( bool fl );
  void slotLayerWidgetClosed();
  void slotStationMode(bool fl);

private:
  bool loadSettings();
  void saveSettings();

  QList<QAction*> actions() const;
  QList<AxisSimple*> findAxis(AxisSimple::AxisType type, const QString& label) const;

  void setupUi();

private:
  QAction* pLAct_  = nullptr;
  QAction* pRAct_  = nullptr;
  QAction* pLRAct_ = nullptr;
  QAction* hLAct_  = nullptr;
  QAction* hRAct_  = nullptr;
  QAction* hLRAct_ = nullptr;
  QAction* wLAct_  = nullptr;
  QAction* wRAct_  = nullptr;
  QAction* wLRAct_ = nullptr;

  QAction* pHideAct_ = nullptr;
  QAction* hHideAct_ = nullptr;
  QAction* wHideAct_ = nullptr;

  QMenu* axisMenu_ = nullptr;
  QMenu* pMenu_ = nullptr;
  QMenu* hMenu_ = nullptr;
  QMenu* wMenu_ = nullptr;

  AeroAxisSettings settings_;
  bool loaded_ = false;

  ::meteo::adiag::CreateAeroDiagWidget* ad_;
  ActionButton* btn_;
  QGraphicsProxyWidget* btnitem_;
  meteo::StationList* stationsList_;
  bool deletevisible_;

};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_AEROACTION_AEROACTION_H
