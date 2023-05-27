#include "tformadaptor.h"

#include <qwidget.h>

#include <cross-commons/debug/tlog.h>

#include "taction.h"
#include "thandler.h"

template<> uiformsh::TFormAdaptor* TFormAdaptor::_instance = 0;

namespace uiformsh {

TFormAdaptor::TFormAdaptor()
  : QObject(),
  mainwindow_(0)
{
}

TFormAdaptor::~TFormAdaptor()
{
}


bool TFormAdaptor::registerHandler( THandler* hndl )
{
  QString name = hndl->name();
  if ( true == hndlmap_.contains(name) ) {
    //error_log << QObject::tr("Ошибка регистрации: handler \'%1\' уже существует").arg(name);
    return false;
  }
  hndlmap_.insert( name, hndl );
  if ( 0 != mainwindow_ ) {
    hndl->setMainwindow(mainwindow_);
  }
  return true;
}

bool TFormAdaptor::registerAction( TAction* action )
{
  QString name = QString::fromStdString(action->config().id());
//  if ( true == actmap_.contains(name) ) {
//    error_log << QObject::tr("Ошибка регистрации: action \'%1\' уже существует").arg(name);
//    return false;
//  }
  connect( action, SIGNAL( triggered() ), this, SLOT( slotActionActivated() ) );
  connect( action, SIGNAL( destroyed() ), this, SLOT( slotRemoveAction() ) );
  actmap_.insert( name, action );
  return true;
}

THandler*TFormAdaptor::handler(const QString& name)
{
  return hndlmap_[name];
}

void TFormAdaptor::slotActionActivated()
{
  TAction* act = qobject_cast<TAction*>( sender() );
  if( 0 == act ) {
    return;
  }

  if( hndlmap_.contains( QString::fromStdString(act->config().handler()) ) == false ) {
    error_log << QString::fromUtf8("Ошибка выполнения: handler \'%1\' не зарегистрирован")
                 .arg( QString::fromStdString(act->config().handler()) );
    return;
  }

//  QString name = act->name();
//  QString options = act->options();
//  QString uuid = act->uuid();
//  bool dock = act->dockwidget();
//  if (false != name.isEmpty()) {
//    //error_log<< QString::fromUtf8("Ошибка выполнения: action \'%1\' не содержит расширения").arg( act->text() );
//    return;
//  }
  QString handler = QString::fromStdString(act->config().handler());
  hndlmap_[handler]->handleEntity(act);
}

void TFormAdaptor::setMainwindow( QWidget* mainwindow )
{
  mainwindow_ = mainwindow;
  QMap< QString, THandler* >::iterator it = hndlmap_.begin();
  QMap< QString, THandler* >::iterator end = hndlmap_.end();
  for ( ; it != end; ++it ) {
    (*it)->setMainwindow(mainwindow_);
  }
}

void TFormAdaptor::slotRemoveAction()
{
  actmap_.remove( sender()->objectName() );
}

void TFormAdaptor::init()
{
  QMap< QString, THandler* >::iterator it = hndlmap_.begin();
  QMap< QString, THandler* >::iterator end = hndlmap_.end();
  for ( ; it != end; ++it ) {
    (*it)->init();
  }
}

}
