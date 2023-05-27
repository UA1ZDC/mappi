#include "forecastaccuracydatamodel.h"

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/services/forecast/tforecast.h>
#include <meteo/commons/services/forecast/tforecastlist.h>


namespace meteo  {
namespace ui {

static const QString forecastGetQuery = "aggregate_get_from_forecast_opr_viewer";
static const QString forecastGetQueryNoConditions = "aggregate_get_no_conditions_from_forecast_opr_viewer";
static const QString forecastCountQuery = "aggregate_count_from_forecast_opr_viewer";
static const QString forecastCountQueryNoConditions = "aggregate_count_no_conditions_from_forecast_opr_viewer";


ForecastAccuracyDataModel::ForecastAccuracyDataModel(int page) : CustomViewerDataModel (page)
{
  loadMethodNames();
  loadPunkts();
  loadCenters();
}

QString ForecastAccuracyDataModel::cellItemText(const QString& column, QHash<int, QString> row)
{
  auto col = columnIndexByName(column);
  auto data = row[col];
  if ( 0 == column.compare("center") ){
    auto value = centerReplaceTexts_.value(data);
    if ( false == value.isEmpty() ) {
      return value;
    }
    else {
      return QObject::tr("Неизвестный центр (%1)").arg(data);
    }
  }
  if ( 0 == column.compare("punkt.punkt_id") ){
    auto value = punktIdsTexts_[data];
    if ( false == value.isEmpty() ){
      return value;
    }
    else {
      return QObject::tr("Неизвестный пункт (%1)").arg(data);
    }
  }
  // if ( 0 == column.compare("val_descr") ){
  //   return valDescrTexts_[data];
  // }
  if ( 0 == column.compare("method") ){
    return methodDescrTexts_.value(data,
                                         QObject::tr("Неизвестный метод (%1)")
                                         .arg(data));
  }
  // if ( 0 == column.compare("type_level") ){
  //   return typeLevelTexts_[data];
  // }
  return data;
}

QString ForecastAccuracyDataModel::cellItemTooltip(const QString& column, QHash<int, QString> row)
{
  auto col = columnIndexByName(column);
  auto data = row[col];
  if ( 0 == column.compare("center") ){
    return centerTooltips_[data];
  }
  if ( 0 == column.compare("punkt._id") ){
    return punktIdsTooltips_[data];
  }
  if ( 0 == column.compare("val_descr") ){
    return valDescrTooltips_[data];
  }
  return QString();
}

/**
 * загружаем названия методов
 */
void ForecastAccuracyDataModel::loadMethodNames()
{
  QString methods_dir = global::kForecastMethodsDir;

  QDateTime cur_dt_ = QDateTime::currentDateTimeUtc();

  StationData cur_station_ = StationData();

  TForecastList *methods_list_ = new TForecastList(cur_dt_,cur_station_);

  if(0 == methods_list_->loadMethodsNoRun(methods_dir)){
    error_log.msgBox()<< msglog::kForecastNoMethods.arg(methods_dir);
    return;
  }

  foreach( TForecast* f, methods_list_->methodsList() ){
    if( nullptr != f ){
      methodDescrTexts_.insert( f->methodName(), f->methodFullName() );
    }
  }

  delete methods_list_;
  return;
}


void ForecastAccuracyDataModel::loadPunkts()
{

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return; }
  auto query = db->queryptrByName("get_all_punkts");
  if(nullptr == query) {return;}
  QString error;
 if(false == query->execInit( &error)){
    //debug_log<<error;
      return ;
    }


  QHash<QString, QString> valueReplace;
  QHash<QString, QString> valueTooltip;

  while ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    QString punktId = doc.valueString("station_id");
    QString punktName = doc.valueString("name");
    valueReplace.insert(punktId, punktName);
    valueTooltip.insert(punktId, tr("id: ") + punktId);
  }

  punktIdsTexts_ = valueReplace;
  punktIdsTooltips_ = valueTooltip;
}

void ForecastAccuracyDataModel::loadDescr()
{
  QHash<QString, QString> value_replace_text;
  QHash<QString, QString> value_tooltip;
  QStringList descriptors = TMeteoDescriptor::instance()->names();
  foreach( const QString& str, descriptors ){
    descr_t descr = TMeteoDescriptor::instance()->descriptor(str);
    QString specification = TMeteoDescriptor::instance()->property(descr).description;
    value_replace_text.insert(QString::number(descr), specification);
    value_tooltip.insert(QString::number(descr), tr("Индекс: ") + QString::number(descr));
  }

  valDescrTexts_ = value_replace_text;
  valDescrTooltips_ = value_tooltip;
}

void ForecastAccuracyDataModel::loadLevelTypes()
{
  QHash<QString, QString> value_replace_text;
  QMap<int, QString> levelTypes = meteo::global::kLevelTypes();
  foreach(int ltype, levelTypes.keys()){
    value_replace_text.insert(QString::number(ltype), levelTypes[ltype]);
  }
  typeLevelTexts_ = value_replace_text;
}

const QString& ForecastAccuracyDataModel::stringTemplateTotalRecords() const
{
  static auto templateStr = QObject::tr("Всего оценено прогнозов: %1");
  return templateStr;
}

const QString& ForecastAccuracyDataModel::windowTitle() const
{
  static auto title = QObject::tr("Оправдываемость результатов прогнозирования");
  return title;
}


void ForecastAccuracyDataModel::loadCenters()
{
  meteo::rpc::Channel* channel = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
  if ( nullptr == channel ) {
    error_log << tr("Не удалось подключиться к сервису sprinf.");
    return;
  }
  //rpc::TController ctrl(channel);
  meteo::sprinf::MeteoCenterRequest req;
  meteo::sprinf::MeteoCenters* resp = channel->remoteCall(&meteo::sprinf::SprinfService::GetMeteoCenters, req, 10000);
  channel->disconnect();
  delete channel;
  if ( nullptr == resp ) {
    error_log << tr("Ошибка при взаимодействии с сервисом sprinf. "
                        "Не получен ответ на запрос GetMeteoCenters.");
  }
  if ( resp->result() == false ) {
    QString errorStr = QString::fromUtf8(resp->error_message().c_str());
    delete resp;
    error_log << tr("Ошибка при выполнении запроса GetMeteoCenters. %1").arg(errorStr);
    return;
  }
  QHash<QString, QString> value_replace_text;
  QHash<QString, QString> value_tooltip;
  for ( int i=0, isz = resp->center_size(); i<isz; ++i ) {
    const ::meteo::sprinf::MeteoCenter& center = resp->center(i);
    value_replace_text.insert(QString::number(center.id()), pbtools::toQString(center.short_name()));
    value_tooltip.insert(QString::number(center.id()), tr("Индекс: ") + QString::number(center.id()));
  }
  centerTooltips_ = value_tooltip;
  centerReplaceTexts_ = value_replace_text;

  delete resp;
}

proto::CustomViewerId ForecastAccuracyDataModel::tableId() const
{
  return proto::CustomViewerId::kViewerForecastAccuracy;
}

}
}
