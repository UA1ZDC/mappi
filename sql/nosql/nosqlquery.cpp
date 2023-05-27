#include "nosqlquery.h"

#include <sql/rapidjson/document.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

#include <qbytearray.h>
#include <qregexp.h>
#include <qregularexpression.h>
#include <qdebug.h>

#include "document.h"
#include "array.h"

namespace meteo {

NosqlQuery::NosqlQuery()
{
}

NosqlQuery::~NosqlQuery()
{
  if ( true == hasresult_ ) {
    bson_destroy(&document_result_);
  }
}

bool NosqlQuery::exec()
{
  return false;
}

bool NosqlQuery::exec( const QString& query )
{
  Q_UNUSED(query);
  return false;
}

bool NosqlQuery::initIterator()
{
  return false;
}

bool NosqlQuery::next()
{
  return false;
}

const DbiEntry& NosqlQuery::result() const
{
  return docentry_;
}

int32_t NosqlQuery::querySize() const
{
  QByteArray arr = query_.toUtf8();

  bson_error_t berror;
  bson_t* bquery = bson_new_from_json( (uint8_t*)arr.data(), arr.size(), &berror);
  if ( nullptr == bquery ) {
    QString error = QObject::tr("Объект bson_t* не создан. Ошибка = %1. Код ошибки = %2. Запрос =\n%3")
      .arg(berror.message)
      .arg(berror.code)
      .arg(query_);
    error_log << error;
    return -1;
  }
  int32_t sz = bquery->len;
  debug_log << "bytearr size =" << arr.size() << "bson_size =" << sz;
  bson_destroy(bquery);
  return sz;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, bool param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1").arg(param? "true" : "false") );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, int16_t param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString::number(param) );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, int32_t param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString::number(param) );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, int64_t param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString::number(param) );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, uint64_t param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString::number(param) );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, float param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString str = QString("%1").arg(param);
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, double param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString str = QString("%1").arg(param);
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const char* param )
{
  QString fn = fullname(paramname);
  return arg( fn, QString::fromUtf8(param) );
}

QString NosqlQuery::escapeString(const QString& param)
{
  return  QString(param).replace('\\', "\\\\")
                        .replace('"', "\\\"")
                        .replace('\n', "\\n");

}

DbiQuery& NosqlQuery::argFieldName(const QString& paramname, const QString& fieldname)
{
  return arg(paramname, fieldname);
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QString& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  auto escapedParam = escapeString(param);
  query_ = query_.replace( fn, QString("\"%1\"").arg(escapedParam) );
  return *this;
}

DbiQuery& NosqlQuery::argWithoutQuotes( const QString& paramname, const QString& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  auto escapedParam = escapeString(param);
  query_ = query_.replace( fn, QString("%1").arg(escapedParam) );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const std::string& param )
{
  return NosqlQuery::arg( paramname, QString::fromStdString( param ) );
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QDateTime& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QDateTime dt(param);
  dt.setTimeSpec(Qt::UTC);
  QString str = QString("{ \"$date\": \"%1\"}").arg( dt.toString( Qt::ISODate) );
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::argDt( const QString& paramname, const QString& param )
{
  QDateTime dt = datetimeFromString(param);
  if ( false == dt.isValid() ) {
    warning_log << QObject::tr("Неверный формат даты = '%1'")
      .arg(param);
    return *this;
  }
  return arg( paramname, dt );
}

DbiQuery& NosqlQuery::argDt( const QString& paramname, const std::string& param )
{
  QDateTime dt = datetimeFromString(param);
  if ( false == dt.isValid() ) {
    warning_log << QObject::tr("Неверный формат даты = '%1'")
      .arg( QString::fromStdString(param) );
    return *this;
  }
  return arg( paramname, dt );
}

DbiQuery& NosqlQuery::argDt( const QString& paramname, const QStringList& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QStringList list;
  for ( auto s : param ) {
    QDateTime dt = datetimeFromString(s);
    if ( false == dt.isValid() ) {
      warning_log << QObject::tr("Неверный формат даты = '%1'")
        .arg(s);
      continue;
    }
    dt.setTimeSpec(Qt::UTC);
    QString str = QString("{ \"$date\": \"%1\"}").arg( dt.toString( Qt::ISODate ) );
    list.append(str);
  }
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Пустой список с датами");
    return *this;
  }
  uninstalledargs_.erase( uninstalledargs_.begin() );
  QString result = QString("[%1]").arg(list.join(","));
  query_ = query_.replace( fn, result );
  return *this;
}

DbiQuery& NosqlQuery::argDt( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QStringList list;
  for ( auto s : param ) {
    QDateTime dt = datetimeFromString(s);
    if ( false == dt.isValid() ) {
      warning_log << QObject::tr("Неверный формат даты = '%1'")
        .arg( QString::fromStdString(s) );
      continue;
    }
    dt.setTimeSpec(Qt::UTC);
    QString str = QString("{ \"$date\": \"%1\"}").arg( dt.toString( Qt::ISODate ) );
    list.append(str);
  }
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Пустой список с датами");
    return *this;
  }
  uninstalledargs_.erase( uninstalledargs_.begin() );
  QString result = QString("[%1]").arg(list.join(","));
  query_ = query_.replace( fn, result );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QDate& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString str = QString("{ \"$date\": \"%1T00:00:00Z\"}").arg( param.toString( Qt::ISODate ) );
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, int16_t param[], int size )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, int32_t param[], int size )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, double param[], int size )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(param[i]);
      }
      else {
        paramstr += QString("%1").arg(param[i]);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QStringList& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = param[i];
      if ( false == val.isEmpty() ) {
        val = QString("\"%1\"").arg(val);
      }
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::fromStdString( param.Get(i) );
      if ( false == val.isEmpty() ) {
        val = QString("\"%1\"").arg(val);
      }
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const google::protobuf::RepeatedField<int32_t>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const google::protobuf::RepeatedField<long unsigned int>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const google::protobuf::RepeatedField<long int>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname,
    const google::protobuf::RepeatedField<long long int>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}


DbiQuery& NosqlQuery::arg( const QString& paramname, const QList<int32_t>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QList<long unsigned int>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QList<long long int>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const QList<double>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param[i] );
      val = QString("%1").arg(val);
//      val = QString("%1").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const google::protobuf::RepeatedField<double>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::number( param.Get(i) );
      val = QString("%1").arg(val);
//      val = QString("{\"$numberDouble\": \"%1\"}").arg(val);
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::argOid( const QString& paramname, const QString& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("{ \"$oid\": \"%1\"}" ).arg(param) );
  return *this;
}

DbiQuery& NosqlQuery::argOid( const QString& paramname, const std::string& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("{ \"$oid\": \"%1\"}" ).arg( QString::fromStdString( param ) ) );
  return *this;
}

DbiQuery& NosqlQuery::argOid( const QString& paramname, const QStringList& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = param[i];
      if ( false == val.isEmpty() ) {
        val = QString("{ \"$oid\":\"%1\"}").arg(val);
      }
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::argOid( const QString& paramname, const google::protobuf::RepeatedPtrField<std::string>& param )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  int size = param.size();
  QString paramstr;
  if ( 1 > size ) {
    paramstr = "[]";
  }
  else {
    paramstr = QString("[");
    for ( int  i = 0; i < size; ++i ) {
      QString val = QString::fromStdString( param.Get(i) );
      if ( false == val.isEmpty() ) {
        val = QString("{ \"$oid\":\"%1\"}").arg(val);
      }
      if ( i + 1 != size ) {
        paramstr += QString("%1,").arg(val);
      }
      else {
        paramstr += QString("%1").arg(val);
      }
    }
    paramstr += QString("]");
  }
  query_ = query_.replace( fn, paramstr );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const meteo::GeoPoint& point )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString str = geopoint2json(point);
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::arg( const QString& paramname, const meteo::GeoVector &vector)
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  QString str = geovector2json(vector);
  query_ = query_.replace( fn, str );
  return *this;
}

DbiQuery& NosqlQuery::argJson( const QString& paramname, const QString& json )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("%1").arg(json) );
  return *this;
}

DbiQuery& NosqlQuery::argJson( const QString& paramname, const QStringList& jsons )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("[ %1 ]").arg(jsons.join(" , ")) );
  return *this;
}

DbiQuery& NosqlQuery::argJsFunc( const QString& paramname, const QString& json )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("{ \"$code\": \"%1\"}").arg( json.simplified() ) );
  return *this;
}

DbiQuery& NosqlQuery::argMd5( const QString& paramname, const QByteArray& md5 )
{
  QByteArray arr = md5.toBase64();
  return argMd5Base64( paramname, QString( arr.data() ) );
}

DbiQuery& NosqlQuery::argMd5Base64( const QString& paramname, const QString& md5 )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  query_ = query_.replace( fn, QString("{ \"$binary\": \"%1\", \"$type\": \"5\" }").arg( md5 ) );
  return *this;
}

DbiQuery& NosqlQuery::argGeoCircle(const QString& , const meteo::GeoPoint&, int)
{
  warning_log << QObject::tr("Функция не определена");
  return *this;
}

DbiQuery& NosqlQuery::argIpAddress( const QString&, const QString&)
{
  warning_log << QObject::tr("Функция не определена");
  return *this;
}

DbiQuery& NosqlQuery::argGeoWithin( const QString& paramname, const meteo::GeoVector& gv )
{
  QString fn = fullname(paramname);
  if ( false == uninstalledargs_.contains(fn) ) {
    warning_log << QObject::tr("Нет места в строке запроса для установки параметра");
  }
  uninstalledargs_.remove(fn);
  if ( 3 > gv.size() ) {
    error_log << QObject::tr("Попытка подставить в качестве аргумента замкнутую область, но точек в области не может быть меньше трех.");
    return *this;
  }
  meteo::GeoVector locgv = gv;
  if ( locgv.first() != locgv.last() ) {
    locgv.append( locgv.first() );
  }
  QStringList pnts;
  for ( auto gp : locgv ) {
    pnts.append( QString("[%1, %2]").arg( gp.lonDeg(), 0, 'f', 2 ).arg( gp.latDeg(), 0, 'f', 2 ) );
  }
//  QString q = QString("{\"$geoWithin\": { \"$polygon\":  [ %1 ]  } }").arg( pnts.join(",") );
  QString q = QString("{\"$geoWithin\": { \"$geometry\": { \"type\": \"Polygon\", \"coordinates\": [[ %1 ]] } } }").arg( pnts.join(",") );

  query_ = query_.replace( fn, q );
  return *this;
}

DbiQuery& NosqlQuery::removeArg( const QString& name, bool* fl )
{
  if ( nullptr != fl ) {
    *fl = false;
  }
  QByteArray arr = query_.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    error_log << QObject::tr("Запрос не является json-документом. Ошибка = %1")
      .arg( doc.GetParseError() );
    return *this;
  }

  QStringList list = name.split(".");
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    list[i] = list[i].replace("@",".");
  }

  rapidjson::Value& val = jsonValue( &doc, name );
  auto it = val.FindMember( list.last().toUtf8().data() );
  if ( it != val.MemberEnd() ) {
    val.EraseMember(it);
  }
//  val.RemoveMember( list.last().toUtf8().data() );

  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  doc.Accept(writer);
  query_ = sb.GetString();
  return *this;
}

DbiQuery& NosqlQuery::removeArg( const QString& name, int num, const QString& arrname, bool* fl )
{
  if ( nullptr != fl ) {
    *fl = false;
  }
  QByteArray arr = query_.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    error_log << QObject::tr("Запрос не является json-документом. Ошибка = %1")
      .arg( doc.GetParseError() );
    return *this;
  }

  QStringList list = name.split(".");
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    list[i] = list[i].replace("@",".");
  }

  QStringList listarr = arrname.split(".");
  for ( int i = 0, sz = listarr.size(); i < sz; ++i ) {
    listarr[i] = listarr[i].replace("@",".");
  }

  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  rapidjson::Value& val = jsonValue( &doc, name )[list.last().toUtf8().data()][num];

  rapidjson::Value& val2 = jsonValue( &val, arrname );

  auto it = val2.FindMember( listarr.last().toUtf8().data() );
  if ( it != val2.MemberEnd() ) {
    val2.EraseMember(it);
  }
//  val2.RemoveMember(listarr.last().toUtf8().data() );
  if ( true == val.IsObject() && true == val2.ObjectEmpty() ) {
    rapidjson::Value& jarr = jsonValue( &doc, name )[list.last().toUtf8().data()];
    int i = 0;
    for ( auto it = jarr.Begin(); it != jarr.End(); ++it ) {
      if ( num > i++ ) {
        continue;
      }
      jarr.Erase(it);
      break;
    }
  }

  doc.Accept(writer);
  query_ = sb.GetString();
  return *this;
}

bool NosqlQuery::isObjectId(const std::string& oid)
{
  return bson_oid_is_valid(oid.c_str(), oid.size());
}

bool NosqlQuery::isObjectId(const QString& oid)
{
  if ( true == oid.isEmpty() ){
    return false;
  }
  return isObjectId(oid.toStdString());
}

QString  NosqlQuery::generateOid()
{
  bson_oid_t oid;
  bson_oid_init (&oid, NULL);
  char data[24];
  bson_oid_to_string(&oid, data);
  QByteArray oidArray = QByteArray::fromRawData(data, 24);
  return QString::fromUtf8(oidArray);
}

void NosqlQuery::removeDogs()
{
  query_.replace( kInvalidDogs, "\"\\1\"" );
  QByteArray arr = query_.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    error_log << QObject::tr("Запрос не является json-документом. Ошибка = %1")
      .arg( doc.GetParseError() );
    return;
  }
  removeDogsRecursive(&doc);
  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  doc.Accept(writer);
  query_ = sb.GetString();
}

void NosqlQuery::removeDogsRecursive( rapidjson::Value* js )
{
  rapidjson::Value& json = *js;
  if ( json.GetType() == rapidjson::kObjectType ) {
    for ( auto it = json.MemberBegin(); it != json.MemberEnd(); ) {
      if ( rapidjson::kStringType == it->value.GetType() ) {
        QString val = it->value.GetString();
        if ( -1 != kDogs.indexIn(val) ) {
          it = json.EraseMember(it);
          continue;
        }
      }
      removeDogsRecursive( &(it->value) );
      if ( true == it->value.IsObject() && true == it->value.ObjectEmpty() ) {
        json.EraseMember(it);
        continue;
      }
      if ( true == it->value.IsArray() && true == it->value.Empty() ) {
        json.EraseMember(it);
        continue;
      }
      ++it;
    }
  }
  else if ( json.GetType() == rapidjson::kArrayType ) {
    for ( auto it = json.Begin(); it != json.End(); ) {
      if ( rapidjson::kStringType == it->GetType() ) {
        QString val = it->GetString();
        if ( -1 != kDogs.indexIn(val) ) {
          it = json.Erase(it);
          continue;
        }
      }
      removeDogsRecursive( &(*it) );
      if ( true == it->IsObject() && true == it->ObjectEmpty() ) {
        json.Erase(it);
        continue;
      }
      if ( true == it->IsArray() && true == it->Empty() ) {
        json.Erase(it);
        continue;
      }
      ++it;
    }
  }
}

}
