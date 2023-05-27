#ifndef METEO_COMMONS_FORECAST_TFDELEGATE_H
#define METEO_COMMONS_FORECAST_TFDELEGATE_H

#include <qstyleditemdelegate.h>

class TFItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TFItemDelegate(QObject *parent = 0);
    ~TFItemDelegate(){}

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const ;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent ( QEvent * event,
                                              QAbstractItemModel * model,
                                              const QStyleOptionViewItem & option,
                                              const QModelIndex & index );

    protected slots:
    void emitCommitData(int);
};

#endif // METEO_COMMONS_FORECAST_TFDELEGATE_H
