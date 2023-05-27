#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVE_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVE_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/map/view/widgets/layersselect.h>

namespace Ui {
  class SaveDocument;
}

namespace meteo {
namespace map {

class Document;

//! Сохранение документа (на данный момент только слои с объектами)
class SaveDocument  : public MapWidget {
  Q_OBJECT
  public:
    SaveDocument( MapWindow* parent );
    ~SaveDocument();

  public slots:
    void save();
    void cancel();

  protected:
    void keyPressEvent( QKeyEvent * e);

  private:
    QString defaultDir_;
    Ui::SaveDocument* ui_;    
    LayersSelect* layers_;

    QObject* watchedEventHandler_;

    void clear();
    Document* currentDocument() const;
    
    bool eventFilter(QObject* watched, QEvent* event);
    void closeEvent(QCloseEvent *);

    void resetWatchedDocument();
    bool installWatchedDocument();

    void saveDefaultDir(const QString& dir) const;
    QString readDefaultDir();
    QString chooseFile();
    
    bool save(const QString& fileName);
};

}
}

#endif
