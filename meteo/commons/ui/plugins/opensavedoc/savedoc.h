#ifndef METEO_COMMONS_UI_PLUGINS_OPENSAVEDOC_SAVEDOC_H
#define METEO_COMMONS_UI_PLUGINS_OPENSAVEDOC_SAVEDOC_H

#include <qmap.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

class QTreeWidgetItem;

namespace Ui{
 class SaveDoc;
}

namespace meteo {
namespace map {

class Layer;

class SaveDoc : public MapWidget
{
  Q_OBJECT
  public:
    SaveDoc(MapWindow* window);
    ~SaveDoc();

    void updateLayers();

  protected:
    bool eventFilter( QObject* watched, QEvent* event );
    void showEvent( QShowEvent* e );

  private:
    Ui::SaveDoc* ui_;
    QMap< Layer*, bool > checkedlayers_;
    QString ext_;

    void muteSignals();
    void turnSignals();

    QString lastPath() const ;
    void saveLastPath() const ;
    QString mapname() const ;

    void setDefaultName();

    void saveFile( const QByteArray& arr );

  private slots:
    void slotItemChanged( QTreeWidgetItem* item, int clmn );
    void slotSelectFile();
    void slotPathChanged();

    void slotOkBtn();
    void slotNoBtn();
};

}
}

#endif
