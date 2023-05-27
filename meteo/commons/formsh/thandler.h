#ifndef THANDLER_H
#define THANDLER_H

#include <qstring.h>
#include <qwidget.h>
#include "taction.h"
#include <meteo/commons/proto/meteomenu.pb.h>

class TDescription;

class THandler : public QObject
{
  Q_OBJECT
  public:
    THandler( const QString& n ) : name_(n) {}
    virtual ~THandler() {}
    const QString name() const { return name_; }
    virtual QWidget* handleEntity( TAction* act = 0 ) = 0;
    virtual QWidget* handleEntity( const QString& name = 0 ) = 0;
    virtual QWidget* widget( const QString& name ) = 0;
    virtual QMap<QString, QWidget*> widgets() = 0;
    virtual void setMainwindow( QWidget* ) {}
    virtual QWidget* mainwindow() { return 0; }
    virtual void init() {}
  private:
    QString name_;
  signals:
    void open(const QString& name, const QString& uuid);
};

#endif
