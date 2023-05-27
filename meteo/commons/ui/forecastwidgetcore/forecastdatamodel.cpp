#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/global.h>
#include <commons/meteo_data/tmeteodescr.h>
#include "forecastdatamodel.h"

namespace meteo {
namespace ui {

static const QString forecastGetQuery = "aggregate_get_from_from_forecast_viewer";
static const QString forecastGetQueryNoConditions = "aggregate_get_no_conditions_from_from_forecast_viewer";
static const QString forecastCountQuery = "aggregate_count_from_forecast_viewer";
static const QString forecastCountQueryNoConditions = "aggregate_count_no_conditions_from_forecast_viewer";


ForecastDataModel::ForecastDataModel( int pageSize) : CustomViewerDataModel (pageSize)
{
  this->loadDescr();
  this->loadLevelTypes();
  this->loadReplaceValues();
  this->loadCenters();
}

/*
qint64 ForecastDataModel::countTotal()
{
  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) { return 0; }
  auto query = db->queryptrByName("count_collection");
  if(nullptr == query) {return 0;}
  query->arg("count_collection",this->tableName());

  if ( false == query->exec()){
    error_log << meteo::msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  auto n =result.valueInt32("n");
  return n;
}*/


void ForecastDataModel::loadReplaceValues()
{/*
  const QString &query = meteo::global::kMongoQueriesNew["sprinf_get_desciption_all"];
  NoSql db(global::mongodbConfForecast());
  if (!db.connect()){
    error_log << QObject::tr("Ошибка при подключении к базе данных");
    return;
  }

  if (!db.execQuery( query)) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
    return;
  }

  if ( !db.toCursor()){
    error_log << QObject::tr("Ошибка при выполнении запроса в базу данных");
    return;
  }

  while (db.next()) {
    Document doc = db.document();
    QString bufrCode = doc.valueString("bufr_code");
    QString code = doc.valueString("values.code");
    QString descr = doc.valueString("values.description");
    bufrCodes_[bufrCode][code] = descr;
  }
*/
}

QString ForecastDataModel::cellItemText(const QString& column, QHash<int, QString> row)
{
  auto col = this->columnIndexByName(column);
  auto data = row[col];
  if ( 0 == column.compare("param.descr") ){
    return this->descrReplaceList_.value(data,
                                         QObject::tr("Неизвестный дескриптор (%1)")
                                         .arg(data));
  }
  if ( 0 == column.compare("level_type" )){
    return this->levelTypeReplaceList_.value(data,
                                             QObject::tr("Неизвестный уровень (%1)")
                                             .arg(data) );
  }

  const QString& value = row[columnIndexByName(column)];
  if ( 0 == column.compare("hour") ) {
    return QString::number(value.toInt() / 3600);
  }
  if ( 0 == column.compare("param.code") ){
    const QString& descr = row[columnIndexByName("param.descr")];
    QString bufrValue = bufrCodes_[descr][value];
    if (bufrValue.isEmpty()) {
      return value;
    } else {
      return bufrValue;
    }
  }
  if ( 0 == column.compare("center") ){
    return this->centerReplaceTexts_.value(data,
                                           QObject::tr("Неизвестный центр(%1)").arg(data));
  }

  return value;
}

void ForecastDataModel::loadCenters()
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
  this->centerTooltips_ = value_tooltip;
  this->centerReplaceTexts_ = value_replace_text;

  delete resp;
}

QString ForecastDataModel::cellItemTooltip(const QString& column, QHash<int, QString> row)
{
  auto col = this->columnIndexByName(column);
  auto data = row[col];
  if ( 0 == column.compare("param.descr") ){
    return this->descrTooltipList_[data];
  }
  if ("hour" == column) {
    return data;
  }
  if ("param.code" == column){
    const QString& descr = row[columnIndexByName("param.descr")];
    QString bufrValue = bufrCodes_[descr][data];
    if (bufrValue.isEmpty()) {
      return data;
    } else {
      return bufrValue;
    }
  }

  return QString();
}

void ForecastDataModel::loadDescr()
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
  this->descrReplaceList_ = value_replace_text;
  this->descrTooltipList_ = value_tooltip;
}


void ForecastDataModel::loadLevelTypes()
{
  QHash<QString, QString> value_replace_text;
  QMap<int, QString> levelTypes = meteo::global::kLevelTypes();
  foreach(int ltype, levelTypes.keys()){
    value_replace_text.insert(QString::number(ltype), levelTypes[ltype]);
  }
  this->levelTypeReplaceList_ = value_replace_text;
}


const QString& ForecastDataModel::stringTemplateTotalRecords() const
{
  static auto templateStr = QObject::tr("Всего прогнозов: %1");
  return templateStr;
}

const QString& ForecastDataModel::windowTitle() const
{
  static auto title = QObject::tr("Результаты прогнозирования");
  return title;
}

proto::CustomViewerId ForecastDataModel::tableId() const
{
  return proto::CustomViewerId::kViewerForecast;
}

}
}
