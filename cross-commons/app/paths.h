#ifndef CROSS_COMMONS_APP_PATHS_H
#define CROSS_COMMONS_APP_PATHS_H

#include <qstring.h>

namespace MnCommon {

const QString COAST_CRUDE       = "gshhs_c.b";
const QString COAST_LOW         = "gshhs_l.b";
const QString COAST_MED         = "gshhs_i.b";
const QString COAST_HI          = "gshhs_h.b";
const QString COAST_FULL        = "gshhs_f.b";

const QString BORDER_CRUDE      = "wdb_borders_c.b";
const QString BORDER_LOW        = "wdb_borders_l.b";
const QString BORDER_MED        = "wdb_borders_i.b";
const QString BORDER_HI         = "wdb_borders_h.b";
const QString BORDER_FULL       = "wdb_borders_f.b";

const QString RIVER_CRUDE       = "wdb_rivers_c.b";
const QString RIVER_LOW         = "wdb_rivers_l.b";
const QString RIVER_MED         = "wdb_rivers_i.b";
const QString RIVER_HI          = "wdb_rivers_h.b";
const QString RIVER_FULL        = "wdb_rivers_f.b";

const QString CITY_FULL         = "vcity.geo";
const QString CITY_USER         = "cityuser.csv";

//! Возвращает путь к директории, в которой расположен (установлен) проект.
//! \threadsafe
QString projectPath();

QString runPath( const QString& appname );
QString runPath();

QString sockPath( const QString& appname, const QString& name );
QString sockPath( const QString& name );

void setApplicationTextName( const QString& name );

const QString & applicationTextName();                   //!< Имя приложения (для пользователя, то есть отображаемое в главном окне)

const QString etcPath();
const QString etcPath( const QString& name );

const QString sharePath();
const QString sharePath( const QString& name );

const QString varPath();
const QString varPath( const QString& name );

const QString logPath();
const QString logPath( const QString& name );

const QString iconsPath();
const QString iconsPath( const QString& name );

const QString iconPath( const QString& name );
const QString iconPath( const QString& name, const QString& appname );

const QString libPath();
const QString libPath( const QString& name );

const QString binPath();
const QString binPath( const QString& name );

const QString sbinPath();
const QString sbinPath( const QString& name );

const QString pluginPath();
const QString pluginPath( const QString& name );

const QString pythonPath();

const char* applicationName(const char* name = 0);

const QString sysHomePath();
const QString obanalPath();
const QString stendPath();
const QString userSettingsPath();

QString randomPath(size_t length, QString prefix="");

}

#define TAPPLICATION_NAME( name ) MnCommon::applicationName(name);

#endif


