#include "savesxf.h"
#include "ui_exportdoc.h"
#include "maprubber.h"
#include "layersselect.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/ui/map/dataexchange/metasxf.h>
#include <meteo/commons/ui/map/dataexchange/sxfexchange.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/ramka.h>

#include <qmap.h>
#include <qtreewidget.h>
#include <qlayout.h>

#define METEO_RSC_FILE QString(MnCommon::projectPath() + meteo::sxf::MetaSxf::instance()->rscfile())
static const QString kEmptyDefaultName = QObject::tr("Пустой документ");

namespace {

  const QString defaultSaveDirConf() { return QDir::homePath() + "/.meteo/" + "exportmap.conf"; }

static const TMap<int,QString> projections = TMap<int,QString>()
<< QPair<int,QString>(1,  QObject::tr("Равноугольная Гаусса-Крюгера"))
<< QPair<int,QString>(5,  QObject::tr("Азимутальная прямая равноугольная (стереографическая)"))
<< QPair<int,QString>(6,  QObject::tr("Азимутальная прямая равнопромежуточная (Постеля)"))
<< QPair<int,QString>(12, QObject::tr("Псевдоконическая произвольная проекция"))
<< QPair<int,QString>(13, QObject::tr("Стереографическая полярная"))
<< QPair<int,QString>(14, QObject::tr("Равноугольная проекция (Чебышева)"))
<< QPair<int,QString>(15, QObject::tr("Гномонимическая проекция (центральная точка 60,80 градусов)"))
<< QPair<int,QString>(16, QObject::tr("Цилиндрическая специальная для бланковой карты"))
<< QPair<int,QString>(17, QObject::tr("Универсальная поперечная проекция Меркатора (UTM)"))
<< QPair<int,QString>(18, QObject::tr("0псевдоцилиндрическая равновеликая синусоидальная проекция Каврайского"))
<< QPair<int,QString>(19, QObject::tr("Псевдоцилиндрическая равновеликая эллиптическая проекция Мольвейде"))
<< QPair<int,QString>(20, QObject::tr("Прямая равнопромежуточная коническая проекция"))
<< QPair<int,QString>(21, QObject::tr("Прямая равновеликая коническая проекция"))
<< QPair<int,QString>(22, QObject::tr("Прямая равноугольная коническая проекция"))
<< QPair<int,QString>(23, QObject::tr("Полярная равноугольная азимутальная (стереографическая) проекция"))
<< QPair<int,QString>(25, QObject::tr("Псевдоцилиндрическая синусоидальная проекция Урмаева"))
<< QPair<int,QString>(26, QObject::tr("Производная равновеликая проекция Аитова-Гамера"))
<< QPair<int,QString>(27, QObject::tr("Равнопромежуточная цилиндрическая проекция"))
<< QPair<int,QString>(28, QObject::tr("Равновеликая цилиндрическая проекция Ламберта"))
<< QPair<int,QString>(29, QObject::tr("Видоизмененная простая поликоническая проекция (международная)"))
<< QPair<int,QString>(30, QObject::tr("Косая равновеликая азимутальная проекция Ламберта"))
<< QPair<int,QString>(31, QObject::tr("Равноугольная поперечно-цилиндрическая проекция"))
<< QPair<int,QString>(32, QObject::tr("Равноугольная топографическая для Системы координат 63 года"))
<< QPair<int,QString>(33, QObject::tr("Широта/долгота цилиндрическая на шаре"))
<< QPair<int,QString>(34, QObject::tr("Цилиндрическая Миллера на шаре ESRI:54003"))
<< QPair<int,QString>(35, QObject::tr("Цилиндрическая прямая равноугольная Меркатора EPSG:3857/3395"))
<< QPair<int,QString>(36, QObject::tr("Цилиндрическая прямая равноугольная Меркатора (Mercator 2SP)"));

}

namespace meteo {
namespace map {

SaveSxf::SaveSxf( MapWindow* parent) :
  MapWidget(parent),
  ui_(new Ui::ExportDoc()),
  rubber_(0),
  layers_(0),
  //saver_(0),
  watchedEventHandler_(0),
  savefs_(true),
  savedb_(true)
{
  ui_->setupUi(this);
  //  ui_->warning->hide();
  ui_->groupBox->hide();

  setPixmap(QPixmap(":/meteo/icons/map/save.png"));

  rubber_ = new MapRubber(mapview(), this, QObject::tr("Экспорт"));
  QGridLayout* gridLayout = new QGridLayout(ui_->wrubber);
  gridLayout->addWidget(rubber_, 0, 0, 1, 1);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  layers_ = new LayersSelect(mapview(), this);
  QGridLayout* gridLayout2 = new QGridLayout(ui_->wlayers);
  gridLayout2->addWidget(layers_, 0, 0, 1, 1);
  gridLayout2->setContentsMargins(0, 0, 0, 0);

  layers_->loadLayers();
  layers_->setCheckedNonBaseLayers(true);
  rubber_->fillCoordsFromDocument();

  defaultDir_ = readDefaultDir();
  setDefaultName();
  // QString title = QString::fromStdString(mapdocument()->info().title());
  // var(title);
  // if (0 != mapdocument() && !title.isEmpty()) {
  //   ui_->lefile->setText(defaultDir_ + title + "_" + mapdocument()->dateString());
  // }

  connect(ui_->applyPushButton, SIGNAL(clicked()), SLOT(slotAccept()));
  connect(ui_->cancelPushButton, SIGNAL(clicked()), SLOT(slotCancel()));
  connect(ui_->btnfile, SIGNAL(clicked()), SLOT(getSaveFileName()));
  connect(rubber_, SIGNAL(accept()), SLOT(slotAccept()));
  connect(ui_->onlyvis, SIGNAL(stateChanged(int)), SLOT(slotOnlyVis(int)));
  
  installWatchedDocument();

  basis_ = QSharedPointer<sxf::PassportBasis>(new sxf::PassportBasis());

  // if ( currentDocument()->projection()->type() == MERCAT &&
  //      !qFuzzyCompare(currentDocument()->projection()->getMapCenter().lon(), 0)) {
  //   ui_->warning->show();
  // }

}

SaveSxf::~SaveSxf()
{
  resetWatchedDocument();

  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}

void SaveSxf::resetWatchedDocument()
{
  if (watchedEventHandler_ != 0) {
    this->removeEventFilter(watchedEventHandler_);
    watchedEventHandler_ = 0;
  }
  if (0 != mapview()) {
    disconnect( mapview(), SIGNAL(sceneChanged(MapScene*)), this, SLOT(slotSceneChanged(MapScene*)));
  }
}

bool SaveSxf::installWatchedDocument()
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

  Document* SaveSxf::currentDocument() const
  {
    if (mapview() != 0 &&
	mapview()->mapscene() != 0 &&
	mapview()->mapscene()->document() != 0) {
      return mapview()->mapscene()->document();
    }

    return 0;
  }


  //! Сброс выделения и обновление значений углов документа при смене сцены [private slot]
void SaveSxf::slotSceneChanged(MapScene* scene)
{
  rubber_->resetRubber(scene);
}

 
  //! Вычисление текущего масштаба
int SaveSxf::currentScale()
{
  map::Document* original = currentDocument();

  if (0 == original) {
    return 0;
  }

  QPoint leftCenter;
  QPoint rightCenter;

  if (basis_->northWest.isValid() && basis_->northEast.isValid()) {
    leftCenter.setX(basis_->band.left());
    leftCenter.setY(basis_->band.center().y());
    rightCenter.setX(basis_->band.right());
    rightCenter.setY(basis_->band.center().y());
  } else {
    int y = original->documentBottomLeft().y() -
      original->documentTopLeft().y();
    leftCenter.setX(original->documentTopLeft().x());
    leftCenter.setY(y);
    rightCenter.setX(original->documentTopRight().x());
    rightCenter.setY(y);
  }

  int scale =  original->mapScaleDenominatorMeters(leftCenter, rightCenter);

  //TODO. временно. пока нет способа спросить у документа
  QList<int> scaleList;
  scaleList <<  100 << 200 << 500 << 1000 << 2000
	    << 2500 << 5000 << 10000 << 25000 << 50000
	    << 100000 << 200000 << 500000 << 1000000 << 2000000
	    << 2500000 << 4000000 << 5000000 << 10000000;
  int prev = scaleList.at(0);
  int idx = 0;
  int resScale = scaleList.last();
  for (idx = 1; idx < scaleList.count(); idx++) {
      int cur = scaleList.at(idx);
      if (scale >= prev && scale <= cur) {
	if (scale - prev > cur - scale) {
	  resScale = scaleList.at(idx);
	} else {
	  resScale = scaleList.at(idx - 1);
	}
	break;
      }
      prev = cur;
    }
  return resScale;
}


bool SaveSxf::eventFilter(QObject* watched, QEvent* event)
{
  // var(event->type());
  // debug_log << watched << watchedEventHandler_;
  if (watched == watchedEventHandler_) {
    if (event->type() == map::LayerEvent::LayerChanged) {
      LayerEvent* le = static_cast<LayerEvent*>(event);
      if (0 != le && (map::LayerEvent::Deleted == le->changeType() || 
		      map::LayerEvent::Added == le->changeType())) {
	layers_->loadLayers();
	layers_->setCheckedNonBaseLayers(true);
      } else if (0 != le && (map::LayerEvent::Visibility == le->changeType())) {
	slotOnlyVis(ui_->onlyvis->checkState());
      }
    }  else if (event->type() == map::MapEvent::MapChanged) {
      if (! rubber_->isRubberPressed()) {
	rubber_->fillCoordsFromDocument();
      }
    }
  }

  return MapWidget::eventFilter(watched, event);
}

void SaveSxf::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    rubber_->removeRubber();
    MapWidget::keyPressEvent(e);
  }
}

void SaveSxf::closeEvent(QCloseEvent*e)
{
  rubber_->removeRubber();
  ui_->lefile->clear();
  MapWidget::closeEvent(e);
}

void SaveSxf::showEvent(QShowEvent* e)
{
  layers_->loadLayers();
  layers_->setCheckedNonBaseLayers(true);
  slotOnlyVis(ui_->onlyvis->checkState());
  if (!rubber_->isRubberPressed()) {
    rubber_->fillCoordsFromDocument();
  }
  
  installWatchedDocument();
  setDefaultName();  

  MapWidget::showEvent(e);
}

void map::SaveSxf::setDefaultName()
{
  QString txt = defaultDir_ + '/' + mapname();
  while ( -1 != txt.indexOf("//") ) {
    txt.replace("//","/");
  }

  ui_->lefile->setText(txt);
}

QString map::SaveSxf::mapname() const
{
  QString txt = ui_->lefile->text();
  QFileInfo fi(txt);
  // if ( false == fi.isDir() ) {
  //   txt = fi.fileName();
  //   if ( false == txt.isEmpty() ) {
  //     if ( !txt.contains( QRegExp("[\\w\\s/]+_\\d{2,4}[.]\\d{2}[.]\\d{2,4}.\\d{2}:\\d{2}[.]\\w{3,4}$") ) ) {
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

  QString name;

  if ( false == txt.isEmpty() ) {
    name = txt + '_' + termstr;
  } else {
    name = kEmptyDefaultName + '_' + termstr;
  }
  name=meteo::global::kTranslitFunc(name);
  return name + ".sxf";
}

void SaveSxf::slotAccept()
{
  map::Document* original = currentDocument();

  if (original == 0) {
    error_log << QString::fromUtf8("Ошибка. Документ не инициализирован");
    return;
  }

  
  if (makePassportBasis()) {
    QStringList layerUuids = layers_->checkedLayers(original);
    if (layerUuids.isEmpty()) {
      QMessageBox::information(this,
			       QString::fromUtf8("Сохранение"),
			       QString::fromUtf8("Необходимо выбрать слои для сохранения"),
			       QMessageBox::Ok);

      return;
    }

    if (execute(layerUuids, *original) == true) {
      slotCancel();
    }
  } else {
    error_log << QString::fromUtf8("Ошибка. Документ не инициализирован");
  }
}

void SaveSxf::slotOnlyVis(int state)
{
  if (state == Qt::Checked) {
    layers_->showUnvisibleLayers(false);
  } else {
    layers_->showUnvisibleLayers(true);
  }
}

bool SaveSxf::makePassportBasis()
{
  if (mapview() == 0 ||
      mapview()->mapscene() == 0 ||
      mapview()->mapscene()->document() == 0 ||
      mapview()->mapscene()->document()->projection() == 0) {
    return false;
  }

  //полярная стереографическая
  // basis_->scale = currentScale();
  // basis_->projection = 23;
  // basis_->mainPoint = GeoPoint(90*M_PI / 180.0, mapview()->mapscene()->document()->center().lon());
  // basis_->firstMajorParallel = 90*M_PI / 180.0;
  // basis_->secondMajorParallel = 0;
  // basis_->northOffset = 0;
  // basis_->eastOffset = 0;

  // var(mapview()->mapscene()->document()->center());
  // var(mapview()->mapscene()->document()->screenCenter());

  // basis_->northWest = uiRubber_->topLeft->coord();
  // basis_->northEast = uiRubber_->topRight->coord();
  // basis_->southEast = uiRubber_->bottomRight->coord();
  // basis_->southWest = uiRubber_->bottomLeft->coord();
  switch (mapview()->mapscene()->document()->projection()->type()) {
  case STEREO:
    basis_->projection = 23;
    break;
  case MERCAT:
    basis_->projection = 36;
    break;
  case CONICH:
    basis_->projection = 20;
    break;
  default: {
    basis_->projection = 36;
  }
  }


  rubber_->getPoints(&basis_->northWest, &basis_->northEast, &basis_->southEast, 
		     &basis_->southWest, &basis_->band);

  basis_->scale = currentScale();
  basis_->mainPoint = mapview()->mapscene()->document()->projection()->getMapCenter();
  basis_->firstMajorParallel = mapview()->mapscene()->document()->projection()->getMapCenter().lat();
  // if (basis_->projection == 36) {
  //   basis_->mainPoint = mapview()->mapscene()->document()->center(); //GeoPoint(0, mapview()->mapscene()->document()->center().lon());
  //   basis_->firstMajorParallel = mapview()->mapscene()->document()->center().lat();//0;
  // } else {
  //   basis_->mainPoint = GeoPoint(mapview()->mapscene()->document()->center().lat(), 0);
  //   basis_->firstMajorParallel = 0;
  // }
  basis_->secondMajorParallel = 0;
  basis_->northOffset = 0;
  basis_->eastOffset = 0;


  if (basis_->projection == 36) {
    checkMercatorBorders();
    basis_->mainPoint = GeoPoint(0, 0);
    basis_->firstMajorParallel = 0;
  }


  // debug_log << "northWest" <<  basis_->northWest.latDeg() << basis_->northWest.lonDeg();
  // debug_log << "northEast" <<  basis_->northEast.latDeg() << basis_->northEast.lonDeg();
  // debug_log << "southWest" <<  basis_->southWest.latDeg() << basis_->southWest.lonDeg();
  // debug_log << "southEast" <<  basis_->southEast.latDeg() << basis_->southEast.lonDeg();
  // debug_log << "scale" << basis_->scale;
  // debug_log << "projection" << basis_->projection;
  // debug_log << "mainPoint" << basis_->mainPoint.latDeg() << basis_->mainPoint.lonDeg();
  // debug_log << "firstMajorParallel" << basis_->firstMajorParallel;
  // debug_log << "secondMajorParallel" << basis_->secondMajorParallel;
  // debug_log << "northOffset" << basis_->northOffset;
  // debug_log << "eastOffset" << basis_->eastOffset;

  return true;
}

void SaveSxf::checkMercatorBorders()
{
  // basis_->northWest = GeoPoint(MnMath::deg2rad(85), MnMath::deg2rad(-180));
  // basis_->northEast = GeoPoint(MnMath::deg2rad(85), MnMath::deg2rad(180));
  // basis_->southEast = GeoPoint(MnMath::deg2rad(-85), MnMath::deg2rad(180)); 
  // basis_->southWest = GeoPoint(MnMath::deg2rad(-85), MnMath::deg2rad(-180));

  Document* current = currentDocument();
  if (0 == current && 0 != current->ramka()) return;

  QPolygon ramka = current->ramka()->calcRamka(2, 2);
  QRect rect = ramka.boundingRect();
  GeoPoint tl = current->screen2coord(rect.topLeft());
  GeoPoint br = current->screen2coord(rect.bottomRight());

  // debug_log << rect.topLeft() << rect.bottomRight();
  // var(tl);
  // var(br);

  if (!rubber_->isRubberPressed()) {
    basis_->northWest = tl;
    basis_->northEast = GeoPoint(tl.lat(), br.lon());
    basis_->southEast = br; 
    basis_->southWest = GeoPoint(br.lat(), tl.lon());
  }
}


void SaveSxf::slotCancel()
{
  //resetWatchedDocument();
  rubber_->removeRubber();
  MapWidget::close();
}


  bool SaveSxf::execute( const QStringList& layerUuids, const map::Document& doc)
{
  bool ok = true;

  //проверка файла
  QString filename = ui_->lefile->text();
  if (filename.isEmpty() == false) {
    if (filename.endsWith(".sxf") == false) {
        filename += QString(".sxf");
    }
  }
  
  QFileInfo fi(filename);
  QDir dir;
  dir.mkpath(fi.dir().path());
  
  if (dir.exists(filename)) {
    if (QMessageBox::Yes !=
	QMessageBox::question(this,
			      QString::fromUtf8("Заменить файл?"),
			      QString::fromUtf8("Файл с именем \"%1\" уже существует. Заменить его?").arg(fi.fileName()))) {
      return false;
    }
  }
  
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  //создание sxf
  Sxf sxf;
  sxf.setPassportBasis(basis_);
  sxf.setCoordType(ui_->degreeFlag->isChecked());
  QString errorMessage;
  QByteArray content(sxf.exportDocument(layerUuids, doc, ui_->onlyvis->checkState() == Qt::Checked));
  QApplication::restoreOverrideCursor();

  //сохранение в файл
  if (sxf.lastError().isEmpty() == true) {
    
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
    QFile f(filename);
    if (f.open(QIODevice::WriteOnly) == true) {
      f.write(content);
      f.close();
      
      info_log << msglog::kFileWriteSuccess.arg(filename);
      // QMessageBox::information(this,
      //                          QString::fromUtf8("Сохранение"),
      //                          QString::fromUtf8("Файл %1 успешно сохранён").arg(f.fileName()),
      //                          QMessageBox::Ok);
      ok = true;
    }
    else {
      ok = false;
      errorMessage = f.errorString();
    }
    QApplication::restoreOverrideCursor();
    
  }
  else {
    errorMessage = sxf.lastError();
    QMessageBox::critical(this,
			  QString::fromUtf8("Ошибка сохранения"),
			  errorMessage,
			  QMessageBox::Cancel);
  }


  //  var(ok);

  return ok;
}

void SaveSxf::getSaveFileName()
{
  QString caption = QString::fromUtf8("Сохранение карты");
  QString filter = QString::fromUtf8("Файл в формате SXF (*.sxf)");

  QString fileName = FileDialog::getSaveFileName(this, caption,
						 ui_->lefile->text(),
						 filter);

  if (!fileName.isEmpty()) {
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);
    saveDefaultDir(defaultDir_);
    ui_->lefile->setText(fileName);
  }
}

  QString SaveSxf::readDefaultDir() 
  {
    QString dir = QDir::homePath();

    QFile file(::defaultSaveDirConf());
    if ( !file.open(QIODevice::ReadOnly) ) {
      return dir;
    }
    
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    meteo::map::exportmap::ExportPath p;
    if ( !TProtoText::fillProto(text, &p) ) {
      file.remove(::defaultSaveDirConf());
      return dir;
    }

    savefs_ = p.save_fs();
    savedb_ = p.save_db();

    return QString::fromStdString(p.path());
  }

  void SaveSxf::saveDefaultDir(const QString& dir)
  {
    meteo::map::exportmap::ExportPath p;
    p.set_path(dir.toStdString());
    p.set_save_fs(savefs_);
    p.set_save_db(savedb_);

    QString text = TProtoText::toText(p);
    if (text.isEmpty()) return;

    
    QFile file(::defaultSaveDirConf());
    if ( !file.open(QIODevice::WriteOnly) ) {
      return;
    }
    file.write(text.toUtf8());
    file.close();
    
  }


} // map
} // meteo


/*
[D] [21:02] [meteo.map.view] widgets/maprubber.cpp:183 667 500 1 
[D] [21:02] [meteo.map.view] widgets/maprubber.cpp:183 883 1000 1.4 
[D] [21:02] [meteo.map.view] widgets/maprubber.cpp:183 1538 2000 2.2 
[D] [21:03] [meteo.map.view] widgets/maprubber.cpp:183 2678 2500 3 
5т - 3,2
10т - 4,4
25т - 5,6
50т = 6.4
100т - 7,6
200т - 8,4
500т - 9,6
1млн - 10,8
2млн - 12
5млн - 12.8
10млн - 14
40млн - 15.6



*/

