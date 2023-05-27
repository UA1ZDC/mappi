#ifndef MASLO_UI_SETTINGS_DATASORTITEM_H
#define MASLO_UI_SETTINGS_DATASORTITEM_H
#include <qtreewidget.h>

class DataSortItem : public QTreeWidgetItem
{
public:
  DataSortItem(QTreeWidget* parent):
    QTreeWidgetItem(parent){
  }
private:
  virtual bool operator<(const QTreeWidgetItem &other) const override;
};

#endif
