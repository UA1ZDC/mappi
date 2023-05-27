#ifndef CROSS_COMMONS_APP_OPTIONS_H
#define CROSS_COMMONS_APP_OPTIONS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

#include <cross-commons/includes/tcommoninclude.h>

namespace commons {

class ArgParser;

class Arg
{
  public:
    Arg( const QString& shortname, const QString& longname, bool val = false );
    Arg( const QString& shortname, bool val = false );
    Arg(){}
    ~Arg(){}

    bool isEmpty() const { return (short_.isEmpty() && long_.isEmpty()); }
    bool valid() const { return ( false == isEmpty() ); }
    bool needval() const { return needval_; }
    bool hasValues() const { return ( 0 != values_.size() ); }
    bool installed() const { return installed_; }

    const QString& shortname() const { return short_; }
    const QString& longname() const { return long_; }

    bool operator==( const Arg& arg ) const ;
//    Arg& operator=( const Arg& arg );

    void setValue( const QString& val );
    const QStringList& values() const { return values_; }
    QString value() const { if ( 0 == values_.size() ) { return QString(); }; return values_[0]; }

    void setInstalled() { installed_ = true; }

    QString help() const ;

    static Arg make( const QString& sh, const QString& lo, bool val = false ) { return Arg( sh, lo, val ); }
    static Arg makeShort( const QString& name, bool val = false ) { return Arg( name, QString(), val ); }
    static Arg makeLong( const QString& name, bool val = false ) { return Arg( QString(), name, val ); }

  private:
    QString short_;
    QString long_;
    bool needval_;
    QStringList values_;
    bool installed_;
};

class ArgParser
{
  public:
    static std::string paramhelp( const Arg& arg, const char* descr );

  public:
    bool parse( int argc, char* argv[] );
    const QString& errorString() const { return error_; }

    Arg& at( const Arg& arg );
    bool hasValues( const Arg& opt ) const ;
    bool installed( const Arg& opt ) const ;

    static ArgParser* instance();
    void clear();

    const QList<Arg>& options() const { return options_; }
    QList<Arg> installedOptions() const ;

  private:
    ArgParser();
    ~ArgParser(){}

    void addOption( const Arg& option );

    bool hasOption( const char* arg ) const ;
    Arg& option( const char* arg );

    DISALLOW_EVIL_CONSTRUCTORS(ArgParser);

  private:
    QList<Arg> options_;
    QStringList argv_;
    QString error_;

    Arg empty_;

    static ArgParser* instance_;

  friend class Arg;
};

}
#endif
