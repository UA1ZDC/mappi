#include <meteo/commons/global/global.h>
#include "msgviewerdatamodel.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

namespace meteo {

static const QString dbTelegrams = "telegramsdb";

static const QString qGetAllMsg = "get_from_msg_viewer";
static const QString qCountAllMsg = "count_from_msg_viewer";
static const QString qCountAllMsgWithoutConditions = "count_no_conditions_from_msg_viewer";


QList<proto::CustomViewerTableRecord> MsgViewerDataModel::load(QList<Condition> conditions) const
{
  conditions << ConditionInt64( column_id_, QObject::tr("Номер"), ConditionInt64::kLessOrEqual, lastId_);
  return CustomViewerDataModel::load(conditions);
}

MsgViewerDataModel::MsgViewerDataModel(int pageSize, const QString& column_id )
  : meteo::CustomViewerDataModel(pageSize),
  column_id_(column_id)
{
  QObject::connect(this, &MsgViewerDataModel::signalConditionsChanged, this, &MsgViewerDataModel::onFiltersChanged);

  leftPanel_ = new LeftPanel();
  void slotChangeCondition();
  QObject::connect(leftPanel_, &LeftPanel::changeCondition, this, &MsgViewerDataModel::slotChangeCondition);
  updateLastTlgId();
}

void MsgViewerDataModel::updateLastTlgId()
{
  static const auto serviceTitle = global::serviceTitle(settings::proto::kCustomViewerService);
  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(settings::proto::kCustomViewerService));

  if ( nullptr == channel ){
    error_log.msgBox() << msglog::kServiceConnectFailedSimple
                          .arg(serviceTitle);
    return;
  }

  proto::Dummy request;
  std::unique_ptr<proto::GetLastTlgIdResponce> responce(channel->remoteCall(&proto::CustomViewerService::GetLastTlgId, request, 10000));

  if ( nullptr == responce ){
    error_log.msgBox() << msglog::kServiceAnsverFailed.arg(serviceTitle);
    return;
  }

  if ( responce->last_tlg_id() <= 0 ){
    error_log << msglog::kServiceAnsverFailed.arg(serviceTitle);
    return;
  }

  lastId_ = responce->last_tlg_id();
}

QStringList MsgViewerDataModel::columnMenuValues(const QString& column)
{
  if ( 0 == column.compare("type") ) {
    return QStringList( { "grib", "alphanum", "fax", "doc", "map", "bufr","rsc",
                          "sxf", "map_ptkpp", "ocean_ptkpp", "cut_ptkpp",
                          "aero_ptkpp"});
  }

  if ( 0 == column.compare("bin") ||
       0 == column.compare("decoded") ||
       0 == column.compare("external") ){
    return QStringList( {"true", "false" });
  }
  return QStringList();
}


const QString& MsgViewerDataModel::stringTemplateTotalRecords() const
{
  static auto templateString = QString::fromUtf8("Всего телеграмм в базе данных: %1");
  return templateString;
}

const QString& MsgViewerDataModel::windowTitle() const
{
  static auto title = QObject::tr("Просмотр метеорологических сообщений");
  return title;
}

QWidget* MsgViewerDataModel::getLeftPanelWidget() const
{
  return leftPanel_;
}

void MsgViewerDataModel::onFiltersChanged()
{
  ConditionData* t1Data = conditionForColumn(columnIndexByName("t1"));
  ConditionData* t2Data = conditionForColumn(columnIndexByName("t2"));

  auto tryParseCondition = [](const ConditionData* data){
    if ( nullptr == data ){
      return QStringList();
    }
    if ( false == data->isEnabled ){
      return QStringList();
    }

    ConditionStringList strListCond( data->condition_ );
    if ( false == strListCond.isNull() ) {
      return strListCond.values();
    }
    ConditionString stringCond(data->condition_);
    if ( false == stringCond.isNull() && ConditionString::kMatch == stringCond.type() ){
      return QStringList( { stringCond.value() });
    }
    return QStringList();
  };


  QStringList t1 = tryParseCondition(t1Data);
  QStringList t2 = tryParseCondition(t2Data);

  if ( 0 == t1.size() ){
    leftPanel_->clear();
    return;
  }
  if ( 0 == t2.size() ){
    leftPanel_->selectT1(t1.first());
  }
  else {
    leftPanel_->selectTT(t1.first(), t2);
  }
  if ( 0 == t2.size() ) {
    leftPanel_->selectT1(t1.first());
  }
}


void MsgViewerDataModel::slotChangeCondition()
{
  QStringList selectedT1 = leftPanel_->selectedT1();
  QStringList selectedT2 = leftPanel_->selectedT2();

  if ( true == selectedT1.isEmpty() && true == selectedT2.isEmpty() ) {
    conditionRemove({ "t1", "t2" });
    return;
  }

  QHash<QString, Condition> conditions;
  if ( true == selectedT1.isEmpty() ) {
    this->conditionRemove({ "t1", "t2" });
    return;
  }


  auto cond = ConditionStringList("t1", columnDisplayName("t1"), selectedT1);
  conditions.insert("t1", cond);

  if ( false == selectedT2.isEmpty() ){
    auto cond = ConditionStringList("t2", "T2", selectedT2);
    conditions.insert("t2", cond);
  }

  if ( false == conditions.isEmpty() ){
    insertManyConditions(conditions);
  }
}

proto::CustomViewerId MsgViewerDataModel::tableId() const
{
  return proto::kViewerMsg;
}

}
