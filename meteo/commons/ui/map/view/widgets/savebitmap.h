#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVEBITMAP_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVEBITMAP_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/map/view/widgets/layersselect.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>


namespace Ui {
  class SaveBitmap;
}

namespace meteo {
namespace map {

  class Document;

  class SaveBitmap : public MapWidget, public SelectActionIface
  {
    Q_OBJECT

    public:
    explicit SaveBitmap(MapWindow* parent);
    ~SaveBitmap();

    void setOptions(const QString& options);

    bool isRubberPressed() const;
    void rubberFinished();
    void resetRubber(MapScene* scene); 
    void removeRubber();
    void fillCoordsFromDocument(map::Document* doc = 0);

  protected:
    void keyPressEvent( QKeyEvent * e);
    void setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect);		

  private slots:
    void slotSave();
    void slotCancel();
    void slotRubberClicked(bool on);

    void slotSceneChanged(MapScene* scene);
    void bandChanged();

    void chooseFile();

  private:
    void clear();
    Document* currentDocument() const;
    QString mapname() const;
    void setDefaultName();

    bool eventFilter(QObject* watched, QEvent* event);
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

    void resetWatchedDocument();
    bool installWatchedDocument();

    void saveDefaultDir(const QString& dir) const;
    QString readDefaultDir();

    bool save();

  private:
    Ui::SaveBitmap* ui_;
    LayersSelect* layers_;
    QRect mapBand_; //!< рамка на карте

    QObject* watchedEventHandler_;

    QString defaultDir_;
    bool savefs_;
    bool savedb_;
    
  };
  
}

}


#endif
