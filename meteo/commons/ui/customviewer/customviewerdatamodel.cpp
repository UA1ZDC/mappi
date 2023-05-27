#include "customviewerdatamodel.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/dateformat.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QMessageBox>
#include <meteo/commons/proto/customviewer.pb.h>
#include <qtimer.h>

namespace meteo {

CustomViewerDataModel::CustomViewerDataModel(int pageSize):
  QObject (nullptr),
  pageSize_(pageSize),
  sortOrder_(Qt::SortOrder::AscendingOrder)
{
  QObject::connect(&databaseCounterTimer_, &QTimer::timeout, this, &CustomViewerDataModel::slotCountDataInDatabase);
  QObject::connect(this, &CustomViewerDataModel::signalLock, this, &CustomViewerDataModel::slotTimerStop);
  databaseCounterTimer_.start(5000);
}


CustomViewerDataModel::~CustomViewerDataModel()
{
  databaseCounterTimer_.stop();
}

void CustomViewerDataModel::initDataModel()
{
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(settings::proto::ServiceCode::kCustomViewerService));

  if ( nullptr == channel ){
    error_log << meteo::msglog::kServiceConnectFailedSimple;
    return;
  }

  proto::ViewerConfigRequest request;
  request.set_viewid(tableId());
  std::unique_ptr<proto::ViewerConfig> responce(channel->remoteCall(&proto::CustomViewerService::LoadConfig, request, 30000));
  if ( nullptr == responce ){
    error_log << meteo::msglog::kServiceRequestFailedErr;
    return;
  }

  config_.CopyFrom(*responce);
  setSort(0, sortOrder_);
}

void CustomViewerDataModel::slotTimerStop(bool state){
  if ( true == state ){
    databaseCounterTimer_.stop();
  }
  else {
    databaseCounterTimer_.start(5000);
  }
}

void CustomViewerDataModel::slotCountDataInDatabase()
{
  QtConcurrent::run( this, &CustomViewerDataModel::threadCountDataInDatabase);
}

void CustomViewerDataModel::setPageSize(int pageSize)
{
  pageSize_ = pageSize;
}

void CustomViewerDataModel::threadCountDataInDatabase()
{
  auto count = countTotal();
  emit signalNewData(count);
}



void CustomViewerDataModel::threadDataLoad(){
  emit signalLock(true);
  clearDocuments();

  allDocuments_ = load(getConditionsEnabled().values());

  emit signalDataLoaded(&allDocuments_, currentPage_);
  emit signalLock(false);
}

void CustomViewerDataModel::setPage(int page)
{
  currentPage_ = page;
  threadDataLoad();
}

void CustomViewerDataModel::addCondition(int column, const Condition& condition, bool enabled)
{
  addConditionRaw(column, condition, enabled);
  emitSignalChangeConditions();
  setFirstPage();
}

void CustomViewerDataModel::addConditionRaw(int index, const Condition& condition,  bool isEnabled)
{
  ConditionData* data = nullptr;
  if ( true == conditions_.contains(index) ){
    data = conditions_[index];
  }
  else {
    data = new ConditionData();
    conditions_.insert(index, data);
  }
  data->isEnabled = isEnabled;
  data->condition_ = condition;
}

void CustomViewerDataModel::addCondition(const QString& column, const Condition& condition, bool enabled)
{
  addCondition(columnIndexByName(column), condition, enabled);
}

void CustomViewerDataModel::conditionEnable(const QString& column, const bool enabled)
{
  conditionEnable(columnIndexByName(column), enabled);
}

void CustomViewerDataModel::conditionEnable( const int column, const bool enabled)
{
  ConditionData* data = conditions_.value(column, nullptr);
  if ( nullptr == data ) {
    error_log << QObject::tr("Отсутствует условие для изменения");
    return;
  }
  if ( enabled == data->isEnabled ){
    error_log << QObject::tr("Ошибка: условие уже находится в требуемом состоянии, изменение состояния не требуется");
    return;
  }

  data->isEnabled = enabled;
  emitSignalChangeConditions();
  setFirstPage();
}

QStringList CustomViewerDataModel::getRecomendedFields()
{
  QStringList cols;
  for ( auto colProto: config_.column() ){
    if ( true == colProto.recomended() ){
      cols << QString::fromStdString(colProto.name());
    }
  }
  return cols;
}

bool CustomViewerDataModel::ifWillRemoveLastRecomendedField(int index)
{
  QStringList recomendedFields = getRecomendedFields();
  QString colName = columnName(index);
  if ( recomendedFields.size() == 0 ){
    return false;
  }

  if ( true == recomendedFields.contains(colName) &&
       true == conditions_.contains(index) &&
      true == conditions_[index]->isEnabled ) {
    int recomendedCnt = 0;
    for (int col : conditions_.keys()){
      if (recomendedFields.contains(colName) && conditions_[col]->isEnabled){
        ++recomendedCnt;
      }
    }
    return 1 == recomendedCnt;
  }
  else  {
    return false;
  }
}

bool CustomViewerDataModel::conditionRemoveRaw( int index )
{
  ConditionData* data = conditions_.value(index, nullptr);
  if ( nullptr == data ){
    return false;
  }
  conditions_.remove(index);
  delete data;
  return true;
}

void CustomViewerDataModel::conditionRemove(const QStringList& columns)
{
  bool updates = false;
  for ( auto col: columns ){
    int index = columnIndexByName(col);
    if ( true == conditionRemoveRaw(index) ) {
      updates = true;
    }
  }

  if ( true == updates ){
    emitSignalChangeConditions();
    setFirstPage();
  }
}

void CustomViewerDataModel::conditionRemove(int index)
{
  if ( true == conditionRemoveRaw(index) ){
    emitSignalChangeConditions();
    setFirstPage();
  }
}

void CustomViewerDataModel::conditionRemove(const QString& column)
{
  conditionRemove(columnIndexByName(column));
}

void CustomViewerDataModel::setNextPage()
{
  setPage(currentPage_ + 1);
}

void CustomViewerDataModel::setPrewPage()
{
  setPage(currentPage_ - 1);
}

void CustomViewerDataModel::setFirstPage()
{
  setPage(0);
}

bool CustomViewerDataModel::hasConditions()
{
  return false == conditions_.isEmpty();
}

void CustomViewerDataModel::clearConditions()
{
  if (conditions_.isEmpty()) return;
  conditions_.clear();
  setFirstPage();
  emitSignalChangeConditions();
}

void CustomViewerDataModel::setSort(const QString& sortColumn, Qt::SortOrder sortOrder)
{
  sortColumn_ = sortColumn;
  sortOrder_ = sortOrder;
}

void CustomViewerDataModel::setSort(int sortColumn, Qt::SortOrder sortOrder)
{
  auto conf = columnConfig(sortColumn);
  if ( nullptr == conf ){
    return;
  }

  sortColumn_ = QString::fromStdString(conf->name());
  sortOrder_ = sortOrder;
}

void CustomViewerDataModel::addConditionByText(int col, const QString& value, const QString& displayValue)
{
  proto::ViewerType datatype = columnType(col);
  auto columnConf = columnConfig(col);
  if ( nullptr == columnConf ){
    error_log << QObject::tr("Не найдена конфигурация колонки");
    return;
  }
  auto column = QString::fromStdString(columnConf->name());
  auto displayName = columnDisplayName(col);

  if (true == value.isEmpty()) {
    Condition condition = ConditionNull(column, displayName, true);
    addCondition(col, condition, true);
    return;
  }

  switch (datatype) {
    case proto::kOid:{
    Condition condition = ConditionOid(column, displayName, value);
    addCondition(col, condition, true);
    return;
  }
  case proto::kLongInt:{
    bool ok = false;
    qint64 convertedValue = value.toLong(&ok);
    if ( false == ok ){
      error_log << QObject::tr("Не удалось преобразовать значение поля %1 в тип long")
                   .arg(value);
      return;
    }

    Condition condition = ConditionInt64(column, displayName, ConditionInt64::kEqual, convertedValue);
    addCondition(col, condition, true);
    return;
  }
  case proto::kDouble:{
    bool ok = false;
    double convertedValue = value.toDouble(&ok);
    if ( false == ok ){
      error_log << QObject::tr("Не удалось преобразовать значение поля %1 в тип double")
                   .arg(value);
      return;
    }

    Condition condition = ConditionDouble(column, displayName, ConditionDouble::kEqual,convertedValue);
    addCondition(col, condition, true);
    return;
  }
  case proto::kInt:{
    bool ok = false;
    qint32 convertedValue = value.toInt(&ok);
    if ( false == ok ){
      error_log << QObject::tr("Не удалось преобразовать значение поля %1 в тип long")
                   .arg(value);
      return;
    }

    Condition condition = ConditionInt32(column, displayName, ConditionInt32::kEqual, convertedValue);
    addCondition(col, condition, true);
    return;
  }
  case proto::kDateTime:{
    const QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
    if ( false == dt.isValid() ){
      error_log << QObject::tr("не удалось преобразовать значение поля %1 к дате")
                   .arg(value);
      return;
    }
    Condition condition = ConditionDateTimeInterval(column, displayName, dt, dt);
    addCondition(col, condition, true);
    return;
  }
  case proto::kBool:{
    bool convertedValue = false;
    if ( "true" == value ) {
      convertedValue = true;
    }
    else if ( "false" == value ) {
      convertedValue = false;
    }
    else {
      error_log << QObject::tr("Не удалось преобразовать строковое значение %1 к boolean")
                   .arg(value);
      return;
    }
    Condition conditon = ConditionBool(column, displayName, convertedValue);
    conditon.setDisplayCondition(displayValue);
    addCondition(col, conditon, true);

    return;
  }
  case proto::kString:{
    Condition condition = ConditionString(column, displayName, ConditionString::kMatch, false, Qt::CaseSensitive, value);
    addCondition(col, condition, true);
    break;
  }
  case proto::kUnknown:
  case proto::kArray: {
    error_log<< QObject::tr("Создание условий не поддерживается классов JSON ARRAY и UNKNOWN");
    break;
  }
  }
}

bool CustomViewerDataModel::columnCanSort(const QString& columnName) const
{
  for ( auto columnProto: config_.column() ){
    if ( 0 == columnName.compare(QString::fromStdString(columnProto.name()))){
      return columnProto.sort();
    }
  }
  return false;
}

bool CustomViewerDataModel::columnCanSort(int col) const
{
  auto conf = columnConfig(col);
  if ( nullptr == conf ){
    return false;
  }
  return conf->sort();
}

int CustomViewerDataModel::columnIndexByName(const QString& name) const
{
  for (int i = 0; i < config_.column_size(); ++i ){
    auto columnProto = config_.column(i);
    auto protoName = QString::fromStdString(columnProto.name());
    if ( 0 == protoName.compare(name) ){
      return i;
    }
  }
  return -1;
}

void CustomViewerDataModel::emitSignalChangeConditions()
{
  emit signalConditionsChanged();
}

CustomViewerDataModel::ConditionData* CustomViewerDataModel::conditionForColumn(int col)
{
  return conditions_.value(col, nullptr);
}


void CustomViewerDataModel::insertManyConditions( const QHash<int, Condition> &newConditions )
{
  for ( int col: newConditions.keys() ){
    Condition condition = newConditions[col];
    addConditionRaw(col, condition, true);
  }
  emitSignalChangeConditions();
  setFirstPage();
}

void CustomViewerDataModel::insertManyConditions( const QHash<QString, Condition> &newConditions )
{

  for ( auto colName: newConditions.keys() ){
    int col = columnIndexByName(colName);
    Condition condition = newConditions[colName];
    addConditionRaw(col, condition, true);
  }
  emitSignalChangeConditions();
  setFirstPage();
}

QStringList CustomViewerDataModel::filterColumns()
{
  QStringList cols;
  for( auto colProto: config_.column() ){
    if ( true == colProto.filter() ){
      cols << QString::fromStdString(colProto.name());
    }
  }
  return cols;
}

QString CustomViewerDataModel::columnHeaderTooltip(const QString& column) const
{
  for ( auto columnProto: config_.column() ){
    auto colname = QString::fromStdString(columnProto.name());
    if ( 0 == colname.compare(column) ){
      return QString::fromStdString(columnProto.tooltip());
    }
  }
  return QString();
}

bool CustomViewerDataModel::hasDocuments()
{
  return !allDocuments_.isEmpty();
}

int CustomViewerDataModel::countDocuments()
{
  return allDocuments_.size();
}

QHash<int,  Condition> CustomViewerDataModel::getConditions() const
{
  QHash<int,  Condition> result;
  for ( int col: conditions_.keys() ){
    ConditionData* data = conditions_[col];
    result.insert(col, data->condition_);
  }
  return result;
}

QHash<int, Condition> CustomViewerDataModel::getConditionsEnabled() const
{

  QHash<int,  Condition> result;
  for ( int col: conditions_.keys() ){
    ConditionData* data = conditions_[col];
    if ( nullptr != data && true == data->isEnabled ){
      result.insert(col, data->condition_);
    }
  }
  return result;
}

QWidget* CustomViewerDataModel::getLeftPanelWidget() const
{
  return nullptr;
}

QList<proto::CustomViewerTableRecord> CustomViewerDataModel::load(QList<Condition> conditions) const
{
  meteo::proto::CustomviewerDataRequest request;
  proto::CustomViewerId id = tableId();
  request.set_table(id);
  for ( auto condition: conditions ){
    auto conditionProto = request.add_conditions();
    condition.serialize(conditionProto);
  }

  request.set_sort_order(getSortOrder());
  request.set_sort_column(getSortColumn().toStdString());

  request.set_skip(getCurrentPage() * getPageSize());
  request.set_limit(getPageSize());

  std::unique_ptr<meteo::rpc::Channel> channel( meteo::global::serviceChannel(meteo::settings::proto::ServiceCode::kCustomViewerService));

  if ( nullptr == channel ){
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::ServiceCode::kCustomViewerService));
    return QList<proto::CustomViewerTableRecord>();
  }

  std::unique_ptr<meteo::proto::CustomviewerDataResponce> resp(channel->remoteCall(&meteo::proto::CustomViewerService::LoadData, request, 30000));

  if ( nullptr == resp ){
    error_log.msgBox() << meteo::msglog::kServiceRequestFailedErr;
    return QList<proto::CustomViewerTableRecord>();
  }

  QList<proto::CustomViewerTableRecord> result;
  for ( auto record : resp->record() ){
    result << record;
  }
  return result;
}

QString CustomViewerDataModel::columnDisplayName(const QString& col) const
{
  const proto::ViewerColumn* conf = columnConfig(col);
  if ( nullptr == conf ){
    return QString();
  }
  return QString::fromStdString(conf->display_name());
}

QString CustomViewerDataModel::columnDisplayName(int col) const
{
  const proto::ViewerColumn* conf = columnConfig(col);
  if ( nullptr == conf ){
    return QString();
  }
  return QString::fromStdString(conf->display_name());
}

proto::CustomViewerId CustomViewerDataModel::tableId() const
{
  return proto::CustomViewerId::kViewerUnknown;
}

bool CustomViewerDataModel::hasColumn(const QString& columnName)
{
  return ( nullptr != columnConfig(columnName) );
}

const proto::ViewerColumn* CustomViewerDataModel::columnConfig(const QString& column) const
{
  for (int i = 0; i < config_.column_size(); ++i ) {
    const proto::ViewerColumn* columnConf = &config_.column(i);
    if ( 0 == column.compare(QString::fromStdString(columnConf->name())) ){
      return columnConf;
    }
  }
  return nullptr;
}

proto::ViewerType CustomViewerDataModel::columnType(const QString& col) const
{
  const proto::ViewerColumn* conf = columnConfig(col);
  if ( nullptr == conf ){
    return proto::ViewerType::kUnknown;
  }
  return conf->type();
}


const proto::ViewerColumn* CustomViewerDataModel::columnConfig(int column) const
{
  if (0 <= column && column < config_.column_size() ){
    return &config_.column(column);
  }
  return nullptr;
}

proto::ViewerType CustomViewerDataModel::columnType(int col) const
{
  const proto::ViewerColumn* conf = columnConfig(col);
  if ( nullptr == conf ){
    return proto::ViewerType::kUnknown;
  }
  return conf->type();
}

qint64 CustomViewerDataModel::countTotal()
{
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(settings::proto::kCustomViewerService));

  if ( nullptr == channel ){
    error_log << msglog::kServiceConnectFailed.arg(global::serviceTitle(settings::proto::kCustomViewerService));
    return 0;
  }

  proto::CustomViewerCountRequest request;
  request.set_viewid(tableId());
  std::unique_ptr<proto::CustomViewerCountResponce> responce(channel->remoteCall(&proto::CustomViewerService::CountData, request, 30000));

  if ( nullptr == responce ){
    error_log << msglog::kServiceRequestFailedErr;
    return 0;
  }

  return responce->count();
}

int CustomViewerDataModel::columnSize()
{
  return config_.column_size();
}

QString CustomViewerDataModel::columnName(int col)
{
  auto columnConf = columnConfig(col);
  if ( nullptr == columnConf ){
    return QString();
  }

  return QString::fromStdString(columnConf->name());
}

QList<proto::ViewerColumnMenu> CustomViewerDataModel::menu(const QString& column) const
{
  QList<proto::ViewerColumnMenu> menus;
  auto columnConf = columnConfig(column);
  if ( nullptr == columnConf ){
    return menus;
  }

  for ( auto menu: columnConf->menu() ){
    menus << menu;
  }
  return menus;
}

QList<proto::CustomViewerField> CustomViewerDataModel::getFieldsForColumn(const QString& column) const
{
  QList<proto::CustomViewerField>  result;
  for ( auto doc: allDocuments_ ){
    for ( auto field: doc.field() ){
      auto fname = QString::fromStdString(field.name());
      if ( 0 == fname.compare(column) ){
        result << field;
      }
    }
  }
  return result;
}

}

