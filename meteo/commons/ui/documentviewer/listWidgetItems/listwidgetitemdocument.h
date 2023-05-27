#ifndef LISTWIDGETITEMDOCUMENT_H
#define LISTWIDGETITEMDOCUMENT_H

#include <QWidget>

#include "info.h"

namespace Ui {
class ListWidgetItemDocument;
}

class ListWidgetItemDocument : public QWidget
{
  Q_OBJECT

public:
  explicit ListWidgetItemDocument( DocumentInfo* info);
  ~ListWidgetItemDocument();

private:
  Ui::ListWidgetItemDocument *ui;
};

#endif // LISTWIDGETITEMDOCUMENT_H
