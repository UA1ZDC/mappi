#include "customvieweritem.h"

CustomViewerItem::CustomViewerItem(int type)
  : QTableWidgetItem(type)
{

}

CustomViewerItem::CustomViewerItem(const QString& text, int type)
  : QTableWidgetItem(text, type)
{

}

CustomViewerItem::CustomViewerItem(const QIcon& icon, const QString& text, int type)
  : QTableWidgetItem(icon, text, type)
{

}

CustomViewerItem::CustomViewerItem(const QTableWidgetItem& other)
  : QTableWidgetItem(other)
{

}

bool CustomViewerItem::operator<(const QTableWidgetItem& other) const
{
  QRegExp re("\\d*");
  if (re.exactMatch(text()) && re.exactMatch(other.text())){
    return text().toDouble() < other.text().toDouble();
  }
  else{
    return QTableWidgetItem::operator<(other);
  }
}
