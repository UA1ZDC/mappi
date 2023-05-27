#include "menu.h"

#include <cross-commons/debug/tlog.h>

#include "mapview.h"

namespace meteo {
namespace map {

Menu::Menu( MapView* v )
  : QMenu(v),
  view_(v)
{
}

Menu::~Menu()
{
}

void Menu::addAction( const QString& section, QAction* a )
{
  QString locname = section;
  if ( 0 == locname.size() ) {
    locname = kStrOther;
    warning_log << QObject::tr("Пустое название для секции меню. Устанавливается значение - '%1'")
      .arg(locname);
  }
  QList<QAction*>& list = actions_[locname];
  if ( false == list.contains(a) ) {
    list.append(a);
  }
}

void Menu::addDocumentAction( QAction* a )
{
  addAction( kStrDocument, a );
}

void Menu::addLayerAction( QAction* a )
{
  addAction( kStrLayer, a );
}

void Menu::addObjectAction( QAction* a )
{
  addAction( kStrObject, a );
}

void Menu::addAnalyseAction( QAction* a )
{
  addAction( kAnalyse, a );
}

//добавить секцию меню по названию. Если уже есть - вернется указатель на нее
QMenu *Menu::addMenu(const QString& title){
  
  QList<QAction *> mmm; 
  foreach(mmm, actions_){
    QMenu* m = nullptr;
    for(int i=0;i<mmm.count();++i ){
      if(nullptr ==mmm.at(i)->menu()) continue;
      if(title == mmm.at(i)->menu()->title()){
        m = mmm.at(i)->menu();
        return m;
      }
    }
  }

  return new QMenu( title,nullptr );
}


void Menu::buildMenu()
{
  QList<QAction*>& list  = actions_[kStrLayer];
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QMenu::addAction( list[i] );
  }
  if ( 0 != list.size() ) {
    QMenu::addSeparator();
  }
  
  list = actions_[kAnalyse];
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QMenu::addAction( list[i] );
  }
  if ( 0 != list.size() ) {
    QMenu::addSeparator();
  }
  
  list = actions_[kStrDocument];
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QMenu::addAction( list[i] );
  }
  if ( 0 != list.size() ) {
    QMenu::addSeparator();
  }

  list = actions_[kStrObject];
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QMenu::addAction( list[i] );
  }
  if ( 0 != list.size() ) {
    QMenu::addSeparator();
  }
  QMapIterator<QString, QList<QAction*> > it(actions_);
  while ( true == it.hasNext() ) {
    it.next();
    const QString& name = it.key();
    if ( kStrDocument == name
      || kStrLayer == name
      || kStrObject == name )
    {
      continue;
    }
    list = it.value();
    for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      QMenu::addAction( list[i] );
    }
    if ( 0 != list.size() ) {
      QMenu::addSeparator();
    }
  }
}

}
}
