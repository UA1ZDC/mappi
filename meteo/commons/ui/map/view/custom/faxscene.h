#ifndef FAXSCENE_H
#define FAXSCENE_H

#include <QGraphicsScene>

class FaxScene : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit FaxScene(QObject *parent = 0);
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

signals:
  void faxMousePress();
  void faxMouseRelease();
  void faxMouseMove();

public slots:

};

#endif // FAXSCENE_H
