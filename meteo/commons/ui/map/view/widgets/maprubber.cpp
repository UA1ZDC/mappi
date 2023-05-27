#include "maprubber.h"
#include "ui_maprubber.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/mapscene.h>

using namespace meteo;
using namespace map;


MapRubber::MapRubber(MapView* view, QWidget* parent, const QString& acceptText):
  QWidget(parent),
  ui_(0),
  view_(view),
  acceptText_(acceptText)
{
  ui_ = new Ui::MapRubber;
  ui_->setupUi(this);
 
  connect(ui_->rubberBtn, SIGNAL(clicked(bool)), SLOT(slotRubberClicked(bool)));
  connect(ui_->topLeft, SIGNAL(changed()), SLOT(slotMajorCoordsEdited()));
  connect(ui_->topRight, SIGNAL(changed()), SLOT(slotMinorCoordsEdited()));
  connect(ui_->bottomRight, SIGNAL(changed()), SLOT(slotMajorCoordsEdited()));
  connect(ui_->bottomLeft, SIGNAL(changed()), SLOT(slotMinorCoordsEdited())); 
}

MapRubber::~MapRubber()
{
  if (0 == ui_) {
    delete ui_;
    ui_ = 0;
  }
}

//! Установка углов рамки
/*! 
  \param tl - верхний левый
  \param tr - верхний правый
  \param br  - нижний правый
  \param bl  - нижний левый
  \param rect  - прямоуголик в экранных координатах
*/
void MapRubber::setPoints(const GeoPoint& tl, const GeoPoint& tr, const GeoPoint& br, 
			  const GeoPoint& bl, const QRect& rect)
{
  band_ = rect;

  disconnect(ui_->topLeft, SIGNAL(changed()), this,  SLOT(slotMajorCoordsEdited()));
  disconnect(ui_->topRight, SIGNAL(changed()), this, SLOT(slotMinorCoordsEdited()));
  disconnect(ui_->bottomRight, SIGNAL(changed()), this, SLOT(slotMajorCoordsEdited()));
  disconnect(ui_->bottomLeft, SIGNAL(changed()), this, SLOT(slotMinorCoordsEdited())); 
  
  ui_->topLeft->setCoord(tl);
  ui_->topRight->setCoord(tr);
  ui_->bottomRight->setCoord(br);
  ui_->bottomLeft->setCoord(bl);

  connect(ui_->topLeft, SIGNAL(changed()), SLOT(slotMajorCoordsEdited()));
  connect(ui_->topRight, SIGNAL(changed()), SLOT(slotMinorCoordsEdited()));
  connect(ui_->bottomRight, SIGNAL(changed()), SLOT(slotMajorCoordsEdited()));
  connect(ui_->bottomLeft, SIGNAL(changed()), SLOT(slotMinorCoordsEdited())); 
}

//! Получение углов рамки
/*! 
  \param tl - верхний левый
  \param tr - верхний правый
  \param br  - нижний правый
  \param bl  - нижний левый
  \param rect  - прямоуголик в экранных координатах
*/
void MapRubber::getPoints(GeoPoint* tl, GeoPoint* tr, GeoPoint* br, GeoPoint* bl, QRect* rect)    
{
  if (0 == tl || 0 == tr || 0 == br || 0 == bl || 0 == rect) {
    return;
  }

  *tl = ui_->topLeft->coord();
  *tr = ui_->topRight->coord();
  *br = ui_->bottomRight->coord();
  *bl = ui_->bottomLeft->coord();
  *rect = band_;
}

bool MapRubber::isRubberPressed() const
{
  return ui_->rubberBtn->isChecked();
}

//! Действрия при завершении рисования рамки
void MapRubber::rubberFinished()
{
}

//! Сброс выделения и обновление значений углов документа при смене сцены
void MapRubber::resetRubber(MapScene* scene)
{
  if (0 != scene && 0 != scene->document()) {
    fillCoordsFromDocument(scene->document());
  }
  ui_->rubberBtn->setChecked(false);
  SelectActionIface::removeAction();
  QApplication::restoreOverrideCursor();
}

void MapRubber::removeRubber()
{
  SelectActionIface::removeAction();
  ui_->rubberBtn->setChecked(false);
  QApplication::restoreOverrideCursor();
}

void MapRubber::acceptRubber()
{
  emit accept();
}

//! Заполнение координат в соответствии с углами документа
void MapRubber::fillCoordsFromDocument(map::Document* doc /*= 0*/)
{
  map::Document* document = 0;
  if (0 != doc) {
    document = doc;
  } else {
    document = currentDocument();
  }

  if (0 != document) {
    ui_->topLeft->setCoord(document->screen2coord(document->documentTopLeft()));
    ui_->topRight->setCoord(document->screen2coord(document->documentTopRight()));
    ui_->bottomRight->setCoord(document->screen2coord(document->documentBottomRight()));
    ui_->bottomLeft->setCoord(document->screen2coord(document->documentBottomLeft()));
    
    band_ = document->documentRect();
  }
}

  //! Обработка нажатия кнопки выделения области
void MapRubber::slotRubberClicked(bool on)
{
  if( true == on ) {
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    SelectActionIface::createAction(view_->mapscene());
  } else {
    SelectActionIface::removeAction();
    QApplication::restoreOverrideCursor();
    fillCoordsFromDocument();
  }
}


//! Перерисовка рамки при изменнении верхнего левого или нижнего правого углов [private slot]
void MapRubber::slotMajorCoordsEdited()
{ 
  if (isRubberPressed()) {
    SelectActionIface::setMajorCoords(ui_->topLeft->coord(), ui_->bottomRight->coord());
  }
}

//! Перерисовка рамки при изменнении верхнего левого или нижнего правого углов [private slot]
void MapRubber::slotMinorCoordsEdited()
{
  if (isRubberPressed()) {
    SelectActionIface::setMajorCoords(ui_->topRight->coord(), ui_->bottomLeft->coord());
  }
}


::meteo::map::Document* MapRubber::currentDocument() const
{

  if (view_ != 0 &&
      view_->mapscene() != 0 &&
      view_->mapscene()->document() != 0) {
    return view_->mapscene()->document();
  }

  return 0;
}

