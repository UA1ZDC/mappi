#ifndef METEO_COMMONS_UI_PLUGINS_ADVECT_PREPEARTRANSFERWIDGET_H
#define METEO_COMMONS_UI_PLUGINS_ADVECT_PREPEARTRANSFERWIDGET_H

#include <QtWidgets>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

class StationWidget;
namespace Ui{
 class PrepearTransferWidget;
}

namespace meteo {
namespace map {

class Document;

class PrepearTransferWidget : public QDialog
{
  Q_OBJECT
public:
  enum Mode {
    kPoint,
    kObjects
  };
   PrepearTransferWidget(Mode mode = Mode::kPoint, QWidget* parent = nullptr);
  ~PrepearTransferWidget();

  void setStartGeoPoint( const GeoPoint& agp );
  GeoPoint getStartGeoPoint();
  void setSkeletMap( const QMap<QString,GeoVector>&  skeletMap) { skeletMap_ = skeletMap; }
  int trajectory();
  bool markersIsVisible();

  meteo::field::AdvectObjectReply* advectObjectReply() { return advObj_reply_; }

private:
  QDateTime dateTime();
  bool createServices();
  double koef();
  float interval();
  float time();
  int field();
  int center();
  bool calcObjectsAdvect();
  void loadSettings();
  void saveSettings();

public slots:
  //void slotFillFieldBox(int);
  void slotDateTimeChanged();
  void slotApplyClose();
  void slotClose();
  void slotLevelChanged(int indx);

private slots:
  void slotLabelBtnIconChange();
  void slotFillFieldTree();

signals:
  void applyTraj();
  void applyObjectsTraj();

private:
  Ui::PrepearTransferWidget* ui_ = nullptr;
  meteo::rpc::Channel* ctrl_field_ = nullptr;
  meteo::field::AdvectObjectReply* advObj_reply_ = nullptr;
  GeoPoint startPoint_;
  Mode mode_;
  QMap<QString,GeoVector> skeletMap_;
  QSettings* opt_;

};

}
}

#endif //METEO_COMMONS_UI_PLUGINS_ADVECT_PREPEARTRANSFERWIDGET_H
