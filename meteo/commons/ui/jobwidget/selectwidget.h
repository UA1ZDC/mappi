#ifndef SELECTWIDGET_H
#define SELECTWIDGET_H

#include <QDialog>
#include <qmap.h>
#include <qmenu.h>
#include <qwidgetaction.h>
#include <qtreewidget.h>
#include <qcheckbox.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>

namespace Ui{
 class Select;
}

class SelectWidget : public QDialog
{
  Q_OBJECT
public:
  SelectWidget(QWidget *parent = 0);
  int level() const;
  int type_level() const;
  QString data_type() const;
  QString templateName();
  meteo::map::proto::ViewMode type() const;
  QString templateNameHuman() const;
  QString levelHuman() const;
  QString typeLevelHuman() const;
  QString hh() const;
  int transparency() const;
  meteo::map::proto::WeatherLayer layer();

  static QString levelHuman(int level, int type_level);
  static QString typeLevelHuman(int type_level);

  void fillForm(const meteo::map::proto::WeatherLayer& layer);
  void clearForm();

private:
  Ui::Select* ui_;
  QMap<int, QString> typelevels_;
  QList<meteo::surf::DataType> checked_;

  void createList();
  void getTypeLevels();
  QMap<QString, QString> getTemplates(meteo::map::proto::ViewMode mode);
  bool checkDataType(const meteo::map::proto::WeatherLayer& layer, meteo::surf::DataType datatype);
  QString groupByDataType(meteo::surf::DataType datatype) const;

private slots:
  void slotTypeLevelChange(const QString& typelevel);
  void slotViewModeChange();
  void slotGroupChange(QTreeWidgetItem* item);
  void slotCheckDataType(QTreeWidgetItem* item);
  void slotCheckGroup(QTreeWidgetItem* item);
};

#endif // SELECTWIDGET_H
