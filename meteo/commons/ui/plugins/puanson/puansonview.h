#ifndef METEO_UI_PLUGINS_PUANSON_PUANSONVIEW_H
#define METEO_UI_PLUGINS_PUANSON_PUANSONVIEW_H

#include <qdialog.h>
#include <qtreewidget.h>

#include <meteo/commons/ui/map/puanson.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui {
  class PuansonView;
}

namespace meteo {
namespace map {

class PuansonView : public MapWidget
{
  Q_OBJECT
  public:
    PuansonView( MapWindow* window );
    ~PuansonView();

    void setPuanson( Puanson* p );

  protected:
    bool eventFilter( QObject* o, QEvent* e );

  private slots:
    void slotItemDoubleClicked( QTreeWidgetItem* item, int clmn );
    void slotItemClicked( QTreeWidgetItem* item, int clmn );

    void slotOkClicked();
    void slotCloseClicked();

    void reject();

    void showMeteogram();
    void tabchangedslot(int);

  private:
    Ui::PuansonView* ui_;
    Puanson* puanson_;
    TMeteoData meteodata_;
    puanson::proto::Puanson punch_;
    bool has_new_;
    bool changed_;

    void setDescription();

    QString getTitle() const ;

    QString coordStr() const ;
    QString strLat() const ;
    QString strLon() const ;
    QString heightStr() const ;
    QString stationStr() const ;
    QString termStr() const ;
    QString hourStr() const ;

    void loadTelegrams();

    void setVisible( const QString& name, bool visible );
    void setVisible( QTreeWidgetItem* item, bool visible );
    bool visible( const QString& name ) const ;

    void setupPreview();
    void showEvent(QShowEvent *event);
    void closeEvent( QCloseEvent* event );
};

}
}

#endif
