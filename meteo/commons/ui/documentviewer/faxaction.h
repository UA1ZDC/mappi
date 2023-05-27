#ifndef FAXACTION_H
#define FAXACTION_H

#include <qwidget.h>

#include <meteo/commons/ui/map/view/actions/action.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/documentviewer/custom/imagedisplaygraphicsitem.h>
#include <meteo/commons/ui/documentviewer/custom/animationgraphicsitem.h>
#include <meteo/commons/ui/documentviewer/custom/animationcontroller.h>

#include <meteo/commons/ui/previewwidget/viewimagepreview.h>


namespace meteo {
namespace map {
  class MarkerFlagItem;
}
}
class WgtDocView;


namespace meteo {
namespace map {

class Faxaction : public Action
{
  Q_OBJECT
public:
  static const QString faxActionName;
  static const int markersCount = 3;

  Faxaction(MapScene* scene);
  static QDialog* toDialog(QWidget* wgt, QWidget* parent);
  void pitToView( ::meteo::ViewImagePreview *source, const QString& currentImageName, const QString& currentImageDisplayName );
  virtual ~Faxaction() override;

  void addAnimationLayer( const AnimationController &controller );
  void removeAnimationLayer();


  virtual void mouseReleaseEvent( QMouseEvent* e ) override;
  virtual void wheelEvent(QWheelEvent* e) override;
  virtual void mouseMoveEvent( QMouseEvent* event ) override;
  int getLayersCount();
  void setLayerHidden( const QString& layerName, bool hidden );


  void sourceImageRectSave();
  void sourceImageRectRestore();
  void targetImageRectSave();
  void targetImageRectRestore();
  void fixImagePosition( bool recreateLayer = true );

  void setCurrentImageName( const QString& name, const QString& displayName );
  void setImage( const QImage& image );
  void removeLayer( const QString& layerName );
  bool hasLayer(const QString& layerName);

  void unpinAll();

  bool eventFilter(QObject *watched, QEvent *event) override;

  meteo::GeoVector getTargetRect();
  QPolygon getSourceRect();

  void setTargetRect( const meteo::GeoPoint points[markersCount] );
  void setSourceRect( const QPoint points[markersCount] );
signals:
  void signalImageLayerRemoved( const QString& imageid );
  void signalImageLayerAdded( const QString& imageid );

private slots:
  void slotSourceDestroyed();
  void slotSceneDestroyed();
  void slotOpenFaxWidget();  
  void slotOpenSateliteWidget();
  void slotOpenMapImageWidget();  
  void slotDestroyed( QObject* obj );

private:
  void destroyMarkers();
  void setMarkersPinned(bool isPinned);
  void reEmitAddedItems();




  meteo::map::Layer* layerByImageName(const QString& imageid);

  meteo::map::MarkerFlagItem* markersTarget_[markersCount];

  meteo::map::MarkerFlagItem* markersSource_[markersCount];

  QHash<QString, QImage> images_;
  QPolygon imageToSource_;
  meteo::GeoVector imageToTarget_;
  QHash<QString, QString> imageLayers_;
  QHash<QString, QString> displayNames_;

  ::meteo::ViewImagePreview *source_;
  QString currentImageName_;  

  AnimationGraphicsItem *animationGraphicsItem_;
};

}
}

#endif
