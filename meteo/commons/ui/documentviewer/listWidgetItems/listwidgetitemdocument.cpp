#include "listwidgetitemdocument.h"
#include "ui_listwidgetitemdocument.h"
#include <cross-commons/debug/tlog.h>

ListWidgetItemDocument::ListWidgetItemDocument(DocumentInfo* info):
  QWidget(nullptr),
  ui(new Ui::ListWidgetItemDocument)
{
  ui->setupUi(this);
  ui->labelDt->setText(info->dt().toString(Qt::ISODate));
  ui->labelName->setText(info->getName());
  ui->labelStationIndex->setText(info->station());
}

ListWidgetItemDocument::~ListWidgetItemDocument()
{
  delete ui;
}
