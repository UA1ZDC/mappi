#include "viewheader.h"

#include <cross-commons/debug/tlog.h>

#include "qevent.h"
#include <qheaderview.h>
#include <qdebug.h>
#include <qtreewidget.h>
#include <qapplication.h>

namespace meteo {

namespace bank {

const QString ViewHeader::conditionNoConditions_ = QObject::tr("no_conditions");
const QString ViewHeader::conditionCustom_ = QObject::tr("edit_action");

ViewHeader::ViewHeader(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{
  connect(this, SIGNAL(sectionResized(int, int, int)), this, SLOT(handleSectionResized(int)));
  connect(this, SIGNAL(sectionMoved(int, int, int)), this, SLOT(handleSectionMoved(int, int, int)));
  setMaximumHeight(26);
  setMinimumHeight(26);
  setSectionsMovable(true);
  setSectionsClickable(true);
  setSortIndicatorShown(true);
  setDefaultAlignment(Qt::AlignLeft);
  QString qss(
              "QHeaderView::down-arrow { subcontrol-position: left; width: 20px; height:20px; image: url(:/meteo/icons/arrow/down-arrow_sort.png);}"
              "QHeaderView::up-arrow { subcontrol-position: left; width: 20px; height:20px;image: url(:/meteo/icons/arrow/up-arrow_sort.png); }"
              "QHeaderView::section:horizontal { padding-left: 20px; padding-top: 5px; padding-bottom: 5px; padding-right: 24px;}"
              );
  setStyleSheet(qss);
}

ViewHeader::~ViewHeader()
{
  for ( auto columnMenu: menu_.values() ){
    delete columnMenu;
  }
}

bool ViewHeader::restoreState(const QByteArray& state)
{
  bool ok = QHeaderView::restoreState(state);
  for (int i = 0; i < count(); ++i){
    if ( ( true == btn_.contains(i)) && ( nullptr != btn_[i] )) {
        btn_[i]->setHidden(isSectionHidden(i));
    }
  }
  return ok;
}

void ViewHeader::init()
{
  foreach( int i, fcolumns_ ){
    // if( false == menu_.contains(i) ){
    if( menu_.contains(i) ){
      delete menu_[i];
      menu_.remove(i);
    }
    if( btn_.contains(i) ){
      delete btn_[i];
      btn_.remove(i);
    }
    QToolButton *b= new QToolButton(this);
    b->setAutoRaise(true);
    btn_.insert(i, b);
    /*
     * Parent у QMenu нельзя задавать, т.к. иначе ViewHeader будет неверно работать в QGraphicsScene
    */
    QMenu* menu = new QMenu();
    QString st("QMenu { menu-scrollable: 1; }");
    menu->setStyleSheet(st);
    connect( menu, SIGNAL(triggered(QAction*)), SLOT(slotActivateMenu(QAction*)) );
    menu->addSeparator();
    menu_.insert(i, menu);
    btn_[i]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down.png"));
    connect(btn_[i], SIGNAL(clicked()), SLOT(slotMenuClicked()));
    btn_[i]->setGeometry(sectionViewportPosition(i)+sectionSize(i)-23, 3, 20, 20);
    QTableWidget* p = qobject_cast<QTableWidget*>(parent());
    if( nullptr != p ){
      btn_[i]->setVisible(!p->isColumnHidden(i));
    }
    //  }
    //  else{
    //    debug_log <<  QObject::tr("Меню с индексом %1 уже добавлено!").arg(i);
    //  }
  }

}

void ViewHeader::paintEvent(QPaintEvent *e)
{  
  slotFixComboPositions();
  QHeaderView::paintEvent(e);
}

void ViewHeader::showEvent(QShowEvent *e)
{
  QHeaderView::showEvent(e);
}

void ViewHeader::setSecondValueDisplayed(int col, bool displayed)
{
  isSecondValueDisplayed_.insert(col, displayed);
}

void ViewHeader::handleSectionResized(int i)
{
  if ( true == isHidden() ){
    return;
  }
  if (nullptr != btn_[i]){    
    btn_[i]->setVisible(sectionSize(i) != 0);
  }

  for (int j=visualIndex(i);j<count();j++) {
    int logical = logicalIndex(j);
    if ( nullptr != btn_[logical] ) {
      btn_[logical]->setGeometry(sectionViewportPosition(logical)+sectionSize(logical)-23, 3, 20, 20);
    }
  }
}

void ViewHeader::handleSectionMoved(int /*logical*/, int oldVisualIndex, int newVisualIndex)
{    
  for (int i=qMin(oldVisualIndex, newVisualIndex);i<count();i++){
    int logical = logicalIndex(i);
    if( nullptr != btn_[logical] ){
      btn_[logical]->setGeometry(sectionViewportPosition(logical)+sectionSize(logical)-23, 3, 20, 20);
    }
  }
  emit movecolumn(oldVisualIndex, newVisualIndex);
}

void ViewHeader::slotMenuClicked()
{
  QToolButton* b = qobject_cast<QToolButton*>( sender() );
  int menuId = btn_.key(b);
  if( nullptr == menu_[menuId] ){
    return;
  }
  menu_[menuId]->clear();
  QAction* act = menu_[menuId]->addAction(QIcon(":/meteo/icons/arrow/no-conditions.png"), tr("(Все)"));
  act->setData(conditionNoConditions_);
  act->setIconVisibleInMenu(true);

  QAction* add_act =  menu_[menuId]->addAction(tr("(Условие...)"));
  add_act->setData(conditionCustom_);
  emit menuAboutToShow( menuId, menu_[menuId] );

  QHash<QString, QString> actions;

  auto it = items_[menuId].begin();
  auto end = items_[menuId].end();

  for ( ; it != end; ++it ) {
    QString original_value = (*it).first;
    if( true == original_value.isEmpty() ){
      continue;
    }
    auto secondValue = (*it).second;
    QString val;
    if ( true == originalValueHidden_ ){
      val = secondValue;
    }
    else if ( original_value == secondValue ) {
      val = original_value;
    }
    else {
      bool needDisplaySecondValue = isSecondValueDisplayed_.value(menuId, true);
      if ( true == needDisplaySecondValue ) {
        val = QObject::tr("%1 (%2)").arg(secondValue).arg(original_value);
      }
      else {
        val = secondValue;
      }
    }
    actions.insert(val, original_value);
  }

  QStringList values = actions.keys();
  values.sort();
  for (const QString& value: values){
    const QString& originalValue = actions[value];
    QAction* act = menu_[menuId]->addAction(value);
    act->setData(originalValue);
  }

  menu_[menuId]->exec( QCursor::pos() );
}

void ViewHeader::slotActivateMenu(QAction *act)
{
  if( act->objectName() == conditionCustom_ ){
    return;
  }
  QMenu* m = qobject_cast<QMenu*>( sender() );
  int id = menu_.key(m);
  if( act->data().toString() == conditionNoConditions_ ){
    setHaveCondition(id, false);
  }else {
    setHaveCondition(id, true);
  }
  emit activate( id, act );
}

void ViewHeader::setHaveCondition(int col, bool ifHaveCondition)
{
  if ( ( false == btn_.contains(col) ) || ( nullptr == btn_[col] ) ) return;

  if( false == ifHaveCondition ){
    btn_[col]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down.png"));
  }
  else {
    btn_[col]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down_blue.png"));
  }
}

void ViewHeader::buildMenu(QMap<int,QSet< QPair<QString, QString> > > values)
{
  QMap<int,QList< QPair<QString, QString> > > listValues;
  for ( auto key: values.keys() ){
    listValues[key] = values[key].toList();
  }
  buildMenu(listValues);
}

void ViewHeader::slotOutsideMenuClicked()
{
  QToolButton* b = qobject_cast<QToolButton*>( sender() );
  int menuId = btn_.key(b);
  if( nullptr == menu_[menuId] ){
    return;
  }
  menu_[menuId]->exec( QCursor::pos() );
}

void ViewHeader::slotFixComboPositions()
{  
  for (int i=0;i<count();i++){
    if( nullptr != btn_[i] ){      
      btn_[i]->setGeometry(sectionViewportPosition(i)-23+sectionSize(i), 3, 20, 20);
      btn_[i]->setVisible(sectionSize(i) != 0);
    }
  }
}

void ViewHeader::fixComboPositions()
{
  QTimer::singleShot(0, this, SLOT(slotFixComboPositions()));
}

void ViewHeader::buildMenu(MenuItems value)
{
  items_ = value;
}

void ViewHeader::setFilterEnabled(int id, bool on)
{
  if( false == btn_.contains(id) || nullptr == btn_[id] ){
    return;
  }
  if( on == false ){
    btn_[id]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down.png"));
  }else{
    btn_[id]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down_blue.png"));
  }
}

void ViewHeader::dropFilters()
{
  foreach( QToolButton* btn, btn_ ){
    if( 0 != btn ){
      btn->setIcon(QIcon(":/meteo/icons/arrow/arrow_down.png"));
    }
  }
}

void ViewHeader::addFilterColumn(int col)
{
  fcolumns_.append(col);
}

void ViewHeader::setFilterColumns(QList<int> list)
{
  fcolumns_ = list;
}

void ViewHeader::setButtonVisible(int id, bool visible)
{
  if( 0 != btn_[id] ){
    btn_[id]->setVisible(visible);
  }
}

QMenu* ViewHeader::getMenu(int col){
    return menu_[col];
}

void ViewHeader::addMenu(QMenu* menu, int col)
{
  //  if( false == menu_.contains(col) ){
  if( menu_.contains(col) ){
    delete menu_[col];
    menu_.remove(col);
  }
  if( btn_.contains(col) ){
    delete btn_[col];
    btn_.remove(col);
  }
  QToolButton *b= new QToolButton(this);
  QString st("QMenu { menu-scrollable: 1; }");
  menu->setStyleSheet(st);
  connect( menu, SIGNAL(triggered(QAction*)), SLOT(slotActivateMenu(QAction*)) );
  menu->addSeparator();
  menu_.insert(col, menu);
  b->setAutoRaise(true);
  btn_.insert(col, b);
  btn_[col]->setIcon(QIcon(":/meteo/icons/arrow/arrow_down.png"));
  connect(btn_[col], SIGNAL(clicked()), SLOT(slotOutsideMenuClicked()));
  btn_[col]->setGeometry(sectionViewportPosition(col)+sectionSize(col)-23, 3, 20, 20);
  QTableWidget* p = qobject_cast<QTableWidget*>(parent());
  if( 0 != p ){
    btn_[col]->setVisible(!p->isColumnHidden(col));
  }

  // }
  // else{
  //   debug_log <<  QObject::tr("Меню с индексом %1 уже добавлено").arg(col);
  // }
}

void ViewHeader::setOriginalValueHidden(bool hidden)
{
  originalValueHidden_ = hidden;
}

void ViewHeader::setMenuToolTip(int col, const QString &tooltip)
{
  if( btn_.contains(col) ){
    if( btn_[col] != 0 ){
      btn_[col]->setToolTip(tooltip);
    }
  }
}

void ViewHeader::clearContent()
{
  foreach(QToolButton* b, btn_){
    if( 0 != b ){
      delete b;
    }
  }
  foreach(QMenu* m, menu_){
    if( 0 != m ){
      delete m;
    }
  }
  btn_.clear();
  menu_.clear();
  fcolumns_.clear();
}

}

}
