#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_DLGSPINBOX_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_DLGSPINBOX_H

#include <qdialog.h>

namespace Ui {
  class DlgSpinBox;
}

namespace meteo {
namespace puanson {

class DlgSpinbox : public QDialog
{
  Q_OBJECT
  public:
    DlgSpinbox( QWidget* parent = 0 );
    ~DlgSpinbox();

    double value() const ;
    void setValue( double val );

  protected:
    void keyReleaseEvent( QKeyEvent* e );

  private:
    Ui::DlgSpinBox* ui_;
};

}
}

#endif
