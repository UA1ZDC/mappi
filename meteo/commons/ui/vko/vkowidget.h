#ifndef METEO_PRODUCT_UI_PLUGINS_VKO_VKOWIDGET_H
#define METEO_PRODUCT_UI_PLUGINS_VKO_VKOWIDGET_H

#include <QDialog>
#include <QDate>
#include <QMap>
#include <QTemporaryFile>
#include <QFileSystemWatcher>
#include <QProcess>
#include <qtreeview.h>

#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/global/formaldocsaver.h>

class QCompleter;

namespace meteo {
  namespace rpc {
    class Channel;
  }
  namespace map {
    namespace proto {
      class ExportResponse;
    }
  }
  
  class UserSelectForm;
}

namespace Ui {
  class VkoWidget;
}

namespace meteo {
  class VkoStations;
  
  class VkoWidget : public QDialog
  {
    Q_OBJECT
   
  public:
    explicit VkoWidget(QWidget *parent = nullptr);
    ~VkoWidget();
		
  private slots:
    void slotOnCreateButton();
    void chooseVko(int row);
    void openTemp(meteo::map::proto::ExportResponse* r);
    
    void slotOfficeClosed();
    void chooseStation();
    
    void choosePopup(const QModelIndex& index);
    void setCompleter();

    void slotShowSelectUsers();
  private:
    void changeClimType(meteo::sprinf::MeteostationType climType);
    void createDocument(meteo::map::proto::Map, bool setmonth = false);
    bool fillStations();
    QString stationText(const QString& index, const QString& ru, const QString en);
    void enableGui(bool enable);    
  private:
    Ui::VkoWidget *ui = nullptr;
    VkoStations *stationsDlg_ = nullptr;
    meteo::UserSelectForm* usersWidget_ = nullptr;
    
    float utc_ = 0;
    meteo::climat::ClimatSaveProp* prop_ = nullptr;
    
    QMap< QString, meteo::map::proto::Map > formals_;
    QMap <QProcess*, QTemporaryFile* > offices_;

    QCompleter *completer_ = nullptr;

    meteo::FormalDocSaver formaldoc_;
  };
  
  
  class VkoStationPopup : public QTreeView
  {
    Q_OBJECT
  public:
    VkoStationPopup();

  public slots:
    void resizeWithText(const QString& text);
   

  private:
    int columns_ = 4;
  };  

}

#endif // VKOWIDGET_H
  
