#ifndef METEO_COMMONS_UI_PLUGINS_OPENSAVEDOC_OPENDOC_H
#define METEO_COMMONS_UI_PLUGINS_OPENSAVEDOC_OPENDOC_H

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui{
 class OpenDoc;
}

namespace meteo {
namespace map {

class OpenDoc : public MapWidget
{
  Q_OBJECT
  public:
    OpenDoc( MapWindow* window );
    ~OpenDoc();

  private:
    Ui::OpenDoc* ui_;
    QString ext_;

    void muteSignals();
    void turnSignals();

    QString lastPath() const ;
    void saveLastPath() const ;

    void setDefaultName();

    void openFile();

  private slots:
    void slotAccept();
    void slotReject();

    void slotSelectFile();
    void slotPathChanged();

    void openMap( const QByteArray& arr );
};

}
}

#endif
