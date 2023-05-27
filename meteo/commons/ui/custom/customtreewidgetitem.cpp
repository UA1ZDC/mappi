#include "customtreewidgetitem.h"

#include <cross-commons/debug/tlog.h>

enum {
  Min = 1,
  Max = 2
};

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidget* parent) :
  QTreeWidgetItem(parent)
{

}

bool CustomTreeWidgetItem::operator<(const QTreeWidgetItem& other) const
{
  int column = treeWidget()->sortColumn();
  QString a = this->text(column);
  QString b = other.text(column);
  {
    //compare integer
    bool aOk = false, bOk = false;
    int aInt = a.toInt(&aOk);
    int bInt = b.toInt(&bOk);
    if ( true == aOk && true == bOk) {
      return aInt < bInt;
    }
  }
  {
    //compare strings
    return a < b;
  }
}

void CustomTreeWidgetItem::setColorMin(const QColor& color)
{
 min_ = color;
 QPixmap pix(20,20);
 pix.fill(color);
 setIcon(Min, QIcon(pix));
}

void CustomTreeWidgetItem::setColorMax(const QColor& color)
{
 max_ = color;
 QPixmap pix(20,20);
 pix.fill(color);
 setIcon(Max, QIcon(pix));
}

void CustomTreeWidgetItem::setFillIcon(int column, const QString& iconPath)
{
  setIcon(column, QIcon(iconPath));  
}

void CustomTreeWidgetItem::setGradient(const TColorGradList& grad )
{
  gradlist_ = grad;
}

QColor CustomTreeWidgetItem::min() const
{
  return min_;
}

QColor CustomTreeWidgetItem::max() const
{
  return max_;
}
