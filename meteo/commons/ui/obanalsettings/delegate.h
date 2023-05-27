#ifndef DELEGATE_H
#define DELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QStandardItem>
#include <QStyledItemDelegate>

//! [0]
class CustomDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CustomDelegate(QObject *parent = 0, const QList<QStandardItem *> list = QList< QStandardItem * >());

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *, const QModelIndex &) const;
    void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

//    void paint( QPainter * painter, const QStyleOptionViewItem &option,
//            const QModelIndex &index ) const;

signals:
    void changed(int);
private slots:
    void slotChanged(int number);
private:
    QList< QStandardItem * > list_;
};
//! [0]

#endif
