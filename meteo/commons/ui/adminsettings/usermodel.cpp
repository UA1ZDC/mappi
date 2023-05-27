#include "usermodel.h"


namespace meteo {

namespace ui {

UserModel::UserModel(QObject* parent /*=*/) :
    QAbstractTableModel(parent),
  users_(TUserSettings::instance()->userList())
{
}

UserModel::~UserModel()
{
}

int UserModel::rowCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return users_.count();
}

int UserModel::columnCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return UserModel::COUNT;
}

QVariant UserModel::headerData(int section, Qt::Orientation orientation, int role /*=*/) const
{
  switch (role) {
    case Qt::TextAlignmentRole :
      return Qt::AlignLeft;

    case Qt::DisplayRole : {
      if (orientation == Qt::Horizontal) {
        switch (section) {
          case UserModel::ID :
            return QObject::tr("ID");

          case UserModel::USERNAME :
            return QObject::tr("Логин");

          case UserModel::FIO :
            return QObject::tr("ФИО");
        }
      }

      break ;
    }
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags UserModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractTableModel::flags(index);
  if (!index.isValid())
    return flags;

  if (index.column() == UserModel::USERNAME)
    flags |= Qt::ItemIsUserCheckable;

  return (flags | Qt::ItemIsEnabled);
}

QVariant UserModel::data(const QModelIndex& index, int role /*=*/) const
{
  if (!index.isValid())
    return QVariant();

  app::User user = (users_.begin() + index.row()).value();
  switch (role) {
    case Qt::TextColorRole :
      if (user.block())
          return QColor(Qt::gray);
      break ;

    case Qt::CheckStateRole :
      switch (index.column()) {
        case UserModel::USERNAME :
          return (user.block() ? Qt::Unchecked : Qt::Checked);
      }
      break;

    case Qt::DisplayRole : {
      QString username = user.username().c_str();
      switch (index.column()) {
        case UserModel::ID :
          return user.id();

        case UserModel::USERNAME :
          return username;

        case UserModel::FIO :
          return TUserSettings::instance()->getUserShortName(username);
      }

      break;
    }
  }

  return QVariant();
}

bool UserModel::setData(const QModelIndex& index, const QVariant& value, int role /*=*/)
{
  if (!index.isValid())
    return false;

  app::User user = (users_.begin() + index.row()).value();
  switch (role) {
    case Qt::CheckStateRole :
      switch (index.column())
        case UserModel::USERNAME :
          TUserSettings::instance()->blockUser(data(index).toString(), (value != Qt::Checked));
          emit dataChanged(index, index);
          return true;
  }

  return false;
}


void UserModel::update()
{
  beginResetModel();
  resetInternalData();
  endResetModel();
}

}

}
