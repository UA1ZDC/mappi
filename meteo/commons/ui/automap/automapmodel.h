#ifndef AUTOMAPMODEL_H
#define AUTOMAPMODEL_H

#include "joblist.h"
#include <meteo/commons/ui/jobwidget/jobwidget.h>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>


namespace meteo {

namespace map {

class AutoMapModel :
  public QAbstractTableModel
{
  Q_OBJECT
  Q_DISABLE_COPY(AutoMapModel)

public :
  typedef enum {
    NAME = 0,
    TIME_T,     // debug mode
    NEXT,
    LAST,
    STATUS,
    VIEW,

    COUNT
  } column_t;

public :
  explicit AutoMapModel(JobList* jobList, QObject* parent = nullptr);
  virtual ~AutoMapModel();

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

  void updatePeriod();
  void update();

private :
  JobWrapper::ptr_t getJob(const QModelIndex& index) const;
  QString timeToStr(const QDateTime& dt) const;

private :
  JobList* jobList_;
};


class AutoMapSortProxyModel :
  public QSortFilterProxyModel
{
public :
  explicit AutoMapSortProxyModel(QObject* parent = nullptr);
  virtual ~AutoMapSortProxyModel();

  virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
};

}

}

#endif // AUTOMAPMODEL_H
