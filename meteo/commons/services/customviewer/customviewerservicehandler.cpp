#include "customviewerservicehandler.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/dateformat.h>
#include <qmimedatabase.h>
#include <meteo/commons/ui/customviewer/decode/tlgdecode.h>
#include <meteo/commons/msgparser/common.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/funcs/tlgtextobject.h>

#include <sql/dbi/dbi.h>
#include "loader.h"
#include "counter.h"
#include "postwork.h"

namespace meteo {

static auto kConfigLoadError = QObject::tr("Ошибка загрузки конфигурации для %1");

CustomViewerServiceHandler::CustomViewerServiceHandler()
{
  props_.insert(proto::kViewerMsg, global::mongodbConfTelegram());
  props_.insert(proto::kViewerJournal, global::mongodbConfJournal());
  props_.insert(proto::kViewerForecast, global::mongodbConfMeteo());
  props_.insert(proto::kViewerForecastAccuracy, global::mongodbConfMeteo());

  props_.insert(proto::kViewerSend, global::mongodbConfTelegram());
  props_.insert(proto::kViewerRcv, global::mongodbConfTelegram());


  counters_.insert(proto::kViewerMsg, &countTelegrams);
  counters_.insert(proto::kViewerJournal, &countJounral);
  counters_.insert(proto::kViewerForecast, &countForecast);
  counters_.insert(proto::kViewerForecastAccuracy, &countForecastAccuracy);
  counters_.insert(proto::kViewerSend, &countSendMsg);
  counters_.insert(proto::kViewerRcv, &countRecvMsg);


  loaders_.insert(proto::kViewerMsg, &queryLoadTelegrams);
  loaders_.insert(proto::kViewerJournal, &queryLoadJournal);
  loaders_.insert(proto::kViewerForecast, &queryLoadForecast);
  loaders_.insert(proto::kViewerForecastAccuracy, &queryLoadForecastAccuracy);

  loaders_.insert(proto::kViewerSend, &queryLoadTelegramsDistinct);
  loaders_.insert(proto::kViewerRcv, &queryLoadTelegramsDistinct);


  auto loadViewerConfig = [] (const QString& path) {
    proto::ViewerConfig *conf = new proto::ViewerConfig();
    if ( false == TProtoText::fromFile(path, conf) ) {
      error_log << QObject::tr("Не могу загрузить конфигурационный файл %1")
                   .arg(path);
    }
    return conf;
  };


  viewerConfigs_.insert(proto::CustomViewerId::kViewerForecast, loadViewerConfig(":/meteo/forecastviewer.conf"));
  viewerConfigs_.insert(proto::CustomViewerId::kViewerForecastAccuracy, loadViewerConfig(":/meteo/forecastaccuracy.conf"));
  viewerConfigs_.insert(proto::CustomViewerId::kViewerMsg, loadViewerConfig(":/meteo/msgviewer.conf") );
  viewerConfigs_.insert(proto::CustomViewerId::kViewerJournal, loadViewerConfig(":/meteo/journalviewer.conf"));
  viewerConfigs_.insert(proto::CustomViewerId::kViewerRcv, loadViewerConfig(":/meteo/rcvviewer.conf"));
  viewerConfigs_.insert(proto::CustomViewerId::kViewerSend, loadViewerConfig(":/meteo/sendviewer.conf"));

  postWork_.insert("datetime_default", &meteo::defaultDatetimeWorker);
  postWork_.insert("msgviewer_post", &meteo::msgviewerPostWorker);
  postWork_.insert("msgviewer_post_route", &meteo::msgviewerRoutePostWorker);
  postWork_.insert("journal_post", &meteo::journalPostWorker);
}

CustomViewerServiceHandler::~CustomViewerServiceHandler()
{
  for ( auto conf : viewerConfigs_ ){
    delete conf;
  }
  viewerConfigs_.clear();
}

ConnectProp CustomViewerServiceHandler::connectPropForDatabase(const meteo::proto::CustomViewerId table){
  return props_.value(table);
}

const proto::ViewerConfig* CustomViewerServiceHandler::configForViewer(const meteo::proto::CustomViewerId table) const
{
  static const proto::ViewerConfig emptyConf;
  auto result = viewerConfigs_.value(table, nullptr);
  if ( nullptr == result ){
    error_log << QObject::tr("Конфигурация для таблицы %1 не задана. Используем пустую конфигурацию");
    return &emptyConf;
  }
  return result;
}

proto::ViewerType CustomViewerServiceHandler::columnType(const proto::ViewerConfig *conf, const QString& col)
{
  for ( auto column: conf->column() ){
    QString name = QString::fromStdString(column.name());
    if ( 0 == name.compare(col) ){
      return column.type();
    }
  }
  return proto::ViewerType::kUnknown;
}

proto::ViewerType CustomViewerServiceHandler::columnType(const meteo::proto::CustomViewerId id, const QString& col) const
{
  const meteo::proto::ViewerConfig *conf = configForViewer(id);
  return columnType(conf, col);
}

QStringList CustomViewerServiceHandler::columnsForCollection(const proto::ViewerConfig *conf)
{
  QStringList columns;
  for ( auto column: conf->column() ){
    columns << QString::fromStdString(column.name());
  }
  return columns;
}

QStringList CustomViewerServiceHandler::columnsForCollection(const meteo::proto::CustomViewerId table) const
{
  const meteo::proto::ViewerConfig *conf = configForViewer(table);
  return columnsForCollection(conf);
}

QHash<QString, QString> CustomViewerServiceHandler::dataFromDocument(const meteo::DbiEntry &doc, const meteo::proto::CustomViewerId id)
{
  QStringList columns = columnsForCollection(id);

  QHash<QString, QString> allFields;
  for (int i = 0; i < columns.size(); ++i){
    const QString& column_name = columns[i];
    QString value = "";
    if ( false == doc.hasField(column_name)) {
      continue;
    }
    proto::ViewerType datatype = columnType(id, column_name);
    switch (datatype){
    case proto::kDateTime:{
      QDateTime dt = doc.valueDt(column_name);
      value = dt.toString(Qt::ISODate);
      break;
    }
    case proto::kDouble: {
      value = QString::number(doc.valueDouble(column_name));
      break;
    }
    case proto::kInt: {
      value = QString::number(doc.valueInt32(column_name));
      break;
    }
    case proto::kOid: {
      value = doc.valueOid(column_name);
      break;
    }
    case proto::kString: {
      value = doc.valueString(column_name);
      break;
    }
    case proto::kBool: {
      value = doc.valueBool(column_name)? "true": "false";
      break;
    }
    case proto::kArray:{
      value = doc.valueArray(column_name).jsonString();
      break;
    }
    case proto::kLongInt:{
      value = QString::number(doc.valueInt64(column_name));
      break;
    }
    case proto::kUnknown: {
      error_log << QObject::tr("Unknown data type");
      value = "UNKNOWN";
      break;
    }
    }
    allFields.insert(column_name, value);
  }
  return allFields;
}


void CustomViewerServiceHandler::LoadData(const ::meteo::proto::CustomviewerDataRequest* request,
                                          ::meteo::proto::CustomviewerDataResponce* response)
{
  QList<Condition> conditions;

  for ( auto conditionProto: request->conditions() ){
    conditions << Condition::deserialize(conditionProto);
  }

  auto dbConf = connectPropForDatabase(request->table());

  LoaderFunction *loader = loaders_.value(request->table(), nullptr);
  if ( nullptr == loader ){
    error_log << QObject::tr("Не найдена фукнция-загрузчик для типа таблицы %1")
                 .arg(request->table());
    return;
  }
  //var(request->DebugString());
  QString sortColumn = QString::fromStdString(request->sort_column());
  QString distinctColumn;
  if(request->has_distinct_column()){
    distinctColumn = QString::fromStdString(request->distinct_column());
  }

  Qt::SortOrder sortOrder = static_cast<Qt::SortOrder>( request->sort_order() );
  int skip  = request->skip();
  int limit = request->limit();

  QString queryQString = loader(conditions, sortColumn, sortOrder, skip, limit, distinctColumn);

  //debug_log << queryQString;

  std::unique_ptr<Dbi> db(meteo::global::db(dbConf));
  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return;
  }
  std::unique_ptr<DbiQuery> query(db->query(queryQString));

  QString err;
  if ( false == query->execInit(&err)) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных: %1")
                 .arg(query->query())
              << err;
    return;
  }

  auto tableConf = configForViewer(request->table());
  //QStringList columns = columnsForCollection(tableConf);

  QList<QHash<QString, QString>> documents;

  while ( true == query->next() ){
    const DbiEntry& doc = query->entry();
    documents << dataFromDocument(doc, request->table());
  }

  for ( auto doc:documents ){
    QHash<QString, QString> fields;
    for ( auto fieldName : doc.keys() ){
      auto value = doc[fieldName];
      fields.insert(fieldName, value);
    }

    auto record = response->add_record();
    for ( auto fieldName: fields.keys() ) {
      auto singleField = record->add_field();
      PostWorkerFunction* post = postWorkFunctionForColumn(request->table(), fieldName);
      post(tableConf, fields, fieldName, singleField);
    }
  }
}

PostWorkerFunction* CustomViewerServiceHandler::postWorkFunctionForColumn(const meteo::proto::CustomViewerId id, const QString& col)
{
  PostWorkerFunction* defaultPostWork = &meteo::defaultPostWorker;

  const meteo::proto::ViewerConfig *conf = configForViewer(id);

  for ( auto colConfig: conf->column() ){
    if ( 0 != col.compare(QString::fromStdString(colConfig.name())) ){
      continue;
    }

    QString fname = QString::fromStdString(colConfig.post());
    if ( true == fname.isEmpty() ){
      return defaultPostWork;
    }
    PostWorkerFunction* activeFunction = postWork_.value(fname, nullptr);
    if ( nullptr == activeFunction ){
      error_log << QObject::tr("Функция обработчик %1 не найдена")
                   .arg(fname);
      return defaultPostWork;
    }
    return activeFunction;
  }

  warning_log << QObject::tr("Колонка не найдена, используем обработчик по умолчанию");
  return defaultPostWork;
}

void CustomViewerServiceHandler::LoadConfig(const ::meteo::proto::ViewerConfigRequest* request,
                                            ::meteo::proto::ViewerConfig* response)
{
  const proto::ViewerConfig *conf  = configForViewer(request->viewid());
  response->CopyFrom(*conf);
}

void CustomViewerServiceHandler::CountData(const ::meteo::proto::CustomViewerCountRequest* request,
                                           ::meteo::proto::CustomViewerCountResponce* response)
{
  auto prop = connectPropForDatabase(request->viewid());
  CounterFunction *counter = counters_.value(request->viewid(), nullptr);
  if ( nullptr == counter ){
    error_log << QObject::tr("Не найдена функция для подсчета данных для таблицы %1")
                 .arg(request->viewid());
    return;
  }

  QList<Condition> conditions;

  for ( auto conditionProto: request->conditions() ){
    conditions << Condition::deserialize(conditionProto);
  }

  qint64 result = counter(conditions);
  response->set_count(result);
}

void CustomViewerServiceHandler::GetLastTlgId(const ::meteo::proto::Dummy* request,
                                              ::meteo::proto::GetLastTlgIdResponce* response)
{
  Q_UNUSED(request);
  static const QString qGetLastTlgId = "get_last_tlg_id";
  std::unique_ptr<Dbi> db(global::dbTelegram());
  if ( nullptr == db ){
    error_log << meteo::msglog::kDbConnectFailed;
    return;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(qGetLastTlgId));

  QString err;
  if ( false == query->execInit(&err) ){
    error_log << meteo::msglog::kDbRequestFailed
              << err;
    response->set_last_tlg_id(0);
    return;
  }

  if ( false == query->next() ){
    return;
  }

  const DbiEntry& entry = query->entry();

  qint64 id = entry.valueInt64("_id");
  response->set_last_tlg_id(id);
}

void CustomViewerServiceHandler::GetAvailableTT(const ::meteo::proto::Dummy* request,
                                                ::meteo::proto::GetAvailableTTResponce* response)
{
  Q_UNUSED(request);

  std::unique_ptr<Dbi> db(global::dbTelegram());
  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return;
  }

  static const auto fingGmiQuery = QObject::tr("find_gmi");
  std::unique_ptr<DbiQuery> t2NullQuery(db->queryByName(fingGmiQuery));
  if ( nullptr == t2NullQuery ){
    error_log << msglog::kDbRequestNotFound.arg(fingGmiQuery);
    return;
  }

  QString err;
  if ( false == t2NullQuery->execInit(&err) ){
    error_log << meteo::msglog::kDbRequestFailed
              << err;
    return;
  }

  while( true == t2NullQuery->next() ) {
    const DbiEntry& entry = t2NullQuery->entry();
    QString name = entry.valueString("name");
    auto t1 = entry.valueString("t1");

    if ( false == entry.hasField("t2") ){
      auto t1Proto = response->add_t1();
      t1Proto->set_name(name.toStdString());
      t1Proto->set_t1(t1.toStdString());
    }
    else {
      auto t2 = entry.valueString("t2");
      auto t2Proto = response->add_t2();
      t2Proto->set_name(name.toStdString());
      t2Proto->set_t1(t1.toStdString());
      t2Proto->set_t2(t2.toStdString());
    }
  }
}

void CustomViewerServiceHandler::GetTypecAreas(const ::meteo::proto::Dummy* request,
                                               ::meteo::proto::GetTypecAreasResponce* response)
{
  Q_UNUSED(request);
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) {
    error_log << msglog::kDbConnectFailed.arg(db->dbname());
    return;
  }

  static const auto queryName = QObject::tr("find_all_typec_areas");
  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if(nullptr == query) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return;
  }

  QString error;
  if(false == query->execInit( &error)){
    error_log << msglog::kDbRequestFailed
              << error;
    return ;
  }

  while ( true == query->next()) {
    const meteo::DbiEntry& document = query->entry();
    auto nameSmb = document.valueString("name_smb");
    auto nameLat = document.valueString("name_lat");

    auto areaProto = response->add_area();
    areaProto->set_name_smb(nameSmb.toStdString());
    areaProto->set_name_lat(nameLat.toStdString());

    if ( true == document.hasField("name_cyr")) {
      auto nameCyr = document.valueString("name_cyr");
      areaProto->set_name_cyr(nameCyr.toStdString());
    }
  }
}

void CustomViewerServiceHandler::GetIcon(const ::meteo::proto::CustomViewerIconRequest* request,
             ::meteo::proto::CustomViewerIconResponce* response)
{
  QString iconPath = QString::fromStdString(request->icon());
  if ( 0 == iconPath.size() ){
    return;
  }

  if ( ':' != iconPath[0] ){
    iconPath = QObject::tr(":/%1").arg(iconPath);
  }

  QFile file (iconPath);

  if ( false == file.open(QIODevice::ReadOnly) ){
    error_log << msglog::kFileNotFound.arg(iconPath);
    return;
  }

  auto iconData = file.readAll();

  QFileInfo info(file);

  QMimeDatabase db;
  QMimeType type = db.mimeTypeForFile(file);
  response->set_icon(iconData.data(), iconData.size());
  response->set_mime_type(type.name().toStdString());
}

QString CustomViewerServiceHandler::getNameByT1(const QString& t1)
{
  //TODO - Это нужно оптимизировать
  QString t1Lo = t1.toLower();

  proto::GetAvailableTTResponce resp;
  GetAvailableTT(nullptr, &resp);
  for ( auto tt: resp.t1() ){
    auto currentT1 = QString::fromStdString(tt.t1()).toLower();
    if ( 0 == currentT1.compare(t1Lo) ){
      return QString::fromStdString(tt.name());
    }
  }

  warning_log << QObject::tr("Значение Т1 %1 не найдено")
                 .arg(t1);
  return QString();
}

QString CustomViewerServiceHandler::getNameByT1T2(const QString& t1, const QString& t2)
{
  //TODO - Это нужно оптимизировать
  auto t1Lo = t1.toLower();
  auto t2Lo = t2.toLower();

  proto::GetAvailableTTResponce resp;
  GetAvailableTT(nullptr, &resp);
  for ( auto tt: resp.t2() ){
    const auto currentT1 = QString::fromStdString(tt.t1()).toLower();
    const auto currentT2 = QString::fromStdString(tt.t2()).toLower();
    if ( 0 == currentT1.compare(t1Lo) && 0 == currentT2.compare(t2Lo) ){
      return QString::fromStdString(tt.name());
    }
  }

  warning_log << QObject::tr("Комбинация ТТ %1 %2 не найдена")
                 .arg(t1)
                 .arg(t2);
  return QString();
}

QString CustomViewerServiceHandler::getNameCyrByNameSMB(const QString& nameSmb)
{
  proto::GetTypecAreasResponce resp;
  GetTypecAreas(nullptr, &resp);

  for ( auto area: resp.area() ){
    auto currentNameSmg = QString::fromStdString(area.name_smb());
    if ( 0 == nameSmb.compare(currentNameSmg) ){
      return QString::fromStdString(area.name_cyr());
    }
  }

  warning_log << QObject::tr("Значение typec_area %1 не найдено")
                 .arg(nameSmb);
  return QString();
}

bool CustomViewerServiceHandler::initContentWidget(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding)
{
  QString msgid = QString::number(tlg.metainfo().id());
  QString header;
  header.append(QString::fromStdString(tlg.header().t1()));
  header.append(QString::fromStdString(tlg.header().t2()));
  header.append(QString::fromStdString(tlg.header().a1()));
  header.append(QString::fromStdString(tlg.header().a2()));
  header.append(QString::number(tlg.header().ii()));
  header.append(QString::fromStdString(tlg.header().cccc()));
  header.append(QString::fromStdString(tlg.header().yygggg()));
  header.append(QString::fromStdString(tlg.header().bbb()));

  QStringList specification;
  QString t1 = QString::fromStdString(tlg.header().t1());
  QString t2 = QString::fromStdString(tlg.header().t2());

  QString groupName = getNameByT1( t1 );
  if ( false == groupName.isEmpty() ){
    specification << groupName;
  }

  QString name = getNameByT1T2( t1, t2);

  if ( false == name.isEmpty() ) {
    specification << name;
  }

  QString nameSMB = QString::fromStdString(tlg.header().a1()) + QString::fromStdString(tlg.header().a2());
  QString nameCyr = getNameCyrByNameSMB(nameSMB);
  if ( false == nameCyr.isEmpty() ) {
    specification << nameCyr;
  }

  QString specificationStr = specification.isEmpty()? QObject::tr("Описание недоступно")
                                                    : specification.join(", ");
  resp->set_description(specificationStr.toStdString());
  QDateTime msgDt = QDateTime::fromString(QString::fromStdString(tlg.metainfo().converted_dt()),
                                          Qt::ISODate);


  if( true ==  tlg.isbinary() ){
    return loadBinMessage(tlg, resp, encoding);
  }
  else{
    return loadTextMessage(tlg, resp, encoding);
  }
}

void loadReplaceDescriptors(const TMeteoData& content)
{
  QList<descr_t> list = content.getDescrList();
  foreach (descr_t descriptor, list) {
    double value = content.getValueCur(descriptor, BAD_METEO_ELEMENT_VAL);
    QMap<int, TMeteoParam> hash = content.getParamList(descriptor);
    QString unit = TMeteoDescriptor::instance()->property(descriptor).unitsRu;
    QString valuetext = QString().setNum(value);
    if( "NO" == unit && false == valuetext.isEmpty() ){
      QString descstr = QString::number(descriptor);
      while( descstr.length() < 6){
        descstr.insert(0, '0');
      }
    }
    if( hash.count() > 1){
      QMap<int, TMeteoParam>::const_iterator it = hash.begin();
      QMap<int, TMeteoParam>::const_iterator end = hash.end();
      while( it != end ){
        double v = it.value().value();
        QString valuetext = QString().setNum(v);
        if( "NO" == unit && false == valuetext.isEmpty() ){
          QString descstr = QString::number(descriptor);
          while( descstr.length() < 6){
            descstr.insert(0, '0');
          }
        }
        ++it;
      }
    }
  }
  for ( int i = 0, sz = content.childsCount(); i < sz; ++i ) {
    loadReplaceDescriptors( *(content.child(i)) );
  }
}

void parseDecodedContentRecursively(const TMeteoData& content, proto::MsgViewerDetailsResponce_ParamList*);
void parseDecodedContent(const TMeteoData& content, proto::MsgViewerDetailsResponce *resp)
{
  int stationIndex = TMeteoDescriptor::instance()->station(content);
  QString dt;
  {
    QDateTime datetime = TMeteoDescriptor::instance()->dateTime(content);
    if (datetime.isValid() == true) {
      dt = dateToHumanFull(datetime);
    }
  }
  QString coord;
  {
    GeoPoint gp;
    if (TMeteoDescriptor::instance()->getCoord(content, &gp) == true) {
      coord = gp.toString();
      coord.remove(QObject::tr("Широта:"));
      coord.remove(QObject::tr("Долгота:"));
    }
  }
  int dataType = content.getValue(TMeteoDescriptor::instance()->descriptor("category"), BAD_METEO_ELEMENT_VAL);

  QString stationName;
  if (stationIndex != BAD_METEO_ELEMENT_VAL && dataType != BAD_METEO_ELEMENT_VAL) {
    QString errorMessage;
    rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
    if ( 0 == ch ) {
      error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом справки");
      return;
    }
    stationName = internal::getStationName( ch, stationIndex, dataType, &errorMessage);
    delete ch;

    if (errorMessage.isEmpty() == false) {
      error_log << errorMessage;
    }

    if (stationName.isEmpty()) {
      stationName = QString::number(stationIndex);
    }
  }

  proto::MsgViewerDetailsResponce_ParamList* header = resp->add_brief();


  QString title;
  if( -9999 != stationIndex ){
    QString stationTitle;
    if( stationName != QString::number(stationIndex)){
      stationTitle = QString("%1 %2").arg(stationIndex).arg(stationName);
    }
    else{
      stationTitle = stationName;
    }
    title = QString( "%1  %2  %3" ).arg(stationTitle).arg(coord).arg(dt);
  }
  else{
    title = QString( "%1  %2" ).arg(coord).arg(dt);
  }
  header->set_title(title.toStdString());

  parseDecodedContentRecursively(content, header);
}

void parseDecodedContentRecursively(const TMeteoData& content, proto::MsgViewerDetailsResponce_ParamList* resp)
{
  auto list = content.getDescrList();
  QStringList strdescrlist;
  for ( auto descriptor : list ) {
    strdescrlist.append( QString::number(descriptor) );
  }
  global::loadBufrTables(strdescrlist);
  for ( auto descriptor : list ) {
    bool spec = false;
    if( descriptor < 7000 || (descriptor > 7999 && descriptor < 10000)){
      spec = true;
    }
    QMap<int, TMeteoParam> hash = content.getParamList(descriptor);
    QString name = TMeteoDescriptor::instance()->name(descriptor);
    QString unit = TMeteoDescriptor::instance()->property(descriptor).unitsRu;
    int k = 0;
    QMap<int, TMeteoParam>::const_iterator it = hash.begin();
    QMap<int, TMeteoParam>::const_iterator end = hash.end();
    while ( it != end ) {
      int q = it.value().quality();
      if ( control::NO_OBSERVE == q ) {
        ++it;
        k++;
        continue;
      }
      double v = it.value().value();

      name = TMeteoDescriptor::instance()->findAdditional( descriptor, k, v );

      QString codetext = it.value().code();
      QString valuetext = QString().setNum(v);
      QString spectext = ( true == spec) ? "true" : "false";
      if( "NO" == unit && false == valuetext.isEmpty() ){
        QString descstr = QString::number(descriptor);
        while ( descstr.length() < 5 ){
          descstr = "0" + descstr;
        }
        QString key = QString("%1_%2").arg(descstr).arg(valuetext);
      }
      QString unitext = unit;
      if( "NO" == unitext ){
        unitext.clear();
      }
      if( "CCCC" == name ){
        QString oldTitle = QString::fromStdString(resp->title());
        QString newTitle = QObject::tr("ИКАО: %1 %2")
            .arg(codetext)
            .arg(oldTitle);
        resp->set_title(newTitle.toStdString());
      }

      QString specification = TMeteoDescriptor::instance()->description(descriptor, k, v);
      if ( "station" == name ){
        QString oldText = QString::fromStdString(resp->title());
        auto text = QObject::tr("Станция: %1,%2").arg(codetext)
                                                .arg(oldText);
        resp->set_title(text.toStdString());
      }

      proto::MsgViewerDetailsResponce_Param* item = resp->add_params();
      item->set_code(codetext.toStdString());
      item->set_value( global::bufrValueDescription( QString::number(descriptor), v ).toStdString() );
      item->set_unit(unitext.toStdString());
      item->set_value(valuetext.toStdString());
      item->set_name(specification.toStdString());
      item->set_quality_text(control::titleForQuality(q).toStdString());
      item->set_descr(name.toStdString());
      item->set_special(spec);

      QColor c;
      if( q == control::MISTAKEN || q == control::DOUBTFUL ){
        c.setRgb(255, 102, 102);
      }
      else if( q == control::NO_CONTROL || q == control::NO_OBSERVE ){
        c.setRgb(255, 255, 255);
      }
      else if( q == control::RIGHT ){
        c.setRgb(152, 255, 152);
      }
      else if( q == control::SPECIAL_VALUE ){
        c.setRgb(255, 204, 153);
      }
      item->set_quality_color(c.name(QColor::NameFormat::HexRgb).toStdString());
      ++it;
      k++;
    }
  }
  for ( int i = 0, sz = content.childsCount(); i < sz; ++i ) {
    parseDecodedContentRecursively( *(content.child(i)), resp );
  }
}

QString CustomViewerServiceHandler::decodeMsg(const QByteArray& raw) const
{
  QStringList checkList = QStringList()
      << "МОСКВА"
      << "РОС"
      << "TEC"
      << "КРА"
      << "ГИДРО"
      << "МЕТ"
      << "ГИДРОМЕТ"
      << "ЦЕНТР"
      << "ПОГОДА"
      << "ПРОГНОЗ"
      << "ДНЕМ"
      << "ТЕМПЕРАТУРА"
      << "ВОЗДУХА"
      << "НОЧЬ";

  QList<QByteArray> codecs = QList<QByteArray>()
      << "KOI8-R"
      << "WINDOWS-1251"
      << "UTF-8"
         ;

  QString msg;
  foreach ( const QByteArray& n, codecs ) {
    QTextCodec* codec = QTextCodec::codecForName(n);
    msg = codec->toUnicode(raw);
    foreach ( const QString& s, checkList ) {
      if ( msg.contains(s, Qt::CaseInsensitive) ) {
        return msg;
      }
    }
  }
  return msg;
}


QString CustomViewerServiceHandler::replaceContent(const QByteArray& arr, bool flag)
{
  QString msg = decodeMsg(arr);

  QString text;
  for ( int i = 0, isz = msg.size(); i < isz; ++i ) {
    QChar c = msg.at(i);
    if ( TlgTextObject::isSpecChar(c) ) {
      if ( true == flag ) {
        text += TlgTextObject::kSpecChars.key(c);
      }

      if ( QChar(012) == c ) { text += "\n"; }
    }
    else {
      text += c;
    }
  }
  return text;
}

void parseGrib(const grib::TGribData& grib, proto::MsgViewerDetailsResponce::GribHumanInfo* resp){

  auto setGenericParam = [resp](const QString& title, const QString& value) {
    auto paramProto = resp->add_generic_params();
    paramProto->set_title(title.toStdString());
    paramProto->set_value(value.toStdString());
  };

  if ( true == grib.has_center() )
  {
    auto title = QObject::tr("Идентификация центра — поставщика/производителя продукции");
    auto value = QString::number(grib.center());
    setGenericParam(title, value);
  }
  if ( true == grib.has_subcenter() )
  {
    auto title = QObject::tr("Указатель подцентра");
    auto value = QString::number(grib.subcenter());
    setGenericParam(title, value);
  }
  if ( true == grib.has_dt() ){
    auto dt = QDateTime::fromString(QString::fromStdString(grib.dt()), Qt::ISODate);
    if ( true == dt.isValid() ){
      auto title = QObject::tr("Время подготовки данных");
      auto value = meteo::dateToHumanFull(dt);
      setGenericParam(title, value);
    }
  }

  if ( true == grib.has_mastertable() ){
    auto title = QObject::tr("Номер версии эталонных таблиц GRIB");
    auto value = QString::number(grib.mastertable());
    setGenericParam(title, value);
  }

  if ( true == grib.has_localtable() ){
    auto title = QObject::tr("Номер версии местных таблиц GRIB");
    auto value = QString::number(grib.localtable());
    setGenericParam(title, value);
  }

  if ( true == grib.has_discipline() ){
    auto title = QObject::tr("Дисциплина");
    auto value = QString::number(grib.discipline());
    setGenericParam(title, value);
  }

  if ( true == grib.has_signdt() ){
    auto title = QObject::tr("Значимость времени начала отсчёта");
    auto value = QString::number(grib.signdt());
    setGenericParam(title, value);
  }

  if ( true == grib.has_status() ){
    auto title = QObject::tr("Состояние производства обработанных данных");
    auto value = QString::number(grib.status());
    setGenericParam(title, value);
  }

  if ( true == grib.has_datatype() ){
    auto title = QObject::tr("Тип обработанных данных");
    auto value = QString::number(grib.datatype());
    setGenericParam(title, value);
  }

  if ( true == grib.has_bitmap() ){
    auto title = QObject::tr("Битовое отображение");
    auto value = QObject::tr("Бинарные данные");
    setGenericParam(title, value);
  }

  auto addGridParam = [resp](const QString& title, const QString& value){
    auto param = resp->add_grid_params();
    param->set_title(title.toStdString());
    param->set_value(value.toStdString());
  };

  if ( true == grib.has_grid() ){
    auto gridInfo = grib.grid();
    if ( true == gridInfo.has_type() ){
      auto title = QObject::tr("Определение сетки");
      auto value = QString::number(gridInfo.type());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_data() ){
      auto title = QObject::tr("Данные");
      auto value = QObject::tr("Бинарные данные");
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_datacrc() ){
      auto title = QObject::tr("Контрольная сумма (данные)");
      auto value = QString::number(gridInfo.datacrc());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_points() ){
      auto title = QObject::tr("Перечень точек в каждом ряду");
      auto value = QObject::tr("Бинарные данные");
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_pointscrc() ){
      auto title = QObject::tr("Контрольная сумма (точки) ");
      auto value = QString::number(gridInfo.pointscrc());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_pointstype() ){
      auto title = QObject::tr("Интерпретация списка чисел");
      auto value = QString::number(gridInfo.pointstype());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_verticcoords() ){
      auto title = QObject::tr("Перечень параметров вертикальной координаты");
      auto value = QObject::tr("Бинарные данные");
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_verticcrc() ){
      auto title = QObject::tr("Контрольная сумма (Перечень параметров вертикальной координаты)");
      auto value = QString::number(gridInfo.verticcrc());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_di() ){
      auto title = QObject::tr("Приращение в направлении i");
      auto value = QString::number(gridInfo.di());
      addGridParam(title, value);
    }

    if ( true == gridInfo.has_dj() ){
      auto title = QObject::tr("Приращение в направлении j");
      auto value = QString::number(gridInfo.dj());
      addGridParam(title, value);
    }
  }

  auto addGrib1Param = [resp](const QString& title, const QString& value){
    auto proto = resp->add_grib1_params();
    proto->set_title(title.toStdString());
    proto->set_value(value.toStdString());
  };

  if ( true == grib.has_product1() ){
    auto product1 = grib.product1();
    if ( true == product1.has_number() ){
      auto title = QObject::tr("Указатель параметра");
      auto value = QString::number(product1.number());
      addGrib1Param(title, value);
    }
    if ( true == product1.has_leveltype() ){
      auto title = QObject::tr("Указатель типа уровня");
      auto value = QString::number(product1.leveltype());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_leveltype() ){
      auto title = QObject::tr("Значение уровня");
      auto value = QString::number(product1.levelvalue());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_p1() ){
      auto title = QObject::tr("Период времени (число единиц времени)");
      auto value = QString::number(product1.p1());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_p2() ){
      auto title = QObject::tr("Период времени или временной интервал между последовательными анализами, последовательными инициализированными анализами или прогнозами, подвергнутый усреднению или накоплению.");
      auto value = QString::number(product1.p2());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_timerange() ){
      auto title = QObject::tr("Указатель времени");
      auto value = QString::number(product1.timerange());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_numavg() ){
      auto title = QObject::tr("Число случаев, включенных в расчет, если статистическая обработка; иначе 0");
      auto value = QString::number(product1.numavg());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_absentavg() ){
      auto title = QObject::tr("Число отсутствующих (утраченных) в расчете в случае статистической обработки");
      auto value = QString::number(product1.absentavg());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_processtype() ){
      auto title = QObject::tr("Указательный номер процесса формирования продукции");
      auto value = QString::number(product1.processtype());
      addGrib1Param(title, value);
    }

    if ( true == product1.has_dt1() ){
      auto dt = QDateTime::fromString(QString::fromStdString(product1.dt1()), Qt::ISODate);
      if ( true == dt.isValid() ){
        auto title = QObject::tr("Первое значение времени");
        auto value = meteo::dateToHumanFull(dt);
        addGrib1Param(title,value);
      }
    }

    if ( true == product1.has_dt2() ){
      auto dt = QDateTime::fromString(QString::fromStdString(product1.dt2()), Qt::ISODate);
      if ( true == dt.isValid() ){
        auto title = QObject::tr("Второе значение времени");
        auto value = meteo::dateToHumanFull(dt);
        addGrib1Param(title,value);
      }
    }

    if ( true == product1.has_forecasttime() ){
      auto title = QObject::tr("Срок прогноза");
      auto value = QString::number(product1.forecasttime());
      addGrib1Param(title, value);
    }
  }

  auto grib2Param = [resp](const QString& title, const QString& value){
    auto param = resp->add_grib2_params();
    param->set_title(title.toStdString());
    param->set_value(value.toStdString());
  };

  if ( true == grib.has_product2() ){
    auto product2 = grib.product2();

    if ( true == product2.has_deftype() ){
      auto title = QObject::tr("Номер образца определения продукции");
      auto value = QString::number(product2.deftype());
      grib2Param(title, value);
    }

    if ( true == product2.has_definition() ){
      auto title = QObject::tr("Определение продукции");
      auto value = QObject::tr("Бинарные данные");
      grib2Param(title,value);
    }

    if ( true == product2.has_defcrc() ){
      auto title = QObject::tr("Контрольная сумма (Определение продукции)");
      auto value = QString::number(product2.defcrc());
      grib2Param(title, value);
    }

    if ( true == product2.has_category() ){
      auto title = QObject::tr("Категория параметра ");
      auto value = QString::number(product2.category());
      grib2Param(title, value);
    }

    if ( true == product2.has_number() ){
      auto title = QObject::tr("Номер параметра");
      auto value = QString::number(product2.number());
      grib2Param(title, value);
    }

    if ( true == product2.has_forecasttime() ){
      auto title = QObject::tr("Срок прогноза");
      auto value = QString::number(product2.forecasttime());
      grib2Param(title, value);
    }

    if ( true == product2.has_surf1_type() ){
      auto title = QObject::tr("Тип первой фиксированной поверхности");
      auto value = QString::number(product2.number());
      grib2Param(title, value);
    }

    if ( true == product2.has_surf1() ){
      auto title = QObject::tr("Величина первой фиксированной поверхности");
      auto value = QString::number(product2.surf1());
      grib2Param(title, value);
    }

    if ( true == product2.has_surf2_type() ){
      auto title = QObject::tr("Тип второй фиксированной поверхности ");
      auto value = QString::number(product2.surf2_type());
      grib2Param(title, value);
    }

    if ( true == product2.has_surf2() ){
      auto title = QObject::tr("Величина второй фиксированной поверхности");
      auto value = QString::number(product2.surf2());
      grib2Param(title, value);
    }

    if ( true == product2.has_processtype() ){
      auto title = QObject::tr("Указательный номер процесса формирования продукции");
      auto value = QString::number(product2.processtype());
      grib2Param(title, value);
    }

    if ( true == product2.has_timerange() ){
      auto title = QObject::tr("Указатель времени");
      auto value = QString::number(product2.timerange());
      grib2Param(title, value);
    }

    if ( true == product2.has_dt1() ){
      auto dt = QDateTime::fromString(QString::fromStdString(product2.dt1()), Qt::ISODate);
      if ( true == dt.isValid() ){
        auto title = QObject::tr("Первое значение времени");
        auto value = meteo::dateToHumanFull(dt);
        addGrib1Param(title,value);
      }
    }

    if ( true == product2.has_dt2() ){
      auto dt = QDateTime::fromString(QString::fromStdString(product2.dt2()), Qt::ISODate);
      if ( true == dt.isValid() ){
        auto title = QObject::tr("Второе значение времени");
        auto value = meteo::dateToHumanFull(dt);
        addGrib1Param(title,value);
      }
    }
  }
}

void CustomViewerServiceHandler::loadDecodedContent(proto::MsgViewerDetailsResponce* resp, const QByteArray& tlg, const QString& msgtype, const QDateTime& dt, const QString& encoding)
{
  if( true == tlg.isEmpty() ){
    return;
  }

  if ( 0 == msgtype.compare("grib") ){
    ::meteo::tlg::MessageNew m = ::meteo::tlg::tlg2proto(tlg);
    auto grib = internal::getDecodedGrib(tlg);
    foreach (const grib::TGribData& entry, grib) {
      parseGrib(entry, resp->add_grib_info());
    }
  }
  else if ( 0 == msgtype.compare("alphanum")) {
    QList<TMeteoData> md;
    bool ok = internal::getDecodedAlphanum(tlg, dt, &md,encoding);
    if (!ok) {
      error_log << QString::fromUtf8("Ошибка раскодирования.");
    }
    else {
      for (int idx = 0; idx < md.size(); idx++) {
        loadReplaceDescriptors(md.at(idx));
      }

      for (int idx = 0; idx < md.size(); idx++) {
        parseDecodedContent(md.at(idx), resp);
      }
    }
  }
  else if ( 0 == msgtype.compare("alphanumgph")) {
    QList<TMeteoData> md;
    bool ok = internal::getDecodedIonex(tlg, &md, encoding);
    if (false == ok) {
      error_log << QString::fromUtf8("Ошибка раскодирования.");
    }
    else {
      for (int idx = 0; idx < md.size(); idx++) {
        loadReplaceDescriptors(md.at(idx));
      }
      for (int idx = 0; idx < md.size(); idx++) {
        parseDecodedContent(md.at(idx), resp);
      }
    }
  }
  else if ( 0 == msgtype.compare("bufr") ) {
    QList<TMeteoData> md;
    bool ok = internal::getDecodedBufr(tlg, dt, &md, encoding);
    if ( false == ok) {
      error_log << QString::fromUtf8("Ошибка раскодирования.");
    }
    else {
      for (int idx = 0; idx < md.size(); idx++) {
        loadReplaceDescriptors(md.at(idx));
      }
      for (int idx = 0; idx < md.size(); idx++) {
        parseDecodedContent(md.at(idx), resp);
      }
    }
  }
}

bool CustomViewerServiceHandler::loadTextMessage(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding)
{
  QByteArray startline = QByteArray(tlg.startline().data().data(), tlg.startline().data().size());
  QByteArray header = QByteArray(tlg.header().data().data(), tlg.header().data().size());
  QByteArray msg = QByteArray(tlg.msg().data(), tlg.msg().size());
  QByteArray end = QByteArray(tlg.end().data(), tlg.end().size());

  QByteArray data =  startline + header + msg + end;
  QString content = replaceContent(data, false);
  QString contentAll = replaceContent(data, true);
  resp->set_tlg(content.toStdString());
  resp->set_tlg_full(content.toStdString());

  QDateTime dt = QDateTime::fromString(QString::fromStdString(tlg.metainfo().converted_dt()),
                                       Qt::ISODate);

  loadDecodedContent(resp, data, QString::fromStdString(tlg.msgtype()), dt, encoding);

  return true;
}

bool CustomViewerServiceHandler::loadBinMessage(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding)
{
  QByteArray startline = QByteArray(tlg.startline().data().data(), tlg.startline().data().size());
  QByteArray header = QByteArray(tlg.header().data().data(), tlg.header().data().size());
  QByteArray msg = QByteArray(tlg.msg().data(), tlg.msg().size());
  QByteArray end = QByteArray(tlg.end().data(), tlg.end().size());
  QByteArray startdata =  startline + header;

  QString content = replaceContent(startdata, false);
  QString end_content = replaceContent(end, false);
  resp->set_tlg((content + QObject::tr("\n *** Бинарные данные ***\n") + end_content).toStdString());

  QString contentFull = replaceContent(startdata, true);
  QString end_contentFull = replaceContent(end, true);
  resp->set_tlg_full((contentFull + QObject::tr("\n *** Бинарные данные ***\n") + end_contentFull).toStdString());

  QString type = QString::fromStdString(tlg.msgtype());

  if ( 0 == type.compare("doc") ){
    warning_log << QObject::tr("Загрузка документов еще не реализована");
    return true;
  }
  else if ( 0 == type.compare("fax") ){
    warning_log << QObject::tr("Загрузка факсов еще не реализована");
    return true;
  }
  else if ( true == QStringList({ "sxf", "rsc", "map_ptkpp", "aero_ptkpp", "ocean_ptkpp", "cut_ptkpp" }).contains(type) ){
    warning_log << QObject::tr("Эти типы то же не работают, что с ними делать непонятно");
    return true;
  }

  loadDecodedContent(resp,
                     startdata + msg + end,
                     type,
                     QDateTime::fromString(QString::fromStdString(tlg.metainfo().converted_dt()), Qt::ISODate),
                     encoding
                     );
  return true;
}

void CustomViewerServiceHandler::GetTelegramDetails(const ::meteo::proto::MsgViewerDetailsRequest* request,
                                                    ::meteo::proto::MsgViewerDetailsResponce* response)
{
  const auto msg = request->msg();
  const auto encoding = QString::fromStdString(request->encoding());
/*
  msgcenter::GetTelegramRequest getTlgRequest;
  getTlgRequest.add_ptkpp_id(msgid.toInt());
  getTlgRequest.set_onlyheader(false);

  std::unique_ptr<rpc::Channel> ch(global::serviceChannel( meteo::settings::proto::kMsgCenter));
  if ( nullptr == ch ) {
    error_log << msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
    return;
  }

  std::unique_ptr<msgcenter::GetTelegramResponse> tlg(ch->remoteCall( &meteo::msgcenter::MsgCenterService::GetTelegram, getTlgRequest, 20000 ));

  if ( nullptr == tlg ){
    error_log << msglog::kServiceAnsverFailed.arg("msgcenter");
    return;
  }
  if ( 1 != tlg->msg_size() ){
    error_log << QObject::tr("Телеграм найдено %1, ожидалось ровно 1, id = ")
                 .arg(tlg->msg_size())
                 .arg(msgid);
    return ;
  }
  */

  initContentWidget(msg, response, encoding);
}

}

