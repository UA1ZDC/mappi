#include "conditiondateinterval.h"
#include <qobject.h>
#include <meteo/commons/global/global.h>
#include <sql/dbi/dbiquery.h>
#include <meteo/commons/global/dateformat.h>

namespace meteo {
namespace internal {

ConditionDateInterval::ConditionDateInterval( const QString& name, const QString& displayName, const QDateTime& dtBegin, const QDateTime& dtEnd) :
  GenericConditionPrivateBase(name, displayName),
  dtBegin_(dtBegin),
  dtEnd_(dtEnd)
{


}

ConditionDateInterval::~ConditionDateInterval()
{

}

bool ConditionDateInterval::isMatch(const QVariant& buf) const
{
  auto convertBufToDateTime = [](const QVariant& buf){
    if ( true == buf.canConvert<QDateTime>() ){
      return buf.value<QDateTime>();
    }
    if ( true == buf.canConvert<QString>() ){
      auto isodate = buf.value<QString>();
      return QDateTime::fromString(isodate, Qt::ISODate);
    }
    return QDateTime();
  };

  QDateTime dt = convertBufToDateTime(buf);
  if ( false == dt.isValid() ){
    warning_log << QObject::tr("Не удалось преобразовать буфер");
    return false;
  }

  return ( ( dtBegin_ <= dt ) && (dt <= dtEnd_ ) );
}

QString ConditionDateInterval::displayCondition() const
{
  if ( false == displayCondition_.isNull() )  {
    return displayCondition_;
  }
  static const QString& templ = QObject::tr("%1 = %2 - %3 ");
  return templ.arg(getFieldDisplayName())
      .arg(meteo::dateToHumanFull(dtBegin_))
      .arg(meteo::dateToHumanFull(dtEnd_));
}

const QDateTime& ConditionDateInterval::dtBegin() const
{
  return dtBegin_;
}

const QDateTime& ConditionDateInterval::dtEnd() const
{
  return dtEnd_;
}

void ConditionDateInterval::serialize( meteo::proto::ConditionProto *out ) const
{
  auto proto = out->mutable_condition_date_interval();
  proto->set_name(getFieldName().toStdString());
  proto->set_display_name(getFieldDisplayName().toStdString());
  proto->set_dt_begin(dtBegin_.toString(Qt::ISODate).toStdString());
  proto->set_dt_end(dtEnd_.toString(Qt::ISODate).toStdString());
}

QString ConditionDateInterval::getStringCondition(const ConnectProp& prop) const
{
  static const QString& queryName = QObject::tr("conditionmaker_datetime_interval");

  std::unique_ptr<DbiQuery> query( global::dbqueryByName(prop, queryName) );
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->argFieldName("field_name",getFieldName());
  query->arg("start_dt",dtBegin_);
  query->arg("end_dt",dtEnd_);
  return query->query();
}

}
}
