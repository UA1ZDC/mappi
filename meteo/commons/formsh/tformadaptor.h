#ifndef TFORMADAPTOR_H
#define TFORMADAPTOR_H

#include <cross-commons/singleton/tsingleton.h>

#include <qmap.h>
#include <qstring.h>
#include <qobject.h>

class TAction;
class THandler;
class TDescription;
class QWidget;

namespace uiformsh {

class TFormAdaptor : public QObject
{
  Q_OBJECT

public:
  TFormAdaptor();
  ~TFormAdaptor();

  bool registerHandler( THandler* hndl );
  bool registerAction( TAction* action );
  THandler* handler( const QString& name );

  void setMainwindow( QWidget* mainwindow );
  void init();

private slots:
  void slotActionActivated();
  void slotRemoveAction();

private:
  TFormAdaptor& operator=( const TFormAdaptor& ) { return *this; }

private:
  QMultiMap< QString, TAction* > actmap_;
  QMap< QString, THandler* > hndlmap_;
  QWidget* mainwindow_;

};

}

typedef TSingleton<uiformsh::TFormAdaptor> TFormAdaptor;

#endif
