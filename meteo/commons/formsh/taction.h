#ifndef TACTION_H
#define TACTION_H

#include <qaction.h>
#include <qmap.h>

#include <meteo/commons/proto/meteomenu.pb.h>

class QWidget;

class TAction : public QAction
{
  Q_OBJECT
public:
  TAction( const QString &text, QObject* parent = 0 );
  meteo::menu::MenuItem config() const { return config_; }
  void setConfig(const meteo::menu::MenuItem& config) { config_ = config; }

//  QString handler() const { return handler_; }
//  QString name() const { return name_; }
//  QString options() const { return options_; }
    QString uuid() const { return uuid_; }
//  bool dockwidget() const { return dockwidget_; }

//  void setHandler( const QString& hndl ) { handler_ = hndl; }
//  void setName( const QString& name ){ name_ = name; }
//  void setOptions( const QString& options ){ options_ = options; }
    void setUuid( const QString& uuid ){ uuid_ = uuid; }
//  void setDockWidget(bool ok){dockwidget_ = ok;}

private:
  meteo::menu::MenuItem config_;


//  QString handler_;
//  QString name_;
//  QString options_;
    QString uuid_;
//  bool dockwidget_;
};

#endif
