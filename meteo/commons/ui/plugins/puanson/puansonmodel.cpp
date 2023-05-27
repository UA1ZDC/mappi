#include "puansonmodel.h"
#include "puansoneditform.h"

PuansonModel::PuansonModel(QObject *parent) :
  QAbstractItemModel(parent)
{
}

Qt::ItemFlags PuansonModel::QAbstractItemModel::flags(const QModelIndex &index) const
{
  if (meteo::map::kVal == index.column())
  {
    return index.flags() | Qt::ItemIsEditable;
  }
  return index.flags();
}
