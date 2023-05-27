#include "layersselect.h"
#include "ui_layersselection.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>

#include "../mapview.h"

using namespace meteo;
using namespace map;

enum Column
{
  kUUID = 0,
  kCHECK,
  kNAME
};


LayersSelect::LayersSelect(MapView* view, QWidget* parent /*= 0*/):
  QWidget(parent),
  ui_(0),
  view_(view)
{
  ui_ = new Ui::LayersSelection;

  ui_->setupUi(this);
  ui_->layersTreeWidget->hideColumn(kUUID);
  ui_->layersTreeWidget->setExpandsOnDoubleClick(false);
  ui_->layersTreeWidget->setItemsExpandable(false);
  ui_->layersTreeWidget->setUniformRowHeights(true);
  connect(ui_->layersTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), 
	  SLOT(slotCheckLayer(QTreeWidgetItem*, int)));
  // connect(ui_->layersTreeWidget, SIGNAL(itemChanged ( QTreeWidgetItem* , int)),
  // 	  SIGNAL(layersChanged()));
}


LayersSelect::~LayersSelect()
{
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}


Document* LayersSelect::currentDocument() const
{ 
  if (view_ != 0 &&
      view_->mapscene() != 0 &&
      view_->mapscene()->document() != 0) {
    return view_->mapscene()->document();
  }
  return 0;
}

//! Возвращает список uuids выделенных слоёв
QStringList LayersSelect::checkedLayers(map::Document* doc) const
{
  QStringList layerUuids;

  if (doc == 0) {
    return layerUuids;
  }

    for (int i = ui_->layersTreeWidget->topLevelItemCount() - 1; i >=0; --i) {
    QTreeWidgetItem* item = ui_->layersTreeWidget->topLevelItem(i);
    if (item != 0 && item->checkState(kCHECK) == Qt::Checked) {
      QList<map::Layer*> allLayers(doc->layers());
      QList<map::Layer*>::const_iterator it(allLayers.begin()),
                                         end(allLayers.end());
      for (; it != end; ++it) {
        map::Layer* layer = *it;
        if (layer != 0 && layer->name() == item->text(kNAME)) {
	  layerUuids.append(layer->uuid());
        }
      }
    }
  }
    return layerUuids;
}

//! Загрзука слоёв в виджет в соответствии с слоями документа
void LayersSelect::loadLayers()
{
  ui_->layersTreeWidget->clear();

  map::Document* original = currentDocument();

  if (original != 0) {
    for (int idx = original->layers().count() - 1; idx >= 0; --idx) {
      map::Layer* layer = original->layers().at(idx);
      if (layer != 0) {
	QTreeWidgetItem* item = new QTreeWidgetItem(ui_->layersTreeWidget);
 	item->setExpanded(false);
    item->setText(kUUID, layer->uuid());
    item->setText(kNAME, layer->name());
    item->setSizeHint(kCHECK, QSize(32, 32));
	// item->setSizeHint(NAME, QSize(32, 32));
	// item->setSizeHint(UUID, QSize(32, 32));
    item->setCheckState(kCHECK, Qt::Unchecked);
      }
    }
  }

  for (int i = 0, sz = ui_->layersTreeWidget->columnCount(); i < sz; ++i) {
    ui_->layersTreeWidget->resizeColumnToContents(i);
  }

}

int LayersSelect::loadNonBaseLayers()
{
  int cnt = 0;
  ui_->layersTreeWidget->clear();

  map::Document* original = currentDocument();

  if (original != 0) {
    for (int idx = original->layers().count() - 1; idx >= 0; --idx) {
      map::Layer* layer = original->layers().at(idx);
      if (layer != 0 && layer->isBase() == false && layer->name() != "Координатная сетка") {
	QTreeWidgetItem* item = new QTreeWidgetItem(ui_->layersTreeWidget);
 	item->setExpanded(false);
    item->setText(kUUID, layer->uuid());
    item->setText(kNAME, layer->name());
    item->setSizeHint(kCHECK, QSize(32, 32));
	// item->setSizeHint(NAME, QSize(32, 32));
	// item->setSizeHint(UUID, QSize(32, 32));
    item->setCheckState(kCHECK, Qt::Unchecked);
	cnt++;
      }
    }
  }

  for (int i = 0, sz = ui_->layersTreeWidget->columnCount(); i < sz; ++i) {
    ui_->layersTreeWidget->resizeColumnToContents(i);
  }

  return cnt;
}

  //! Выборка всех видимых слоев
void LayersSelect::setCheckedVisibleLayers()
{
  map::Document* original = currentDocument();  
  
  if (original != 0) {
    foreach (map::Layer* layer, original->layers()) {
      if (layer != 0 && layer->visible() == true) {
	for (int i = 0, sz = ui_->layersTreeWidget->topLevelItemCount(); i < sz; ++i) {
	  QTreeWidgetItem* item = ui_->layersTreeWidget->topLevelItem(i);
      if (item != 0 && item->text(kUUID) == layer->uuid()) {
        item->setCheckState(kCHECK, Qt::Checked);
	  }
	}
      }
    }
  }
  emit layersChanged();
}  

//! Выбор всех не базовых слоев
/*!
  \param onlyVisible true - только видимых базовых слоев
*/

void LayersSelect::setCheckedNonBaseLayers(bool onlyVisible)
{
   map::Document* original = currentDocument();  
  
  if (original != 0) {
    foreach (map::Layer* layer, original->layers()) {
      if (layer != 0 && layer->isBase() == false && (!onlyVisible || layer->visible())) {
	for (int i = 0, sz = ui_->layersTreeWidget->topLevelItemCount(); i < sz; ++i) {
	  QTreeWidgetItem* item = ui_->layersTreeWidget->topLevelItem(i);
      if (item != 0 && item->text(kUUID) == layer->uuid()) {
        item->setCheckState(kCHECK, Qt::Checked);
	  }
	}
      }
    }
  }
  emit layersChanged();
}

//! Выбор слоя при нажатии на любое место строки
void LayersSelect::slotCheckLayer(QTreeWidgetItem* item, int column)
{
  if (item != 0 && column != kCHECK) {
    Qt::CheckState currentState = item->checkState(kCHECK);
    if (currentState == Qt::Checked) {
      item->setCheckState(kCHECK, Qt::Unchecked);
    }
    else {
      item->setCheckState(kCHECK, Qt::Checked);
    }
  }
  emit layersChanged();
}

//! Показать/скрыть из списка невидимые слои
void LayersSelect::showUnvisibleLayers(bool show)
{
  map::Document* original = currentDocument();  
  
  if (original != 0) {
    foreach (map::Layer* layer, original->layers()) {
      if (layer != 0) {
	for (int i = 0, sz = ui_->layersTreeWidget->topLevelItemCount(); i < sz; ++i) {
	  QTreeWidgetItem* item = ui_->layersTreeWidget->topLevelItem(i);
	  if (item != 0 && item->text(kUUID) == layer->uuid()) {
	    if (layer->visible() == false) {
	      item->setCheckState(kCHECK, Qt::Unchecked);
	      item->setHidden(!show);
	    } else {
	      item->setHidden(false);
	    }
	  }
	}
      }
    }
  }
  emit layersChanged();
}
