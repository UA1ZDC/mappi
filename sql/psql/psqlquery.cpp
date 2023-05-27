#include "psqlquery.h"

#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

#include <qbytearray.h>
#include <qmutex.h>
#include <qregexp.h>

#include <sql/rapidjson/stringbuffer.h>
#include <sql/rapidjson/writer.h>

#include <sql/nosql/document.h>
#include <sql/nosql/array.h>
#include <sql/nosql/document.h>


namespace meteo {

static QMutex mutexq( QMutex::Recursive );

PsqlQuery::PsqlQuery()
  :DbiQuery(),
  res_(nullptr),
  size_(0)
{
  null_ = "NULL";
}

PsqlQuery::PsqlQuery( Psql* db )
  : DbiQuery(db),
  res_(nullptr),
  size_(0),
  psql_(db)
{
  null_ = "NULL";
}

PsqlQuery::PsqlQuery( const QString& q, Psql* d )
  : DbiQuery( q, d ),
  res_(nullptr),
  size_(0),
  psql_(d)
{
  null_ = "NULL";
}

PsqlQuery::~PsqlQuery()
{
  PQclear(res_);
  res_ = nullptr;
}

QString PsqlQuery::errorMessage() const
{
  if ( nullptr == db_ ) {
    return QString();
  }
  return QString::fromUtf8( PQerrorMessage(static_cast<Psql*>(db_)->connection_) );
}

QString PsqlQuery::escapeString( const QString& str )
{
  if ( nullptr == psql_ || false == psql_->connected() ) {
    QString newstr(str);
    newstr.replace("'","''");
    return QString("'%1'").arg(newstr);
  }
  QByteArray arr = str.toUtf8();
  char* escap = PQescapeLiteral( psql_->connection_, arr.data(), arr.size() );
  QString newstr(escap);
  PQfreemem(escap);
  return newstr;
}

bool PsqlQuery::exec()
{
  if ( nullptr == db_ ) {
    error_log << QObject::tr("Не установлено подключение к БД!");
    docresult_ = PsqlEntry();
    return false;
  }
  if ( true == query_.isEmpty() ) {
    error_log << QObject::tr("Пустой запрос!");
    docresult_ = PsqlEntry();
    return false;
  }

  size_ = 0;
  columns_.clear();
  PQclear(res_);
  res_ = nullptr;
  currentrow_ = -1;

 // debug_log << "UNINST =" << uninstalledargs_;
  for ( auto it = uninstalledargs_.begin(), end = uninstalledargs_.end() ; it != end; ++it ) {
    query_.replace( fullname(it.key()), getNULL() );
  }
 // debug_log << query_;
  bool fl = db_->execQuery(this);
  if ( false == fl ) {
    PQclear(res_);
    res_ = nullptr;
    docresult_ = PsqlEntry();
    return false;
  }
  size_ = PQntuples(res_);
  int fieldsize =PQnfields(res_); 
  for ( int i = 0; i < fieldsize; ++i ) {
    columns_.append( PQfname( res_, i ) );
  }
  if ( 0 != size_ && 0 != columns_.size() ) {
    QHash< QString, QString > vals;
    int clmn = 0;
    for ( auto key : columns_ ) {
      if ( 0 == PQgetisnull( res_, 0, clmn ) ) { //Есть результат!
        vals.insert( key, QString::fromUtf8( PQgetvalue( res_, 0, clmn ) ) );
      }
      ++clmn;
    }
    docresult_ = PsqlEntry(vals);
  }
  return true;
}

bool PsqlQuery::exec( const QString& q )
{
  setQuery(q);
  return exec();
}

bool PsqlQuery::initIterator()
{
  if ( nullptr == res_ ) {
    return false;
  }
  currentrow_ = -1;
  return true;
}

bool PsqlQuery::next()
{
  ++currentrow_;
  if ( currentrow_ >= size_ || 0 > currentrow_ ) {
    return false;
  }
  entry_ = PsqlEntry(this);
  return true;
}

const DbiEntry& PsqlQuery::result() const
{
  return docresult_;
}

DbiQuery& PsqlQuery::argOid(const QString& paramname, const QString& param )
{
  return arg( paramname, (uint64_t)param.toULongLong() );
}

DbiQuery& PsqlQuery::argOid(const QString& paramname, const std::string& param )
{
  return argOid( paramname, QString::fromStdString(param) );
}

DbiQuery& PsqlQuery::argOid(const QString& paramname, const QStringList& list )
{
  QList<uint64_t> ulist;
  for ( auto v : list ) {
    ulist.append( v.toULongLong() );
  }
  return arg( paramname, ulist );
}

DbiQuery& PsqlQuery::argOid(const QString& paramname,
                     const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QList<uint64_t> ulist;
  for ( auto v : param ) {
    ulist.append( QString::fromStdString(v).toULongLong() );
  }
  return arg( paramname, ulist );
}

DbiQuery& PsqlQuery::argJsFunc(const QString& paramname, const QString& func )
{
  error_log << QObject::tr("Не реализовано!");
  Q_UNUSED(paramname);
  Q_UNUSED(func);
  return *this;
}

DbiQuery& PsqlQuery::argMd5( const QString& paramname, const QByteArray& md5 )
{
  QByteArray arr = md5.toBase64();
  return argMd5Base64( paramname, QString( arr.data() ) );
}

DbiQuery& PsqlQuery::argMd5Base64( const QString& paramname, const QString& md5 )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
   if ( md5.isEmpty() ) {
     query_ = query_.replace( fn, getNULL() );
   }
   else
   {
     query_ = query_.replace( fn, "'"+md5+"'" );
    }

  return *this;
}


DbiQuery& PsqlQuery::argDt(const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QStringList paramlst;
  for ( auto s : param ) {
     paramlst.append(QString::fromStdString(s));
    }
  return argDt( paramname,paramlst );
}

DbiQuery& PsqlQuery::argDt(const QString& paramname, const QStringList& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString result;
  QString fn = fullname(paramname);
  int size = param.size();
   if ( 1 > size ) {
     result = getNULL();
   }
   else
   {
      QStringList list;
      for ( auto s : param ) {
        QDateTime dt = datetimeFromString(s);
        if ( false == dt.isValid() ) {
          warning_log << QObject::tr("Неверный формат даты = '%1'")
            .arg(s);
          continue;
        }
        dt.setTimeSpec(Qt::UTC);
        QString str = QString("'%1'").arg(dt.toString(Qt::ISODate));
        str.remove("Z");
        if ("NULL" != str ) {
          str = QString("'%1'").arg(str);
        }
        list.append(str);
      }
      if ( 0 == list.size() ) {
          result = getNULL();
        } else{
          result = QString("ARRAY[%1]::TIMESTAMP WITHOUT TIME ZONE[]").arg(list.join(","));
        }
     }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, result );
  return *this;
}

DbiQuery& PsqlQuery::argDt(const QString& paramname, const std::string& param )
{
  QDateTime dt = datetimeFromString(param);
  return arg(paramname, dt);
}

DbiQuery& PsqlQuery::argDt(const QString& paramname, const QString& param )
{
  QDateTime dt = datetimeFromString(param);
  return arg(paramname, dt);
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const QList<double>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::DOUBLE PRECISION[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedField<double>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::DOUBLE PRECISION[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedField<int32_t>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedField<long unsigned int>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedField<long int>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedField<long long int>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}


DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const QList<int32_t>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const QList<long unsigned int>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const QList<long long int>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname,
                          const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QStringList paramlst;
  for ( auto s : param ) {
     paramlst.append(QString::fromStdString(s));
    }
  return arg( paramname,paramlst );
}

DbiQuery& PsqlQuery::arg(const QString& paramname, const std::string& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString tmp= QString::fromStdString( param );
  tmp = escapeString(tmp);
//  tmp.replace("'","''");
  tmp = QString("%1::TEXT").arg(tmp);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, tmp );
  return *this;
}

DbiQuery& PsqlQuery::argJson( const QString& paramname, const QStringList& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else
  {
    auto lastindx = param.size() - 1;
    int32_t curindx = 0;
    for ( auto doc : param ) {
//      doc.replace("'","''");
      doc = escapeString(doc);
      doc = QString("%1").arg(doc);
      paramstr += doc;
      if ( curindx < lastindx ) {
        paramstr += ",";
      }
      ++curindx;
    }
    paramstr = QString("ARRAY[%1]::jsonb[]").arg(paramstr);
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::argJson( const QString& paramname,const QString& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( param.isEmpty() ) {
    paramstr = getNULL();
  }
  else
  {
    paramstr = QString("'%1'::jsonb").arg( param );
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, bool param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString strparam = ( true == param ) ? "t" : "f";
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("'%1'::BOOLEAN").arg(strparam) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, int16_t param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::SMALLINT").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, int32_t param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::INTEGER").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, int64_t param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::BIGINT").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, uint64_t param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::BIGINT").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, float param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::REAL").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, double param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1::DOUBLE PRECISION").arg(param) );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const char* param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr(param);
  if ( paramstr.isEmpty() ) {
    paramstr = getNULL();
  }
  else
  {
//    paramstr.replace("'","''");
    paramstr = escapeString(paramstr);
    paramstr = QString("%1::TEXT").arg(paramstr);
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const QString& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr=param;
  if ( param.isEmpty() ) {
    paramstr = getNULL();
  }
  else
  {
//    paramstr.replace("'","''");
    paramstr = escapeString(paramstr);
    paramstr = QString("%1::TEXT").arg(paramstr);
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::argIpAddress( const QString& paramname, const QString& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr=param;
  if ( param.isEmpty() ) {
    paramstr = getNULL();
  }
  else
  {
    paramstr = escapeString(paramstr);
    paramstr = QString("%1").arg(paramstr);
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::argWithoutQuotes( const QString& paramname, const QString& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr=param;
  if ( param.isEmpty() ) {
    paramstr = getNULL();
  }
  else
  {
    paramstr.replace("'","''");
//    paramstr = escapeString(paramstr);
    paramstr = QString("%1").arg(paramstr);
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const QDateTime& dt )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( false == dt.isValid() ) {
    paramstr = getNULL();
  }
  else
    {
      paramstr = QString("'%1'::TIMESTAMP WITHOUT TIME ZONE").arg(dt.toString(Qt::ISODate).remove("Z"));
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const QDate& dt )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( false == dt.isValid() ) {
    paramstr = getNULL();
  }
  else
    {
      paramstr = QString("'%1'::DATE").arg(dt.toString(Qt::ISODate));
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const QStringList& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = param[i];
      if ("NULL" != val ) {
        val = QString("'%1'").arg(val);
      }
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]::TEXT[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, int16_t param[], int size )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]::SMALLINT[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, int32_t param[], int size )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( 1 > size ) {
      paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]::INTEGER[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, double param[], int size )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( 1 > size ) {
      paramstr = getNULL();
  }
  else {
    paramstr = QString("ARRAY[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]::DOUBLE PRECISION[]");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const Document& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( true == param.isEmpty() ) {
      paramstr = getNULL();
    } else {
      paramstr = QString("'%1'").arg( param.jsonPostgresql() );
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const Array& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( true == param.isEmpty() ) {
      paramstr = getNULL();
    } else {
      paramstr = QString("'%1'").arg( param.jsonPostgresql() );
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const QList<Document>& param )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( 0 == param.size() ) {
      paramstr = getNULL();
  } else
    {
      auto lastindx = param.size() - 1;
      int32_t curindx = 0;
      for ( auto doc : param ) {
        paramstr += doc.jsonPostgresql();
        if ( curindx < lastindx ) {
          paramstr += ",";
        }
        ++curindx;
      }
      paramstr = QString("'[%1]::jsonb'").arg(paramstr);
    }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg( const QString& paramname, const GeoPoint& gp )
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( false == gp.isValid() ) {
      paramstr = getNULL();
  } else
    {
    //paramstr = QString("ST_Point(%1,%2)").arg( gp.lonDeg() ).arg( gp.latDeg() );
    paramstr = QString("ST_SetSRID(ST_Point(%1,%2), 4326)").arg( gp.lonDeg() ).arg( gp.latDeg() );
    } uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::arg(const QString& paramname, const meteo::GeoVector& param )
{
  QString fn = fullname(paramname);
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString paramstr;
  int size = param.size();
  if ( 1 > size ) {
    paramstr = getNULL();
  }
  else {
    paramstr = geovector2json(param);
  }
  paramstr = QString("ST_GeomFromGeoJSON('%1')").arg(paramstr);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& PsqlQuery::argGeoCircle(const QString& paramname, const meteo::GeoPoint& gp, int radiusmeter ){
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  QString paramstr;
  if ( false == gp.isValid() || radiusmeter<0 ) {
      paramstr = getNULL();
  } else
    {
    paramstr = QString("ST_Buffer(ST_SetSRID(ST_Point(%1,%2)::geography, 4326), %3)::geometry")
               .arg(gp.lonDeg())
               .arg(gp.latDeg())
               .arg(radiusmeter);
    //paramstr = QString("ST_Point(%1,%2)").arg( gp.lonDeg() ).arg( gp.latDeg() );
    //paramstr = QString("ST_SetSRID(ST_Point(%1,%2), 4326)").arg( gp.lonDeg() ).arg( gp.latDeg() );
    } uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, paramstr );
  return *this;
}


DbiQuery& PsqlQuery::argGeoWithin(const QString& paramname, const meteo::GeoVector& gv )
{
  return arg(paramname, gv);
  return *this;
}

DbiQuery& PsqlQuery::argFieldName(const QString& paramname, const QString& fieldName)
{
  if ( 0 == uninstalledargs_.size() ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
    return *this;
  }
  QString fn = fullname(paramname);
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, fieldName );
  return *this;
}

}
