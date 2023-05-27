#ifndef CTRLCLICKBUTTON_H
#define CTRLCLICKBUTTON_H

#include <qpushbutton.h>

namespace meteo {
namespace commons {

class CtrlClickButton : public QPushButton
{
  Q_OBJECT
public:
  CtrlClickButton( QWidget* parent );

signals:
  void signalCtrlClick();

protected:
  void mousePressEvent(QMouseEvent *e);


};

}
}

#endif
