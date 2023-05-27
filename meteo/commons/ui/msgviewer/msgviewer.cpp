#include "msgviewer.h"
#include <cross-commons/debug/tmap.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>
#include <QRegExp>
#include "msgviewerdatamodel.h"

namespace meteo {

enum {
  kConditionCir = -2
};
/*
QWidget* createDialog( QWidget* parent, const QString& options  )
{
  Q_UNUSED(options)
  MsgViewer* dlg = new MsgViewer( parent );
  if ( true == dlg->init() ){
    dlg->show();
    return dlg;
  }else{
    QMessageBox::information( nullptr, QObject::tr("Ошибка инициализации"), dlg->lastError());
    return nullptr;
  }
}

bool registerDialog()
{
  mapauto::WidgetHandler* hndl = WidgetHandler::instance();
  return hndl->registerWidget( "message_viewer", createDialog );
}
static bool res = registerDialog();
*/

MsgViewer::MsgViewer(QWidget* parent) :
  CustomViewer(parent),
  tableMenu_(nullptr),
  contentWidget_(nullptr),
  tableBtn_(nullptr)
{
  setObjectName("msg_viewer");
  setColumnId("id");
  this->setDataModel(new MsgViewerDataModel(100, getColumnId()) );

  tableBtn_ = new QToolButton;
  tableBtn_->setText(QObject::tr("Все телеграммы"));
  tableBtn_->setLayout(new QHBoxLayout(tableBtn_));
  tableBtn_->setMinimumSize(QSize(195,32));
  tableBtn_->setPopupMode(QToolButton::InstantPopup);
  tableBtn_->setArrowType(Qt::DownArrow);
  tableBtn_->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  addWidgetToPanel(5, tableBtn_);
  connect(tableBtn_, SIGNAL(clicked()), SLOT(slotTableMenuClicked()));
}

MsgViewer::~MsgViewer()
{
}

bool MsgViewer::init()
{
  if( nullptr == tableMenu_ ){
    tableMenu_ = new QMenu( this );
    tableMenu_->addAction(QObject::tr("Все телеграммы"))->setData("telegrams");
    QMenu* circ = tableMenu_->addMenu(QObject::tr("Циркулярные"));
    circ->addAction(QObject::tr("Внешние"))->setData("telegrams_circ_ex");
    circ->addAction(QObject::tr("Внутренние"))->setData("telegrams_circ_in");
    QMenu* addr = tableMenu_->addMenu(QObject::tr("Адресные"));
    addr->addAction(QObject::tr("Входящие"))->setData("telegrams_addr_incoming");
    addr->addAction(QObject::tr("Исходящие"))->setData("telegrams_addr_outgoing");
    connect(tableMenu_, SIGNAL(triggered(QAction*)), SLOT(slotChangeTable(QAction*)));
  }
  bool res = CustomViewer::init();
  connect(tablewidget(), SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(slotItemDoubleClicked(QTableWidgetItem*)));
  if( nullptr != cellMenu() ){
    QAction* act = cellMenu()->actions().first();
    QAction* openAct = new QAction(QObject::tr("Открыть"), cellMenu());
    cellMenu()->insertAction(act, openAct);
    connect(openAct, SIGNAL(triggered()), SLOT(slotOpen()));
  }

  auto prop = meteo::global::mongodbConfTelegram();

  QDateTime dt = QDateTime(QDate::currentDate(), QTime(0,0));

  Condition condition = ConditionDateTimeInterval("dt", getDataModel()->columnDisplayName("dt"), dt, dt.addDays(1));

  getDataModel()->addCondition("dt", condition, true);

  QObject::connect(getDataModel(), &CustomViewerDataModel::signalConditionsChanged, this, &MsgViewer::slotFiltersChanged);

  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("dt"), false);
  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("msg_dt"), false);

  return res;
}

void MsgViewer::slotFiltersChanged()
{
  auto conditinons = getDataModel()->getConditionsEnabled();

  if ( true == conditinons.contains(kConditionCir) ){
    auto condition = conditinons[kConditionCir];
    this->tableBtn_->setText(condition.displayCondition());
  }
  else {
    this->tableBtn_->setText(QObject::tr("Все телеграммы"));
  }
}

void MsgViewer::setDefaultSectionSize()
{
  if( 0 == header() ){
      return;
  }
  header()->blockSignals(true);
  header()->resizeSection(logicalIndexByName( getColumnId() ), 110);
  header()->resizeSection(logicalIndexByName("t1"), 65);
  header()->resizeSection(logicalIndexByName("t2"), 65);
  header()->resizeSection(logicalIndexByName("a1"), 65);
  header()->resizeSection(logicalIndexByName("a2"), 65);
  header()->resizeSection(logicalIndexByName("ii"), 62);
  header()->resizeSection(logicalIndexByName("cccc"), 90);
  header()->resizeSection(logicalIndexByName("yygggg"), 105);
  header()->resizeSection(logicalIndexByName("bbb"), 80);
  header()->resizeSection(logicalIndexByName("dt"), 180);
  header()->resizeSection(logicalIndexByName("route"), 510);
  header()->resizeSection(logicalIndexByName("dt"), 180);
  header()->resizeSection(logicalIndexByName("format"), 100);
  header()->resizeSection(logicalIndexByName("decoded"), 25);
  header()->resizeSection(logicalIndexByName("size"), 100);
  header()->resizeSection(logicalIndexByName("type"), 26);
  header()->resizeSection(logicalIndexByName("external"), 26);
  header()->blockSignals(false);
}

void MsgViewer::setDefaultSectionOrder()
{

}

void MsgViewer::setDefaultSectionVisible()
{
  if( 0 == tablewidget() ){
      return;
  }
  tablewidget()->setColumnHidden(logicalIndexByName( getColumnId() ), true);
  tablewidget()->setColumnHidden(logicalIndexByName("ii"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("bbb"), true);
}

bool MsgViewer::getTelegram(const QString& id, meteo::tlg::MessageNew* tlgout)
{
  meteo::msgcenter::GetTelegramRequest req;
  req.add_ptkpp_id(id.toInt());
  req.set_onlyheader(false);
  std::unique_ptr<meteo::rpc::Channel> ch(meteo::global::serviceChannel( meteo::settings::proto::kMsgCenter));
  if ( nullptr == ch ) {
    QApplication::restoreOverrideCursor();
    error_log.msgBox() << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
    return false;
  }
  std::unique_ptr<msgcenter::GetTelegramResponse> res(ch->remoteCall( &meteo::msgcenter::MsgCenterService::GetTelegram, req, 20000 ));

  if ( nullptr == res ) {
    QApplication::restoreOverrideCursor();
    error_log.msgBox() << meteo::msglog::kServerAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
    return false;
  }
  if ( 0 == res->msg_size() ) {
    warning_log.msgBox() << QObject::tr("Ответ от сервиса получен, но не содержит телеграмму!");
    return false;
  }
  tlgout->CopyFrom(res->msg(0));
  return true;
}

void MsgViewer::slotTableMenuClicked()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if( 0 == btn ){
    return;
  }
  if( 0 != tableMenu_ ){
    QRect widgetRect = btn->geometry();
    widgetRect.moveTopLeft(btn->parentWidget()->mapToGlobal(widgetRect.topLeft()));
    tableMenu_->exec( widgetRect.bottomLeft() );
  }
}

void MsgViewer::slotItemDoubleClicked(QTableWidgetItem* item)
{
  if( nullptr == item || nullptr == tablewidget() ){
    return;
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QMap<QString, QString> cell_value;
  for( int i = 0; i < tablewidget()->columnCount(); i++ ){
    QTableWidgetItem* cell = tablewidget()->item(item->row(), i);
    QTableWidgetItem* header = tablewidget()->horizontalHeaderItem(i);
    if( nullptr != cell && nullptr != header ){
      QString data = cell->data(Qt::UserRole).toString();
      QString col_name = header->data(Qt::UserRole).toString();
      cell_value.insert(col_name, data);
    }
  }
  QString id = tablewidget()->item(item->row(), 0)->data(Qt::UserRole).toString();
  meteo::tlg::MessageNew *res = new tlg::MessageNew();
  if ( false == getTelegram(id, res) ){
    QApplication::restoreOverrideCursor();
    return;
  } else
  if( nullptr == contentWidget_ ){
    contentWidget_ = new ContentWidget(this, getColumnId() );
  }
  contentWidget_->init(res, cell_value);
  contentWidget_->show();
  QApplication::restoreOverrideCursor();
}

void MsgViewer::slotPanelClosed()
{
  CustomViewer::slotPanelClosed();
  if ( nullptr != tableMenu_ && true == checkpanel()->isHidden()){
    tableBtn_->setText("Все телеграммы");
  }  
}

void MsgViewer::slotChangeTable(QAction* act)
{
  QString displayFilter;
  bool isNullDDDD = false;
  bool external = false;

  if (act->data().toString().compare("telegrams_addr_incoming") == 0){
    displayFilter = "Адресные входящие";
    isNullDDDD = false;
    external = true;
  }
  else if (act->data().toString().compare("telegrams_addr_outgoing") == 0){
    displayFilter = "Адресные исходящие";
    isNullDDDD = false;
    external = false;
  }
  else if (act->data().toString().compare("telegrams_circ_ex") == 0){
    displayFilter = "Циркулярные внешние";
    isNullDDDD = true;
    external = true;
  }
  else if (act->data().toString().compare("telegrams_circ_in") == 0){
    displayFilter = "Циркулярные внутренние";
    isNullDDDD = true;
    external = false;
  }
  else {
    displayFilter = QObject::tr("Все телеграммы");
    getDataModel()->conditionRemove(kConditionCir);
    return;
  }

  this->tableBtn_->setText(displayFilter);

  ConditionString conditionDdd = (true == isNullDDDD )?
        ConditionString("dddd", "dddd", ConditionString::kMatch, false, Qt::CaseSensitive, ""):
        ConditionString("dddd", "dddd", ConditionString::kNotMatch, false, Qt::CaseSensitive, "");


  ConditionBool conditionExternal = ConditionBool("external", "Внешние", external);

  Condition conditionSumm = ConditionLogicalAnd({ conditionDdd, conditionExternal });
  conditionSumm.setDisplayCondition(displayFilter);

  getDataModel()->addCondition(kConditionCir, conditionSumm, true );
}

void MsgViewer::slotUpdate()
{
  auto dataModel = qobject_cast<MsgViewerDataModel*>(getDataModel());
  dataModel->updateLastTlgId();
  CustomViewer::slotUpdate();
}

void MsgViewer::slotOpen()
{
  if( nullptr == tablewidget() ){
    return;
  }
  QTableWidgetItem* item = tablewidget()->currentItem();
  if( nullptr == item ){
    return;
  }
  slotItemDoubleClicked(item);
}


}
