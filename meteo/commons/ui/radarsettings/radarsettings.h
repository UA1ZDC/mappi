#ifndef METEO_COMMONS_UI_RADARCORE_RADARSETTINGS_H
#define METEO_COMMONS_UI_RADARCORE_RADARSETTINGS_H

#include <qdialog.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/global/radarparams.h>

class QTreeWidget;
class QTreeWidgetItem;

namespace Ui{
 class RadarSettings;
 class AddRadarDescr;
}

namespace meteo {
namespace map {

class RadarSettings : public QWidget
{
  Q_OBJECT
  public:
    RadarSettings( app::MainWindow* parent );
    ~RadarSettings();

    QTreeWidget* radartypes() const ;

  protected:
    void closeEvent( QCloseEvent* e );
    
  private:
    Ui::RadarSettings* ui_;
    QMap< QString, puanson::proto::Puanson > punches_;
    RadarParams params_;

  private slots:
    void slotAccept();
    void slotReject();

    void slotAddRadarDescr();
    void slotRmRadarDescr();

    void slotItemSelectionChanged();
    void slotItemClicked( QTreeWidgetItem* item, int clmn );
    void slotItemDoubleClicked( QTreeWidgetItem* item, int clmn );
    void slotItemChanged( QTreeWidgetItem* item, int clmn );

    void slotPunchCurrentIndexChanged( int indx );

    void slotBtnMinColorClicked();
    void slotBtnMaxColorClicked();
    void slotBtnGradientClicked();

    void slotMinChanged( int val );
    void slotMaxChanged( int val );
    void slotStepChanged( int val );
    void slotEmptyChanged( int val );

  private:
    void muteSignals();
    void turnSignals();

    void loadPunches();

    void showSymbol();
    void showGradient();

    void loadSettings();
    void saveSettings();

    proto::RadarColors currentParams();

    QTreeWidgetItem* currentItem();

    proto::RadarColor fromItem( QTreeWidgetItem* item);
    void setItemData( const proto::RadarColor& clr, QTreeWidgetItem* item );
    void setupItem( QTreeWidgetItem* item, const proto::RadarColor& clr );
    void setupProtoFromGradientControls( QTreeWidgetItem* item = 0 );
    void setupGradientFromProto();
    void setupLegendaFromProto();

    QColor minColor();
    QColor maxColor();

    void setColorMin( const QColor& clr );
    void setColorMax( const QColor& clr );
};

class AddRadarDescr: public QDialog
{
  Q_OBJECT
  public:
    AddRadarDescr( RadarSettings* parent );
    ~AddRadarDescr();

    int descr() const ;
    QString name() const ;
    QString description() const ;

  protected:
    void keyReleaseEvent( QKeyEvent* ev );

  private:
    Ui::AddRadarDescr* ui_;
    RadarSettings* params_;

    void loadLastDescriptors();
    void saveLastdescriptors();

    void loadDescriptors( const QStringList& descrlist );

    QTreeWidgetItem* currentItem() const ;
                        
  public slots:
    void accept();
    void slotItemDoubleClicked( QTreeWidgetItem* item, int column );
};

}
}

#endif
