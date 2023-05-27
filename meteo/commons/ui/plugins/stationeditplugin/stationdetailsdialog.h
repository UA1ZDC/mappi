#ifndef STATIONDETAILSWIDGET_H
#define STATIONDETAILSWIDGET_H

#include <qwidget.h>
#include <qmap.h>
#include <qlineedit.h>
#include <QCloseEvent>

#include <meteo/commons/proto/sprinf.pb.h>

namespace Ui {
class StationDetailsWidget;
}

namespace meteo {
namespace map {

class StationDetailsDialog : public QWidget
{
  Q_OBJECT

public:
  explicit StationDetailsDialog(QWidget *parent = nullptr);
  ~StationDetailsDialog();
  void setData(meteo::sprinf::Station st, bool force = false);
  void setTypesMap(QMap <QString, int> typesMap);
  void setCountriesMap(QMap<int, QString> countriesMap);
  meteo::sprinf::Station getEditedStation();
  bool hasChanges();

private:
  Ui::StationDetailsWidget *ui_;
  QMap <QString, int> stationTypesMap_;
  QMap<int, QString> countriesMap_;

  QList<QLineEdit*> allSwitchedWidgets_;
  QMap<int, QList<QLineEdit*>> enabledWidgetsByType_;

  meteo::sprinf::Station station_;

  void initWidgetBlockings();
  double getAirstripDirection();
  double getDangerLevel();
  double getFloodLevel();
  double getPoimaLevel();

private:
  void blockChildrenSignals(bool block);

public slots:
  void saveChanges();

private slots:
  void switchWidgetsAvailability();
  void checkChanges();
  void checkAirstripLineEdit();
  void checkHydroLineEdit();
  void checkCcccLineEdit();
};

} // namespace meteo {
} // namespace map {

#endif // STATIONDETAILSWIDGET_H
