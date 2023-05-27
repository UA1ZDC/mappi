#ifndef MAPPI_UI_PLUGINS_SESSIONS_RECEIVERIMAGEVIEW_H
#define MAPPI_UI_PLUGINS_SESSIONS_RECEIVERIMAGEVIEW_H

#include <qgraphicsview.h>

class QGraphicsScene;

namespace mappi {

class ReceiverImageView : public QGraphicsView
{
  Q_OBJECT
public:
  ReceiverImageView(QWidget* parent = nullptr);
  ReceiverImageView(QGraphicsScene* scene, QWidget* parent = nullptr);

protected:
  QPoint oldPoint_;
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SESSIONS_RECEIVERIMAGEVIEW_H
