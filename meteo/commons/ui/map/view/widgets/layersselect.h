#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_LAYERSSELECT_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_LAYERSSELECT_H

#include <qwidget.h>

class QTreeWidgetItem;

namespace Ui {
  class LayersSelection;
}

namespace meteo {
namespace map {
  class Document;
  class MapView;

  //! Виджет для выбора слоев при печати, сохранении
  class LayersSelect : public QWidget
  {
    Q_OBJECT

    public:
    explicit LayersSelect(MapView* view, QWidget* parent = 0);
    ~LayersSelect();

    QStringList checkedLayers(map::Document* doc) const;

    void loadLayers();
    int loadNonBaseLayers();
    void setCheckedVisibleLayers();
    void setCheckedNonBaseLayers(bool onlyVisible);
    void showUnvisibleLayers(bool show);

  signals:
    void layersChanged();
				  
  private slots:
    void slotCheckLayer(QTreeWidgetItem* item, int column);

  private:
    Document* currentDocument() const;

  private:
    Ui::LayersSelection* ui_;
    MapView* view_;
  };

}
}

#endif
