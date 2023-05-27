#include "ui_savebitmap.h"
#include "layersselect.h"
#include "savebitmap.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <qimage.h>
#include <qregexp.h>

#define DEFAULT_DIR QDir::homePath() + "/.meteo/" + "exportbitmap.conf"
static const QString kEmptyDefaultName = QObject::tr("Пустой документ");

using namespace meteo;

//-----------

map::SaveBitmap::SaveBitmap(MapWindow* parent):
  MapWidget(parent),
  ui_(0),
  watchedEventHandler_(0),
  savefs_(true),
  savedb_(true)
{

  //setPixmap(QPixmap(":/meteo/icons/map/print.png"));

  ui_ = new Ui::SaveBitmap;
  ui_->setupUi(this);

  layers_ = new LayersSelect( mapview(), this);
  QGridLayout* gridLayout2 = new QGridLayout(ui_->wlayers);
  gridLayout2->addWidget(layers_, 0, 0, 1, 1);
  gridLayout2->setContentsMargins(0, 0, 0, 0);

  layers_->loadLayers();
  layers_->setCheckedVisibleLayers();
  layers_->showUnvisibleLayers(false);

  fillCoordsFromDocument();

  defaultDir_ = readDefaultDir();
  QStringList ext;
  ext << "JPEG" << "PNG" << "BMP" << "TIFF";
  ui_->dbext->addItems(ext);

  setDefaultName();
 

  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(slotCancel()));
  connect(ui_->saveButton, SIGNAL(clicked()), SLOT(slotSave()));

  connect(ui_->rubberBtn, SIGNAL(clicked(bool)), SLOT(slotRubberClicked(bool)));
  connect(ui_->btnfile, SIGNAL(clicked()), SLOT(chooseFile()));
  connect(ui_->mapScale,  SIGNAL(valueChanged(double)), SLOT(bandChanged()));
  //connect(layers_, SIGNAL(layersChanged()), SLOT(bandChanged()));
  //connect(ui_->dbext, SIGNAL(currentIndexChanged(const QString &)), SLOT(extChanged(const QString &)));

  bandChanged();
  installWatchedDocument();
}

map::SaveBitmap::~SaveBitmap()
{
  clear();
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}

void map::SaveBitmap::clear()
{
  resetWatchedDocument();
  removeRubber();
}

void map::SaveBitmap::setOptions(const QString& options)
{
  Q_UNUSED(options);
  clear();
  //  installWatchedDocument();
}

map::Document* map::SaveBitmap::currentDocument() const
{
  if (mapview() != 0 &&
      mapview()->mapscene() != 0 &&
      mapview()->mapscene()->document() != 0) {
    return mapview()->mapscene()->document();
  }

  return 0;
}

bool map::SaveBitmap::installWatchedDocument()
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

void map::SaveBitmap::resetWatchedDocument()
{
  if (watchedEventHandler_ != 0) {
    this->removeEventFilter(watchedEventHandler_);
    watchedEventHandler_ = 0;
  }
  if (0 != mapview()) {
    disconnect( mapview(), SIGNAL(sceneChanged(MapScene*)), this, SLOT(slotSceneChanged(MapScene*)));
  }
}

bool map::SaveBitmap::eventFilter(QObject* watched, QEvent* event)
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
      }
    }

  }

  return MapWidget::eventFilter(watched, event);
}

void map::SaveBitmap::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    clear();
    MapWidget::keyPressEvent(e);
  }
}

void map::SaveBitmap::closeEvent(QCloseEvent*e)
{
  clear();
  MapWidget::closeEvent(e);
}

void map::SaveBitmap::showEvent(QShowEvent *)
{
  layers_->loadLayers();
  layers_->setCheckedVisibleLayers();
  layers_->showUnvisibleLayers(false);
  if (! isRubberPressed()) {
    fillCoordsFromDocument();
  }
  installWatchedDocument();
  setDefaultName();
}

void map::SaveBitmap::slotCancel()
{
  MapWidget::close();
}


void map::SaveBitmap::setDefaultName()
{
  QString txt = defaultDir_ + '/' + mapname();
  while ( -1 != txt.indexOf("//") ) {
    txt.replace("//","/");
  }
  ui_->lefile->setText(txt);
}

QString map::SaveBitmap::mapname() const
{
  QString txt = ui_->lefile->text();
  QFileInfo fi(txt);
  // if ( false == fi.isDir() ) {
  //   txt = fi.fileName();
  //   var(txt);
  //   if ( false == txt.isEmpty() ) {
  //     if ( !txt.contains( QRegExp("[\\w\\s/_]+\\d{2,4}[.]\\d{2}[.]\\d{2,4}_\\d{2}:\\d{2}(?:[.]\\w{3,4})?$") ) ) {
  // 	debug_log << "not contain\n";
  //       return txt;
  //     }
  //     else {
  //       txt = "";
  //     }
  //   }
  // }
  txt = "";
  txt = QString::fromStdString( mapdocument()->info().title() );
  QString termstr;
  if ( 0 != mapdocument() ) {
    termstr = mapdocument()->date().toString("yyyy.MM.dd_hh:mm");
  }
  if ( true == termstr.isEmpty() ) {
    termstr = QDateTime::currentDateTime().toString("yyyy.MM.dd_hh:mm");
  }
  if ( false == txt.isEmpty() ) {
    return txt + '_' + termstr;
  }
  return kEmptyDefaultName + '_' + termstr;
}

  //! Сброс выделения и обновление значений углов документа при смене сцены [private slot]
void map::SaveBitmap::slotSceneChanged(MapScene* scene)
{
  Q_UNUSED(scene);
  resetRubber(scene);
}


void map::SaveBitmap::setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect)
{
  Q_UNUSED(nw);
  Q_UNUSED(ne);
  Q_UNUSED(sw);
  Q_UNUSED(se);
  //  var(rect);
  mapBand_ = rect;
  bandChanged();
}



//! Заполнение координат в соответствии с углами документа
void map::SaveBitmap::fillCoordsFromDocument(map::Document* doc /*= 0*/)
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

bool map::SaveBitmap::isRubberPressed() const
{
  return ui_->rubberBtn->isChecked();
}


//! Действрия при завершении рисования рамки
void map::SaveBitmap::rubberFinished()
{
}

//! Сброс выделения и обновление значений углов документа при смене сцены
void map::SaveBitmap::resetRubber(MapScene* scene)
{
  if (0 != scene && 0 != scene->document()) {
    fillCoordsFromDocument(scene->document());
  }
  ui_->rubberBtn->setChecked(false);
  SelectActionIface::removeAction();
  QApplication::restoreOverrideCursor();
}

void map::SaveBitmap::removeRubber()
{
  SelectActionIface::removeAction();
  if (0 != ui_) {
    ui_->rubberBtn->setChecked(false);
  }
  QApplication::restoreOverrideCursor();
}


  //! Обработка нажатия кнопки выделения области
void map::SaveBitmap::slotRubberClicked(bool on)
{
  if (on) {
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    SelectActionIface::createAction( mapview()->mapscene());
  } else {
    SelectActionIface::removeAction();
    QApplication::restoreOverrideCursor();
    fillCoordsFromDocument();
  }
}

//!
void map::SaveBitmap::bandChanged()
{
  Document* original = currentDocument();
  if (0 == original) return;

  int w = mapBand_.width() * ui_->mapScale->value() + 2*original->ramkaWidth();
  int h = mapBand_.height() * ui_->mapScale->value() + 2*original->ramkaWidth();
  ui_->imgSize->setText(QString::number(w) + " x " + QString::number(h));
}

void map::SaveBitmap::slotSave()
{
  if (save() && ui_->closeFlag->checkState() == Qt::Checked) {
      close();
  }
}

bool map::SaveBitmap::save()
{
  Document* original = currentDocument();
  if ( nullptr == original ) {
    return false;
  }


  QRect targetBand = QRect(0, 0, mapBand_.width() * ui_->mapScale->value() + 2*original->ramkaWidth(),
			   mapBand_.height() * ui_->mapScale->value() + 2*original->ramkaWidth());


  int rw = original->ramkaWidth();
  float  scale = ui_->mapScale->value();


  QSize docSize(mapBand_.width(), mapBand_.height());
  map::Document* doc = original->stub( original->screen2coord(mapBand_.center()), docSize );
  if ( nullptr == doc ) {
    return false;
  }

  QPoint tl = doc->documentTopLeft();
  GeoPoint geo_tl = doc->screen2coord(tl);
  //  var(scale);
  double scaleKoef = 1.0/( qPow( 2.0, doc->scale() ) );
  doc->setScale(log2(1.0/(scale*scaleKoef)));
  //  doc->setScale(doc->scale() + log2(1/scale));

  doc->resizeDocument(targetBand.size());

  QPoint newtl = doc->coord2screen(geo_tl);
  QPoint deltapos = tl - newtl;
  QPoint scrcntr = doc->coord2screen(doc->screenCenter());
  doc->setScreenCenter( scrcntr - (deltapos + doc->documentRect().topLeft() + QPoint(rw, rw)));
  //  doc->setOffCenterMap( QPoint(0,0) );

  doc->ramka()->calcRamka(0, 0);

  QStringList uuids = layers_->checkedLayers(doc);

  //  QImage img(targetBand.width(), targetBand.height(), QImage::Format_ARGB32_Premultiplied);
  QPixmap img(targetBand.size());
  img.fill(doc->backgroundColor());

  QPainter painter;
  painter.begin(&img);

  QRect source(doc->documentRect());
  doc->drawDocument(&painter, uuids, QRect(0, 0, img.width(), img.height()),
		    source);

  painter.end();

  delete doc;

  bool ok = true;

  //QString ext = "bmp";

  QString fileName = ui_->lefile->text();
  if (!fileName.isEmpty()) {
    // if (!fileName.contains(QRegExp("[\\w/]+[.][\\w]{3,4}$"))) {
    // 	fileName += ".bmp";
    // }
    //ext = fileName.section('.', -1);

    fileName += "." + ui_->dbext->currentText().toLower();
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);

    QFileInfo fi(fileName);
    QDir dir;
    dir.mkpath(fi.dir().path());

    if (dir.exists(fileName)) {
      if (QMessageBox::Yes !=
	  QMessageBox::question(this,
				QString::fromUtf8("Заменить файл?"),
				QString::fromUtf8("Файл с именем \"%1\" уже существует. Заменить его?").arg(fi.fileName()))) {
	return false;
      }
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ok = img.save(fileName);
    QApplication::restoreOverrideCursor();
    if (!ok) {
      error_log << QObject::tr("Не удалось записать файл %1").arg(fileName);
      QMessageBox::critical(this, QObject::tr("Ошибка сохранения"),
			    QObject::tr("Не удалось записать файл %1").arg(fileName));
    } else {
      info_log << msglog::kFileWriteSuccess.arg(fileName);
    }
    
  } else {
    QMessageBox::information(this, QObject::tr("Сохранение в файл"),
      		       QObject::tr("Необходимо ввести название файла"));
    return false;
  }

  saveDefaultDir(defaultDir_);

  return ok;
}

void map::SaveBitmap::chooseFile()
{
  QString caption = QString::fromUtf8("Выбор файла для сохранения");

  // QString filter = QString::fromUtf8("Файл BMP (*.bmp);;"
  // 				     "Файл JPEG (*.jpg);;"
  // 				     "Файл PNG (*.png);;"
  // 				     "Файл TIFF (*.tiff)");

  // QString selectedFilter;
  QString fileName = FileDialog::getSaveFileName(this, caption,
						 ui_->lefile->text()// ,
						 // filter, &selectedFilter
						 );

  if (!fileName.isEmpty()) {
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);

    // QRegExp rx("\\w\\s[(*]{2}(.\\w{3,4})");
    // rx.indexIn(selectedFilter);

    // selectedFilter = rx.cap(1);

    // if (fileName.endsWith(selectedFilter) == false) {
    //   fileName += QString(selectedFilter);
    // }
    ui_->lefile->setText(fileName);
  }
}


void map::SaveBitmap::saveDefaultDir(const QString& dir) const
{
  meteo::map::exportmap::ExportPath p;
  p.set_path(dir.toStdString());

  QString text = TProtoText::toText(p);
  if (text.isEmpty()) return;


  QFile file(DEFAULT_DIR);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
}

QString map::SaveBitmap::readDefaultDir()
{
  QString dir = QDir::homePath();

  QFile file(DEFAULT_DIR);
  if ( !file.open(QIODevice::ReadOnly) ) {
    return dir;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  meteo::map::exportmap::ExportPath p;
  if ( !TProtoText::fillProto(text, &p) ) {
    file.remove(DEFAULT_DIR);
    return dir;
  }
  savefs_ = p.save_fs();
  savedb_ = p.save_db();

  return QString::fromStdString(p.path());
}
