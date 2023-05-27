#ifndef FIELDSYNOPWIDG_H
#define FIELDSYNOPWIDG_H

#include <QtGui>


namespace meteo {
namespace map {


class FieldSynopWidget : public QDialog
{
  Q_OBJECT

public:

  enum Type{
    CloseButton  = 0,
    DoubleButton = 1
  };

  FieldSynopWidget(QWidget *parent = 0);
  ~FieldSynopWidget();

  void setText(const QString& text);
  const QString text() const;

  Type typeWidget() const;
  void setCloseVisible(bool on);
  void setBtnVisible(bool on);
  void setTypeWidget(Type type);
  void setViewportPos(const QPoint& pos);
  QPoint viewportPos() const;
  void setFrameColor(const QColor& color);


private slots:
  void slotClose();



private:
  QLabel* label_;
  QToolButton* close_;
  Type type_;
  QColor color_;
  QHBoxLayout* layout_;
  QPoint viewportPos_;

  void mousePressEvent(QMouseEvent *);
  void paintEvent(QPaintEvent*);
  void moveEvent(QMoveEvent *);


};

}
}

#endif // FIELDSYNOPWIDG_H
