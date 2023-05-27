#include "lgmonitor.h"
//#include <sql/nspgbase/ns_pgbase.h>
//#include <sql/nspgbase/tsqlquery.h>
#include <meteo/commons/global/global.h>
#include <ui_tlgmonitorform.h>
#include <meteo/commons/global/dateformat.h>

enum {
  kT1       = 0,
  kT2       = 1,
  kA1       = 2,
  kA2       = 3,
  kMessage  = 4,
  kCCCC     = 5,
  kYYGGgg   = 6,
  kBBB      = 7,
  kDate     = 8
};

Lgmonitor::Lgmonitor(QWidget *parent) :
  QWidget(parent),
  settings_(0),
  ui_(new Ui::TlgMonitorForm),
  Maxmessages_(5),
  group_settings_(0),
  conditionsWidget_(0),
  choiceCondition_(0),
  checkpanel_(0),
  header_(0),
  contentWidget_(0),
  channel_(0),
  msg_(new meteo::tlg::MessageNew),
  tree_(0),
  thread_(0),
  threadWork_(0)
{
  ui_->setupUi(this);
  header_ = new meteo::bank::ViewHeader(this);
  tree_ = new ViewTreeWidget(this);
  qRegisterMetaType<meteo::tlg::MessageNew>("meteo::tlg::MessageNew");
  conditionsWidget_ = new RegExpConditionDialog(this);
  ui_->spinBox->setRange(5,100);
  ui_->choicecond->setIcon(QIcon(":/meteo/icons/filter-btn.png"));
  QString fileName = this->objectName().replace("::","_");
  group_settings_ = new QSettings(QDir::homePath() + "/.meteo/" + fileName + "GroupSettings.ini", QSettings::IniFormat);

  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + fileName + ".ini", QSettings::IniFormat);

  QObject::connect( ui_->spinBox,  SIGNAL(valueChanged(int)),SLOT(slotMaxmessages(int)));
  QObject::connect( tree_,  SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),SLOT(slotOpenFile(QTreeWidgetItem*,int)) );
  QObject::connect( header_, SIGNAL(activate(int,QAction*)),this,SLOT(slotAboutColumnMenuActivated(int,QAction*)) );
  QObject::connect( header_, SIGNAL(menuAboutToShow(int,QMenu*)),this,SLOT(slotColumnFilterMenuActivated(int,QMenu*)) );
  QObject::connect(qApp, SIGNAL(aboutToQuit()),SLOT(slotPause()));
  QObject::connect( ui_->choicecond,SIGNAL(clicked(bool)),SLOT(slotOpenDialCond()) );

  tree_->setAlternatingRowColors(true);
  tree_->setHeader(header_);
  QVBoxLayout* vbl = new QVBoxLayout( ui_->treeSpace );
  vbl->setMargin(0);
  vbl->addWidget( tree_ );
  tree_->setSortingEnabled(false);
  tree_->setColumnCount(9);
  header_->addFilterColumn(kT1     );
  header_->addFilterColumn(kT2     );
  header_->addFilterColumn(kA1     );
  header_->addFilterColumn(kA2     );
  header_->addFilterColumn(kCCCC   );
  header_->addFilterColumn(kYYGGgg );
  header_->addFilterColumn(kBBB    );
  header_->addFilterColumn(kDate   );
  header_->init();
  columns_.append("T1");
  columns_.append("T2");
  columns_.append("A1");
  columns_.append("A2");
  columns_.append("Сообщение");
  columns_.append("CCCC");
  columns_.append("YYGGgg");
  columns_.append("BBB");
  columns_.append("Дата");
  tree_->setHeaderLabels(columns_);

  createBottomPanel();
  slotRestoreGeometryAndState();
  slotLoadCondList();
  header_->fixComboPositions();
  header_->setSortIndicatorShown(false);

  QObject::connect(header_,SIGNAL(sectionResized(int,int,int)),this,SLOT(slotSaveGeometryAndState()) );
  QObject::connect(header_,SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),this,SLOT(slotSaveGeometryAndState()) );
  QObject::connect(qApp,SIGNAL(aboutToQuit()),SLOT(slotSaveGeometryAndState()));
  header_->setStretchLastSection(true);
}

Lgmonitor::~Lgmonitor()
{
  delete ui_;
  delete settings_; settings_ = 0;
  delete group_settings_; group_settings_ = 0;
  delete header_; header_ = 0;
  delete tree_; tree_ = 0;
}

void Lgmonitor::slotPause()
{
  if( 0 != thread_){
    thread_->quit();
    thread_->wait();
    if(!thread_->isFinished()){
      thread_->terminate();
    }
    thread_->wait();
    threadWork_->deleteLater();
    threadWork_ = 0;
    thread_->deleteLater();
    thread_ = 0;
  }
}

void Lgmonitor::slotPlay()
{
  initThread();
}

void Lgmonitor::slotMaxmessages(int maxmes)
{
  Maxmessages_ = maxmes;
  settings_->setValue("maxmes",maxmes);
  while(tree_->topLevelItemCount()>=Maxmessages_){
    delete tree_->topLevelItem(tree_->topLevelItemCount() - 1);
  }
}

void Lgmonitor::slotOpenFile(QTreeWidgetItem* item,int column)
{
  Q_UNUSED(column);
  QByteArray arr = item->data(kMessage,Qt::UserRole+1).toByteArray();
  std::string str(arr.data(), arr.size());
  meteo::tlg::MessageNew* res = new meteo::tlg::MessageNew;
  res->ParseFromString(str);
  QMap <QString,QString> cell_value;
  cell_value.insert("type",QString::fromStdString(res->msgtype()));
  cell_value.insert("channel",QString::fromStdString(res->metainfo().from()));
  cell_value.insert("converted_dt",QString::fromStdString(res->metainfo().converted_dt()));
  cell_value.insert("id",QString::number(res->metainfo().id()));

  if( 0 == contentWidget_ ){
    contentWidget_ = new meteo::ContentWidget(this);
  }
  contentWidget_->init(res, cell_value);
  contentWidget_->show();
}

void Lgmonitor::slotColumnFilterMenuActivated(int clmn, QMenu *action)
{
  Q_UNUSED(action);
  QTreeWidgetItemIterator it(tree_,QTreeWidgetItemIterator::All);
  while (*it){
    addItem(*it, clmn);
    ++it;
  }
  header_->buildMenu(menu_);
  menu_.clear();
}

void Lgmonitor::slotAboutColumnMenuActivated(int clmn, QAction *action)
{
  if(action->data().toString() == meteo::bank::ViewHeader::conditionCustom_){
    if( 0 == conditionsWidget_ ){
      conditionsWidget_ = new RegExpConditionDialog(this);
    }
    if( columns_.size() > clmn  ){
      if(checkpanelcond_.contains(clmn)){
        conditionsWidget_->setCondition(columns_[clmn], checkpanelcond_.value(clmn));
      }
      else{
        conditionsWidget_->setCondition(columns_[clmn]);
      }
      if( QDialog::Accepted == conditionsWidget_->exec() ){
        if( !conditionsWidget_->sql().isEmpty() ){
          conditions_.insert(clmn,conditionsWidget_->sql());
          condmap_.insert(clmn,conditionsWidget_->sql());
          checkpanelcond_.insert(clmn,conditionsWidget_->checkpanelcond());
          if( true == checkmap_.contains(columns_[clmn]) ){
            if( checkmap_.contains(columns_[clmn]) ){
              delete checkmap_[columns_[clmn]];
              checkmap_.remove(columns_[clmn]);
            }
          }
          QCheckBox* check = new QCheckBox(conditionsWidget_->checkpanelcond(), checkpanel_);
          check->setChecked(true);
          checkpanel_->addCheckBox(check);
          checkmap_.insert(columns_[clmn], check);
          if( false == checkpanel_->isVisible() && checkmap_.count() > 0){
            checkpanel_->show();
          }
          connect(check, SIGNAL(clicked(bool)), SLOT(slotSqlConditionChecked(bool)));
          currentcondition_ = "0";
        }
      }
      else{
        if(0 != header_ && false == checkmap_.contains(columns_[clmn])){
          header_->setFilterEnabled(clmn, false);
        }
      }
    }
    treeFilter();
  }
  else if(action->data().toString() == meteo::bank::ViewHeader::conditionNoConditions_ ){
    conditions_.remove(clmn);
    action->data().clear();
    if( checkmap_.contains(columns_[clmn]) ){
      delete checkmap_[columns_[clmn]];
      checkmap_.remove(columns_[clmn]);
    }
    if( 0 == checkmap_.count() ){
      checkpanel_->close();
    }
  }
  else if( false == action->data().toString().isEmpty() ){
    createConditionByText(clmn, action->data().toString());
  }
}

void Lgmonitor::slotSaveGeometryAndState()
{
  settings_->setValue("geometry",header_->saveGeometry());
  settings_->setValue("state"   ,header_->saveState()   );
}

void Lgmonitor::slotRestoreGeometryAndState()
{
  header_->restoreGeometry(settings_->value("geometry").toByteArray());
  header_->restoreState   (settings_->value("state"   ).toByteArray());
  ui_->spinBox->setValue(settings_->value("maxmes").toInt());
  return;
}

void Lgmonitor::addItem( QTreeWidgetItem *item, int col )
{
  if( 0 == tree_ ){
    return;
  }
  QString value = item->text(col);
  if( -1 == findValue(col, value) ){
    if( false == value.isEmpty() ){
      QPair<QString, QString> p;
      p.first = value;
      menu_[col].append(p);
    }
  }
}

bool Lgmonitor::filteritems(/*QTreeWidgetItem *item*/QStringList* strlst)
{
//  for ( int i = 0, sz = strlst->size(); i < sz; ++i ) {
//    if ( i == 4 ) {
//      debug_log << "message...";
//    }
//    else {
//      debug_log << strlst->at(i);
//    }
//  }

//    debug_log << conditions_;

  int finishcond=0;
  if(conditions_.isEmpty()){
    return true;
  }
  else{
    QList<QString> condlist = conditions_.values();
    foreach (const QString& str, condlist) {
      QStringList clist = str.split(",");
      QRegExp regexpcond1;
      QRegExp regexpcond2;
      if ( clist.size() < 2 ) {
        continue;
      }
      if(clist[1] == "1"){
        regexpcond1.setCaseSensitivity(Qt::CaseInsensitive);
      }
      else{
        regexpcond1.setCaseSensitivity(Qt::CaseSensitive);
      }
      int cond1;
      int cond2;
      QString And_Or;
      if( 3 == clist.count()){
//        debug_log << "PATTERN =" << clist[0] << strlst->at(conditions_.key(str));
        regexpcond1.setPattern(clist[0]);
        finishcond = regexpcond1.indexIn(/*item->text(conditions_.key(str))*/strlst->at(conditions_.key(str)));
      }
      else if(clist.count() > 3){
        if(clist[5] == "1"){
          regexpcond2.setCaseSensitivity(Qt::CaseInsensitive);
        }
        else{
          regexpcond2.setCaseSensitivity(Qt::CaseSensitive);
        }
        regexpcond1.setPattern(clist[0]);
        regexpcond2.setPattern(clist[4]);
        And_Or = clist[3];
        cond1 = regexpcond1.indexIn(strlst->at(conditions_.key(str)));
        cond2 = regexpcond2.indexIn(strlst->at(conditions_.key(str)));
        if("AND" == And_Or){
          if(-1 == cond1 || -1 == cond2){
            finishcond = -1;
          }
          else if(cond1 == cond2 && -1 != cond1){
            finishcond = 0;
          }
        }
        else{
          if(-1 == cond1 && -1 == cond2){
            finishcond = -1;
          }
          else if(-1 != cond1 || -1 != cond2){
            finishcond = 0;
          }
        }
      }
      if( -1 == finishcond ){
        return false;
      }
    }
    if( -1 != finishcond){
      return true;
    }
  }
  return false;
}

void Lgmonitor::createConditionByText(int col, const QString &text)
{
  conditions_[col] = (QString("%1,%2,%3").arg(text).arg("0").arg("0"));
  if( true == checkmap_.contains(columns_[col]) ){
    if( checkmap_.contains(columns_[col]) ){
      delete checkmap_[columns_[col]];
      checkmap_.remove(columns_[col]);
    }
  }
  QString title = columns_[col];
  QString value = text;
  QCheckBox* check = new QCheckBox(QString("%1 = %2").arg(title).arg(value), checkpanel_);
  check->setChecked(true);
  check->setProperty("column_name", columns_[col]);
  check->setProperty("column_value", text);
  check->setProperty("column_number", col);
  checkpanel_->addCheckBox(check);
  condmap_.insert(col,(QString("%1").arg(text)));
  checkpanelcond_.insert(col,QString("%1 = %2 %3").arg(title).arg(value).arg("Регистр_Вкл"));
  checkmap_.insert(columns_[col], check);
  currentcondition_ = "0";
  if( false == checkpanel_->isVisible() && checkmap_.count() > 0){
    checkpanel_->show();
  }
  connect(check, SIGNAL(clicked(bool)), SLOT(slotSqlConditionChecked(bool)));
  treeFilter();
}

void Lgmonitor::treeFilter()
{
  QTreeWidgetItemIterator it(tree_,QTreeWidgetItemIterator::All);
  QList<QTreeWidgetItem*> itemsToDelete;
  while (*it) {
    QStringList* strlst = new QStringList;
      strlst->append((*it)->text(kT1));
      strlst->append((*it)->text(kT2));
      strlst->append((*it)->text(kA1));
      strlst->append((*it)->text(kA2));
      strlst->append((*it)->text(kMessage));
      strlst->append((*it)->text(kCCCC));
      strlst->append((*it)->text(kYYGGgg));
      strlst->append((*it)->text(kBBB));
      strlst->append((*it)->text(kDate));
    bool finish = filteritems(strlst);
    if( false == finish){
      itemsToDelete.append((*it));
    }
    ++it;
  }

  QList<QTreeWidgetItem*> selectedItems( tree_->selectedItems() );
  for ( QTreeWidgetItem * item : itemsToDelete ){
    delete item;
  }
}

int Lgmonitor::findValue(int col, const QString& value)
{
  QList< QPair<QString, QString> >::const_iterator it = menu_[col].begin();
  QList< QPair<QString, QString> >::const_iterator end = menu_[col].end();
  int index = 0;
  while( it != end ){
    if( (*it).first == value ){
      return index;
    }
    ++it;
    index++;
  }
  return -1;
}

void Lgmonitor::createBottomPanel()
{
  if( 0 == checkpanel_ ){
    checkpanel_ = new CheckPanel(this);
    connect(checkpanel_, SIGNAL(panelClosed()), SLOT(slotPanelClosed()));
    ui_->horizontalLayout->insertWidget(1,checkpanel_);
    checkpanel_->setMinimumHeight(32);
    checkpanel_->setMaximumHeight(32);
    checkpanel_->setVisible(false);
  }
}

void Lgmonitor::slotSqlConditionChecked(bool on)
{
  QCheckBox* check = qobject_cast<QCheckBox*>(sender());
  if( 0 == check || 0 == conditionsWidget_ ){
    return;
  }
  int column_number = columns_.indexOf(checkmap_.key(check));
  if( false == on ){
    conditions_.remove(column_number);
    checkpanelcond_.remove(column_number);
  }
  else{
    int reg, translit;
    if ( true == check->text().contains(QObject::tr("Регистр_Вкл"))) {
      reg = 1;
    }
    else {
      reg = 0;
    }
    if ( true == check->text().contains(QObject::tr("Транслит_Вкл"))) {
      translit = 1;
    }
    else {
      translit = 0;
    }
    QString cond = QString("%1,%2,%3").arg(condmap_.value(column_number)).arg(reg).arg(translit);
    conditions_.insert(column_number, cond);
    checkpanelcond_.insert(column_number, check->text());
    treeFilter();
  }
}

void Lgmonitor::slotPanelClosed()
{
  if( 0 != header_ ){
    header_->dropFilters();
  }
  foreach( QCheckBox* check, checkmap_ ){
    if( 0 != check ){
      delete check;
    }
  }
  checkmap_.clear();
  conditions_.clear();
  checkpanelcond_.clear();
  currentcondition_.clear();
  slotSaveCondList();
}

void Lgmonitor::slotServiceDisconnected()
{
    debug_log << tr("Связь с send.service потерята");
    QTimer::singleShot( 5000, this, SLOT(slotInit()) );
}

void Lgmonitor::slotGetMsg(meteo::tlg::MessageNew msg)
{
  ui_->waitLabel->setVisible(false);
  QStringList* strlst = new QStringList;
  strlst->append(QString::fromStdString(msg.header().t1()));
  strlst->append(QString::fromStdString(msg.header().t2()));
  strlst->append(QString::fromStdString(msg.header().a1()));
  strlst->append(QString::fromStdString(msg.header().a2()));
  strlst->append(QString::fromStdString(msg.msg()));
  strlst->append(QString::fromStdString(msg.header().cccc()));
  strlst->append(QString::fromStdString(msg.header().yygggg()));
  strlst->append(QString::fromStdString(msg.header().bbb()));
  strlst->append(QString::fromStdString(msg.metainfo().converted_dt()));

  bool filt = filteritems(strlst);
  delete strlst;
  if( true == filt ){
    QString str = QString::fromStdString(msg.msg());
    if(msg.isbinary()){
      str = "*** БИНАРНЫЕ ДАННЫЕ ***";
    }
    str.replace("\r", "\\r");
    str.replace("\n", "\\n");
    std::string st;
    msg.SerializeToString(&st);
    QByteArray arr(st.data(),st.size());
    if( tree_->topLevelItemCount() >= Maxmessages_ ){
      tree_->insertTopLevelItem(0,tree_->takeTopLevelItem((Maxmessages_ - 1)) );
      tree_->topLevelItem(0)->setText(kT1,QString::fromStdString(msg.header().t1()) );
      tree_->topLevelItem(0)->setText(kT2,QString::fromStdString(msg.header().t2()) );
      tree_->topLevelItem(0)->setText(kA1,QString::fromStdString(msg.header().a1()) );
      tree_->topLevelItem(0)->setText(kA2,QString::fromStdString(msg.header().a2()) );
      tree_->topLevelItem(0)->setText(kCCCC,QString::fromStdString(msg.header().cccc()) );
      tree_->topLevelItem(0)->setText(kYYGGgg,QString::fromStdString(msg.header().yygggg()) );
      tree_->topLevelItem(0)->setText(kBBB,QString::fromStdString(msg.header().bbb()) );
      QDateTime dt = QDateTime::fromString(QString::fromStdString(msg.metainfo().converted_dt()), Qt::ISODate);
      tree_->topLevelItem(0)->setText(kDate, meteo::dateToHumanFull(dt));
      tree_->topLevelItem(0)->setText(kMessage,str);
      tree_->topLevelItem(0)->setData(kMessage,Qt::UserRole+1, arr);
    }
    else{
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setData(kMessage,Qt::UserRole+1, arr);
      item->setText(kMessage,str);
      item->setText(kT1,    QString::fromStdString(msg.header().t1()));
      item->setText(kT2,    QString::fromStdString(msg.header().t2()));
      item->setText(kA1,    QString::fromStdString(msg.header().a1()));
      item->setText(kA2,    QString::fromStdString(msg.header().a2()));
      item->setText(kCCCC,  QString::fromStdString(msg.header().cccc()));
      item->setText(kYYGGgg,QString::fromStdString(msg.header().yygggg()));
      item->setText(kBBB,   QString::fromStdString(msg.header().bbb()));
      QDateTime dt = QDateTime::fromString(QString::fromStdString(msg.metainfo().converted_dt()), Qt::ISODate);
      item->setText(kDate,  meteo::dateToHumanFull(dt));
      tree_->insertTopLevelItem(0,item);
    }
  }
  qApp->processEvents(QEventLoop::ExcludeSocketNotifiers);
}

void Lgmonitor::initThread()
{
  threadWork_ = new ReceiveMessThread();
  thread_ = new QThread;
  threadWork_->moveToThread(thread_);
  connect(thread_,SIGNAL(started()),threadWork_,SLOT(slotInit()) );
  connect(threadWork_,SIGNAL(sendOneMessage(meteo::tlg::MessageNew)),this,SLOT(slotGetMsg(meteo::tlg::MessageNew)) );
  thread_->start();
}

void Lgmonitor::slotOpenDialCond()
{
  if( 0 == choiceCondition_ ){
    choiceCondition_ = new choicecondition(this);
  }
  if(currentcondition_.isEmpty()){
    currentcondition_ = "0";
  }
  if(currentcondition_ == "0" /*&& group_conditions_.contains("0")*/ ){
    group_conditions_["0"] = conditions_;
    group_human_conditions_["0"] = checkpanelcond_;
  }
  if((!group_conditions_.contains("0")) && currentcondition_.isEmpty()){
    group_conditions_.insert("0",conditions_);
    group_human_conditions_.insert("0",checkpanelcond_);
    currentcondition_ = "0";
  }
  choiceCondition_->setGroupCondition(group_conditions_,currentcondition_,conditions_,group_human_conditions_);
  if( QDialog::Accepted == choiceCondition_->exec() ){
    slotPanelClosed();
    currentcondition_ = choiceCondition_->currGroup();
    conditions_ = group_conditions_[currentcondition_];
    checkpanelcond_ = group_human_conditions_[currentcondition_];
    group_conditions_ = choiceCondition_->groupCondList();
    group_human_conditions_ = choiceCondition_->groupCondHumList();
    namescond_ = group_conditions_.keys();
    condmap_ = conditions_;
    slotSaveCondList();
    loadCheckBox();
    treeFilter();
  }
}

void Lgmonitor::slotLoadCondList()
{
  currentcondition_ = group_settings_->value("currentcondition").toString();
  namescond_ = group_settings_->value("namescondition").toStringList();
  foreach (QString str, namescond_) {
    group_conditions_.insert(str,restoreCondition(str,"conditions"));
    group_human_conditions_.insert(str,restoreCondition(str,"checkpanelconditions"));
  }
  if(!currentcondition_.isEmpty()){
    checkpanelcond_ = group_human_conditions_[currentcondition_];
    conditions_ = group_conditions_[currentcondition_];
  }
  else{
    currentcondition_ = "0";
  }
  loadCheckBox();
}

QMap<int, QString> Lgmonitor::restoreCondition(QString str,QString regexporhuman)
{
  QMap<int,QString> cond;
  group_settings_->beginGroup(regexporhuman+str);
  QStringList keys = group_settings_->childKeys();
  foreach (QString key, keys) {
    cond[key.toInt()] = group_settings_->value(key).toString();
  }
  group_settings_->endGroup();
  return cond;
}

void Lgmonitor::slotSaveCondList()
{
  group_settings_->clear();
  group_settings_->setValue("currentcondition",currentcondition_);
  group_settings_->setValue("namescondition",namescond_);
  foreach (QString str, group_conditions_.keys()) {
    group_settings_->beginGroup("conditions" + str);
    QMapIterator<int, QString> i(group_conditions_[str]); // ВАЖНО
    while (true==i.hasNext()) {
      i.next();
      group_settings_->setValue( QString::number(i.key()),i.value());
    }
    group_settings_->endGroup();
  }
  foreach (QString str, group_human_conditions_.keys()) {
    group_settings_->beginGroup("checkpanelconditions"+str);
    QMapIterator<int, QString> j(/*checkpanelcond_*/group_human_conditions_[str]);
    while (true==j.hasNext()) {
      j.next();
      group_settings_->setValue( QString::number(j.key()),j.value());
    }
    group_settings_->endGroup();
  }
}

void Lgmonitor::loadCheckBox()
{
  group_settings_->beginGroup("checkpanelconditions"+currentcondition_);
  QStringList keys = group_settings_->childKeys();
  foreach (QString key, keys) {
    QCheckBox* check = new QCheckBox(group_settings_->value(key).toString(), checkpanel_);
    check->setChecked(true);
    checkpanel_->addCheckBox(check);
    checkmap_.insert(columns_[key.toInt()], check);
    if( false == checkpanel_->isVisible() && checkmap_.count() > 0){
      checkpanel_->show();
    }
    connect(check, SIGNAL(clicked(bool)), SLOT(slotSqlConditionChecked(bool)));
    if(0 != header_ && false == checkmap_.contains(columns_[key.toInt()])){
      header_->setFilterEnabled(key.toInt(), false);
    }
  }
  group_settings_->endGroup();
}

void Lgmonitor::setVisible(bool a)
{
  QWidget::setVisible(a);
  if(false == a){
    slotPause();
  }
  else{
    slotPlay();
  }
}

ViewTreeWidget::ViewTreeWidget(QWidget *parent) : QTreeWidget( parent )
{

}

void ViewTreeWidget::scrollContentsBy(int dx, int dy)
{
  QTreeWidget::scrollContentsBy(dx, dy);
  meteo::bank::ViewHeader* h = qobject_cast<meteo::bank::ViewHeader*>(header());
  if (dx != 0 && h != 0){
    h->fixComboPositions();
  }
}
