#ifndef METEO_PRODUCT_UI_PLUGINS_VKO_VKOSTATIONS_H
#define METEO_PRODUCT_UI_PLUGINS_VKO_VKOSTATIONS_H

#include <QDialog>

#include <meteo/commons/proto/sprinf.pb.h>

class QTreeWidgetItem;

namespace meteo {
  namespace climat {
    class ClimatSaveProp;
  }  
}

namespace Ui {
  class VkoStations;
}

namespace meteo {
  class VkoStations : public QDialog
  {
    Q_OBJECT
  public:
    explicit VkoStations(QWidget *parent = nullptr);
    ~VkoStations();

    void fillStations(const meteo::climat::ClimatSaveProp&, const sprinf::StationTypes& types);
    void setCurrent(const QString& station, int type, int cltype);
    
    QString curIndex();
    QString curRuName();
    QString curEnName();
    QDate curDateStart();
    QDate curDateEnd();
    int curUTC();
    QString curCoord();
    meteo::sprinf::MeteostationType climType();
    int curStationType();

    void removeSelection();
    void resetCurrent();
		
  private slots:
    void selectStation();
  
  private:
    Ui::VkoStations* ui_ = nullptr;
    bool loaded_ = false;
    QTreeWidgetItem* cur_ = nullptr;
  };
    
}

#endif
