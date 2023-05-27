#include "viewtablewidget.h"
#include <meteo/commons/ui/viewheader/viewheader.h>

ViewTableWidget::ViewTableWidget(QWidget *parent) : QTableWidget( parent )
{

}

void ViewTableWidget::scrollContentsBy(int dx, int dy)
{
    QTableWidget::scrollContentsBy(dx, dy);
}
