#ifndef METEO_COMMONS_UI_PLUGINS_GEODEZ_GEODEZ_H
#define METEO_COMMONS_UI_PLUGINS_GEODEZ_GEODEZ_H

#include <QWidget>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui {
class Geodez;
}

namespace meteo {
namespace map {

class Document;

class Geodez : public QDialog
{
  Q_OBJECT

public:
  explicit Geodez(QWidget *parent = 0);
  ~Geodez();

private:
  Ui::Geodez *ui_;

  MapView* view_;

private slots:

  void slotSetNewParallels();

  void slotSetDefaultValue();

  void slotReject();
};





}
}
#endif // GEODEZ_H
