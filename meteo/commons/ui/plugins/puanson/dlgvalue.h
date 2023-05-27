#ifndef METEO_COMMONS_UI_PLUGINS_PUANSON_DLGVALUE_H
#define METEO_COMMONS_UI_PLUGINS_PUANSON_DLGVALUE_H

#include <qdialog.h>

#include <commons/meteo_data/meteo_data.h>

namespace Ui {
class DlgValue;
}

namespace meteo {
namespace map {

class DlgValue : public QDialog
{
  Q_OBJECT
  public:
    DlgValue( const QString& title, double val, QWidget* parent );
    ~DlgValue();

    double value() const ;
    control::QualityControl qual() const ;

  private:
    Ui::DlgValue* ui_;
    double oldval_;

  private slots:
    void slotOkClicked();
    void slotNoClicked();
};

}
}

#endif
