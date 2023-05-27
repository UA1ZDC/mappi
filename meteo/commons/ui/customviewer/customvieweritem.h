#ifndef CUSTOMVIEWERITEM_H
#define CUSTOMVIEWERITEM_H

#include <QTableWidgetItem>

class CustomViewerItem : public QTableWidgetItem
{
public:
  CustomViewerItem(int type = Type);
  CustomViewerItem(const QString &text, int type = Type);
  CustomViewerItem(const QIcon &icon, const QString &text, int type = Type);
  CustomViewerItem(const QTableWidgetItem &other);
private:
  bool operator<(const QTableWidgetItem &other) const;
};

#endif // CUSTOMVIEWERITEM_H
