#ifndef TPGSETTINGS_H
#define TPGSETTINGS_H

#include <qstring.h>

#include <cross-commons/app/paths.h>
#include <sql/proto/dbsettings.pb.h>

namespace meteo {

class ConnectProp{
  public:
    ConnectProp() {
    }
    ConnectProp( const QString& ahost,const QString& aname,
        const QString& alog, const QString& apass, uint16_t aport = 5432, const QString& auserDb = QString()) {
      setLogin(alog);
      setPass(apass);
      setName(aname);
      setHost(ahost);
      setPort(aport);
      setUserDb(auserDb);
    }
    const ConnectProp& operator=(const ConnectProp& src) {
      setLogin(src.login());
      setPass(src.pass());
      setName(src.name());
      setHost(src.host());
      setPort(src.port());
      setUserDb(src.userDb());
      setDriver( src.driver() );
      return *this;
    }
    bool operator==(const ConnectProp& rhs) const {
      return host_ == rhs.host()  &&
             name_ == rhs.name()  &&
             login_ == rhs.login() &&
             pass_ == rhs.pass() &&
             port_ == rhs.port() &&
             userDb_ == rhs.userDb() &&
             dbdriver_ == rhs.driver();
    }
    bool operator!=(const ConnectProp& rhs) const {
      return !(*this == rhs);
    }
    void setLogin( const QString& src ) {
      login_ = src;
    }
    void setPass( const QString& src ) {
      pass_ = src;
    }
    void setName( const QString& src ) {
      name_ = src;
    }
    void setHost( const QString& src ) {
      host_ = src;
    }
    void setPort( uint16_t src ) {
      port_ = src;
    }
    void setUserDb( const QString& src ) {
      userDb_ = src;
    }
    void setDriver( meteo::settings::DbDriver d ) { dbdriver_ = d; }
    void clear() {
      host_ = QString();
      name_ = QString();
      login_ = QString();
      pass_ = QString();
      port_ = 5432;
      userDb_ = QString();
      dbdriver_ = meteo::settings::kMongo;
    }

    const QString& host() const { return host_; }
    const QString& name() const { return name_; }
    const QString& login() const { return login_; }
    const QString& pass() const { return pass_; }
    uint16_t port() const { return port_; }
    const QString& userDb() const { return userDb_; }
    meteo::settings::DbDriver driver() const { return dbdriver_; }

    QString print() const
    {
      return QString("[ConnectProp\n")
        + "\thost   = " + host() + "\n"
        + "\tport   = " + QString::number( port() ) + "\n"
        + "\tname   = " + name() + "\n"
        + "\tlogin  = " + login() + "\n"
        + "\tuserDb = " + userDb() + "\n"
        +  "\tdbdriver = " + driver() + "\n"
        + "\tpass   = ******";
    }

   /* friend TLog& operator<<(TLog& o, const ConnectProp& p) {
      o << p.print();
      return o;
    }
    */
  private:
    QString host_;
    QString name_;
    QString login_;
    QString pass_;
    uint16_t port_;
    QString userDb_;
    meteo::settings::DbDriver dbdriver_;
};

ConnectProp dbparams( const QString& appname = MnCommon::applicationName(),
    bool* ok = 0, const QString& filename = QString("db.conf") );
}

#endif
