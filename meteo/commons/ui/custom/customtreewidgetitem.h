#ifndef CUSTOMTREEWIDGETITEM_H
#define CUSTOMTREEWIDGETITEM_H

#include <qtreewidget.h>

#include <commons/funcs/tcolorgrad.h>

class CustomTreeWidgetItem : public QTreeWidgetItem
{
  public:
    CustomTreeWidgetItem(QTreeWidget* parent);
    void setColorMin(const QColor& color);
    void setColorMax(const QColor& color);
    void setFillIcon(int column, const QString& iconPath);
    void setGradient( const TColorGradList& list );

    QColor min() const;
    QColor max() const;
    bool hasGradient() const { return gradlist_.size() != 0; }
    const TColorGradList& gradient() const { return gradlist_; }
    void removeGradient() { return gradlist_.clear(); }

  private:
    QColor min_;
    QColor max_;
    TColorGradList gradlist_;
    bool operator<(const QTreeWidgetItem &other) const;
};

#endif
