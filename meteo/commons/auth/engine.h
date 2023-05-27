#pragma once

#include "gecos.h"
#include <qstringlist.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/proto/authconf.pb.h>
#include <commons/textproto/tprototext.h>


namespace meteo {

namespace auth {

class Engine
{
public :
    Engine();
    virtual ~Engine();

    virtual QString className() const = 0;

    virtual bool init() = 0;

    virtual bool isAdmin(const QString& username) = 0;
    virtual QString homeDir(const QString& username) = 0;

    virtual QStringList userList() = 0;
    virtual QStringList groupList(const QString& username) = 0;

    virtual bool tryAuth(const QString& username, const QString& password) = 0;
    virtual bool registry(const QString& username, const QString& password) = 0;

    virtual bool getGecos(const QString& username, Gecos& gecos) = 0;
    virtual bool setGecos(const QString& username, const Gecos& gecos) = 0;

    virtual QString lastError();
    virtual void setLastError(const QString& msg);
    virtual bool hasError() const;

private :
    QString lastError_;
};

}

}
