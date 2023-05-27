#ifndef METEO_COMMONS_UI_MAP_VIEW_CUSTOM_FAXVIEWER_H
#define METEO_COMMONS_UI_MAP_VIEW_CUSTOM_FAXVIEWER_H

#include <cstdint>

#include <qscopedpointer.h>
#include <qstring.h>
#include <qwidget.h>
#include <qprinter.h>

#include "faxscene.h"

class QGraphicsView;
class QGraphicsPixmapItem;
class QImage;
class QString;
class PrintOptionsTab;

namespace Ui {
  class FaxViewer;
}

namespace meteo {

namespace rpc {
  class Channel;
}
namespace faxes {
  class FaxResponse_Image;
} // faxes
}

class FaxViewer : public QWidget
{
  Q_OBJECT

public:
  explicit FaxViewer(QWidget* parent = 0);
  ~FaxViewer();

  void init(const QString& faxid);
  void setPixmap(const QPixmap& image);
  FaxScene* scene();
  QPixmap originalPixmap() const { return original_; }

  QGraphicsView* graphicsView() const;
  QGraphicsPixmapItem* item() const;

protected:
  bool eventFilter(QObject* obj, QEvent* ev);

private slots:
  void slotZoomIn();
  void slotZoomOut();
  void slotZoomToOriginal();
  void slotZoomToWindow();
  void slotRotateAnticlockwise();
  void slotRotateClockwise();
  void chooseFile();
  void print();

private:
  QByteArray getFaxImageFromService(const QString& faxid);
  QPixmap addImageTransparence(const QPixmap& pixmap) const;
  void setEnabledActions(bool enabled);
  void clearScene();
  void rotateImage(qreal angle);
  void scaleImage(qreal deltaFactor);
  QPoint adjustPosition(PrintOptionsTab* optionsPage, const QSize& imageSize, const QSize & viewportSize);
  QSize adjustSize(PrintOptionsTab* optionsPage, int printerResolution, const QSize & viewportSize);

private:
  Ui::FaxViewer* ui_ = nullptr;
  QGraphicsPixmapItem* item_ = nullptr;
  QPixmap original_;
  QString lastError_;

  qreal scaleFactor_;
  int rotateAngle_;

  FaxScene* scene_ = nullptr;
};

#endif // METEO_NOVOST_UI _MSGVIEWER_FAX_FAXVIEWER_H
