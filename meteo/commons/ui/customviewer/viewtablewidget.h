#ifndef VIEWTABLEWIDGET_H
#define VIEWTABLEWIDGET_H
#include <QObject>
#include <QTableWidget>

class ViewTableWidget : public QTableWidget
{
  Q_OBJECT
public:
  ViewTableWidget(QWidget* parent);
  void scrollContentsBy(int dx, int dy);
};

#endif // VIEWTABLEWIDGET_H
