#ifndef MapWidget_H
#define MapWidget_H

#include <qdialog.h>

namespace Ui{
 class MapWidget;
}

namespace meteo {
namespace map {

class MapWindow;
class MapView;
class MapScene;
class Document;

class MapWidget : public QDialog
{
  Q_OBJECT
  public:
    MapWidget( MapWindow* window );
    virtual ~MapWidget();

    void setTitle(const QString& title);
    void setPixmap(QPixmap pix);
    void setMapWindow(MapWindow* w);

    MapWindow* window() const { return window_; }
    MapView* mapview() const ;
    MapScene* mapscene() const ;
    Document* mapdocument() const ;


  protected:
    MapWindow* window_;

    void resizeEvent( QResizeEvent* event );
    void moveEvent( QMoveEvent* event );
    void closeEvent( QCloseEvent* event );
    void showEvent( QShowEvent* event );
    void hideEvent( QHideEvent* event );

  protected slots:
    void reject();

signals:
    void closed();
};

}
}

#endif // MapWidget_H
