#ifndef METEO_NOVOST_UI_COLORALERT_DELEGATE_H
#define METEO_NOVOST_UI_COLORALERT_DELEGATE_H

#include <qabstractitemmodel.h>
#include <qstyleditemdelegate.h>
#include <qtreewidget.h>

#include "conditionitem.h"
#include "conditionedit.h"

namespace meteo {
namespace tablo {

class Delegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit Delegate(QObject* parent = 0);
  virtual ~Delegate(){}

  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex &index) const;
  virtual void setEditorData(QWidget *editor, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel * model, const QModelIndex& index ) const;
  virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

public slots:
  void slotVisibilityChanged(bool visible);

private:
  QSize size_;
};

} // tablo
} // meteo

#endif // METEO_NOVOST_UI_COLORALERT_DELEGATE_H
