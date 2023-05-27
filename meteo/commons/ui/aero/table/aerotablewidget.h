#ifndef METEO_COMMONS_UI_AERODIAGRAM_AEROCORE_AEROTABLEWIDGET_H
#define METEO_COMMONS_UI_AERODIAGRAM_AEROCORE_AEROTABLEWIDGET_H

#include <QtWidgets>

//#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <meteo/commons/zond/placedata.h>

namespace Ui{
  class AeroTableWidget;
  class TabAeroTableWidget;
}

//для сортировки чисел, а не текста
class TableWidgetNumberItem : public QTableWidgetItem {
  public:
  bool operator<(const QTableWidgetItem &other) const
  {
    return text().toFloat() < other.text().toFloat();
  }    
};


namespace meteo {
namespace aero {

//class Document;
class CreateAeroDiagWidget;

class TabAeroTableWidget : public QDialog
{
  Q_OBJECT
public:
  TabAeroTableWidget(QWidget *parent = 0);
  ~TabAeroTableWidget();
  
  void addAeroTable(const QList<zond::PlaceData> &pd );
  
private:
  Ui::TabAeroTableWidget* ui_;
};




class AeroTableWidget : public QDialog
{
  Q_OBJECT
public:
   AeroTableWidget(QWidget *parent = 0);
  ~AeroTableWidget();

  void fillAeroTable(const zond::PlaceData &apd);
public slots:
  void slotSaveAeroTable();

private:
  void readConfig();
  void saveConfig(const QString& dir);

private:
  Ui::AeroTableWidget* ui_;
  meteo::map::exportmap::ExportPath conf_;
  QString dname_;
};

}
}

#endif
