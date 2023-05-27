#ifndef METEO_PRODUCT_UI_OCEANDIAGRAM_OCEANTABLE_OCEANTABLEWIDGET_H
#define METEO_PRODUCT_UI_OCEANDIAGRAM_OCEANTABLE_OCEANTABLEWIDGET_H

#include <QtWidgets>

//#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <meteo/commons/zond/placedata.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

namespace Ui{
  class OceanTableWidget;
  class TabOceanTableWidget;
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
namespace odiag {

//class Document;
class CreateOceanDiagWidget;

class TabOceanTableWidget : public QDialog
{
  Q_OBJECT
public:
  TabOceanTableWidget(QWidget *parent = 0);
  ~TabOceanTableWidget();
  
  //void addOceanTable(const QList<zond::PlaceData> &pd );
  
private:
  Ui::TabOceanTableWidget* ui_;
};




class OceanTableWidget : public QDialog
{
  Q_OBJECT
public:
   OceanTableWidget(QWidget *parent = 0);
  ~OceanTableWidget();

  void fillOceanTable(const QString name, const QList<TMeteoData> &apd);
public slots:
  void slotSaveOceanTable();

private:
  void readConfig();
  void saveConfig(const QString& dir);

private:
  Ui::OceanTableWidget* ui_;
  meteo::map::exportmap::ExportPath conf_;
  QString dname_;
};

}
}

#endif
