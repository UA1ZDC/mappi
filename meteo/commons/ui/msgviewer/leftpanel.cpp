#include "leftpanel.h"
#include "ui_leftpanel.h"

#include <meteo/commons/rpc/rpc.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/customviewer.pb.h>

namespace meteo {

static const QString& kDbTelegrams = QObject::tr("telegramsdb");

enum {
  kCheckCol = 0,
  kIconCol  = 1,
  Id        = 2,
  Name      = 3,
  T1        = 4,
  T2        = 5,
  A1        = 6,
  A2        = 7,
  GroupId   = 8
};

QString stringList2JsonArray(QStringList list)
{
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QString s = list.at(i);
    s = "\"" + s + "\"";
    list[i] = s;
  }
  return list.join(",");
}

LeftPanel::LeftPanel(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::LeftPanel)
{
  ui_->setupUi(this);

  lineEdit_ = new LineEdit(this);
  ui_->horizontalLayout->insertWidget(1, lineEdit_);

  connect(ui_->groupTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotCheckGroup(QTreeWidgetItem*)));
  connect(ui_->groupTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotCheckGroup(QTreeWidgetItem*)));
  connect(ui_->itemTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotCheckItem(QTreeWidgetItem*)));
  connect(lineEdit_, SIGNAL(textChanged(QString)), SLOT(slotSearch(QString)));  
  createList();
  if (ui_->groupTree->topLevelItemCount() > 0)
    ui_->groupTree->setCurrentItem(ui_->groupTree->topLevelItem(0));

  ui_->itemTree->setColumnHidden(Id, true);
  ui_->itemTree->setColumnHidden(T1, true);
  ui_->itemTree->setColumnHidden(T2, false);
  ui_->itemTree->setColumnHidden(A1, true);
  ui_->itemTree->setColumnHidden(A2, true);
  ui_->itemTree->setColumnHidden(GroupId, true);

  ui_->groupTree->setColumnHidden(Id, true);
  ui_->groupTree->setColumnHidden(T1, false);
  ui_->groupTree->setColumnHidden(T2, true);
  ui_->groupTree->setColumnHidden(A1, true);
  ui_->groupTree->setColumnHidden(A2, true);

  auto groupTreeHeader = ui_->groupTree->header();
  groupTreeHeader->setSectionResizeMode(Name, QHeaderView::ResizeMode::Stretch);
  groupTreeHeader->setSectionResizeMode(kCheckCol, QHeaderView::ResizeMode::Fixed);  
  groupTreeHeader->setStretchLastSection(false);

  auto itemTreeHeader = ui_->itemTree->header();
  itemTreeHeader->setSectionResizeMode(Name, QHeaderView::ResizeMode::Stretch);  
  itemTreeHeader->setStretchLastSection(false);
}

LeftPanel::~LeftPanel(){
  if (nullptr != this->ui_){
    delete this->ui_;
    this->ui_ = nullptr;
  }
}

void LeftPanel::clearT2()
{
  ui_->itemTree->blockSignals(true);
  ui_->groupTree->blockSignals(true);

  for ( int i = 0; i < ui_->itemTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->itemTree->topLevelItem(i);
    currentItem->setCheckState(kCheckCol, Qt::CheckState::Unchecked);
  }

  lastT2_.clear();
  ui_->itemTree->blockSignals(false);
  ui_->groupTree->blockSignals(false);
}


void LeftPanel::selectT1(const QString& t1)
{
  ui_->itemTree->blockSignals(true);
  ui_->groupTree->blockSignals(true);


  for ( int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->groupTree->topLevelItem(i);
    if ( t1 == currentItem->text(T1) ){
      currentItem->setCheckState( kCheckCol, Qt::CheckState::Checked );
      ui_->groupTree->setCurrentItem(currentItem);
      updateItemTree(currentItem->text(Id));
    } else {
      currentItem->setCheckState(kCheckCol, Qt::CheckState::Unchecked);
    }
  }

  lastT1_ = t1;

  for ( int i = 0; i < ui_->itemTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->itemTree->topLevelItem(i);
    currentItem->setCheckState(kCheckCol, Qt::CheckState::Unchecked);
  }

  lastT2_.clear();
  ui_->itemTree->blockSignals(false);
  ui_->groupTree->blockSignals(false);
}


void LeftPanel::selectTT(const QString& t1, const QStringList& t2)
{
  ui_->itemTree->blockSignals(true);
  ui_->groupTree->blockSignals(true);


  for ( int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->groupTree->topLevelItem(i);
    if ( t1 == currentItem->text(T1) ){
      currentItem->setCheckState( kCheckCol, Qt::CheckState::Checked );
      ui_->groupTree->setCurrentItem(currentItem);
      updateItemTree(currentItem->text(Id));
    } else {
      currentItem->setCheckState(kCheckCol, Qt::CheckState::Unchecked);
    }
  }

  lastT1_ = t1;

  for ( int i = 0; i < ui_->itemTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->itemTree->topLevelItem(i);    
    if ( t1 == currentItem->text(T1) && true == t2.contains( currentItem->text(T2)) ){
      currentItem->setCheckState( kCheckCol, Qt::CheckState::Checked );
    }
    else {
      currentItem->setCheckState(kCheckCol, Qt::CheckState::Unchecked);
    }
  }

  lastT2_.clear();
  if ( false == t2.isEmpty() ){
    for (const QString& singleT2: t2){
      lastT2_ << singleT2;
    }
  }

  ui_->itemTree->blockSignals(false);
  ui_->groupTree->blockSignals(false);
}

void LeftPanel::clear()
{
  ui_->itemTree->blockSignals(true);
  ui_->groupTree->blockSignals(true);
  lastT1_ = QString();
  lastT2_.clear();

  QTreeWidgetItem *selected = nullptr;

  for ( int i = 0; i < ui_->itemTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->itemTree->topLevelItem(i);
    currentItem->setCheckState( kCheckCol, Qt::Unchecked );        
    currentItem->setSelected(false);
  }
  for ( int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->groupTree->topLevelItem(i);
    currentItem->setCheckState( kCheckCol, Qt::Unchecked );       
    if ( true == currentItem->isSelected() ){
      selected = currentItem;
    }
  }  

  if ( nullptr != selected ){
    updateItemTree(selected->text(Id));

  }

  ui_->itemTree->blockSignals(false);
  ui_->groupTree->blockSignals(false);
}

QTreeWidget *LeftPanel::getT1Tree()
{
  return ui_->groupTree;
}

void LeftPanel::addItemToChecked(const QString &id)
{
  for ( int i = 0, sz = ui_->itemTree->topLevelItemCount(); i < sz; ++i ) {
    if ( ui_->itemTree->topLevelItem(i)->text(0) == id ) {
      checked_id_.append(ui_->itemTree->topLevelItem(i)->text(0));
    }
  }
}

void LeftPanel::setT1Enabled(const QStringList list)
{
  bool select = true;
  for ( int i = 0, sz = ui_->groupTree->topLevelItemCount(); i < sz; ++i ) {
    if ( false == list.contains(ui_->groupTree->topLevelItem(i)->text(T1)) ) {
      ui_->groupTree->topLevelItem(i)->setHidden(true);
    }
    else {
      if ( select ) {
        ui_->groupTree->topLevelItem(i)->setSelected(true);
        select = false;
      }
    }
  }
  updateItemTree(ui_->groupTree->currentItem()->text(Id));
}

QTreeWidget *LeftPanel::getT2Tree()
{
  return ui_->itemTree;
}

void LeftPanel::createList()
{
  static const int timeout = 5000;
  if ( false == tryCreateList() ){
    QTimer::singleShot(timeout, this, &LeftPanel::createList);
  }
}

bool LeftPanel::tryCreateList()
{
  static const auto service = settings::proto::kCustomViewerService;
  static const auto serviceTitle = global::serviceTitle(service);

  std::unique_ptr<rpc::Channel> channel(global::serviceChannel(service));
  if ( nullptr == channel ){
    error_log << msglog::kServiceConnectFailedSimple.arg(serviceTitle);
    return false;
  }

  proto::Dummy request;
  std::unique_ptr<proto::GetAvailableTTResponce> responce(channel->remoteCall(&proto::CustomViewerService::GetAvailableTT,
                                                                              request,
                                                                              10000));

  if ( nullptr == responce ){
    error_log << msglog::kServiceAnsverFailed.arg(serviceTitle);
    return false;
  }

  ui_->groupTree->clear();
  QMap<QString, QString> groupIdMap;

  for( auto t1Proto: responce->t1() ) {
    auto name = QString::fromStdString(t1Proto.name());
    auto t1 = QString::fromStdString(t1Proto.t1());
    auto id = QString::fromStdString(t1Proto.t1());
    QString t2;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->groupTree);
    item->setText(Name, name);
    item->setText(Id, id);    
    item->setText(T1, t1);
    item->setText(T2, t2);
    item->setIcon(kIconCol, QIcon(":/meteo/icons/group.png"));
    item->setCheckState( kCheckCol, Qt::Unchecked );
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);

    groupIdMap.insert(t1, id);
  }

  for( int i = 0; i < ui_->groupTree->columnCount(); i++ ){
    ui_->groupTree->resizeColumnToContents(i);
  }

  ui_->itemTree->clear();
  items_group_.clear();

  QList<QTreeWidgetItem*> allItems;

  for ( auto t2Proto: responce->t2() ) {
    auto name = QString::fromStdString(t2Proto.name());
    auto t1 = QString::fromStdString(t2Proto.t1());
    auto t2 = QString::fromStdString(t2Proto.t2());
    auto id = QObject::tr("%1-%2").arg(t1).arg(t2);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    allItems << item;
    item->setText(Name, name);
    item->setText(Id, id);    
    item->setText(T1, t1);
    item->setText(T2, t2);
    QString group_id = groupIdMap[t1];
    item->setText(GroupId, group_id);
    item->setIcon(kIconCol, QIcon(":/meteo/icons/item.png"));
    item->setCheckState( kCheckCol, Qt::Unchecked );    
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);    
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    items_group_.insert(id, group_id);
  }
  ui_->itemTree->addTopLevelItems(allItems);

  for( int i = 0; i < ui_->itemTree->columnCount(); i++ ){
    ui_->itemTree->resizeColumnToContents(i);
  }
  ui_->groupTree->sortByColumn(Name, Qt::AscendingOrder);
  ui_->itemTree->sortByColumn(Name, Qt::AscendingOrder);

  if ( 0 != ui_->groupTree->topLevelItemCount() ){
    auto first = ui_->groupTree->topLevelItem(0);
    first->setSelected(true);
    updateItemTree(first->text(Id));
  }

  return true;
}

QStringList LeftPanel::selectedT1()
{
  QStringList result;
  if ( false == lastT1_.isEmpty() ) {
    result << lastT1_
           << lastT1_.toUpper()
           << lastT1_.toLower();
  }
  result.removeDuplicates();
  return result;

}

QStringList LeftPanel::selectedT2()
{
  QStringList result;
  for ( auto t2: lastT2_ ){
    result << t2
               << t2.toUpper()
               << t2.toLower();
  }
  result.removeDuplicates();
  return result;
}

void LeftPanel::updateItemTree(const QString& group_id)
{
  QTreeWidgetItemIterator it2( ui_->itemTree );
  while( *it2 ){
    (*it2)->setHidden( true );
    ++it2;
  }
  bool needFilter = !group_id.isEmpty();
  QTreeWidgetItemIterator it( ui_->itemTree );
  int i = 0;
  while( *it ){
    (*it)->setHidden( needFilter );
    (*it)->setCheckState(kCheckCol, Qt::Unchecked);
    if ( checked_id_.contains( (*it)->text(Id)) ){
      (*it)->setCheckState(kCheckCol ,Qt::Checked);
    }
    ++it;
    ++i;
  }
  if( !needFilter ){
    return;
  }
  QList<QTreeWidgetItem*> foundItems;
  foundItems  = ui_->itemTree->findItems( group_id, Qt::MatchContains, GroupId );
  foundItems += ui_->itemTree->findItems( group_id, Qt::MatchContains, GroupId );

  it = QTreeWidgetItemIterator( ui_->itemTree, QTreeWidgetItemIterator::Hidden );
  while( *it ){
    if( foundItems.contains(*it) ){
      (*it)->setHidden( false );
      foundItems.removeOne( *it );
    }
    ++it;
  }  
}

QTreeWidgetItem* LeftPanel::itemByT1(const QString& t1)
{
  for ( int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i ){
    auto currentItem = ui_->groupTree->topLevelItem(i);
    const QString currentItemT1 = currentItem->text(T1);
    if ( t1 == currentItemT1 ){
      return currentItem;
    }
  }
  return nullptr;
}

void LeftPanel::slotCheckItem(QTreeWidgetItem* item)
{
  if ( true == this->isHidden() ) return;
  ui_->groupTree->blockSignals(true);
  ui_->itemTree->blockSignals(true);

  const QString& t1 = item->text(T1);
  const QString& t2 = item->text(T2);

  if ( false == item->isSelected() ) {
    ui_->itemTree->setCurrentItem(item);
  }

  if ( ( Qt::Checked == item->checkState(kCheckCol) ) && ( false == lastT2_.contains(t2) || t1 != lastT1_) ) {
    auto t1Item = itemByT1(t1);
    if ( nullptr != t1Item && Qt::Unchecked == t1Item->checkState(kCheckCol) ) {
      t1Item->setCheckState(kCheckCol, Qt::Checked);
      lastT2_.clear();
    }
    lastT1_ = t1;
    lastT2_ << t2;
    slotUpdate();
  }


  if ( ( Qt::Unchecked == item->checkState(kCheckCol) && ( true == lastT2_.contains(t2)) && t1 == lastT1_) ) {
    lastT2_.remove(t2);
    lastT2_.remove(t2.toLower());
    lastT2_.remove(t2.toUpper());
    if ( true == lastT2_.isEmpty() ){
      auto selectedItems = ui_->groupTree->selectedItems();

      if ( 1 == selectedItems.size() ){
        lastT1_ = QString();
        selectedItems.first()->setCheckState(kCheckCol, Qt::CheckState::Unchecked );
      }
    }
    slotUpdate();
  }

  ui_->groupTree->blockSignals(false);
  ui_->itemTree->blockSignals(false);
}

bool LeftPanel::hasSelectedT1(){
  for (int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i){
    auto currentItem = ui_->groupTree->topLevelItem(i);
    if ( Qt::Checked == currentItem->checkState(kCheckCol) ){
      return true;
    }
  }
  return false;
}

void LeftPanel::slotCheckGroup(QTreeWidgetItem* item )
{
  if ( true == this->isHidden() ) return;
  ui_->groupTree->blockSignals(true);
  ui_->itemTree->blockSignals(true);

  const QString& t1 = item->text(T1);
  const QString& groupId = item->text(Id);

  auto selectedT1Items = ui_->groupTree->selectedItems();

  updateItemTree(groupId);

  if ( false == item->isSelected() && Qt::Checked == item->checkState(kCheckCol) ) {
    ui_->groupTree->setCurrentItem(item);
  }

  if ( t1 != lastT1_ &&  Qt::Checked == item->checkState(kCheckCol) ) {
    for (int i = 0; i < ui_->groupTree->topLevelItemCount(); ++i){
      auto t1Item = ui_->groupTree->topLevelItem(i);
      if ( t1Item != item && Qt::Checked == t1Item->checkState(kCheckCol) ) {
        t1Item->setCheckState(kCheckCol, Qt::Unchecked);
      }
    }
    lastT1_ = t1;
    lastT2_.clear();
    slotUpdate();
  }
  bool hasSelectedT1Item = hasSelectedT1();

  if (  t1 == lastT1_ && Qt::Unchecked ==  item->checkState(kCheckCol) && false == hasSelectedT1Item) {
    if ( false == lastT2_.isEmpty() ){
      lastT2_.clear();
      for (int i = 0; i < ui_->itemTree->topLevelItemCount(); ++i){
        auto t2Item = ui_->itemTree->topLevelItem(i);
        t2Item->setCheckState(kCheckCol, Qt::Unchecked);
      }
    }
    lastT1_ = QString();
    slotUpdate();
  }
  ui_->groupTree->blockSignals(false);
  ui_->itemTree->blockSignals(false);
}

void  LeftPanel::hideEditFilterButton(bool ){

}

void LeftPanel::slotSearch(const QString& text)
{
  QString searchText = text;

  bool needFilter = !searchText.isEmpty();

  if( !needFilter ){
    showAllItems();
    return;
  }
  QTreeWidgetItemIterator it2( ui_->groupTree );
  while( *it2 ){
    (*it2)->setHidden( true );
    ++it2;
  }
  QTreeWidgetItemIterator it( ui_->itemTree );
  while( *it ){
    (*it)->setHidden( needFilter );
    ++it;
  }
  QList<QTreeWidgetItem*> foundItems;
  foundItems  = ui_->itemTree->findItems( searchText, Qt::MatchContains, Name );
  foundItems += ui_->itemTree->findItems( searchText, Qt::MatchContains, Name );

  it = QTreeWidgetItemIterator( ui_->itemTree, QTreeWidgetItemIterator::Hidden );
  while( *it ){
    if( foundItems.contains(*it) ){
      (*it)->setHidden( false );
      showParentItems( (*it)->text(GroupId) );
      foundItems.removeOne( *it );
    }
    ++it;
  }
}

void LeftPanel::showParentItems(const QString& id)
{
  QTreeWidgetItemIterator it( ui_->groupTree );
  QList<QTreeWidgetItem*> foundItems;
  foundItems  = ui_->groupTree->findItems( id, Qt::MatchFixedString, Id);
  foundItems += ui_->groupTree->findItems( id, Qt::MatchFixedString, Id);

  it = QTreeWidgetItemIterator( ui_->groupTree, QTreeWidgetItemIterator::Hidden );
  while( *it ){
    if( foundItems.contains(*it) ){
      (*it)->setHidden( false );
      foundItems.removeOne( *it );
    }
    ++it;
  }
}

void LeftPanel::showAllItems()
{
  QTreeWidgetItemIterator it2( ui_->groupTree );
  while( *it2 ){
    (*it2)->setHidden( false );
    ++it2;
  }
  QTreeWidgetItemIterator it( ui_->itemTree );
  while( *it ){
    (*it)->setHidden( false );
    ++it;
  }

}

void LeftPanel::slotUpdate()
{
  emit changeCondition();
}

}
