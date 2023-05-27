#ifndef PUANSONMODEL_H
#define PUANSONMODEL_H

#include <QAbstractItemModel>

class PuansonModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit PuansonModel(QObject *parent = 0);
  Qt::ItemFlags flags ( const QModelIndex & index ) const ;

signals:

public slots:

};

#endif // PUANSONMODEL_H
