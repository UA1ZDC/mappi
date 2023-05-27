#include "printdoc.h"
#include "ui_print.h"
#include "layersselect.h"
#include "printpreview.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/view/mapwindow.h>


#include <qprintpreviewdialog.h>

using namespace meteo;
using namespace map;

namespace {

  namespace printdoc {
    enum PageScaling {
      kNone = 0,
      kFitArea = 1,
      kShrinkToPage = 2, 
      kScale   = 3
    };
  }
}

//-----------

PrintDoc::PrintDoc( MapWindow* parent):
  //  QWidget(parent),
  mapwindow_(parent),
  ui_(0),
  printer_(0),
  watchedEventHandler_(0),
  previewDialog_(0),
  initOk_(false)
{

  // setTitle(QObject::tr("Карта"));
  // setPixmap(QPixmap(":/meteo/icons/map/print.png"));

  if (0 == parent || 0 == parent->mapview()) {
    return;
  }

  ui_ = new Ui::PrintDoc;

  ui_->setupUi(this);

  ui_->paperSize->insertItems(0, QStringList()
			      << "A0" << "A1" << "A2" << "A3" << "A4" << "A5" << "A6" << "A7" << "A8" << "A9"
			      << "B0" << "B1" << "B2" << "B3" << "B4" << "B5" << "B6" << "B7" << "B8" << "B9"
			      << "B10" << "C5E" << "Comm10E" << "DLE" << "Executive" << "Folio" << "Ledger"
			      << "Legal" << "Letter" << "Tabloid");
  paperSizes_ << QPrinter::A0 << QPrinter::A1 << QPrinter::A2 << QPrinter::A3 << QPrinter::A4 
	      << QPrinter::A5 << QPrinter::A6 << QPrinter::A7 << QPrinter::A8 << QPrinter::A9 
	      << QPrinter::B0 << QPrinter::B1 << QPrinter::B2 << QPrinter::B3 << QPrinter::B4 
	      << QPrinter::B5 << QPrinter::B6 << QPrinter::B7 << QPrinter::B8 << QPrinter::B9 
	      << QPrinter::B10 << QPrinter::C5E << QPrinter::Comm10E << QPrinter::DLE 
	      << QPrinter::Executive << QPrinter::Folio << QPrinter::Ledger << QPrinter::Legal 
	      << QPrinter::Letter << QPrinter::Tabloid << QPrinter::Custom;
  
  ui_->paperSize->setCurrentIndex(4);

  ui_->orient->setAlign(kTopLeft);

  layers_ = new LayersSelect( mapview(), this);
  QGridLayout* gridLayout2 = new QGridLayout(ui_->wlayers);
  gridLayout2->addWidget(layers_, 0, 0, 1, 1);
  gridLayout2->setContentsMargins(0, 0, 0, 0);

  layers_->loadLayers();
  layers_->setCheckedVisibleLayers();
  layers_->showUnvisibleLayers(false);

  fillCoordsFromDocument();

  printer_ = new QPrinter;
  printer_->setOutputFormat(QPrinter::PdfFormat);
  qreal left, top, right, bottom;
  printer_->getPageMargins ( &left, &top, &right, &bottom, QPrinter::Millimeter);
  ui_->leftMargin->setValue(left);
  ui_->rightMargin->setValue(right);
  ui_->topMargin->setValue(top);
  ui_->bottomMargin->setValue(bottom);

  // ui_->cancelButton->setVisible(false);
  // ui_->printButton->setVisible(false);

  // connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(slotCancel())); 
  // connect(ui_->printButton, SIGNAL(clicked()), SLOT(slotPrint()));
  connect(ui_->preview, SIGNAL(stateChanged(int)), SLOT(previewStateChaged(int)));
  
  //интерактивная область выделения
  connect(ui_->rubberBtn, SIGNAL(clicked(bool)), SLOT(slotRubberClicked(bool)));
  connect(ui_->fitMapMode,     SIGNAL(currentIndexChanged(int)), SLOT(changeFitMode(int)));
  connect(ui_->mapScale,  SIGNAL(valueChanged(double)), SLOT(bandChanged()));

  //фиксированная рамка
  connect(ui_->fixedBand, SIGNAL(stateChanged(int)), SLOT(changedFixedBandMode(int)));
  connect(ui_->paperHorCnt,    SIGNAL(valueChanged(int)), SLOT(bandChanged()));
  connect(ui_->paperVerticCnt, SIGNAL(valueChanged(int)), SLOT(bandChanged()));

  //параметры листа
  connect(ui_->landscape, SIGNAL(toggled(bool)), SLOT(bandChanged()));
  connect(ui_->paperSize, SIGNAL(currentIndexChanged(int)), SLOT(bandChanged()));
  connect(ui_->overlap,   SIGNAL(valueChanged(double)), SLOT(bandChanged()));
  connect(ui_->orient,    SIGNAL(posChanged(int)),   SLOT(bandChanged()));
  connect(ui_->topMargin, SIGNAL(valueChanged(double)), SLOT(bandChanged()));
  connect(ui_->rightMargin,  SIGNAL(valueChanged(double)), SLOT(bandChanged()));
  connect(ui_->bottomMargin, SIGNAL(valueChanged(double)), SLOT(bandChanged()));
  connect(ui_->leftMargin, SIGNAL(valueChanged(double)), SLOT(bandChanged()));

  installWatchedDocument();
  connect(layers_, SIGNAL(layersChanged()), SLOT(bandChanged()));  

  initOk_ = true;

  bandChanged();
}

PrintDoc::~PrintDoc()
{
  initOk_ = false;
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
  resetWatchedDocument();
  if (0 != printer_) {
    delete printer_;
    printer_ = 0;
  }
  if (0 != previewDialog_) {
    delete previewDialog_;
    previewDialog_ = 0;
  }
}

void PrintDoc::setOptions(const QString& options)
{ 
  Q_UNUSED(options);
  resetWatchedDocument();
  installWatchedDocument();
}

::meteo::map::Document* PrintDoc::currentDocument() const
{
  if (mapview() != 0 &&
      mapview()->mapscene() != 0 &&
      mapview()->mapscene()->document() != 0) {
    return mapview()->mapscene()->document();
  }

  return 0;
}

bool PrintDoc::installWatchedDocument()
{
  Document* current = currentDocument();

  if (0 != current &&
      current->eventHandler() != 0) {
    watchedEventHandler_ = current->eventHandler();
    watchedEventHandler_->installEventFilter(this);

    connect( mapview(), SIGNAL(sceneChanged(MapScene*)), this, SLOT(slotSceneChanged(MapScene*)));

    return true;
  }
  return false;
}

void PrintDoc::resetWatchedDocument()
{
  if (watchedEventHandler_ != 0) {
    this->removeEventFilter(watchedEventHandler_);
    watchedEventHandler_ = 0;
  }
  
  if (0 != mapview()) {
    disconnect( mapview(), SIGNAL(sceneChanged(MapScene*)), this, SLOT(slotSceneChanged(MapScene*)));
  }
}

bool PrintDoc::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == watchedEventHandler_) {
    if (event->type() == map::LayerEvent::LayerChanged) {
      LayerEvent* le = static_cast<LayerEvent*>(event);
      if (0 != le && (map::LayerEvent::Deleted == le->changeType() || 
		      map::LayerEvent::Added == le->changeType() ||
		      map::LayerEvent::Visibility == le->changeType())) {
	  layers_->loadLayers();
	  layers_->setCheckedVisibleLayers();
	  layers_->showUnvisibleLayers(false);
      }
    } else if (event->type() == map::MapEvent::MapChanged) {
      if (! isRubberPressed()) {
	fillCoordsFromDocument();
	bandChanged();
      }
    } else if (event->type() == map::DocumentEvent::DocumentChanged) {
      if (! isRubberPressed()) {
	fillCoordsFromDocument();
	bandChanged();
      } else {
	if (0 != previewDialog_)  {
	  previewDialog_->updatePreview();
	}
      }      
    }
  } else if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (0 != keyEvent && (Qt::Key_Return == keyEvent->key() ||
			  Qt::Key_Enter  == keyEvent->key())) {
      return true;
    }
    
  } 

  return QWidget::eventFilter(watched, event);
}

void PrintDoc::finished(int result)
{
  if (result == QDialog::Accepted) {
    redrawPreview();
  } 

  close();
}

void PrintDoc::close()
{
  initOk_ = false;
  resetWatchedDocument();
  removeRubber();
  previewClosed();
}

void PrintDoc::showEvent(QShowEvent *e)
{
  if (!initOk_) {
    if (! isRubberPressed()) {
      fillCoordsFromDocument();
    }

    layers_->loadLayers();
    layers_->setCheckedVisibleLayers();
    layers_->showUnvisibleLayers(false);
    
    installWatchedDocument();
    initOk_ = true;
  }
  
  QWidget::showEvent(e);
}

  //! Сброс выделения и обновление значений углов документа при смене сцены [private slot]
void PrintDoc::slotSceneChanged(MapScene* scene)
{
  Q_UNUSED(scene);
  resetRubber(scene);
}

// void PrintDoc::slotCancel()
// { 
//   close();
// }


void PrintDoc::setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect)
{
  //  trc;
  Q_UNUSED(nw);
  Q_UNUSED(ne);
  Q_UNUSED(sw);
  Q_UNUSED(se);
  //  var(rect);
  mapBand_ = rect;
  bandChanged();
}

//! Заполнение координат в соответствии с углами документа
void PrintDoc::fillCoordsFromDocument(map::Document* doc /*= 0*/)
{
  map::Document* document = 0;
  if (0 != doc) {
    document = doc;
  } else {
    document = currentDocument();
  }

  if (0 != document) {
    int w = document->ramkaWidth();
    if (document->property().doctype() == map::proto::kGeoMap) {
      mapBand_ = document->documentRect();
    } else {
      mapBand_ = document->contentRect();
    }
    
    mapBand_.adjust(w, w, -w, -w);
  }
}

bool PrintDoc::isRubberPressed() const
{
  return ui_->rubberBtn->isChecked();
}


//! Действрия при завершении рисования рамки
void PrintDoc::rubberFinished()
{
}

//! Сброс выделения и обновление значений углов документа при смене сцены
void PrintDoc::resetRubber(MapScene* scene)
{
  ui_->rubberBtn->setChecked(false);
  SelectActionIface::removeAction();
  if (0 != scene && 0 != scene->document()) {
    fillCoordsFromDocument(scene->document());
  }
  bandChanged();
  QApplication::restoreOverrideCursor();
}

void PrintDoc::removeRubber()
{
  SelectActionIface::removeAction();
  ui_->rubberBtn->setChecked(false);
  ui_->fixedBand->setCheckState(Qt::Unchecked);
  QApplication::restoreOverrideCursor();
}

  //! Обработка нажатия кнопки выделения области
void PrintDoc::slotRubberClicked(bool on)
{
  if( true == on ) {
    if (ui_->fixedBand->checkState() == Qt::Checked) {
      ui_->fixedBand->setCheckState(Qt::Unchecked);
      ui_->paperHorCnt->setEnabled(false);
      ui_->paperVerticCnt->setEnabled(false);
      ui_->fitMapMode->setEnabled(true);
      ui_->mapScale->setEnabled(true);
      ui_->orient->setEnabled(true);
      SelectActionIface::removeAction(false);
      QApplication::restoreOverrideCursor();
    }
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    SelectActionIface::createAction(mapview()->mapscene());
  } else {
    SelectActionIface::removeAction();
    QApplication::restoreOverrideCursor();
    fillCoordsFromDocument();
    bandChanged();
  }
}

void PrintDoc::changeFitMode(int index)
{
  if (index == printdoc::kShrinkToPage || index == printdoc::kScale) {
    ui_->mapScale->setEnabled(false);
  } else {
    ui_->mapScale->setEnabled(true);
  }
  bandChanged();
}

void PrintDoc::changedFixedBandMode(int state)
{
  if (state == Qt::Checked) {
    ui_->paperHorCnt->setEnabled(true);
    ui_->paperVerticCnt->setEnabled(true);
    ui_->fitMapMode->setEnabled(false);
    ui_->mapScale->setEnabled(false);
    ui_->orient->setEnabled(false);
    if (isRubberPressed()) {
      ui_->rubberBtn->setChecked(false);
      SelectActionIface::removeAction(false);
      QApplication::restoreOverrideCursor();
    }
    SelectActionIface::createAction( mapview()->mapscene(), calcFixedBand());
    
  } else {
    ui_->paperHorCnt->setEnabled(false);
    ui_->paperVerticCnt->setEnabled(false);
    ui_->fitMapMode->setEnabled(true);
    ui_->mapScale->setEnabled(true);
    ui_->orient->setEnabled(true);
    SelectActionIface::removeAction();
    QApplication::restoreOverrideCursor();
    fillCoordsFromDocument();
    bandChanged();
  }
}


//Предпросмотр
void PrintDoc::previewStateChaged(int state)
{
  if (state == Qt::Checked) {
    if (0 != previewDialog_) {
      delete previewDialog_;
    }
    previewDialog_ = new PrintPreview(printer_);
    previewDialog_->setWindowTitle(QObject::tr("Предварительный просмотр"));
    connect(previewDialog_, SIGNAL(paintRequested(QPrinter *)), SLOT(redrawPreview()));
    connect(previewDialog_, SIGNAL(closed()), SLOT(previewClosed()));
    previewDialog_->show();
    previewDialog_->raise();
    previewDialog_->activateWindow();
    previewDialog_->setAllPages();
  } else {
    previewClosed();
  }
}

void PrintDoc::previewClosed()
{
  ui_->preview->setCheckState(Qt::Unchecked);
  if (0 != previewDialog_) {
    previewDialog_->deleteLater();
    previewDialog_ = nullptr;
  }
}

//! При изменении параметров печати, перерисовка превью и, при необходимости, рамки
void PrintDoc::bandChanged()
{
  setPrinterSettings();

  if (ui_->fixedBand->checkState() == Qt::Checked) {
    mapBand_ = calcFixedBand();
    SelectActionIface::setBand(mapBand_);
  } 

  if (0 != previewDialog_)  {
    previewDialog_->updatePreview();
  }
}

void PrintDoc::redrawPreview()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  int woffset = 0;
  int hoffset = 0;
  int colCnt = ui_->paperHorCnt->value();
  int rowCnt = ui_->paperVerticCnt->value();

  if (ui_->fixedBand->checkState() != Qt::Checked) {
    calcRubberBand(&colCnt, &rowCnt);
    calcOffset(colCnt, rowCnt, &woffset, &hoffset);
  }

  redrawPreview(colCnt, rowCnt, woffset, hoffset);
  QApplication::restoreOverrideCursor();
}


QRect PrintDoc::calcFixedBand()
{
  Document* doc = currentDocument();
  if (0 == doc) return QRect();

  QRect rect = doc->documentRect();
  float dx = rect.width()*0.85;
  float dy = rect.height()*0.85;

  targetBand_.setWidth(printer_->pageRect().width() * ui_->paperHorCnt->value());
  targetBand_.setHeight(printer_->pageRect().height() * ui_->paperVerticCnt->value());

  double wunit = printer_->width() / printer_->widthMM();
  double hunit = printer_->height() / printer_->heightMM();

  double wscale = dx / (targetBand_.width() - ui_->overlap->value()* wunit*(ui_->paperHorCnt->value()-1) - 2*doc->ramkaWidth());
  double hscale = dy / (targetBand_.height() - ui_->overlap->value()* hunit *(ui_->paperVerticCnt->value()-1) - 2*doc->ramkaWidth());
  double maxScale = qMin(wscale, hscale);

  dx = (targetBand_.width() - ui_->overlap->value() * wunit * (ui_->paperHorCnt->value()-1) - 2*doc->ramkaWidth())*maxScale;
  dy = (targetBand_.height() - ui_->overlap->value()* hunit *(ui_->paperVerticCnt->value()-1) - 2*doc->ramkaWidth())*maxScale;
  QRect  band = QRect(rect.center()-QPoint(dx/2, dy/2), QSize(dx, dy));
  return band;
}

void PrintDoc::calcRubberBand(int* colCnt, int* rowCnt)
{
  Document* doc = currentDocument();
  if (0 == doc) return;

  //mapBand_ уже установлена через setPoints

  targetBand_ = QRect(0, 0, mapBand_.width() * ui_->mapScale->value() + 2*doc->ramkaWidth(), 
		      mapBand_.height() * ui_->mapScale->value() + 2*doc->ramkaWidth());

  //количество страниц
  int col = ceil(targetBand_.width()/double(printer_->pageRect().width()));
  int row = ceil(targetBand_.height()/double(printer_->pageRect().height()));
  //с учетом перекрытия
  double wunit = printer_->width() / printer_->widthMM();
  double hunit = printer_->height() / printer_->heightMM();
  *colCnt = ceil((targetBand_.width() + ui_->overlap->value()* wunit *(col-1))/double(printer_->pageRect().width()));
  *rowCnt = ceil((targetBand_.height() + ui_->overlap->value()* hunit *(row-1))/double(printer_->pageRect().height()));
  if (*colCnt > col) {
    *colCnt = ceil((targetBand_.width() + ui_->overlap->value()* wunit *(*colCnt-1))/double(printer_->pageRect().width()));
  }
  if (*rowCnt > row) {
    *rowCnt = ceil((targetBand_.height() + ui_->overlap->value()* hunit *(*rowCnt-1))/double(printer_->pageRect().height()));
  }


    //масштаб к бумаге
  switch (ui_->fitMapMode->currentIndex()) {
  case printdoc::kShrinkToPage: {
    double wscale = printer_->pageRect().width()/(double)(mapBand_.width() + 2*doc->ramkaWidth());
    double hscale = printer_->pageRect().height()/(double)(mapBand_.height() + 2*doc->ramkaWidth());
    double maxScale = qMin(wscale, hscale);
    targetBand_.setWidth((mapBand_.width()  + 2*doc->ramkaWidth()) * maxScale);
    targetBand_.setHeight((mapBand_.height() + 2*doc->ramkaWidth()) * maxScale);
    
    *colCnt = 1;
    *rowCnt = 1;
  }
    break;
  // case printdoc::kScale: {
  //   targetBand_.setWidth(targetBand_.width() + ui_->overlap->value()*(*colCnt-1));
  //   targetBand_.setHeight(targetBand_.height() + ui_->overlap->value()*(*rowCnt-1));

  //   *colCnt = 1;
  //   *rowCnt = 1;

 
  //   break;
  // }
  case printdoc::kFitArea: {
    targetBand_.setWidth(printer_->pageRect().width() * (*colCnt));
    targetBand_.setHeight(printer_->pageRect().height()* (*rowCnt));
  }
  break;
  default: {
    targetBand_.setWidth(targetBand_.width() + ui_->overlap->value()* wunit *(*colCnt-1));
    targetBand_.setHeight(targetBand_.height() + ui_->overlap->value()* hunit *(*rowCnt-1));
  }
  }
}

//! Расчет смещения карты относительно бумаги при использовании выравнивания
void PrintDoc::calcOffset(int colCnt, int rowCnt, int* woffset, int* hoffset)
{
  switch (ui_->orient->align()) {
  case kBottomCenter:
  case kCenter:
  case kTopCenter: 
    *woffset = abs(targetBand_.width() - printer_->pageRect().width()*colCnt) / 2;
    break;
  case kBottomRight:
  case kRightCenter:
  case kTopRight:
    *woffset = abs(targetBand_.width() - printer_->pageRect().width()*colCnt);
    break;
  }

  switch (ui_->orient->align()) {
  case kLeftCenter:
  case kCenter:
  case kRightCenter:
    *hoffset = abs(targetBand_.height() - printer_->pageRect().height()*rowCnt) / 2;
    break;
  case kBottomLeft:
  case kBottomCenter:
  case kBottomRight:
    *hoffset = abs(targetBand_.height() - printer_->pageRect().height()*rowCnt);
    break;
  }
}

//! Настройки принтера из ui-шки
void PrintDoc::setPrinterSettings()
{
  printer_->setPageMargins (ui_->leftMargin->value(),  ui_->topMargin->value(), 
			    ui_->rightMargin->value(), ui_->bottomMargin->value(), QPrinter::Millimeter);
  if (ui_->landscape->isChecked()) {
    printer_->setOrientation(QPrinter::Landscape);
  } else {
    printer_->setOrientation(QPrinter::QPrinter::Portrait);
  }
  
  if (ui_->paperSize->currentIndex() >= paperSizes_.size()) {
    printer_->setPaperSize(QPrinter::A4);
  } else {
    printer_->setPaperSize(paperSizes_.at(ui_->paperSize->currentIndex()));
  }

}

void PrintDoc::redrawPreview(int colCnt, int rowCnt, int woffset, int hoffset)
{
  if ((ui_->fitMapMode->isEnabled() && ui_->fitMapMode->currentIndex() == printdoc::kScale)) {
    redrawPreview(woffset, hoffset);
    return;
  }

  Document* original = currentDocument();
  if (0 == original) return;

  int rw = original->ramkaWidth();
  float  scale = 1;

  double xscale = 1;
  double yscale = 1;
  double wunit = printer_->width() / printer_->widthMM();
  double hunit = printer_->height() / printer_->heightMM();

 
  if (ui_->fitMapMode->isEnabled() && ui_->fitMapMode->currentIndex() == printdoc::kFitArea) {
    xscale = ui_->mapScale->value();
    yscale = ui_->mapScale->value();
  } else {
    xscale = (targetBand_.width() - 2*rw - ui_->overlap->value()* wunit *(colCnt-1))/double(mapBand_.width());
    yscale = (targetBand_.height() -  2*rw - ui_->overlap->value()* hunit *(rowCnt-1))/double(mapBand_.height());
  }

  scale = qMin(xscale, yscale);
  
  // var(xscale);
  // var(yscale);

  QSize docSize(mapBand_.width(), mapBand_.height());
  map::Document* doc = original->stub( original->screen2coord(mapBand_.center()), docSize );
  if (0 == doc) return;
  //var(doc->scale());

  QPoint tl = doc->documentRect().topLeft();//doc->documentTopLeft();
 GeoPoint geo_tl = doc->screen2coord(tl);
  //  var(scale);
  double scaleKoef = 1.0/( qPow( 2.0, doc->scale() ) );
  doc->setScale(log2(1.0/(scale*scaleKoef)));
  //  doc->setScale(doc->scale() + log2(1/scale));

  doc->resizeDocument(targetBand_.size() - QSize(ui_->overlap->value()*wunit*(colCnt-1), 
						 ui_->overlap->value()*hunit*(rowCnt-1)));

  QPoint newtl = doc->coord2screen(geo_tl);
  QPoint deltapos = tl - newtl;
  QPoint scrcntr = doc->coord2screen(doc->screenCenter());
  doc->setScreenCenter( scrcntr - (deltapos + doc->documentRect().topLeft() + QPoint(rw, rw)));
  doc->setOffCenterMap( QPoint(0,0) );
  
  QStringList uuids = layers_->checkedLayers(doc);
  QImage img( doc->documentRect().normalized().size(), QImage::Format_ARGB32_Premultiplied );
//  img.fill( qRgba( 0, 0, 0, 0 ) );
  img.fill( doc->backgroundColor() );
  QPainter painter;
//  painter.begin(printer_);
  painter.begin(&img);
  doc->drawDocument( &painter, uuids, img.rect(), doc->documentRect() );
  painter.end();
  //  img.save("./test.png");
  painter.begin(printer_);

  int wRemain = targetBand_.width();
  int hRemain = targetBand_.height();
  int hcurOff = hoffset;   //для выравнивания, сдвиг только для первого ряда 
  bool ok = true;

  for (int row = 0; row < rowCnt; row++) {
    wRemain = targetBand_.width();
   
    int h = qMin(hRemain, printer_->pageRect().height() - hcurOff);
    int wcurOff = woffset; //для выравнивания, сдвиг только для первой колонки 
    for (int col = 0; col < colCnt; col++) {
      int w = qMin(wRemain, printer_->pageRect().width() - wcurOff);

      QRect source(printer_->pageRect().width()*col - woffset + wcurOff + 0, 
  		   printer_->pageRect().height()*row - hoffset + hcurOff + 0, 
  		   w, h);

      source.moveLeft(source.left() - ui_->overlap->value()*col * wunit);
      source.moveTop(source.top() - ui_->overlap->value()*row * hunit);
      
//      painter.setClipRect(QRect(wcurOff, hcurOff, w, h));
      
//      painter.save();
//      painter.setBrush(QBrush(doc->backgroundColor()));
//      painter.drawRect(QRect(wcurOff,hcurOff,printer_->pageRect().width() - wcurOff, 
//			     printer_->pageRect().height() - hcurOff));
//      painter.restore();

      QRect target(wcurOff,hcurOff,printer_->pageRect().width() - wcurOff, printer_->pageRect().height() - hcurOff);
      if ( target.width() > source.width() ) {
        target.setWidth( source.width() );
      }
      if ( target.height() > source.height() ) {
        target.setHeight( source.height() );
      }
      painter.drawImage( target, img, source );
      
//      doc->drawDocument(&painter, uuids, QRect(wcurOff,hcurOff,printer_->pageRect().width() - wcurOff, 
//      					       printer_->pageRect().height() - hcurOff), 
//       			source);
      if (!(row == rowCnt - 1 && col == colCnt - 1)) {
  	if (! printer_->newPage()) {
  	  ok = false;
  	  break;
  	}
      }
      wRemain -= w;
      wcurOff = 0;
      if (!ok) break;
    }
    hRemain -= h;
    hcurOff = 0;
  }

  painter.end();

  delete doc;
  if (!ok) {
    error_log << QObject::tr("Ошибка печати");
    QMessageBox::critical(this, QObject::tr("Ошибка печати"), 
  			  QObject::tr("Ошибка печати"));
    // qWarning("failed in flushing page to disk, disk full?");
  }

}

// масштаб картинки
void PrintDoc::redrawPreview(int woffset, int hoffset)
{  
  Document* original = currentDocument();
  if (0 == original) return;

  double wscale = (printer_->pageRect().width())/(double)(mapBand_.width() + 2*original->ramkaWidth());
  double hscale = (printer_->pageRect().height())/(double)(mapBand_.height() + 2*original->ramkaWidth());
  double maxScale = qMin(wscale, hscale);
  int rw = original->ramkaWidth();

  targetBand_ = QRect(0, 0, (mapBand_.width()+ 2*rw)*maxScale, (mapBand_.height()+ 2*rw)*maxScale);

  calcOffset(1, 1, &woffset, &hoffset);

  woffset /= maxScale;
  hoffset /= maxScale;

  // var(xscale);
  // var(yscale);

  QSize docSize(mapBand_.width() + 2*rw, mapBand_.height() + 2*rw);
  map::Document* doc = original->stub( original->screen2coord(mapBand_.center()), docSize );
  if (0 == doc) return;
  //var(doc->scale());

  QStringList uuids = layers_->checkedLayers(doc);
  QPainter painter;
  painter.begin(printer_);

  bool ok = true;



  int w = printer_->pageRect().width();
  int h = printer_->pageRect().height();

  if (maxScale < 1) {
    w = mapBand_.width() + 2*rw;
    h = mapBand_.height() + 2*rw;
  }

  QRect source(doc->documentRect().x(), 
	       doc->documentRect().y(), 
	       w, h);
  
  // painter.setClipRect(QRect(woffset, hoffset, qMin(w, mapBand_.width() + 2*original->ramkaWidth()), qMin(h, mapBand_.height() + 2*original->ramkaWidth())));
  
  painter.scale(maxScale, maxScale);
  painter.translate(woffset, hoffset);
  woffset = 0;
  hoffset = 0;
    
  painter.save();
  painter.setBrush(QBrush(doc->backgroundColor()));
  painter.drawRect(QRect(woffset, hoffset, mapBand_.width() + 2*rw,  
			 mapBand_.height() + 2*rw));
  painter.restore();
  
  if (maxScale > 1) {
    doc->drawDocument(&painter, uuids, QRect(woffset, hoffset, printer_->pageRect().width(), 
					     printer_->pageRect().height()), 
		      source);
  } else {
    doc->drawDocument(&painter, uuids, QRect(woffset, hoffset, mapBand_.width() + 2*rw, 
					     mapBand_.height() + 2*rw), 
		      source);
  }

  painter.end();

  delete doc;
  if (!ok) {
    error_log << QObject::tr("Ошибка печати");
    QMessageBox::critical(this, QObject::tr("Ошибка печати"), 
  			  QObject::tr("Ошибка печати"));
    // qWarning("failed in flushing page to disk, disk full?");
  }

}

//! Печать
// void PrintDoc::slotPrint()
// {
//   QPrintDialog *dialog = new QPrintDialog(printer_);
//   dialog->setOption(QAbstractPrintDialog::PrintShowPageSize, false);
//   dialog->setWindowTitle(QObject::tr("Печать"));
//   if (dialog->exec() != QDialog::Accepted) {
//     delete dialog;
//     return;
//   }

//   redrawPreview();

//   slotCancel();
  
//   delete dialog;
// }
