#include "dictmodel.h"


namespace meteo {

namespace ui {

DictModel::DictModel(const internal::TUserSettings::dict_t& dict, QObject* parent /*=*/) :
    QAbstractTableModel(parent),
  dict_(dict)
{
}

DictModel::~DictModel()
{
}

int DictModel::rowCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return dict_.count();
}

int DictModel::columnCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return DictModel::COUNT;
}

QVariant DictModel::headerData(int section, Qt::Orientation orientation, int role /*=*/) const
{
  switch (role) {
    case Qt::TextAlignmentRole :
      return Qt::AlignLeft;

    case Qt::DisplayRole : {
      if (orientation == Qt::Horizontal) {
        switch (section) {
          case DictModel::ID :
            return QObject::tr("ID");

          case DictModel::NAME :
            return QObject::tr("Наименование");
        }
      }

      break;
    }
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags DictModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
  if (!index.isValid())
    return defaultFlags;

  return defaultFlags | Qt::ItemIsEnabled;
}

QVariant DictModel::data(const QModelIndex& index, int role /*=*/) const
{
  if (!index.isValid())
    return QVariant();

  switch (role) {
    case Qt::DisplayRole : {
      internal::TUserSettings::dict_t::const_iterator it = dict_.begin() + index.row();
      switch (index.column()) {
        case DictModel::ID :
          return it.key();

        case DictModel::NAME :
          return it.value();
      }

      break;
    }
  }

  return QVariant();
}

void DictModel::update()
{
  beginResetModel();
  resetInternalData();
  endResetModel();
}

}

}
