#pragma once

#include <meteo/commons/settings/tusersettings.h>
#include <QAbstractTableModel>


namespace meteo {

namespace ui {

class DictModel :
  public QAbstractTableModel
{
  Q_OBJECT
  Q_DISABLE_COPY(DictModel)

public :
  typedef enum {
    ID = 0,
    NAME,

    COUNT
  } column_t;

public :
  explicit DictModel(const internal::TUserSettings::dict_t& dict, QObject* parent = nullptr);
  virtual ~DictModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  void update();

private :
  const internal::TUserSettings::dict_t& dict_;
};

}

}
