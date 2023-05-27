#ifndef METEO_COMMONS_UI_MAP_VIEW_ACTIONS_MOUSECOORDACTION_H
#define METEO_COMMONS_UI_MAP_VIEW_ACTIONS_MOUSECOORDACTION_H

#include <QList>
#include <QString>
#include "renderitem.h"
#include <meteo/commons/ui/map/view/actions/action.h>

class QGraphicsProxyWidget;

class ActionButton;

namespace meteo {

class GeoPoint;

namespace map {

class CoordWidget;
class BaloonProxyWidget;

class BaloonAction : public Action
{
  Q_OBJECT

public:
  explicit BaloonAction( MapScene* scene, const QString& name );
  ~BaloonAction();

  void mouseMoveEvent( QMouseEvent* e );
  void wheelEvent(QWheelEvent *event);
  void resizeEvent(QResizeEvent* event);

protected:
  void adjustProxy();
  void onDefaultPos();
  bool onHover();
  bool toMouseReleaseEvent(QMouseEvent* e);

protected:
  CoordWidget* coords_;
  BaloonProxyWidget* baloonProxy_;
  QList<QObject*> baloonList_;
  QMap<QToolButton*, RenderItem*> renderList_;
  bool handing_;

private slots:
  void slotRemoveProxy(QObject *obj);
  void slotRemoveRenderItem();
public slots:
  void deleteWidget();


};

class MouseCoordAction : public BaloonAction
{
  Q_OBJECT

public:
  explicit MouseCoordAction( MapScene* scene );
  ~MouseCoordAction();

  void mouseMoveEvent( QMouseEvent* e );
  void mouseReleaseEvent(QMouseEvent* e);

  //! Устанавливает форматную строку для отображаемых значений.
  void setTextFormat(const QString& textFormat) { textFormat_ = textFormat; }

  static QString coordToString(const GeoPoint& gp);
  static QString coordToString(const GeoPoint& gp, const QString& textFormat);

private slots:
  void slotTurnCoordWidget(bool on);
  void slotTurnCoordWidgetOff();

private:
  void createWidget();

private:
  ActionButton* btncoords_;
  QString textFormat_;
};


} // map
} // meteo

#endif // METEO_MAP_COMMONS_UI_VIEW_ACTIONS_MOUSECOORDACTION_H
