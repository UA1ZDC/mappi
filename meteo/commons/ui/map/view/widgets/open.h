#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_OPEN_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_OPEN_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui {
  class OpenDocument;
}

namespace meteo {
namespace map {

class Document;

class OpenDocument : public MapWidget
{
  Q_OBJECT
  public:
    OpenDocument( MapWindow* w );
    ~OpenDocument();
    
    public slots:
      void cancel();
      void chooseFile();
      bool open();
  
  protected:
      void keyPressEvent( QKeyEvent * e);
      void closeEvent(QCloseEvent *);
      void readLayers();

  private:
      QString defaultDir_;
      Ui::OpenDocument* ui_;    

      void clear();
      Document* currentDocument() const;
      QString readDefaultDir();
      bool open(const QString& fileName);
};

}
}

#endif
