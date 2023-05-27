#include "drawtoolsaction.h"

#include "controlpanelpixmapwidget.h"
#include "controlpaneltextwidget.h"
#include "drawtoolbarwidget.h"
#include "polygonpropwidget.h"
#include "pixmappropwidget.h"
#include "frontpropwidget.h"
#include "textpropwidget.h"
#include "propwidget.h"
#include "undoredo.h"
#include "pixmaptoolgraphicsitem.h"
#include "pixmapdrawobject.h"
#include "geographtextitem.h"
#include "polygondrawobject.h"
#include "highlightobject.h"
#include "groupdrawobject.h"
#include "textdrawobject.h"
#include "groupobject.h"
#include "drawobject.h"


#include <cross-commons/debug/tlog.h>
#include <commons/obanal/interpolorder.h>

#include <commons/textproto/pbtools.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/objectmimedata.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/menu.h>

#include <qgraphicsproxywidget.h>
#include <qgraphicsscene.h>
#include <qgraphicsitem.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qevent.h>
#include <qtimer.h>


namespace meteo {
namespace map {

//! Расстояние пройденое курсором между событиями press и release, для исключения
//! ложного срабатывания события mouseClickEvent(), при смещении указателя на
//! большое расстояние.
static QLineF gMoveDistance;
const qreal kLength = 30.0;


DrawToolsAction::DrawToolsAction(MapScene* scene)
  : Action(scene, kDrawToolsActionName)
{
  clipboard_ = QApplication::clipboard();
  actBtn_ = new ActionButton();
  actBtn_->setToolTip(QObject::tr("Рисование"));
  toolbar_ = new DrawToolbarWidget( actBtn_ );
  actBtn_->setIcon(QIcon(":/meteo/icons/map/paint.png"));
  widgetdraw_ = scene_->addWidget(actBtn_);
  item_ = widgetdraw_;
  item_->setZValue(10001);
  scene->addAction(this, kTopLeft, QPoint(0,3));
  drawPanel_ = new QGraphicsProxyWidget(item_);
  drawPanel_->setWidget(toolbar_);
  drawPanel_->setPos(0,32);
  drawPanel_->hide();

  lineProp_ = new PolygonPropWidget( PolygonPropWidget::kLine, scene->mapview()->window() );
  frontProp_ = new FrontPropWidget( scene->mapview()->window() );
  polygonProp_ = new PolygonPropWidget( PolygonPropWidget::kPolygon, scene->mapview()->window() );
  textProp_ = new TextPropWidget( scene->mapview()->window() );
  pixmapProp_ = new PixmapPropWidget( scene->mapview()->window() );

  linePW_ = new QGraphicsProxyWidget(drawPanel_);
  linePW_->setWidget(lineProp_);
  lineProp_->setProxyWidget(linePW_);
  linePW_->setPos(30,0);
  linePW_->hide();

  textPW_ = new QGraphicsProxyWidget(drawPanel_);
  textPW_->setWidget(textProp_);
  textProp_->setProxyWidget(textPW_);
  textPW_->setPos(30,30*3);
  textPW_->hide();
  frontPW_ = new QGraphicsProxyWidget(drawPanel_);
  frontPW_->setWidget(frontProp_);
  frontProp_->setProxyWidget(frontPW_);
  frontPW_->setPos(30,30*2);
  frontPW_->hide();
  polygonPW_ = new QGraphicsProxyWidget(drawPanel_);
  polygonPW_->setWidget(polygonProp_);
  polygonProp_->setProxyWidget(polygonPW_);
  polygonPW_->setPos(30,30);
  polygonPW_->hide();
  pixmapPW_ = new QGraphicsProxyWidget(drawPanel_);
  pixmapPW_->setWidget(pixmapProp_);
  pixmapProp_->setProxyWidget(pixmapPW_);
  pixmapPW_->setPos(30,30*4);
  pixmapPW_->hide();
  timer_ = new QTimer(this);
  timer_->setInterval(3000);
  timer_->setSingleShot(true);
  QObject::connect( actBtn_, SIGNAL(clicked(bool)), SLOT(slotSwitch()) );
  toolbar_->setPalette( actBtn_->palette() );

  QList<PropWidget*> list;
  list << lineProp_ << frontProp_ << polygonProp_ << textProp_ << pixmapProp_;
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    QObject::connect( list.at(i), SIGNAL(valueChanged()), SLOT(slotApplyProp()) );
    QObject::connect( list.at(i), SIGNAL(finish()), SLOT(slotFinishEditing()) );
    QObject::connect( list.at(i), SIGNAL(deleteCurrent()), SLOT(slotDeleteEditing()) );
    QObject::connect( list.at(i), SIGNAL(closed()), SLOT(slotFinishDraw()) );
    QObject::connect( list.at(i), SIGNAL(enter()), SLOT(slotOffActions()) );
    QObject::connect( list.at(i), SIGNAL(leave()), SLOT(slotOnActions()) );
    QObject::connect( this, SIGNAL(deleteBtnOn()), list.at(i), SLOT(slotOnDelBtn()));
    QObject::connect( this, SIGNAL(deleteBtnOff()), list.at(i), SLOT(slotOffDelBtn()));
  }

  QObject::connect( toolbar_, SIGNAL(toolChanged(DrawToolbarWidget::Tool)), SLOT(slotToolActivate(DrawToolbarWidget::Tool)) );
  QObject::connect( toolbar_, SIGNAL(undo()), &undoStack_, SLOT(undo()) );
  QObject::connect( toolbar_, SIGNAL(redo()), &undoStack_, SLOT(redo()) );
  QObject::connect( toolbar_, SIGNAL(textHovered()), timer_, SLOT(start()));
  QObject::connect( toolbar_, SIGNAL(textLeave()), timer_, SLOT(stop()));
  QObject::connect( timer_, SIGNAL(timeout()), textProp_, SLOT(show()));
  QObject::connect( textProp_, SIGNAL(leave()), textProp_, SLOT(hide()));
  undoStack_.setUndoLimit(100);
  QString path = pixmapProp_->getPixmapIconPath();
  toolbar_->setPixmapIcon(path);
  editText_ = nullptr;
  QSize size = lineProp_->size();
  size.setHeight(size.height()-48);
  linePW_->resize(size);
  scene_->document()->eventHandler()->installEventFilter(this);
}

DrawToolsAction::~DrawToolsAction()
{
  deleteAllObject();
  delete drawPanel_;
}

void DrawToolsAction::mousePressEvent(QMouseEvent* e)
{
  grabed_ = true;
  if ( !gMoveDistance.isNull() ) {
    gMoveDistance.setP1(e->pos());
  }
  else {
    gMoveDistance = QLine(e->pos(), e->pos());
  }
  if ( false == enabled_ ) {
    return;
  }
  if ( nullptr != editableObject_ ) {
    bool movable = false;
    PolygonDrawObject* polyObj = dynamic_cast<PolygonDrawObject*>(editableObject_);
    if ( nullptr != polyObj ) {
      QLine section = polyObj->skeletSectionAt(scene_->mapview()->mapToScene(e->pos()).toPoint(), 20);
      QList<MarkerItem*> markers = polyObj->skeletMarkersAt(scene_->mapview()->mapToScene(e->pos()).toPoint());
      movable = ( !section.isNull() && markers.size() == 0 );
      if ( state_ == State::kCreate ) {
        setMarkers(polyObj);
      }
    }

    GroupDrawObject* grObj = dynamic_cast<GroupDrawObject*>(editableObject_);
    if ( nullptr != grObj ) {
      QLine section = grObj->skeletSectionAt(scene_->mapview()->mapToScene(e->pos()).toPoint(), 20);
      movable = !section.isNull();
    }
    if ( e->button() == Qt::LeftButton ) {
      editableObject_->setMovable(movable);
    }
  }
}

void DrawToolsAction::mouseReleaseEvent(QMouseEvent* e)
{
  grabed_ = false;
  clearMarkers();
  gMoveDistance.setP2(e->pos());

  if ( false == enabled_ ) {
    return;
  }
  if ( gMoveDistance.length() < 20) {
    mouseClickEvent(e);
  }

  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    drawObjects_[i]->mouseReleaseEvent(e);
  }
  if ( nullptr != editableObject_ ) {
    editableObject_->mouseReleaseEvent(e);
  }

  if ( nullptr != editableObject_ ) {
    if ( editableObject_->isMovable() ) {
      editableObject_->setMovable(false);
    }

    editableObject_->calcSceneCoords();
    editableObject_->update();
  }

  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    drawObjects_[i]->calcSceneCoords();
  }
  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    selectedObjects_[i]->calcSceneCoords();
  }
  if ( nullptr != highlightObj_ ) {
    highlightObj_->calcSceneCoords();
  }
}

void DrawToolsAction::mouseDoubleClickEvent(QMouseEvent* e)
{
  if ( !isWidgetUnderMouse(e->pos()) ) {
    return;
  }

  if ( false == enabled_ ) {
    return;
  }
}

void DrawToolsAction::mouseMoveEvent(QMouseEvent* e)
{
  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    drawObjects_[i]->mouseMoveEvent(e);
  }
  if ( ( false == enabled_ )
       || ( ( currentTool() != DrawToolbarWidget::kCursor )
            && ( currentTool() != DrawToolbarWidget::kGroup ) ) ) {
    return;
  }
  if ( true == grabed_ ) {
    return;
  }
  if ( nullptr == editableObject_ ) {
    Object* obj = getTopParentAt(view()->mapToScene(e->pos()).toPoint());
    if ( nullptr == obj ) {
      if ( nullptr != highlightObj_ ) {
        highlightObj_->removeFromScene();
      }
      delete highlightObj_;
      highlightObj_ = nullptr;
    }
    else {
      highlightObject(obj);
    }
  }
  else {
    editableObject_->mouseMoveEvent(e);
  }
}

void DrawToolsAction::wheelEvent(QWheelEvent* e)
{
  Q_UNUSED( e );
  if ( nullptr != editableObject_ ) {
    editableObject_->calcSceneCoords();
    editableObject_->update();
  }

  if ( false == enabled_ ) {
    return;
  }
  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    drawObjects_[i]->calcSceneCoords();
  }
  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    selectedObjects_[i]->calcSceneCoords();
  }
  if ( nullptr != highlightObj_ ) {
    highlightObj_->calcSceneCoords();
  }
}

void DrawToolsAction::mouseClickEvent(QMouseEvent* e)
{
  if ( widgetdraw_->isUnderMouse() ) {
    // метод mouseReleaseEvent() вызывается до того как состояние кнопки изменится
    widgetdraw_->update();
  }
  if ( isWidgetUnderMouse(e->pos()) ) {
    return;
  }
  if ( false == enabled_ ) {
    return;
  }
  QPoint scenePos = view()->mapToScene(e->pos()).toPoint();
  if ( ( currentTool() == DrawToolbarWidget::kPixmap )
        && ( e->button() == Qt::LeftButton ) ){
    if ( nullptr == editPixmap_ ) {
      editPixmap_ = new PixmapToolGraphicsItem(200);

      QBrush brush(QColor(50, 150, 255));
      QPen pen(brush, 2);
      editPixmap_->setStyle(PixmapToolGraphicsItem::kNormalStyleRole, pen, brush);
      editPixmap_->setStyle(PixmapToolGraphicsItem::kHoverStyleRole, QPen(QBrush(Qt::blue), 2), brush);
      editPixmap_->setStyle(PixmapToolGraphicsItem::kSelectedStyleRole, pen, brush);
      scene_->addItem( editPixmap_ );
      editPixmap_->setGeoPos( scene_->screen2coord(scenePos) );
      editPixmap_->initGeoPixmap();
      PixmapPropWidget* pixmapW = qobject_cast<PixmapPropWidget*>(pixmapProp_);
      editPixmap_->setImage( pixmapProp_->getPixmapIconPath() );
      editPixmap_->setProperty( pixmapProp_->toProperty() );
      editPixmap_->slotSetImgColor(pixmapW->color());
      controlPanel_ = new GeoProxyWidget();
      controlPanelPixmap_ = new ControlPanelPixmapWidget();
      scene_->addItem(controlPanel_);
      scene_->addVisibleItem(controlPanel_);
      controlPanel_->setWidget(controlPanelPixmap_);
      controlPanelPixmap_->setParentProxyWidget(controlPanel_);
      QPointF p = editPixmap_->mapToScene(editPixmap_->boundingRect().topLeft()-QPoint(-32,32));
      controlPanel_->setGeoPos(scene_->document()->screen2coord(p));
      controlPanel_->show();
      controlPanelPixmap_->show();
      pixmapPW_->hide();
      QObject::connect( editPixmap_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()));
      QObject::connect( editPixmap_, SIGNAL(valueChanged(meteo::Property)), pixmapW, SLOT(slotSetProperty(meteo::Property)));
      QObject::connect( editPixmap_, SIGNAL(anchorChanged(GeoPoint)), controlPanelPixmap_, SLOT(slotSetGeoPosProxyWidget(GeoPoint)));
      QObject::connect( pixmapProp_, SIGNAL(colorChanged(QColor)), editPixmap_, SLOT(slotSetImgColor(QColor)) );
      QObject::connect( pixmapProp_, SIGNAL(colorChanged(QColor)), controlPanelPixmap_, SLOT(slotSetColor(QColor)) );
      QObject::connect( pixmapW, SIGNAL(posChanged(Position)), controlPanelPixmap_, SLOT(slotSetPixPos(Position)) );
      QObject::connect( pixmapW, SIGNAL(pixIndexChanged(int)), controlPanelPixmap_, SLOT(slotSetPixIndex(int)) );
      QObject::connect( controlPanelPixmap_, SIGNAL(apply()), pixmapW, SIGNAL(finish()) );
      QObject::connect( controlPanelPixmap_, SIGNAL(forDelete()), pixmapW, SIGNAL(deleteCurrent()) );
      QObject::connect( controlPanelPixmap_, SIGNAL(reset()), pixmapW, SLOT(slotDefaultProp()) );
      QObject::connect( controlPanelPixmap_, SIGNAL(colorChanged(QColor)), pixmapW, SLOT(slotSetColor(QColor)));
      QObject::connect( controlPanelPixmap_, SIGNAL(pixChanged(QString)), pixmapW, SLOT(slotSetPix(QString)) );
      QObject::connect( controlPanelPixmap_, SIGNAL(posChanged(Position)), pixmapW, SLOT(slotSetPixmapPos(Position)) );
      QObject::connect( controlPanelPixmap_, SIGNAL(pixIndexChanged(int)), pixmapW, SLOT(slotSetPixIndex(int)) );
      emit deleteBtnOn();
    }
    else if (false == editPixmap_->inBottomLeftRect(scenePos)) {
      editPixmap_->finish(getLayer());
      pixmapPW_->show();
    }
  }
  if ( ( currentTool() == DrawToolbarWidget::kText )
        && ( e->button() == Qt::LeftButton ) ) {
    if ( nullptr == editText_ ) {
      editText_ = new GeoGraphTextItem(QObject::tr("Текст"));
      editText_->setTextAlignment(Qt::AlignHCenter);
      scene_->addItem(editText_);
      editText_->setGeoPos( scene_->screen2coord(scenePos) );
      editText_->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable /*| QGraphicsItem::ItemIsMovable */);
      Qt::TextInteractionFlags myFlags = ( Qt::TextSelectableByKeyboard | Qt::TextEditable );
      editText_ ->setTextInteractionFlags(myFlags);
      editText_->setPos(view()->mapToScene(e->pos()).toPoint());
      editText_->show();
      editText_->setProperty(textProp_->toProperty());
      editText_->slotUpdateGeometry();
      controlPanel_ = new GeoProxyWidget();
      controlPanelText_ = new ControlPanelTextWidget();
      scene_->addItem(controlPanel_);
      scene_->addVisibleItem(controlPanel_);
      controlPanel_->setWidget(controlPanelText_);
      controlPanelText_->setParentProxyWidget(controlPanel_);
      controlPanelText_->setProperty(textProp_->toProperty());
      controlPanel_->show();
      controlPanelText_->show();
      QTextDocument* doc = editText_->document();
      QTextCursor cursor(doc);
      editText_->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
      cursor.select(QTextCursor::Document);
      editText_->setTextCursor(cursor);
      QObject::connect( editText_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()));
      QObject::connect( textProp_, SIGNAL(insertSymb(QChar)), editText_, SLOT(slotInsertSymb(QChar)));
      QObject::connect( editText_, SIGNAL(anchorChanged(GeoPoint)), controlPanelText_, SLOT(slotSetParentProxyPos(GeoPoint)));
      QObject::connect( textProp_, &PropWidget::valueChanged, [=]{ if ( nullptr != controlPanelText_ ) { controlPanelText_->setProperty(textProp_->toProperty());}});
      TextPropWidget* textW = qobject_cast<TextPropWidget*>(textProp_);
      QObject::connect( controlPanelText_, SIGNAL(propertyChange(meteo::Property)), textW, SLOT(slotSetProperty(meteo::Property)) );
      QObject::connect( controlPanelText_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()) );
      QObject::connect( controlPanelText_, SIGNAL(apply()), textW, SIGNAL(finish()) );
      QObject::connect( controlPanelText_, SIGNAL(insertSymb(QChar)), editText_, SLOT(slotInsertSymb(QChar)) );
      textPW_->hide();
      emit deleteBtnOn();
    }
    else {
      if ( false == editText_->boundingRect().contains( editText_->mapFromScene( scenePos) ) ) {
        editText_->finish(getLayer());
      }
    }
  }
  if ( ( ( currentTool() == DrawToolbarWidget::kCursor ) )
        && ( e->button() == Qt::LeftButton ) ) {
    if ( nullptr != editText_ ) {
      if ( false == editText_->boundingRect().contains( editText_->mapFromScene( scenePos) ) ) {
        editText_->finish(getLayer());

      }
    }
    if ( nullptr != editPixmap_ ) {
      if (false == editPixmap_->inBottomLeftRect(scenePos)) {
        editPixmap_->finish(getLayer());
      }
    }
  }
  if ( ( currentTool() == DrawToolbarWidget::kGroup )
       && ( e->button() == Qt::LeftButton )) {
    if ( e->modifiers() == Qt::KeyboardModifier::ControlModifier ) {
      if (nullptr != groupObject_) {
        if ( nullptr != highlightObj_ ) {
          if ( nullptr != highlightObj_->object()) {
            groupObject_->removeObject(highlightObj_->object());
          }
        }
      }
      return;
    }
    if ( nullptr == groupObject_ ) {
      groupObject_ = new GroupObject();
      scene_->addItem(groupObject_);
    }
    if ( nullptr != highlightObj_ ) {
      if ( nullptr != highlightObj_->object()) {
        groupObject_->addObject(highlightObj_->object());
      }
    }
    return;
  }
  if ( ( nullptr != editableObject_ )
       && ( e->button() == Qt::LeftButton ) ) {
    switch ( currentTool() ) {
    case DrawToolbarWidget::kLine:
    case DrawToolbarWidget::kPolygon:
    case DrawToolbarWidget::kFront:
    {
      {
        // исключаем добавление узла при перетаскивании объекта
        // свойство movable выставляется в true перед перетаскиванием
        if ( !editableObject_->isMovable() ) {
          static_cast<PolygonDrawObject*>(editableObject_)->appendNode(scenePos);
          setMarkers(static_cast<PolygonDrawObject*>(editableObject_));
          editableObject_->update();
          emit deleteBtnOn();
        }
      }
      break;
    }
    case DrawToolbarWidget::kNoTool:
    case DrawToolbarWidget::kCursor:
    case DrawToolbarWidget::kPixmap:
    case DrawToolbarWidget::kText:
    case DrawToolbarWidget::kGroup:
      break;
    }
    return;
  }
  if ( ( nullptr != highlightObj_ )
       && ( highlightObj_->isPointOnObject(scenePos, 25) )
       && ( e->button() == Qt::LeftButton ) ) {
    if ( QApplication::keyboardModifiers() == Qt::ControlModifier ) {
      bool added = false;
      for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
        if ( selectedObjects_.at(i)->object() == highlightObj_->object() ) {
          selectedObjects_[i]->removeFromScene();
          delete selectedObjects_[i];
          selectedObjects_.removeAt(i);
          added = true;
          break;
        }
      }
      if ( !added ) {
        selectedObjects_.append(highlightObj_);
        highlightObj_ = nullptr;
      }
    }
    else if ( state_ != State::kEdit) {
      for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
        selectedObjects_[i]->removeFromScene();
        delete selectedObjects_[i];
      }
      selectedObjects_.clear();

      if ( highlightObj_->object()->type() == kPolygon ) {
        slotFinishEditing();
        beginEdit(const_cast<Object*>(highlightObj_->object()));
      }
      else if ( highlightObj_->object()->type() == kGroup ) {
        slotFinishEditing();
        beginEdit(const_cast<Object*>(highlightObj_->object()));
      }
      else if ( highlightObj_->object()->type() == kText ) {
        slotFinishEditing();
        beginEdit(const_cast<Object*>(highlightObj_->object()));
      }
      else if ( highlightObj_->object()->type() == kPixmap ) {
        slotFinishEditing();
        beginEdit(const_cast<Object*>(highlightObj_->object()));
      }
      else if ( highlightObj_->object()->type() == kIsoline ) {
        slotFinishEditing();
        beginEdit(const_cast<Object*>(highlightObj_->object()));
      }
      else {
        debug_log << "object type" << highlightObj_->object()->type() << "no process";
      }
      if ( nullptr != highlightObj_) {
        highlightObj_->removeFromScene();
        delete highlightObj_;
        highlightObj_ = nullptr;
      }
    }

    return;
  }

  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    selectedObjects_[i]->removeFromScene();
    delete selectedObjects_[i];
  }
  selectedObjects_.clear();
}

void DrawToolsAction::keyReleaseEvent(QKeyEvent* event)
{
  if ( false == drawPanel_->isVisible() ) {
    return;
  }
 if (nullptr != editText_ && true == editText_->hasFocus()) {
   if ( Qt::Key_Escape == event->key()) {
     editText_->clearFocus();
   }
   return;
 }
 switch (event->key()) {
 case Qt::Key_Escape:
   switch (currentTool()) {
   case DrawToolbarWidget::Tool::kNoTool:
     slotHide();
     break;
   case DrawToolbarWidget::Tool::kCursor:
     if (!( nullptr != editText_ || nullptr != editPixmap_ || nullptr != editableObject_ )) {
       toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kNoTool);
     }
     slotTextAbort();
     slotPixmapAbort();
     slotGroupAbort();
     slotToolActivate(currentTool());
     QApplication::restoreOverrideCursor();
     break;
   case DrawToolbarWidget::Tool::kGroup:
     if ( nullptr != groupObject_) {
       slotGroupAbort();
       if ( State::kEdit == state_ ) {
         toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
       }
     }
     else {
       toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
     }
     break;
   default:
     slotDeleteEditing();
     toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
     QApplication::restoreOverrideCursor();
     break;
   }
   break;
 case Qt::Key::Key_Delete:
   slotDeleteEditing();
   break;
 case Qt::Key::Key_Enter:
 case Qt::Key_Return:
   slotFinishEditing();
   break;
 case Qt::Key_X:
   if ( Qt::KeyboardModifier::ControlModifier == event->modifiers() ) {
     cutObjects();
   }
   break;
 case Qt::Key_C:
   if ( Qt::KeyboardModifier::ControlModifier == event->modifiers() ) {
     copyObjects();
   }
   break;
 default:
   break;
 }
}

QList<QString> DrawToolsAction::getSelectedObjects()
{
  QList<QString> list;
  foreach (HighlightObject* highObj, selectedObjects_) {
     list.append( highObj->object()->uuid() );
  }
  return list;
}

QString DrawToolsAction::getHighLightObject()
{
  QString uuid;
  if ( nullptr != highlightObj_ ) {
    uuid = highlightObj_->object()->uuid();
  }
  return uuid;
}

Layer* DrawToolsAction::getLayer()
{
  if ( layerUuid_.isEmpty() ) {
    if ( scene_->document()->eventHandler() == nullptr ) {
      error_log << tr("Новый слой не будет создан, т.к. отсутствует объект EventHandler, необходимый для "
                          "получения событий об изменении слоёв");
      return nullptr;
    }

//    scene_->document()->eventHandler()->installEventFilter(this);

    return createLayer();
  }

  if ( scene_->document()->hasLayer(layerUuid_) ) {
    if ( scene_->document()->isLayerActive(layerUuid_) ) {
      return scene_->document()->layer(layerUuid_);
    }

    Layer* l = scene_->document()->activeLayer();
    if ( nullptr != l ) {
      layerUuid_ = l->uuid();
      return l;
    }

    return createLayer();
  }

  return createLayer();
}

bool DrawToolsAction::eventFilter(QObject* obj, QEvent* e)
{
  if ( scene_->document()->eventHandler() == obj ) {
    if ( e->type() == map::LayerEvent::LayerChanged ) {
      LayerEvent* event = static_cast<LayerEvent*>(e);
      if ( nullptr != event ) {
        if ( event->changeType() == LayerEvent::Deleted ) {
          QList<HighlightObject*> fordelete;
          for (HighlightObject* highObj : selectedObjects_) {
            if (  highObj->layerUuid() == event->layer() ) {
              fordelete.append(highObj);
            }
          }
          for (HighlightObject* highObj : fordelete ) {
            if ( highObj == highlightObj_ ) {
              highlightObj_ = nullptr;
            }
            selectedObjects_.removeAll(highObj);
            highObj->removeFromScene();
            delete highObj;
          }
          if ( nullptr != highlightObj_
            && event->layer() == highlightObj_->layerUuid() ) {
            highlightObj_->removeFromScene();
            delete highlightObj_;
            highlightObj_ = nullptr;
          }
        }
        if ( event->changeType() == LayerEvent::Deleted && event->layer() == layerUuid_ ) {
          layerUuid_.clear();
        }
      }
    }
  }

  return Action::eventFilter(obj, e);
}

void DrawToolsAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu ) {
    return;
  }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());

  QList<QAction*> acts;

  setupContextMenu(menu, pos);
  if ( nullptr != editableObject_ ) {
    acts += editableObject_->setupContextMenu(menu, pos);
  }
  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    acts += drawObjects_[i]->setupContextMenu(menu, pos);
  }

  foreach ( QAction* act, acts ) {
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }
}

void DrawToolsAction::slotToolActivate(DrawToolbarWidget::Tool currentTool)
{
  if ( nullptr != editableObject_ ) {
    editableObject_->removeFromScene();
  }
  delete editableObject_;
  editableObject_ = nullptr;
  slotPixmapAbort();
  slotTextAbort();
  slotGroupAbort();
  deleteSelectedObjects();
  if ( nullptr != highlightObj_ ) {
    highlightObj_->removeFromScene();
  }
  delete highlightObj_;
  highlightObj_ = nullptr;
  switch ( currentTool ) {
  case DrawToolbarWidget::kCursor:
    slotOnActions();
    linePW_->hide();
    frontPW_->hide();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  case DrawToolbarWidget::kLine:
    linePW_->show();
    frontPW_->hide();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  case DrawToolbarWidget::kPolygon:
    linePW_->hide();
    frontPW_->hide();
    polygonPW_->show();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  case DrawToolbarWidget::kFront:
    linePW_->hide();
    frontPW_->show();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  case DrawToolbarWidget::kText:
    linePW_->hide();
    frontPW_->hide();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  case DrawToolbarWidget::kPixmap:
    linePW_->hide();
    frontPW_->hide();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->show();
    break;
  default:
    linePW_->hide();
    frontPW_->hide();
    polygonPW_->hide();
    textPW_->hide();
    pixmapPW_->hide();
    break;
  }

  if ( DrawToolbarWidget::kNoTool != currentTool ) {
    actBtn_->setChecked(true);
    enabled_ = true;
  }
  else {
    slotOnActions();
    enabled_ = false;
  }
  if ( DrawToolbarWidget::kLine == currentTool
       || DrawToolbarWidget::kPolygon == currentTool
       || DrawToolbarWidget::kFront == currentTool
       )
  {
    if ( state_ == State::kNone ) {
      state_ = State::kCreate;
    }
    editableObject_ = new PolygonDrawObject(scene_);
    GeoPolygon tmp(scene_->document()->projection());
    editableObject_->setObject(&tmp);
  }
  else if ( DrawToolbarWidget::kText == currentTool ) {
   editableObject_ = new TextDrawObject(scene_);
   GeoText tmp(scene_->document()->projection());
   editableObject_->setObject(&tmp);
  }
  else if ( DrawToolbarWidget::kPixmap == currentTool ) {
   editableObject_ = new PixmapDrawObject(scene_);
   GeoPixmap tmp(scene_->document()->projection());
   editableObject_->setObject(&tmp);
  }

  slotApplyProp();
}

void DrawToolsAction::slotApplyProp()
{
  PropWidget* w = qobject_cast<PropWidget*>(sender());
  if (w == pixmapProp_) {
    QString path = pixmapProp_->getPixmapIconPath();
    toolbar_->setPixmapIcon(path);
    if ( nullptr != editPixmap_) {
      editPixmap_->setImage(path);
      PixmapPropWidget* pixmapW = qobject_cast<PixmapPropWidget*>(pixmapProp_);
      editPixmap_->slotSetImgColor(pixmapW->color());
      editPixmap_->setProperty(pixmapProp_->toProperty());
    }
  }
  else if (w == textProp_) {
    if ( nullptr != editText_ ) {
      editText_->setProperty(textProp_->toProperty());
    }
  }
  if ( nullptr == editableObject_ ) {
    return;
  }
  if ( nullptr == w ) {
    w = currentWidget();
  }
  if ( nullptr == w ) {
    return;
  }
  if ( w->hasValue() ) {
    editableObject_->setObjectValue(w->value(), "4' '.2'0'", w->unit());
  }
  else {
    editableObject_->removeObjectValue();
  }
  editableObject_->setObjectProperty(w->toProperty());
  editableObject_->setObjectText(w->text());
  editableObject_->setObjectPixmap(w->pixmap());
  editableObject_->update();
}

void DrawToolsAction::slotFinishDraw()
{
  if ( nullptr != editableObject_ ) {
    slotApplyProp();

    Layer* l = getLayer();
    if ( nullptr != l ) {
      Object* obj = l->objectByUuid(editableObject_->id());
      delete obj;

      const Object* tmp = editableObject_->object();
      if ( nullptr != tmp ) {
        undoStack_.push(new AddObjectCommand(l->uuid(), tmp->copy(scene_->document()->projection()), scene_));
      }
    }

    editableObject_->removeFromScene();
    delete editableObject_;
    editableObject_ = nullptr;

  }
  if ( nullptr != editPixmap_) {
    editPixmap_->finish(getLayer());
  }
  if ( nullptr != editText_ ) {
    editText_->finish(getLayer());
  }

  activateTool(DrawToolbarWidget::kCursor);
}

Layer* DrawToolsAction::createLayer()
{
  QString name = tr("Слой пользователя: рисование (%1)")
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
  Layer* l = new Layer(scene_->document(), name);
  l->setActive();
  layerUuid_ = l->uuid();
  return l;
}

void DrawToolsAction::slotFinishEditing()
{
  clearMarkers();
  if ( nullptr != editPixmap_ ) {
    editPixmap_->finish(getLayer());
  }

  if (nullptr != editText_) {
    editText_->finish(getLayer());
  }
  if ( nullptr != groupObject_ ) {
    groupObject_->finish();
    delete groupObject_;
    groupObject_ = nullptr;
    if ( State::kEdit == state_ ) {
      toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
    }
  }
  if ( nullptr != editableObject_ ) {
    slotApplyProp();
    Layer* l = getLayer();
    if ( nullptr != l ) {
      Object* obj = l->objectByUuid(editableObject_->id());
      delete obj;

      const Object* tmp = editableObject_->object();
      if ( nullptr != tmp ) {
        undoStack_.push(new AddObjectCommand(l->uuid(), tmp->copy(scene_->document()->projection()), scene_));
      }
    }

    if ( DrawToolbarWidget::kCursor == currentTool() ) {
      PropWidget* prop = nullptr;
      if ( nullptr != toolbar_ ) {
        prop = widgetForTool( currentToolForEditObject() );
      }
      if ( nullptr != prop ) {
        prop->hide();
      }
    }

    editableObject_->removeFromScene();
    delete editableObject_;
    editableObject_ = nullptr;
    emit deleteBtnOff();
    slotToolActivate(currentTool());
  }
  if ( State::kEdit == state_ ) {
    state_ = State::kNone;
  }
}

void DrawToolsAction::slotAbortDraw()
{
  if ( nullptr != editableObject_ ) {
    editableObject_->removeFromScene();
    delete editableObject_;
    editableObject_ = nullptr;
  }

  if ( nullptr != highlightObj_ ) {
    highlightObj_->removeFromScene();
    delete highlightObj_;
    highlightObj_ = nullptr;
  }
  slotPixmapAbort();
  slotTextAbort();
  slotGroupAbort();
  activateTool(DrawToolbarWidget::kCursor);
}

void DrawToolsAction::slotDeleteEditing()
{
  clearMarkers();
  deleteObject(editableObject_);
  editableObject_ = nullptr;
  if ( nullptr != editPixmap_) {
    slotPixmapDelete();
  }
  if ( nullptr != editText_ ) {
    slotTextDelete();
  }
  if ( nullptr != groupObject_ ) {
    if ( nullptr != highlightObj_ ) {
      highlightObj_->removeFromScene();
    }
    delete highlightObj_;
    highlightObj_ = nullptr;
    slotGroupDelete();
    if ( State::kEdit == state_ ) {
      toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
    }
  }
  if ( State::kEdit == state_ ) {
    state_ = State::kNone;
  }
  slotToolActivate(toolbar_->currentTool());
  emit deleteBtnOff();
}

void DrawToolsAction::deleteObject(DrawObject* object)
{
  if ( nullptr == object ) {
    return;
  }

  Layer* l = getLayer();
  if ( nullptr != l ) {
    Object* obj = l->objectByUuid(object->id());
    delete obj;
  }

  object->removeFromScene();
  delete object;
}


void DrawToolsAction::slotProcessMenuAction()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if ( nullptr == act ) {
    return;
  }

  processContextMenu(act);
  if ( nullptr != editableObject_ ) {
    editableObject_->processContextMenu(act);
  }
  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    drawObjects_[i]->processContextMenu(act);
  }
}

bool DrawToolsAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = scene_->mapview()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( nullptr != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}

void DrawToolsAction::deleteSelectedObjects()
{
  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    selectedObjects_[i]->removeFromScene();
    delete selectedObjects_[i];
  }
  selectedObjects_.clear();
}

void DrawToolsAction::beginEdit(Object* obj)
{
  state_ = State::kEdit;
  if ( nullptr == toolbar_ ) {
    error_log << tr("Невозможно начать редактирование объекта, нет доступа к виджету панели инструментов");
    return;
  }

  DrawToolbarWidget::Tool tool = DrawToolbarWidget::kCursor;
  if ( obj->type() == kText ) {
    tool = DrawToolbarWidget::kText;
  }
  else if ( obj->type() == kPolygon || obj->type() == kIsoline ) {
    tool = DrawToolbarWidget::kLine;
    if ( obj->closed() ) {
      tool = DrawToolbarWidget::kPolygon;
    }
    else if ( pbtools::toQString(obj->property().ornament()).endsWith("front") ) {
      tool = DrawToolbarWidget::kFront;
    }
  }
  else if ( obj->type() == kGroup ) {
    tool = DrawToolbarWidget::kGroup;
  }
  else if ( obj->type() == kPixmap ) {
    tool = DrawToolbarWidget::kPixmap;
  }
  PropWidget* prop = nullptr;
  prop = widgetForTool( tool );
  if ( tool == DrawToolbarWidget::kPolygon
       || tool == DrawToolbarWidget::kFront
       || tool == DrawToolbarWidget::kGroup
       || tool == DrawToolbarWidget::kLine) {
    slotToolActivate(tool);
  }
  if ( tool == DrawToolbarWidget::kGroup ) {
    toolbar_->setCurrentTool(tool);
  }
  if ( nullptr != prop ) {
    prop->setProperty(obj->property());
    prop->setValue(obj->value(), obj->hasValue());
    GeoText* txt = mapobject_cast<GeoText*>(obj);
    if ( nullptr != txt ) {
      if ( nullptr == editText_ ) {
        editText_ = new GeoGraphTextItem();
        editText_->setTextAlignment(Qt::AlignHCenter);
        editText_->setProperty(txt->property());
        editText_->setPlainText(txt->text());
        editText_->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable /*| QGraphicsItem::ItemIsMovable */);
        Qt::TextInteractionFlags myFlags = ( Qt::TextSelectableByKeyboard | Qt::TextEditable );
        editText_ ->setTextInteractionFlags(myFlags);

        scene_->addItem( editText_ );
        editText_->setGeoPos(txt->skelet().at(0));
        editGeoTxt_ = txt;
        txt = nullptr;
        if ( nullptr != editableObject_ ) {
          editableObject_->removeFromScene();
        }
        delete editableObject_;
        editableObject_ = nullptr;
        editText_->slotUpdateGeometry();
        controlPanel_ = new GeoProxyWidget();
        controlPanelText_ = new ControlPanelTextWidget();
        scene_->addItem(controlPanel_);
        scene_->addVisibleItem(controlPanel_);
        controlPanel_->setWidget(controlPanelText_);
        controlPanelText_->setParentProxyWidget(controlPanel_);
        QPointF p = editText_->mapToScene(editText_->boundingRect().bottomLeft());
        controlPanel_->setGeoPos(scene_->document()->screen2coord(p));
        controlPanelText_->setProperty(textProp_->toProperty());
        controlPanel_->show();
        controlPanelText_->show();
        QObject::connect( editText_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()));
        QObject::connect( editText_, SIGNAL(anchorChanged(GeoPoint)), controlPanelText_, SLOT(slotSetParentProxyPos(GeoPoint)));
        QObject::connect( textProp_, SIGNAL(insertSymb(QChar)), editText_, SLOT(slotInsertSymb(QChar)));
        QObject::connect( textProp_, &PropWidget::valueChanged, [=]{ controlPanelText_->setProperty(textProp_->toProperty());}  );
        TextPropWidget* textW = qobject_cast<TextPropWidget*>(textProp_);
        QObject::connect( controlPanelText_, SIGNAL(propertyChange(meteo::Property)), textW, SLOT(slotSetProperty(meteo::Property)) );
        QObject::connect( controlPanelText_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()) );
        QObject::connect( controlPanelText_, SIGNAL(apply()), textW, SIGNAL(finish()) );
        QObject::connect( controlPanelText_, SIGNAL(insertSymb(QChar)), editText_, SLOT(slotInsertSymb(QChar)) );
        return;
      }
    }
    GeoPixmap* pix = mapobject_cast<GeoPixmap*>(obj);
    if ( nullptr != pix ) {
      prop->setPixmap(pix->image());
      if ( nullptr == editPixmap_ ) {
        editPixmap_ = new PixmapToolGraphicsItem(200);
        scene_->addItem(editPixmap_);
        editPixmap_->setGeoPixmap(pix);
        editPixmap_->pos();
        editGeoPix_ = pix;
        pix = nullptr;
        if ( nullptr != editableObject_ ) {
          editableObject_->removeFromScene();
        }
        delete editableObject_;
        editableObject_ = nullptr;
        emit deleteBtnOn();
        controlPanel_ = new GeoProxyWidget();
        controlPanelPixmap_ = new ControlPanelPixmapWidget();
        scene_->addItem(controlPanel_);
        scene_->addVisibleItem(controlPanel_);
        controlPanel_->setWidget(controlPanelPixmap_);
        controlPanelPixmap_->setParentProxyWidget(controlPanel_);
        QPointF p = editPixmap_->mapToScene(editPixmap_->boundingRect().topLeft() - QPoint(-32,32));
        controlPanel_->setGeoPos(scene_->document()->screen2coord(p));
        controlPanel_->show();
        controlPanelPixmap_->show();
        PixmapPropWidget* pixmapW = qobject_cast<PixmapPropWidget*>(pixmapProp_);
        QObject::connect( editPixmap_, SIGNAL(forDelete()), SLOT(slotDeleteEditing()) );
        QObject::connect( editPixmap_, SIGNAL(valueChanged(meteo::Property)), pixmapW, SLOT(slotSetProperty(meteo::Property)) );
        QObject::connect( editPixmap_, SIGNAL(anchorChanged(GeoPoint)), controlPanelPixmap_, SLOT(slotSetGeoPosProxyWidget(GeoPoint)) );
        QObject::connect( pixmapProp_, SIGNAL(colorChanged(QColor)), editPixmap_, SLOT(slotSetImgColor(QColor)) );
        QObject::connect( pixmapProp_, SIGNAL(colorChanged(QColor)), controlPanelPixmap_, SLOT(slotSetColor(QColor)) );
        QObject::connect( pixmapW, SIGNAL(posChanged(Position)), controlPanelPixmap_, SLOT(slotSetPixPos(Position)) );
        QObject::connect( pixmapW, SIGNAL(pixIndexChanged(int)), controlPanelPixmap_, SLOT(slotSetPixIndex(int)) );
        QObject::connect( controlPanelPixmap_, SIGNAL(apply()), pixmapW, SIGNAL(finish()) );
        QObject::connect( controlPanelPixmap_, SIGNAL(forDelete()), pixmapW, SIGNAL(deleteCurrent()) );
        QObject::connect( controlPanelPixmap_, SIGNAL(reset()), pixmapW, SLOT(slotDefaultProp()) );
        QObject::connect( controlPanelPixmap_, SIGNAL(colorChanged(QColor)), pixmapW, SLOT(slotSetColor(QColor)) );
        QObject::connect( controlPanelPixmap_, SIGNAL(pixChanged(QString)), pixmapW, SLOT(slotSetPix(QString)) );
        QObject::connect( controlPanelPixmap_, SIGNAL(posChanged(Position)), pixmapW, SLOT(slotSetPixmapPos(Position)) );
        QObject::connect( controlPanelPixmap_, SIGNAL(pixIndexChanged(int)), pixmapW, SLOT(slotSetPixIndex(int)) );
        return;
      }
    }
  }

  if ( nullptr != editableObject_ ) {
    editableObject_->removeFromScene();
  }
  delete editableObject_;
  editableObject_ = nullptr;

  switch ( tool ) {
    case DrawToolbarWidget::kNoTool:
    case DrawToolbarWidget::kCursor: {
    } break;
    case DrawToolbarWidget::kGroup: {
    if ( nullptr == groupObject_ ) {
      groupObject_ = new GroupObject();
      scene_->addItem(groupObject_);
    }
    if ( nullptr != obj) {
      groupObject_->addObject(obj);
    }
//      GroupDrawObject* tmp = new GroupDrawObject(scene_, obj->uuid());
//      tmp->setObject(obj);
//      editableObject_ = tmp;
    } break;
    case DrawToolbarWidget::kLine:
    case DrawToolbarWidget::kPolygon:
    case DrawToolbarWidget::kFront: {
      PolygonDrawObject* tmp = new PolygonDrawObject(scene_, obj->uuid());
      tmp->setObject(obj);
      editableObject_ = tmp;
    } break;
    case DrawToolbarWidget::kText: {
    } break;
    case DrawToolbarWidget::kPixmap: {
    } break;
  }

  // запоминаем активный слой
  if ( scene_->document()->eventHandler() != nullptr ) {
//    scene_->document()->eventHandler()->installEventFilter(this);

    Layer* l = scene_->document()->activeLayer();
    if ( nullptr != l ) {
      layerUuid_ = l->uuid();
    }
  }
  emit deleteBtnOn();
}

void DrawToolsAction::slotShow()
{
  if (nullptr != drawPanel_)
  {
    drawPanel_->show();
    if (nullptr != toolbar_) {
      if ( toolbar_->currentTool() != DrawToolbarWidget::Tool::kCursor ) {
        toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kCursor);
      }
    }
  }
}

void DrawToolsAction::slotHide()
{
  if (nullptr != drawPanel_)
  {
    drawPanel_->hide();
    if ( nullptr != editableObject_ ) {
      editableObject_->removeFromScene();
    }
    delete editableObject_;
    editableObject_ = nullptr;
    slotPixmapAbort();
    slotTextAbort();
    slotGroupAbort();
    deleteSelectedObjects();
    if ( nullptr != highlightObj_ ) {
      highlightObj_->removeFromScene();
    }
    delete highlightObj_;
    highlightObj_ = nullptr;
    actBtn_->blockSignals(true);
    actBtn_->setChecked(false);
    toolbar_->setCurrentTool(DrawToolbarWidget::Tool::kNoTool);
    actBtn_->blockSignals(false);
  }
}

void DrawToolsAction::slotSwitch()
{
  if ( true == actBtn_->isChecked() ) {
    slotShow();
  }
  else {
    slotHide();
  }
}

void DrawToolsAction::slotShowTool(DrawToolbarWidget::Tool tool)
{
  QList<QGraphicsProxyWidget*> list;
  list << linePW_ << polygonPW_ << frontPW_ << textPW_ << pixmapPW_;
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    list.at(i)->hide();
  }
  QGraphicsProxyWidget* widget = proxyWidgetForTool(tool);
  if ( nullptr != widget ) {
    widget->show();
  }
}

void DrawToolsAction::slotHideTool(DrawToolbarWidget::Tool tool)
{
  QGraphicsProxyWidget* widget = proxyWidgetForTool(tool);
  if ( nullptr != widget ) {
    widget->hide();
  }
}

void DrawToolsAction::slotOffActions()
{
  Action* a = scene_->getAction("scaleaction");
  if ( nullptr == a ) {
    return;
  }
  a->setDisabled(true);
}

void DrawToolsAction::slotOnActions()
{
  Action* a = scene_->getAction("scaleaction");
  if ( nullptr == a ) {
    return;
  }
  a->setDisabled(false);
}

void DrawToolsAction::slotPixmapDelete()
{
  if  ( nullptr != editPixmap_ ) {
    pixmapProp_->setProperty(editPixmap_->toProperty());
    delete editPixmap_;
    editPixmap_ = nullptr;
    if ( nullptr != controlPanel_) {
      scene_->removeVisibleItem(controlPanel_);
      controlPanel_->hide();
      controlPanelPixmap_->hide();
      controlPanel_->deleteLater();
      controlPanel_ = nullptr;
      controlPanelPixmap_->deleteLater();
      controlPanelPixmap_ = nullptr;
    }
  }
  if ( state_ == State::kEdit ) {
    delete editGeoPix_;
    editGeoPix_ = nullptr;
  }
  emit deleteBtnOff();
}

void DrawToolsAction::slotTextDelete()
{
  if ( nullptr != editText_ ) {
    delete editText_;
    editText_ = nullptr;
    if ( nullptr != controlPanel_ ) {
      scene_->removeVisibleItem(controlPanel_);
      controlPanel_->hide();
      controlPanelText_->hide();
      controlPanel_->deleteLater();
      controlPanel_ = nullptr;
      controlPanelText_->deleteLater();
      controlPanelText_ = nullptr;
    }
  }
  if ( state_ == State::kEdit ) {
    delete editGeoTxt_;
    editGeoTxt_ = nullptr;
  }
  emit deleteBtnOff();
}

void DrawToolsAction::slotGroupDelete()
{
  if ( nullptr != groupObject_ ) {
    groupObject_->deleteObjects();
    delete groupObject_ ;
    groupObject_  = nullptr;
  }
}

void DrawToolsAction::slotPixmapAbort()
{
  if  ( nullptr != editPixmap_ ) {
    pixmapProp_->setProperty(editPixmap_->toProperty());
    delete editPixmap_;
    editPixmap_ = nullptr;
    if ( nullptr != controlPanel_) {
      scene_->removeVisibleItem(controlPanel_);
      controlPanel_->hide();
      controlPanelPixmap_->hide();
      controlPanel_->deleteLater();
      controlPanel_ = nullptr;
      controlPanelPixmap_->deleteLater();
      controlPanelPixmap_ = nullptr;
    }
  }
  if ( state_ == State::kEdit ) {
   state_ = State::kNone;
  }
  emit deleteBtnOff();
}

void DrawToolsAction::slotTextAbort()
{
  if ( nullptr != editText_ ) {
    delete editText_;
    editText_ = nullptr;
    if ( nullptr != controlPanel_ ) {
      scene_->removeVisibleItem(controlPanel_);
      controlPanel_->hide();
      controlPanelText_->hide();
      controlPanel_->deleteLater();
      controlPanel_ = nullptr;
      controlPanelText_->deleteLater();
      controlPanelText_ = nullptr;
    }
  }
  if ( state_ == State::kEdit ) {
   state_ = State::kNone;
  }
  emit deleteBtnOff();
}

void DrawToolsAction::slotGroupAbort()
{
  if ( nullptr != groupObject_ ) {
    groupObject_->abort();
    scene_->removeItem(groupObject_);
    delete groupObject_;
    groupObject_ = nullptr;
  }
  if ( state_ == State::kEdit ) {
   state_ = State::kNone;
  }
}

void DrawToolsAction::slotAppendLastMarker()
{
  if (nullptr == lastMarker_) {
    return;
  }
  if ( QApplication::keyboardModifiers() == Qt::ControlModifier ) {
    return;
  }
  QLineF length(lastMarker_->pos(), lastPosLM_);
  int count;
  if ( length.length() > kLength ) {
    PolygonDrawObject* polyObj = static_cast<PolygonDrawObject*>(editableObject_);
    if (nullptr != polyObj) {
      count = polyObj->markersCount();
      polyObj->appendNode(lastMarker_->pos().toPoint(), count-1, false);
      lastPosLM_ = polyObj->lastMarker()->pos();
    }
  }
}

void DrawToolsAction::slotAppendFirstMarker()
{
  if (nullptr == firstMarker_) {
    return;
  }
  if ( QApplication::keyboardModifiers() == Qt::ControlModifier ) {
    return;
  }
  QLineF length(firstMarker_->pos(), lastPosFM_);
  if ( length.length() > kLength ) {
    PolygonDrawObject* polyObj = static_cast<PolygonDrawObject*>(editableObject_);
    if (nullptr != polyObj) {
      polyObj->appendNode(firstMarker_->pos().toPoint(), 1, false);
      lastPosFM_ = polyObj->firstMarker()->pos();
    }
  }
}

Object* DrawToolsAction::getTopParentAt(const QPoint& scenePos) const
{
  Layer* l = scene_->document()->activeLayer();
  if ( nullptr == l ) {
    return nullptr;
  }

  Object* obj = scene_->document()->objectNearScreenPoint(scenePos, l);
  if ( nullptr == obj ) {
    return nullptr;
  }

  while ( obj->parent() != nullptr ) {
    obj = obj->parent();
  }

  return obj;
}

void DrawToolsAction::highlightObject(Object* object)
{
  if ( nullptr == object ) {
    return;
  }

  if ( object->layer()->isBase() ) {
    return;
  }
  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    if ( selectedObjects_[i]->object()->uuid() == object->uuid() ) {
      return;
    }
  }

  if ( nullptr != highlightObj_ && highlightObj_->object()->uuid() != object->uuid() ) {
    highlightObj_->removeFromScene();
    delete highlightObj_;
    highlightObj_ = nullptr;
  }

  if ( nullptr == highlightObj_ ) {
    highlightObj_ = new HighlightObject(object, scene_);
    highlightObj_->setLayerUuid( object->layer()->uuid() );
  }
}

DrawToolbarWidget::Tool DrawToolsAction::currentTool() const
{
  if ( nullptr == toolbar_ ) {
    error_log << tr("Невозможно определить текущий инструмент рисования, отсутствует виджет панели инструментов");
    return DrawToolbarWidget::kNoTool;
  }

  return toolbar_->currentTool();
}

PropWidget* DrawToolsAction::currentWidget() const
{
  switch ( currentTool() ) {
  case DrawToolbarWidget::kLine:
    return lineProp_;
  case DrawToolbarWidget::kFront:
    return  frontProp_;
  case DrawToolbarWidget::kText:
    return  textProp_;
  case DrawToolbarWidget::kPolygon:
    return  polygonProp_;
  case DrawToolbarWidget::kPixmap:
    return  pixmapProp_;
  default:
    return nullptr;
  }
}

PropWidget* DrawToolsAction::widgetForTool(const DrawToolbarWidget::Tool tool) const
{
  switch ( tool ) {
  case DrawToolbarWidget::kLine:
    return lineProp_;
  case DrawToolbarWidget::kFront:
    return  frontProp_;
  case DrawToolbarWidget::kText:
    return  textProp_;
  case DrawToolbarWidget::kPolygon:
    return  polygonProp_;
  case DrawToolbarWidget::kPixmap:
    return  pixmapProp_;
  default:
    return nullptr;
  }
}

QGraphicsProxyWidget* DrawToolsAction::proxyWidgetForTool(const DrawToolbarWidget::Tool tool) const
{
  switch ( tool ) {
  case DrawToolbarWidget::kLine:
    return linePW_;
  case DrawToolbarWidget::kFront:
    return  frontPW_;
  case DrawToolbarWidget::kText:
    return  textPW_;
  case DrawToolbarWidget::kPolygon:
    return  polygonPW_;
  case DrawToolbarWidget::kPixmap:
    return  pixmapPW_;
  default:
    return nullptr;
  }
}

DrawToolbarWidget::Tool DrawToolsAction::currentToolForEditObject() const
{
  if ( nullptr == toolbar_ ) {
    error_log << tr("Невозможно определить текущий инструмент рисования, отсутствует виджет панели инструментов");
    return DrawToolbarWidget::kCursor;
  }

  if ( nullptr == editableObject_ || nullptr == editableObject_->object() ) {
    return toolbar_->currentTool();
  }

  const Object* obj = editableObject_->object();

  DrawToolbarWidget::Tool tool = toolbar_->currentTool();
  if ( obj->type() == kText ) {
    tool = DrawToolbarWidget::kText;
  }
  else if ( obj->type() == kPolygon || obj->type() == kIsoline ) {
    tool = DrawToolbarWidget::kLine;
    if ( obj->closed() ) {
      tool = DrawToolbarWidget::kPolygon;
    }
    else if ( pbtools::toQString(obj->property().ornament()).endsWith("front") ) {
      tool = DrawToolbarWidget::kFront;
    }
  }
  else if ( obj->type() == kGroup ) {
    tool = DrawToolbarWidget::kGroup;
  }
  else if ( obj->type() == kPixmap ) {
    tool = DrawToolbarWidget::kPixmap;
  }

  return tool;
}

void DrawToolsAction::activateTool(DrawToolbarWidget::Tool tool)
{
  if ( nullptr == toolbar_ ) {
    error_log << tr("Невозможно переключить инструмент рисования, отсутствует виджет панели инструментов");
    return;
  }

  toolbar_->setCurrentTool(tool);
}

void DrawToolsAction::setupContextMenu(Menu* menu, const QPoint& screenPos) const
{
  Q_UNUSED( screenPos );

  if ( nullptr == menu ) {
    return;
  }

  QAction* act = nullptr;

  if ( nullptr != editableObject_ ) {
    act = new QAction(tr("Завершить редактирование"), menu);
    act->setData(ActId::kFinishAct);
    menu->addObjectAction(act);
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }

  if ( nullptr != highlightObj_ ) {
    if ( highlightObj_->object()->type() == kGroup ) {
      act = new QAction(tr("Разгруппировать"), menu);
      act->setData(ActId::kUnGroupAct);
      menu->addObjectAction(act);
      QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
    }
  }

  if ( groupObject_ == nullptr  && ( nullptr != highlightObj_ || selectedObjects_.size() > 0 ) ) {
    act = new QAction(tr("Удалить"), menu);
    act->setData(ActId::kDeleteAct);
    act->setIcon(QIcon(QPixmap(":/meteo/icons/delete-16.png")));
    menu->addObjectAction(act);
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }

  if ( selectedObjects_.size() > 1 ) {
    act = new QAction(tr("Группировать"), menu);
    act->setData(ActId::kGroupAct);
    menu->addObjectAction(act);
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }

  if ( nullptr != highlightObj_ || selectedObjects_.size() > 0 ) {
    act = new QAction(tr("Копировать"), menu);
    act->setData(ActId::kCopyAct);
    menu->addObjectAction(act);
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }

  if ( ( nullptr == groupObject_ && nullptr != highlightObj_ ) || selectedObjects_.size() > 0  ) {
    act = new QAction(tr("Вырезать"), menu);
    act->setData(ActId::kCutAct);
    menu->addObjectAction(act);
    QObject::connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
  }
}

void DrawToolsAction::processContextMenu(QAction* act)
{
  if ( nullptr == act ) {
    return;
  }

  ActId id = static_cast<ActId>( act->data().toInt() );
  GeoGroup* gr = nullptr;
  switch (id) {
  case ActId::kFinishAct:
    slotFinishEditing();
    break;
  case ActId::kDeleteAct:
    for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
      selectedObjects_[i]->removeFromScene();
      delete selectedObjects_[i]->object();
      delete selectedObjects_[i];
    }
    selectedObjects_.clear();
    if ( nullptr != highlightObj_ ) {
      highlightObj_->removeFromScene();
      delete highlightObj_->object();
      delete highlightObj_;
      highlightObj_ = nullptr;
    }
    break;
  case ActId::kGroupAct:
    gr = new GeoGroup(getLayer());
    for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
      gr->add(const_cast<Object*>(selectedObjects_.at(i)->object()));
    }
    if ( nullptr != highlightObj_ ) { highlightObj_->removeFromScene(); }
    delete highlightObj_;
    highlightObj_ = nullptr;
    deleteSelectedObjects();
    break;
  case ActId::kUnGroupAct:
    if ( nullptr != highlightObj_ && highlightObj_->object()->type() == kGroup ) {
      GeoGroup* grObj = static_cast<GeoGroup*>(const_cast<Object*>(highlightObj_->object()));
      grObj->ungroup();
      delete grObj;
      if ( nullptr != highlightObj_ ) { highlightObj_->removeFromScene(); }
      delete highlightObj_;
      highlightObj_ = nullptr;
    }
    break;
  case ActId::kCopyAct:
    copyObjects();
    break;
  case ActId::kCutAct:
    cutObjects();
    break;
  default:
    break;
  }
}

void DrawToolsAction::deleteAllObject()
{
  delete highlightObj_;
  highlightObj_ = nullptr;

  delete editableObject_;
  editableObject_ = nullptr;


  for ( int i=0,isz=drawObjects_.size(); i<isz; ++i ) {
    delete drawObjects_[i];
  }

  for ( int i=0,isz=selectedObjects_.size(); i<isz; ++i ) {
    delete selectedObjects_[i];
  }
}

void DrawToolsAction::setMarkers(PolygonDrawObject* polyObj)
{
  setLastMarker(polyObj);
  setFirstMarker(polyObj);
}

void DrawToolsAction::setLastMarker(PolygonDrawObject *polyObj)
{
  if ( nullptr != lastMarker_ ) {
//    QObject::disconnect(lastMarker_, SIGNAL(changePos()), this, SLOT(slotAppendLastMarker()));
  }
  lastMarker_ = polyObj->lastMarker();;
  if ( nullptr == lastMarker_ ) {
    return;
  }
  lastPosLM_ = lastMarker_->pos();
  QObject::connect(lastMarker_, SIGNAL(changePos()), SLOT(slotAppendLastMarker()));
}

void DrawToolsAction::setFirstMarker(PolygonDrawObject *polyObj)
{
  if ( nullptr != firstMarker_ ) {
//    QObject::disconnect(firstMarker_, SIGNAL(changePos()), this, SLOT(slotAppendFirstMarker()));
  }
  firstMarker_ = polyObj->firstMarker();;
  if ( nullptr == firstMarker_ ) {
    return;
  }
  lastPosFM_ = firstMarker_->pos();
  QObject::connect(firstMarker_, SIGNAL(changePos()), SLOT(slotAppendFirstMarker()));
}

void DrawToolsAction::clearLastMarker()
{
  if ( nullptr != lastMarker_ ) {
//    QObject::disconnect(lastMarker_, SIGNAL(changePos()), this, SLOT(slotAppendLastMarker()));
  }
  lastMarker_ = nullptr;
}

void DrawToolsAction::clearFirstMarker()
{
  if ( nullptr != firstMarker_ ) {
//    QObject::disconnect(firstMarker_, SIGNAL(changePos()), this, SLOT(slotAppendFirstMarker()));
  }
  firstMarker_ = nullptr;
}

void DrawToolsAction::clearMarkers()
{
  clearLastMarker();
  clearFirstMarker();
}

void DrawToolsAction::copyObjects()
{
  if ( clipboard_ == nullptr) {
    error_log << QObject::tr("Ошибка. Невозможно выполнить операцию \"Копировать\"");
    return;
  }
  QList< QPair< PrimitiveTypes, std::string > > list;
  ObjectMimeData* data = new ObjectMimeData();
  if ( true == selectedObjects_.isEmpty() ) {
    if ( nullptr != highlightObj_ ) {
      serializeObject(highlightObj_->object(),list);
      data->setSerialObjectsList(list);
      clipboard_->setMimeData(data);
    }
    return;
  }
  for( auto highObj : selectedObjects_ ) {
    if ( nullptr != highObj && nullptr != highObj->object() ) {
      serializeObject(highObj->object(),list);
    }
  }
  data->setSerialObjectsList(list);
  clipboard_->setMimeData(data);
  return;
}

void DrawToolsAction::cutObjects()
{
  if ( clipboard_ == nullptr) {
    error_log << QObject::tr("Ошибка. Невозможно выполнить операцию \"Вырезать\"");
    return;
  }
  ObjectMimeData* data = new ObjectMimeData();
  QList< QPair< PrimitiveTypes, std::string > > list;
  if ( true == selectedObjects_.isEmpty() ) {
    if ( nullptr != highlightObj_ ) {
      serializeObject(highlightObj_->object(), list);
      highlightObj_->removeFromScene();
      Object* obj = const_cast<Object*>(highlightObj_->object());
      delete obj;
      obj = nullptr;
      delete highlightObj_;
      highlightObj_ = nullptr;
      data->setSerialObjectsList(list);
      clipboard_->setMimeData(data);
    }
    return;
  }
  for( auto highObj : selectedObjects_ ) {
    if ( nullptr != highObj && nullptr != highObj->object()) {
      serializeObject(highObj->object(),list);
    }
    Object* obj = const_cast<Object*>(highObj->object());
    highObj->removeFromScene();
    delete obj;
    obj = nullptr;
  }
  data->setSerialObjectsList(list);
  clipboard_->setMimeData(data);
  selectedObjects_.clear();
  return;
}

void DrawToolsAction::serializeObject(const Object* object, QList<QPair<PrimitiveTypes, std::string> >& list )
{
  auto serialize = [=]() {
    int size = 0;
    char* str = nullptr;
    if ( object->type() == PrimitiveTypes::kGroup ) {
      size = object->childDataSize();
      str = new char[size];
      size = object->serializeChildsToArray(str);
    }
    else {
      size = object->dataSize();
      str =  new char[size];
      size = object->serializeToArray(str);
    }
    std::string string(str, size);
    delete str;
    return string;
  };
  if ( nullptr != object) {
    switch (object->type()) {
    case PrimitiveTypes::kText:
    case PrimitiveTypes::kPixmap:
    case PrimitiveTypes::kPolygon:
    case PrimitiveTypes::kGroup:
      list.append(QPair<PrimitiveTypes, std::string>(static_cast<PrimitiveTypes>(object->type()), serialize()));
      break;
    default:
      break;
    }
  }
}

}
}
