#include "faxviewer.h"
#include "ui_faxviewer.h"
#include "printoptionstab.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>

#include <qdir.h>
#include <qapplication.h>
#include <qbytearray.h>
#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qimage.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qprintdialog.h>
#include <qpushbutton.h>
#include <qtabwidget.h>

#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/proto/document_service.pb.h>

FaxViewer::FaxViewer(QWidget* parent) :
  QWidget(parent),
  ui_(new Ui::FaxViewer()),
  scaleFactor_(1.0),
  rotateAngle_(0),
  scene_(new FaxScene(this))
{
  ui_->setupUi(this);

  ui_->imageGraphicsView->setScene(scene_);
  ui_->imageGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

  ui_->saveButton->setIcon(QIcon(":/meteo/icons/save.png"));
  ui_->printButton->setIcon(QIcon(":/meteo/icons/print.png"));

  connect(ui_->plusButton, SIGNAL(clicked()), SLOT(slotZoomIn()));
  connect(ui_->minusButton, SIGNAL(clicked()), SLOT(slotZoomOut()));
  connect(ui_->originalSizeButton, SIGNAL(clicked()), SLOT(slotZoomToOriginal()));
  connect(ui_->windowSizeButton, SIGNAL(clicked()), SLOT(slotZoomToWindow()));
  connect(ui_->anticlockwiseButton, SIGNAL(clicked()), SLOT(slotRotateAnticlockwise()));
  connect(ui_->clockwiseButton, SIGNAL(clicked()), SLOT(slotRotateClockwise()));
  connect(ui_->saveButton, SIGNAL(clicked()), SLOT(chooseFile()));
  connect(ui_->printButton, SIGNAL(clicked()), SLOT(print()));

  ui_->imageGraphicsView->installEventFilter(this);

}

FaxViewer::~FaxViewer()
{
  delete ui_;
  ui_ = nullptr;
}

bool FaxViewer::eventFilter(QObject* obj, QEvent* ev)
{
  if (obj == ui_->imageGraphicsView->viewport()) {
    switch (ev->type()) {
      case QEvent::Wheel:
        if ((static_cast<QWheelEvent*>(ev))->delta() > 0) {
          slotZoomIn();
        }
        else {
          slotZoomOut();
        }
        return true;
      default:
        break;
    }
  }
  else if (obj == ui_->imageGraphicsView) {
    switch (ev->type()) {
      case QEvent::Resize:
        if (original_.isNull() == true &&
            lastError_.isEmpty() == false) {
          clearScene();
          QGraphicsScene* scene = ui_->imageGraphicsView->scene();
          QPixmap background(static_cast<QResizeEvent*>(ev)->size());
          background.fill(QColor(Qt::lightGray));

          scene->setSceneRect(0,0,background.width(),background.height());
          scene->addPixmap(background);
          scene->addText(QString::fromUtf8("Ошибка загрузки изображения: %1")
                         .arg(lastError_));
        }
        break;
      default:
        break;
    }
  }
  return false;
}

void FaxViewer::slotZoomIn()
{
  scaleImage(1.1);
}

void FaxViewer::slotZoomOut()
{
  scaleImage(0.9);
}

void FaxViewer::scaleImage(qreal deltaFactor)
{
  scaleFactor_ *= deltaFactor;
  ui_->imageGraphicsView->scale(deltaFactor, deltaFactor);
}

QPoint FaxViewer::adjustPosition(PrintOptionsTab* tab, const QSize& imageSize, const QSize& viewportSize)
{
  Qt::Alignment alignment = tab->alignment();
  int posX, posY;

  if (alignment & Qt::AlignLeft) {
    posX = 0;
  }
  else if (alignment & Qt::AlignHCenter) {
    posX = (viewportSize.width() - imageSize.width()) / 2;
  }
  else {
    posX = viewportSize.width() - imageSize.width();
  }

  if (alignment & Qt::AlignTop) {
    posY = 0;
  }
  else if (alignment & Qt::AlignVCenter) {
    posY = (viewportSize.height() - imageSize.height()) / 2;
  }
  else {
    posY = viewportSize.height() - imageSize.height();
  }
  return QPoint(posX, posY);
}

QSize FaxViewer::adjustSize(PrintOptionsTab* tab, int printerResolution, const QSize& viewportSize)
{
  QSize size = original_.size();
  PrintOptionsTab::ScaleMode scaleMode = tab->scaleMode();
  if (scaleMode == PrintOptionsTab::ScaleToPage) {
    bool imageBiggerThanPaper =
        size.width() > viewportSize.width() || size.height() > viewportSize.height();
    if (imageBiggerThanPaper || tab->enlargeSmallerImages()) {
      size.scale(viewportSize, Qt::KeepAspectRatio);
    }
  }
  else if (scaleMode == PrintOptionsTab::ScaleToCustomSize) {
    double wImg = tab->scaleWidth();
    double hImg = tab->scaleHeight();
    size.setWidth(int(wImg * printerResolution));
    size.setHeight(int(hImg * printerResolution));
  }
  else {
    // No scale
    const double INCHES_PER_METER = 100. / 2.54;
    int dpmX = original_.toImage().dotsPerMeterX();
    int dpmY = original_.toImage().dotsPerMeterY();
    if (dpmX > 0 && dpmY > 0) {
      double wImg = double(size.width()) / double(dpmX) * INCHES_PER_METER;
      double hImg = double(size.height()) / double(dpmY) * INCHES_PER_METER;
      size.setWidth(int(wImg * printerResolution));
      size.setHeight(int(hImg * printerResolution));
    }
  }
  return size;
}

void FaxViewer::slotZoomToWindow()
{
  slotZoomToOriginal();

  QGraphicsScene* scene = ui_->imageGraphicsView->scene();

  qreal w = (rotateAngle_ % 180 == 0) ? scene->width() : scene->height();
  qreal h = (rotateAngle_ % 180 == 0) ? scene->height() : scene->width();

  qreal sx = ui_->imageGraphicsView->viewport()->width() / w;
  qreal sy = ui_->imageGraphicsView->viewport()->height() / h;

  scaleFactor_ = (sx < sy) ? sx : sy;
  ui_->imageGraphicsView->scale(scaleFactor_, scaleFactor_);
}

void FaxViewer::slotZoomToOriginal()
{
  ui_->imageGraphicsView->scale(1.0/scaleFactor_, 1.0/scaleFactor_);
  scaleFactor_ = 1.0;
}

void FaxViewer::slotRotateAnticlockwise()
{
  rotateImage(-90.0);
}

void FaxViewer::slotRotateClockwise()
{
  rotateImage(90.0);
}

void FaxViewer::chooseFile()
{
  QString caption = QString::fromUtf8("Выбор файла для сохранения");

  QString filter = QString::fromUtf8("Файл BMP (*.bmp);;"
				     "Файл JPEG (*.jpg);;"
				     "Файл PNG (*.png);;"
				     "Файл TIFF (*.tiff)");

  QString defaultDir = QDir::homePath();

  QString selectedFilter;
  QString fileName = meteo::FileDialog::getSaveFileName(this, caption,
						  QObject::tr("Без имени"),
						  filter, &selectedFilter);

  if (!fileName.isEmpty()) {
    defaultDir = fileName.left(fileName.lastIndexOf("/") + 1);

    QRegExp rx("\\w\\s[(*]{2}(.\\w{3,4})");
    rx.indexIn(selectedFilter);

    selectedFilter = rx.cap(1);

    if (fileName.endsWith(selectedFilter) == false) {
      fileName += QString(selectedFilter);
    }

    if( true == fileName.isEmpty() ){
      return;
    }

    QFile file( fileName );
    if ( false == file.open(QIODevice::WriteOnly) ) {
      error_log << QObject::tr("Не удалось открыть файл для записи %1")
        .arg( file.fileName() );
      return;
    }
    selectedFilter.remove(".");

    QSize sz;
    if( ( rotateAngle_ % 180) != 0 ){
      sz.setHeight(original_.width());
      sz.setWidth(original_.height());
    }
    else{
      sz.setHeight(original_.height());
      sz.setWidth(original_.width());
    }
    QPixmap  pix(sz);
    QPainter painter(&pix);
    QImage image = original_.toImage();
    painter.setWindow(pix.rect());
    painter.translate(pix.rect().center());
    painter.rotate(rotateAngle_);

    if( ( rotateAngle_ % 180) != 0 ){
      painter.drawImage(pix.rect().topLeft()+QPoint(-pix.height()/2, -pix.width()/2), image);
    }
    else{
      painter.drawImage(pix.rect().topLeft()+QPoint(-pix.width()/2, -pix.height()/2), image);
    }
    if( false == pix.save(&file, selectedFilter.toLatin1().constData()) ){
      error_log << QObject::tr("Не удалось сохранить файл %1")
        .arg( file.fileName() );
    }
  }
}


void FaxViewer::print()
{
  QPrinter printer;
  PrintOptionsTab* tab= new PrintOptionsTab(original_.size());
  QPrintDialog *dialog = new QPrintDialog(&printer);
  dialog->setOptionTabs(QList<QWidget*>() << tab);
  dialog->setWindowTitle(QObject::tr("Печать"));

  foreach( QPushButton* b, dialog->findChildren<QPushButton*>() ){
    if( "&Параметры >>" == b->text() ){
      b->click();
    }
  }
  QTabWidget* tw = dialog->findChild<QTabWidget*>();
  if( nullptr != tw ){
    tw->setCurrentWidget(tab);
  }
  dialog->adjustSize();

  bool wantToPrint = dialog->exec();
  if (!wantToPrint) {
    return;
  }
  QPainter painter(&printer);
  QRect rect = painter.viewport();
  QSize size = adjustSize(tab, printer.resolution(), rect.size());
  QPoint pos = adjustPosition(tab, size, rect.size());
  painter.setViewport(pos.x(), pos.y(), size.width(), size.height());
  QImage image = original_.toImage();
  painter.setWindow(image.rect());
  painter.drawImage(0, 0, image);
}

void FaxViewer::rotateImage(qreal angle)
{
  rotateAngle_ += static_cast<int>(angle);
  ui_->imageGraphicsView->rotate(angle);
}

void FaxViewer::init(const QString& faxid)
{
  QByteArray raw = getFaxImageFromService(faxid);
  QPixmap map;
  map.loadFromData(raw);
  setPixmap(addImageTransparence(map));
}

void FaxViewer::setPixmap(const QPixmap& image)
{
  QGraphicsScene* scene = ui_->imageGraphicsView->scene();
  if ( nullptr == scene ) {
    return;
  }
  clearScene();

  original_ = image;
  bool ok = !image.isNull();

  if (ok == true) {
    scene->setSceneRect(0, 0, image.width(), image.height());
    item_ = scene->addPixmap(original_);
  }
  else {
    item_ = nullptr;
    QPixmap background(ui_->imageGraphicsView->sceneRect().size().toSize());
    background.fill(QColor(Qt::lightGray));

    scene->setSceneRect(0,0,background.width(),background.height());
    scene->addPixmap(background);
    scene->addText(QString::fromUtf8("Ошибка загрузки изображения %1")
                   .arg(lastError_));
  }
  setEnabledActions(ok);
}

FaxScene*FaxViewer::scene()
{
  return scene_;
}

QPixmap FaxViewer::addImageTransparence(const QPixmap& img) const
{
  QImage result(img.toImage().convertToFormat(QImage::Format_ARGB32));
  for (int i = 0, w = result.width(); i < w; ++i) {
    for (int j = 0, h = result.height(); j < h; ++j) {
      if (result.pixel(i,j) == qRgba(255,255,255,255) ) {
        result.setPixel(i,j, qRgba(0,0,0,0));
      }
    }
  }
  return QPixmap::fromImage(result);
}

QGraphicsView* FaxViewer::graphicsView() const
{
  return ui_ != nullptr ? ui_->imageGraphicsView : nullptr;
}

QGraphicsPixmapItem* FaxViewer::item() const
{
  return item_;
}

QByteArray FaxViewer::getFaxImageFromService(const QString& faxid)
{
  lastError_.clear();

  meteo::map::proto::FaxRequest request;
  request.set_header_only(false);
  auto faxRecord = request.add_faxes();
  faxRecord->set_id(faxid.toStdString());

  auto channel = meteo::global::serviceChannel(meteo::settings::proto::kMap);
  if (nullptr == channel){
    error_log << QObject::tr("Не удается установить соединение с сервисом");
    return QByteArray();
  }
  auto reply = channel->remoteCall(&meteo::map::proto::DocumentService::GetFaxes, request, 1000);
  QByteArray data = QByteArray::fromStdString(reply->faxes(0).msg());

  return  data;
}

void FaxViewer::clearScene()
{
  slotZoomToOriginal();
  rotateImage(-rotateAngle_);

  QGraphicsScene* scene = ui_->imageGraphicsView->scene();
  foreach (QGraphicsItem* item, scene->items()) {
    scene->removeItem(item);
    delete item;
  }
  item_ = nullptr;

  ui_->imageGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_->imageGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QSize viewSize = ui_->imageGraphicsView->viewport()->size();
  scene->setSceneRect(0, 0, viewSize.width(), viewSize.height());

  ui_->imageGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  ui_->imageGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void FaxViewer::setEnabledActions(bool enabled)
{
  foreach (QToolButton* btn, findChildren<QToolButton*>()) {
    btn->setEnabled(enabled);
  }

  if (enabled == true) {
    ui_->imageGraphicsView->viewport()->installEventFilter(this);
    ui_->imageGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui_->imageGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }
  else {
    ui_->imageGraphicsView->viewport()->removeEventFilter(this);
    ui_->imageGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui_->imageGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
}
