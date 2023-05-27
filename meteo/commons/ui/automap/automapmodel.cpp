#include "automapmodel.h"


namespace meteo {

namespace map {

// class AutoMapModel
AutoMapModel::AutoMapModel(JobList* jobList, QObject* parent /*=*/) :
    QAbstractTableModel(parent),
  jobList_(jobList)
{
}

AutoMapModel::~AutoMapModel()
{
  jobList_ = nullptr;
}

int AutoMapModel::rowCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return jobList_->count();
}

int AutoMapModel::columnCount(const QModelIndex& parent /*=*/) const
{
  Q_UNUSED(parent)
  return AutoMapModel::COUNT;
}

QVariant AutoMapModel::headerData(int section, Qt::Orientation orientation, int role /*=*/) const
{
  switch (role) {
    case Qt::TextAlignmentRole :
      // return Qt::AlignCenter;
      return Qt::AlignLeft;

    case Qt::DisplayRole : {
      if (orientation == Qt::Horizontal) {
        switch (section) {
          case AutoMapModel::NAME :
            return QObject::tr("Наименование");

          case AutoMapModel::TIME_T :
            return QObject::tr("След. запуск (time_t)");

          case AutoMapModel::NEXT :
            return QObject::tr("След. запуск");

          case AutoMapModel::LAST :
            return QObject::tr("Пред. запуск");

          case AutoMapModel::STATUS :
            return QObject::tr("Статус");

          case AutoMapModel::VIEW :
            return QObject::tr("Просмотр");
        }
      }

      break;
    }
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags AutoMapModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
  if (!index.isValid())
    return defaultFlags;

  if (index.column() == AutoMapModel::NAME)
    defaultFlags |= Qt::ItemIsUserCheckable;

  return defaultFlags | Qt::ItemIsEnabled;
}

QVariant AutoMapModel::data(const QModelIndex& index, int role /*=*/) const
{
  if (!index.isValid())
    return QVariant();

  JobWrapper::ptr_t job = getJob(index);
  switch (role) {
    case Qt::TextColorRole :
      if (!job->raw().enabled())
          return QColor(Qt::gray);
      break ;

    // case Qt::BackgroundColorRole
    // case Qt::TextAlignmentRole :
    //   return Qt::AlignCenter;

    case Qt::CheckStateRole :
      switch (index.column()) {
        case AutoMapModel::NAME :
          return (job->raw().enabled() ? Qt::Checked : Qt::Unchecked);
      }
      break;

    case Qt::DisplayRole : {
      const Period& period = job->fixedPeriod();
      switch (index.column()) {
        case AutoMapModel::NAME :
          return job->title();

        case AutoMapModel::TIME_T :
          return period.nearest().toTime_t();

        case AutoMapModel::NEXT :
          return timeToStr(period.nearest());

        case AutoMapModel::LAST :
          return timeToStr(period.last());

        case AutoMapModel::STATUS :
          switch (job->getState()) {
            case JobWrapper::MAP_FORMED :
              return QObject::tr("карты сформированы");

            case JobWrapper::MAP_NOT_FORMED :
              return QObject::tr("нет сформированных карт");
          }
      }

      break;
    }
  }

  return QVariant();
}

bool AutoMapModel::setData(const QModelIndex& index, const QVariant& value, int role /*=*/)
{
  if (!index.isValid())
    return false;

  JobWrapper::ptr_t job = getJob(index);
  switch (role) {
    case Qt::CheckStateRole :
      switch (index.column())
        case AutoMapModel::NAME :
          jobList_->setEnabled(index.row(), (value == Qt::Checked));
          break ;
  }

  emit dataChanged(index, QAbstractTableModel::index(index.row(), AutoMapModel::STATUS));
  // emit dataChanged(index, index);
  return true;
}

bool AutoMapModel::insertRows(int row, int count, const QModelIndex& parent /*=*/)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();
  return true;
}

bool AutoMapModel::removeRows(int row, int count, const QModelIndex& parent /*=*/)
{
  beginRemoveRows(parent, row, row + count - 1);
  endRemoveRows();
  return true;
}

JobWrapper::ptr_t AutoMapModel::getJob(const QModelIndex& index) const
{
  return jobList_->get(index.row());
}

QString AutoMapModel::timeToStr(const QDateTime& dt) const
{
  return dt.toString("hh:mm").replace("T", " ").replace("Z", "");
}

void AutoMapModel::updatePeriod()
{
  for (int row = 0; row < jobList_->count(); ++row) {
    jobList_->get(row)->ajustFixedPeriod();
    emit dataChanged(index(row, AutoMapModel::TIME_T), index(row, AutoMapModel::LAST));
  }
}

void AutoMapModel::update()
{
  beginResetModel();
  resetInternalData();
  endResetModel();
}


// class AutoMapSortProxyModel
AutoMapSortProxyModel::AutoMapSortProxyModel(QObject* parent /*=*/) :
    QSortFilterProxyModel(parent)
{
}

AutoMapSortProxyModel::~AutoMapSortProxyModel() {
}

bool AutoMapSortProxyModel::insertRows(int row, int count, const QModelIndex& parent /*=*/)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();
  return true;
}

bool AutoMapSortProxyModel::removeRows(int row, int count, const QModelIndex& parent /*=*/)
{
  beginRemoveRows(parent, row, row + count - 1);
  endRemoveRows();
  return true;
}

}

}
