#pragma once

#include <meteo/commons/settings/tusersettings.h>
#include <QAbstractTableModel>


namespace meteo {

namespace ui {

class UserModel :
  public QAbstractTableModel
{
  Q_OBJECT
  Q_DISABLE_COPY(UserModel)

public :
  typedef enum {
    ID = 0,
    USERNAME,
    FIO,

    COUNT
  } column_t;

public :
  explicit UserModel(QObject* parent = nullptr);
  virtual ~UserModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  void update();

private :
  const QMap<QString, app::User>& users_;
};

}

}
