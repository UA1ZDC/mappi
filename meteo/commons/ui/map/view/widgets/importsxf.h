#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_IMPORTSXF_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_IMPORTSXF_H

#include <qtreewidget.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/proto/exportmap.pb.h>

namespace Ui {
  class ImportSxf;
}



namespace meteo {
  class SxfToMap;
  
  namespace map {
    class Document;
    class MapScene;

  
    //! Импорт картографической основы из sxf
    class ImportSxf : public MapWidget {
      Q_OBJECT
      
      public:
      ImportSxf( MapWindow* parent );
      ~ImportSxf();
      
      void setOptions(const QString& options);
      
    protected:
      void keyPressEvent( QKeyEvent * e);
      void closeEvent(QCloseEvent *);
				    
    private slots:
      void accept();
      void cancel();
      void chooseSxf();
      void chooseRsc();
      void checkLayer(QTreeWidgetItem* item, int column);
      void importedObjects(int loaded, int cnt);
      void expandColumns(QTreeWidgetItem* /*item*/);
      
    private:
      Document* currentDocument() const;
      void readConf();
      void saveConf() const;
      bool readFile(const QString& fileName, QByteArray *ba);
      bool readSxfDescription();
      void fillLayers();      
      void clear();
      
    private:
      Ui::ImportSxf* ui_;
      meteo::map::exportmap::ExportPath conf_;
      meteo::SxfToMap* sxf_;
    };
    
  }
}


#endif
