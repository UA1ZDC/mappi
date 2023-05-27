#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLSACTION_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLSACTION_H

#include <qundostack.h>

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/global/common.h>

#include "drawtoolbarwidget.h"
#include "geoproxywidget.h"

class QTimer;
class ActionButton;
class QClipboard;


namespace meteo {

class MarkerItem;

namespace map {

class Layer;
class Object;
class GeoText;
class GeoPixmap;
class GroupObject;
class DrawObject;
class HighlightObject;
class GeoGraphTextItem;
class PolygonDrawObject;
class PixmapToolGraphicsItem;
class ControlPanelTextWidget;
class ControlPanelPixmapWidget;

const QString kDrawToolsActionName = "drawtoolsaction";

class DrawToolsAction : public Action
{
  Q_OBJECT
public:
  enum State {
    kNone,
    kCreate,
    kEdit
  };
  enum ActId {
    kNoAct,
    kFinishAct,
    kDeleteAct,
    kGroupAct,
    kUnGroupAct,
    kCutAct,
    kCopyAct,
    kRaspred
  };
  DrawToolsAction(MapScene* scene);
  virtual ~DrawToolsAction();

  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void wheelEvent(QWheelEvent* e);
  void mouseClickEvent(QMouseEvent* e);
  void keyReleaseEvent(QKeyEvent* event);
  QList<QString> getSelectedObjects();
  QString getHighLightObject();

  //! Возвращает слой, на котором осуществляется работа с объектами. При необходимости
  //! создаёт пользовательский слой.
  Layer* getLayer();

  //! В данном методе реализована обработка событий передаваемых сценой
  virtual bool eventFilter(QObject* obj, QEvent* e);
  //!
  virtual void addActionsToMenu(Menu *menu) const;


private slots:
  void slotToolActivate(DrawToolbarWidget::Tool currentTool);
  void slotApplyProp();

  void slotFinishDraw();
  void slotFinishEditing();
  void slotAbortDraw();

  void slotDeleteEditing();

  void slotProcessMenuAction();


  void slotShow();
  void slotHide();
  void slotSwitch();

  void slotShowTool( DrawToolbarWidget::Tool tool );
  void slotHideTool( DrawToolbarWidget::Tool tool );

  void slotOffActions();
  void slotOnActions();

  void slotPixmapDelete();
  void slotTextDelete();
  void slotGroupDelete();

  void slotPixmapAbort();
  void slotTextAbort();
  void slotGroupAbort();

  void slotAppendLastMarker();
  void slotAppendFirstMarker();

private:
  Layer* createLayer();
  bool isWidgetUnderMouse(const QPointF& screenPos) const;
  void deleteSelectedObjects();
  void beginEdit(Object* obj);

  Object* getTopParentAt(const QPoint& scenePos) const;
  void highlightObject(Object* object);

  DrawToolbarWidget::Tool currentTool() const;
  PropWidget* currentWidget() const;
  PropWidget* widgetForTool(const DrawToolbarWidget::Tool tool) const;
  QGraphicsProxyWidget* proxyWidgetForTool(const DrawToolbarWidget::Tool tool) const;

  DrawToolbarWidget::Tool currentToolForEditObject() const;
  void activateTool(DrawToolbarWidget::Tool tool);

  void setupContextMenu(Menu* menu, const QPoint& screenPos) const;
  void processContextMenu(QAction* act);

  void deleteObject( DrawObject* object );
  void deleteAllObject();
  void setMarkers( PolygonDrawObject* polyObj );
  void setLastMarker( PolygonDrawObject* polyObj );
  void setFirstMarker( PolygonDrawObject* polyObj );
  void clearLastMarker();
  void clearFirstMarker();
  void clearMarkers();
  void copyObjects();
  void cutObjects();
  void serializeObject(const Object *object, QList<QPair<meteo::PrimitiveTypes, std::string> >& list);

private:
  bool enabled_ = false;
  bool grabed_ = false;
  ActionButton* actBtn_ = nullptr;

  PropWidget* lineProp_ = nullptr;
  PropWidget* frontProp_ = nullptr;
  PropWidget* polygonProp_ = nullptr;
  PropWidget* textProp_ = nullptr;
  PropWidget* pixmapProp_ = nullptr;

  QGraphicsProxyWidget* linePW_ = nullptr;
  QGraphicsProxyWidget* frontPW_ = nullptr;
  QGraphicsProxyWidget* polygonPW_ = nullptr;
  QGraphicsProxyWidget* textPW_ = nullptr;
  QGraphicsProxyWidget* pixmapPW_ = nullptr;
  QGraphicsProxyWidget* widgetdraw_ = nullptr;
  QGraphicsProxyWidget* drawPanel_ = nullptr;
  GeoProxyWidget* controlPanel_ = nullptr;

  ControlPanelTextWidget* controlPanelText_     = nullptr;
  ControlPanelPixmapWidget* controlPanelPixmap_ = nullptr;

  HighlightObject* highlightObj_ = nullptr;

  QString layerUuid_;

  PixmapToolGraphicsItem* editPixmap_ = nullptr;
  GeoGraphTextItem* editText_;
  DrawObject* editableObject_ = nullptr;
  QList<DrawObject*> drawObjects_;
  QList<HighlightObject*> selectedObjects_;
  DrawToolbarWidget* toolbar_ = nullptr;
  QUndoStack undoStack_;
  State state_ = State::kNone;
  GeoText* editGeoTxt_ = nullptr;
  GeoPixmap* editGeoPix_ = nullptr;
  MarkerItem* lastMarker_ = nullptr;
  MarkerItem* firstMarker_ = nullptr;
  QPointF lastPosLM_;
  QPointF lastPosFM_;
  QTimer* timer_ = nullptr;
  GroupObject* groupObject_ = nullptr;
  QClipboard* clipboard_ = nullptr;

signals:
  void deleteBtnOff();
  void deleteBtnOn();

};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_DRAWTOOLSACTION_H
