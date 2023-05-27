#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_WGTDOCVIEWMODEL_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_WGTDOCVIEWMODEL_H

#include <qimage.h>
#include <qobject.h>
#include <qmutex.h>
#include <qtemporaryfile.h>
#include <qhash.h>
#include <qdatetime.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <meteo/commons/ui/documentviewer/datamodel/dataservice.h>
#include <meteo/commons/proto/documentviewer.pb.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/documentviewer/faxaction.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

namespace meteo {
namespace documentviewer {

class WgtDocViewPanel;

class DataModel : public QObject
{
  Q_OBJECT
public:

  enum TransparencyMode {
    kTransparencyNone,
    kTransparencyBlack,
    kTransparencyWhite
  };

  DataModel() = delete;
  DataModel( DocumentHeader::DocumentClass type );
  virtual ~DataModel();
  bool preloadDocuments(const QStringList documents);
  bool premakeTransparentImages(const QStringList documents);
  void setDocumentClass( DocumentHeader::DocumentClass docClass );
  const QImage& getImage( DocumentHeader::DocumentClass documentClass, const QString& image );
  QImage getTransparentImage( DocumentHeader::DocumentClass docClass, const QString& image );
  const QImage& getImage( DocumentHeader* document );
  DocumentHeader::DocumentClass getDocumentClass( ) { return  this->documentClass_; }

  void setLeftPanel(WgtDocViewPanel* lp) { leftPanel_ = lp;}

  void setTransparencySettings( TransparencyMode mode, int value );

  void setDateRange( const QDateTime& beginDt, const QDateTime& endDt );
  const QDateTime& getBeginDt() { return  dtBegin_; }
  const QDateTime& getEndDt() { return dtEnd_; }

  void refresh( );
  void loadDocument( const DocumentUID uid );
  void loadDocument( const QString& file );  
  void printCurrentDocument( DocumentHeader::DocumentClass docClass, const QString& fileName );
  void openDocument( DocumentHeader* doc );

  void openDocument( DocumentHeader::DocumentClass docClass, const QString& docname );
  void cleanFilters();

  void filterFaxTT( Condition  tt );
  void filterFaxAA( Condition  aa);
  void filterFaxCCCC( Condition  cccc);
  void filterFaxII( Condition i );

  const Condition getFilterFaxTT( ) const;
  const Condition getFilterFaxAA( ) const;
  const Condition getFilterFaxCCCC( ) const;
  const Condition getFilterFaxII( ) const;

  const Condition getFilterMapImageName() const;
  const Condition getFilterMapImageHour() const;
  const Condition getFilterMapImageCenter() const;
  const Condition getFilterMapImageModel() const;
  const Condition getFilterMapImageJobName() const;
  void filterMapImageName( Condition name );
  void filterMapImageHour( Condition  hour );
  void filterMapImageCenter( Condition  center );
  void filterMapImageModel( Condition model );
  void filterMapImageJobName( Condition jobname );

  const Condition getFilterFormalDocumentName() const;
  const Condition getFilterFormalDocumentHour() const;
  const Condition getFilterFormalDocumentCenter() const;
  const Condition getFilterFormalDocumentModel() const;
  const Condition getFilterFormalDocumentJobName() const;
  void filterFormalDocumentName( Condition name );
  void filterFormalDocumentHour( Condition hour );
  void filterFormalDocumentCenter( Condition  center );
  void filterFormalDocumentModel( Condition  model );
  void filterFormalDocumentJobName( Condition  jobname );


  void filterSateliteTT( Condition tt);
  void filterSateliteAA( Condition aa);
  void filterSateliteCCCC( Condition cccc);
  void filterSateliteII( Condition ii);
  const Condition getFilterSateliteTT() const ;
  const Condition getFilterSateliteAA() const ;
  const Condition getFilterSateliteCCCC() const ;
  const Condition getFilterSateliteII( ) const ;



  void savePinning( const QString& name,
                    meteo::GeoPoint geoPoints[meteo::map::Faxaction::markersCount],
                    QPoint imagePoints[meteo::map::Faxaction::markersCount] );


public slots:  
  void slotSaveAnimation( const QStringList documents, const QString& outputFileName, int delay );

private:
  void filterDocuments();
  void openLocalDocument();
  static QImage makeTransparentFax( const QImage& original, const QColor &color );
  static QImage makeTransparentSateliteImage(const QImage& original, TransparencyMode transp, int value );


signals:  
  void signalListLoaded( const QList<DocumentHeader*> *header);
  void signalFileLoaded( const QList<QImage>& file );

private slots:
  void slotListLoaded( const QList<DocumentHeader*> *header);
  void slotFileChanged( const DocumentUID& uid );
  void slotFileLoaded( const QList<QImage>& file );  



private:
  QList<DocumentHeader*> completeList_;
  QList<DocumentHeader*> filteredList_;

  DataService data_;

  DocumentHeader::DocumentClass documentClass_;
  QDateTime dtBegin_;
  QDateTime dtEnd_;  

  Condition filterFaxTT_;
  Condition filterFaxAA_;
  Condition filterFaxCCCC_ ;
  Condition filterFaxII_ ;

  Condition filterSateliteTT_ ;
  Condition filterSateliteAA_ ;
  Condition filterSateliteCCCC_;
  Condition filterSateliteII_ ;

  Condition filterMapImageName_;
  Condition filterMapImageHour_ ;
  Condition filterMapImageCenter_;
  Condition filterMapImageModel_;
  Condition filterMapImageJobName_;

  Condition filterFormalDocumentName_;
  Condition filterFormalDocumentHour_;
  Condition filterFormalDocumentCenter_;
  Condition filterFormalDocumentModel_;
  Condition filterFormalDocumentJobName_;

  DocumentUID lastLoadedDocument_;

  TransparencyMode currentTransparencyMode_ = TransparencyMode::kTransparencyNone;
  int currentTransparencyValue_ = 0;
  QHash<DocumentUID, QImage> transparentFaxImages_;
  QHash<DocumentUID, QImage> transparentSateliteImages_;
  QHash<DocumentUID, QImage> transparentMapImages_;

  WgtDocViewPanel *leftPanel_;
};


}
}
#endif
