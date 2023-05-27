#include "paths.h"

#include <qdir.h>

namespace MnCommon {

static QString applicationTextName_;
static QString applicationDescr_;

QString projectPath()
{
  return T_PROJECT_DIR;
}

void setApplicationTextName( const QString& name )
{
  applicationTextName_ = name;
}

const QString& applicationTextName()
{
  return applicationTextName_;
}

const QString etcPath()
{
  return etcPath( applicationName() );
}

const QString etcPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/etc/";
  }
  return MnCommon::projectPath() + "/etc/" + name + "/";
}

const QString sharePath()
{
  return sharePath( applicationName() );
}

const QString sharePath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/share/";
  }
  return MnCommon::projectPath() + "/share/" + name + "/";
}

const QString varPath()
{
  return varPath( applicationName() );
}

const QString varPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/var/";
  }
  return MnCommon::projectPath() + "/var/" + name + "/";
}

const QString logPath()
{
  return logPath( MnCommon::applicationName() );
}

const QString logPath( const QString& name )
{
  return MnCommon::projectPath() + QString("/var/log/") + name;
}

const QString iconsPath()
{
  return sharePath() + "icons/";
}

const QString iconsPath( const QString& name )
{
  if ( name.isNull() ) {
    return sharePath() + "icons/";
  }
  return sharePath(name) + "icons/";
}

const QString iconPath( const QString& name )
{
  return iconsPath() + name;
}

const QString iconPath( const QString& name, const QString& appname )
{
  return iconsPath(appname) + name;
}


QString runPath( const QString&  appname )
{
  return MnCommon::projectPath() + "/var/" + appname + "/run/";
}

QString runPath()
{
  return MnCommon::projectPath() + "/var/" + applicationName() + "/run/";
}

QString sockPath( const QString& appname, const QString& name )
{
  Q_UNUSED( appname );

  return userSettingsPath() + "/run/" + name + ".sock";
}

QString sockPath( const QString& name )
{
  return userSettingsPath() + "/run/" + name + ".sock";
}

const char *applicationName(const char* name)
{
  static const char* appName = "";

  if( 0 != name){
    appName = name;
  }

  Q_ASSERT( !QString(appName).isEmpty() );
  return appName;
}

QString applicationDescr(const QString& name)
{
  if( true != name.isEmpty()){
    applicationDescr_ = name;
  }
  return applicationDescr_;
}

const QString binPath()
{
  return binPath(QString());
}

const QString binPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/bin/";
  }
  return MnCommon::projectPath() + "/bin/";
}

const QString sbinPath()
{
  return sbinPath( applicationName() );
}

const QString sbinPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/sbin/";
  }
  return MnCommon::projectPath() + "/sbin/";
}

const QString libPath()
{
  return libPath( applicationName() );
}

const QString libPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/lib/";
  }
  return MnCommon::projectPath() + "/lib/";
}

const QString pluginPath()
{
  return pluginPath( applicationName() );
}

const QString pluginPath( const QString& name )
{
  if ( name.isNull() ) {
    return MnCommon::projectPath() + "/lib/plugins/";
  }
  return MnCommon::projectPath() + "/lib/plugins/" + name;
}

const QString sysHomePath()
{
  return QString("/home/vgmdaemon/");
}

const QString obanalPath()
{
  return MnCommon::sysHomePath() + "/obanal/";
}

const QString userSettingsPath()
{
  return QDir::homePath() + "/.meteo/";
}

const QString stendPath()
{
  return MnCommon::sysHomePath() + "/stend/";
}

const QString pythonPath()
{
  return MnCommon::projectPath() + "/include.py/";
}

QString randomPath(size_t length, QString prefix)
{
  srand(time(NULL));
  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n(str.begin(), length, randchar);
  return prefix + QString::fromStdString(str) + "/";
}


} // MnCommon
