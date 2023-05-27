#ifndef LISTWIDGETITEMMAP_H
#define LISTWIDGETITEMMAP_H

#include <QWidget>

namespace Ui {
class ListWidgetItemMap;
}

class ListWidgetItemMap : public QWidget
{
  Q_OBJECT

public:
  explicit ListWidgetItemMap(QWidget *parent = 0);
  ~ListWidgetItemMap();

private:
  Ui::ListWidgetItemMap *ui;
};

#endif // LISTWIDGETITEMMAP_H
