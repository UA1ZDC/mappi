#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_DOUBLESPINACTION_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_DOUBLESPINACTION_H

#include <qspinbox.h>

class QAction;

namespace meteo {
namespace map {

class DoubleSpinAction : public QDoubleSpinBox
{
  Q_OBJECT
  public:
    DoubleSpinAction( QWidget* parent, QAction* a );
    ~DoubleSpinAction();

  protected:
    void keyPressEvent( QKeyEvent* e );

  private:

    QAction* action_;
};

}
}

#endif
