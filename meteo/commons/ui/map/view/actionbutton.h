#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QToolButton>

const int kActionBtnSize = 32;
const int kActionBtnIconSize = 24;
class ActionButton : public QToolButton
{
  Q_OBJECT
public:
  explicit ActionButton(QWidget * parent=0);

protected:
  virtual void enterEvent(QEvent* e);
  virtual void leaveEvent(QEvent* e);

signals:
  void hovered();
  void leave();

public slots:

};

#endif // ACTIONBUTTON_H
