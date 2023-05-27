#include "listwidgetitemmap.h"
#include "ui_listwidgetitemmap.h"

ListWidgetItemMap::ListWidgetItemMap(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ListWidgetItemMap)
{
  ui->setupUi(this);
}

ListWidgetItemMap::~ListWidgetItemMap()
{
  delete ui;
}
