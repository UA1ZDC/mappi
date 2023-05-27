#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_WGTPREVIEW_VIEWIMAGEPREVIEW_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_WGTPREVIEW_VIEWIMAGEPREVIEW_H

#include <qgraphicsview.h>

namespace meteo {

class ViewImagePreview : public QGraphicsView
{
  Q_OBJECT
public:
  explicit ViewImagePreview( QWidget* wgt = nullptr);
  virtual ~ViewImagePreview() override;

  void clear();
  bool hasImages();
  bool setImage( const QString& path );
  bool setImage( const QImage& image );
  bool setImages( const QList<QImage>& imageList );

  QPointF mapToImage( const QPointF &scenePoint );
  QPointF mapFromImage( const QPointF &imagePoint );
  QRectF imageRect( );
  QRectF getVisibleRect();

  void rotate( int angle );

  void scaleInc();
  void scaleDec();
  void scaleReset();
  void scaleOptimal();

protected:
  virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
  virtual void wheelEvent(QWheelEvent *event) override;
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
private slots:
  void slotDestroyed();  

private:

  static QTransform createTransform(double awidth, double aheight, double ascale, double offsetX, double offsetY, int angle);
  void forceRepaint();
  void scrollToCenter();
  QSize imagesTotalSize();
  void resizeSceneToContent();
  void setScale( double newScale );
  double getScale();


signals:
  void signalDocumentResized();

private:
  QPointF mousePos_;
  QGraphicsScene *scene_;
  QList<QImage> backgroundImages_;
  double scale_;
  int angle_;

  double currentOffsetX = 0, currentOffsetY = 0;
  QRectF imageRect_;
};

}
#endif
