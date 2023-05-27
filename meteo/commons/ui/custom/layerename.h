#ifndef METEO_COMMONS_UI_CUSTOM_LAYERENAME_H
#define METEO_COMMONS_UI_CUSTOM_LAYERENAME_H

#include <qdialog.h>

namespace Ui {
  class LayerRename;
}

class LayerRename : public QDialog
{
  Q_OBJECT
  public:
    LayerRename( const QString& text, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~LayerRename();

    QString name() const ;

  private:
    Ui::LayerRename* ui_;
};

#endif
