#include "journalwidgetdatamodel.h"

#include <qicon.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgsettings.pb.h>
#include <meteo/commons/settings/settings.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

namespace meteo {

static auto kJournalDbName = QObject::tr("journaldb");
static auto kJournalCollection = QObject::tr("journal");

static auto queryCountCollection = QObject::tr("count_collection");

static const int defaultUpdateTimeout = 10000;

static auto kId = QObject::tr("_id");
static auto kPriority = QObject::tr("priority");
static auto kSender = QObject::tr("sender");
static auto kUsername = QObject::tr("username");
static auto kFileName = QObject::tr("file_name");
static auto kLineNumber = QObject::tr("line_number");
static auto kIp = QObject::tr("IP");
static auto kHost = QObject::tr("host");
static auto kMessage = QObject::tr("message");
static auto kDt = QObject::tr("dt");

JournalWidgetDataModel::JournalWidgetDataModel(int pageSize):
  meteo::CustomViewerDataModel(pageSize),
  lastDt_(QDateTime::currentDateTimeUtc()),
  leftPanel_(new journal::JournalLeftPanel())
{
}

void JournalWidgetDataModel::initDataModel()
{
  CustomViewerDataModel::initDataModel();

  QDateTime dt = QDateTime(QDate::currentDate(), QTime(0,0));
  Condition condition = ConditionDateTimeInterval("dt", columnDisplayName("dt"), dt, dt.addDays(1) );

  addCondition(kDt, condition, true );

  updateTimer_.setInterval(defaultUpdateTimeout);
  connect(&updateTimer_, SIGNAL(timeout()), this, SLOT(slotTimeout()));
  updateTimer_.start();

  QObject::connect(leftPanel_, &journal::JournalLeftPanel::signalFilterChanged,
                   this, &JournalWidgetDataModel::slotOnLeftPanelStateChanged);
  QObject::connect(this, &JournalWidgetDataModel::signalConditionsChanged,
                   this, &JournalWidgetDataModel::slotFiltersChanged );
}

void JournalWidgetDataModel::forceUpdate()
{
  emit signalUpdate();
}


void JournalWidgetDataModel::slotTimeout()
{
 // debug_log << __FUNCTION__;
  QDateTime now = QDateTime::currentDateTimeUtc();
  ConditionDateTimeInterval dtCondition(kDt, kDt, lastDt_, now);

  auto valueFromRecord = [] ( const proto::CustomViewerTableRecord &record, const QString& sfield)
  {
    for ( auto field: record.field() ){
      auto fieldName = QString::fromStdString(field.name());
      if ( 0 == sfield.compare(fieldName) ){
        return QString::fromStdString(field.value());
      }
    }
    return QString();
  };

  auto reglevelForValue = [] ( const tlog::Priority prior ){
    auto displayConfig = global::Settings::instance()->displayMessageConfig();
    if ( tlog::kDebug == prior ){
      return displayConfig.debug();
    }
    if ( tlog::kInfo == prior ) {
      return displayConfig.info();
    }
    if ( tlog::kWarning == prior ){
      return displayConfig.warning();
    }
    if ( tlog::kError == prior ){
      return displayConfig.error();
    }
    if ( tlog::kCritical == prior ) {
      return displayConfig.critical();
    }
    return displayConfig.debug();
  };

  auto newRecords = this->load({ dtCondition } );
 // debug_log << dtCondition.getJsonCondition();
  for ( auto record: newRecords ) {
    auto title = QObject::tr("Протоколы работы");
    auto msg = valueFromRecord(record, kMessage);

    auto dt = valueFromRecord(record, kDt);
    auto priority = static_cast<tlog::Priority>(
          valueFromRecord(record, kPriority).toInt()
          );

    meteo::settings::RegLevel reg = reglevelForValue(priority);

   // debug_log << reg << priority;
    if ( meteo::settings::kShow == reg ) {
      meteo::dbusSendNotification(title, msg, QString(), 8000, QStringList());
    }
  }

  lastDt_ = now;
}

const QString& JournalWidgetDataModel::stringTemplateTotalRecords() const
{
  static auto stringTemplate = QObject::tr("Всего записей в журнале: %1");
  return stringTemplate;
}

const QString& JournalWidgetDataModel::windowTitle() const
{
  static auto title = QObject::tr("Протоколы работы");
  return title;
}

QWidget* JournalWidgetDataModel::getLeftPanelWidget() const
{
  return leftPanel_;
}

void JournalWidgetDataModel::slotOnLeftPanelStateChanged()
{
  auto selectedItems = leftPanel_->selectedSources();
  if ( 0 == selectedItems.length() ) {
    if ( true == hasCondition(kSender) ){
      conditionRemove(kSender);
      return;
    }
    else {
      //Все уже синхронизированно, нет необходимости в обработке
      return;
    }
  }

  QStringList wrappedItems;
  for ( auto item: selectedItems ){
    wrappedItems << QObject::tr("\"%1\"").arg(item);
  }

  ConditionStringList condition("sender", "Отправитель", selectedItems);

  addCondition(kSender, condition, true );
}

void JournalWidgetDataModel::slotFiltersChanged()
{
  auto kSenderIndex = columnIndexByName(kSender);
  ConditionData* senderData = conditionForColumn(kSenderIndex);

  if ( nullptr == senderData ) {
    leftPanel_->unsetAllCheckboxes();
    return;
  }

  if ( false == senderData->isEnabled ){
    leftPanel_->unsetAllCheckboxes();
    return;
  }

  auto tryParseAsStringCondition = [](const Condition& c){
    const ConditionString casted(c);
    if ( true == casted.isNull() ){
      return QStringList();
    }

    if ( ConditionString::kMatch != casted.type() ){
      return QStringList();
    }
    return QStringList({ casted.value() });
  };

  auto tryParseAsStringListCondition = [](const Condition& c){
    ConditionStringList casted(c);
    if ( true == casted.isNull() )    {
      return QStringList();
    }
    return casted.values();
  };

  QStringList values;
  values << tryParseAsStringCondition(senderData->condition_)
         << tryParseAsStringListCondition(senderData->condition_);

  leftPanel_->setList(values);
}

proto::CustomViewerId JournalWidgetDataModel::tableId() const
{
  return proto::CustomViewerId::kViewerJournal;
}

}
