#include "dbiquery.h"

#include "dbi.h"
#include "dbientry.h"
#include "dbiarray.h"

#include <sql/rapidjson/document.h>
#include <sql/rapidjson/writer.h>
#include <sql/rapidjson/stringbuffer.h>
#include <sql/nosql/document.h>
#include <sql/nosql/array.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {

static const QChar  kBracketSymbol('@');

DbiQuery::DbiQuery( )
{
}


DbiQuery::DbiQuery( Dbi* db )
  :db_(db)
{
}

DbiQuery::DbiQuery( const QString& q, Dbi* d )
  : db_(d)
{
  setQuery(q);
}

DbiQuery::~DbiQuery()
{
  db_ = nullptr;
}

DbiQuery& DbiQuery::setQuery( const QString& q )
{
  query_ = q;
  parseUninstalledArgsInQueryString();
  return *this;
}

bool DbiQuery::exec( const QString& q )
{
  setQuery(q);
  return exec();
}

bool DbiQuery::exec()
{
  return db_->execQuery(this);
}

bool DbiQuery::execInit( QString* error )
{
  if ( false == exec() ) {
    if (nullptr != error ) {
       *error = QObject::tr("Не удается выполнить запрос в базу данных: \n '%1'").arg(query_);
    }
    return false;
  }
  if ( false == initIterator() ) {
    if ( nullptr != error ) {
      *error = QObject::tr("Нет курсора в результате выполнения запроса %1")
              .arg( query() );
    }
    return false;
  }
  return true;
}

QString DbiQuery::fullname( const QString& paramname ) const
{
  QString locname = paramname;
  if ( 0 == paramname.length() ) {
    error_log << QObject::tr("Пустое наименование аргумента!");
    return QString("@!WRONGEMPTY!@");
  }
  if ( kBracketSymbol != locname.front() ) {
    locname.prepend(kBracketSymbol);
  }
  if ( kBracketSymbol != locname.back() ) {
    locname.append(kBracketSymbol);
  }
  return locname;
}

QDateTime DbiQuery::datetimeFromString( const std::string& str )
{
  return datetimeFromString( QString::fromStdString(str) );
}

QDateTime DbiQuery::datetimeFromString( const QString& str )
{
  QDateTime dt;
  if ( true == str.isEmpty() ) {
    return dt;
  }
  QString locstr(str);
  locstr.replace('Z', "" );
  dt = QDateTime::fromString( locstr, Qt::ISODate );
  if ( true == dt.isValid() ) {
    return dt;
  }
  dt = QDateTime::fromString( locstr, "yyyy-MM-dd hh:mm:ss" );
  return dt;
}

QDate DbiQuery::dateFromString( const std::string& str )
{
  return dateFromString( QString::fromStdString(str) );
}

QDate DbiQuery::dateFromString( const QString& str )
{
  QDate dt;
  if ( true == str.isEmpty() ) {
    return dt;
  }
  QString locstr(str);
  locstr.replace('Z', "" );
  dt = QDate::fromString( locstr, Qt::ISODate );
  if ( true == dt.isValid() ) {
    return dt;
  }
  dt = QDate::fromString( locstr, "yyyy-MM-dd" );
  return dt;
}

QString DbiQuery::geopoint2json( const meteo::GeoPoint& gp )
{
  QString json = QString("{\"type\": \"Point\", \"coordinates\": [ %1, %2 ], \"alt\": %3}")
    .arg( QString("{\"$numberDouble\":\"%1\"}").arg( gp.lonDeg() ) )
    .arg( QString("{\"$numberDouble\":\"%1\"}").arg( gp.latDeg() ) )
    .arg( QString("{\"$numberDouble\":\"%1\"}").arg( gp.alt() ) );
  return json;
}

meteo::GeoPoint DbiQuery::json2geopoint( const QString& json, bool* ok )
{
  bool lockok = false;
  if ( nullptr != ok ) {
    *ok = lockok;
  }
  bson_error_t berror;
  QByteArray buf = json.toUtf8();
  bson_t* bson = bson_new_from_json( (uint8_t*)buf.data(), buf.size(), &berror);
  if ( nullptr == bson ) {
    error_log << QObject::tr("Объект bson_t* не создан. Ошибка = %1. Код ошибки = %2. Запрос =\n%3")
      .arg(berror.message)
      .arg(berror.code)
      .arg(json);
    return meteo::GeoPoint();
  }
  Document doc(bson);
  meteo::GeoPoint gp;
  Array arr = doc.valueArray( "coordinates", &lockok );
  if ( false == lockok ) {
    return gp;
  }
  int i = 0;
  while ( true == arr.next() ) {
    if ( 0 == i ) {
      gp.setLonDeg( arr.valueDouble() );
    }
    else if ( 1 == i ) {
      gp.setLatDeg( arr.valueDouble() );
    }
    else {
      warning_log << QObject::tr("В объекте типа 'Point' размер массива > 2 = %3")
        .arg(i);
      break;
    }
    ++i;
  }
  double alt = doc.valueDouble("alt", &lockok );
  if ( true == lockok ) {
    gp.setAlt(alt);
  }
  else {
    warning_log << QObject::tr("В объекте типа Point не найдено поле 'alt'");
  }
  if ( nullptr != ok ) {
    *ok = lockok;
  }
  return gp;
}

QString DbiQuery::geovector2json(const meteo::GeoVector &gv)
{
  QString json = QString("{ \"type\" : \"Polygon\", \"coordinates\" : [[ %1 ]], \"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"EPSG:4326\"}} }");
  QStringList allCoords;
  for ( int i = 0, sz = gv.size(); i < sz; ++i ) {
    QString coords = "[%1,%2]";
    coords = coords
        .arg(gv.at(i).lonDeg(), 0, 'f', 4)
        .arg(gv.at(i).latDeg(), 0, 'f', 4);
    allCoords << coords;
  }
  json = json.arg(allCoords.join(","));
  return json;
}

QList<meteo::GeoVector> DbiQuery::json2geovector(const QString &json, bool *ok)
{
  Q_UNUSED(json);
  Q_UNUSED(ok);
  not_impl;
  return QList<meteo::GeoVector>();
}

bool DbiQuery::find_doc_field( const QString& field, bson_iter_t* iter, const bson_t* doc )
{
  bool res = false;
  if ( true == field.contains(".") ) {
    bson_iter_t iter1;
    bson_iter_init( &iter1, doc );
    res = bson_iter_find_descendant( &iter1, field.toUtf8().data(), iter );
  }
  else {
    bson_iter_init( iter, doc );
    res = bson_iter_find( iter, field.toUtf8().data() );
  }
  return res;
}

rapidjson::Value& DbiQuery::jsonValue( rapidjson::Value* doc, const QString& fieldname )
{
  if ( nullptr == doc ) {
    error_log << QObject::tr("Параметр doc = 0");
    return *doc;
  }
  QStringList list = fieldname.split(".");
  if ( 2 > list.size() ) {
    return (*doc);
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    list[i] = list[i].replace("@", "." );
  }
  if ( false == doc->HasMember( list[0].toUtf8().data() ) ) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    doc->Accept(writer);
    error_log << QObject::tr("В Json-документе %1 не найдено поле %2")
      .arg( sb.GetString() )
      .arg( list[0] );
    return *doc;
  }
  rapidjson::Value* val = &((*doc)[list[0].toUtf8().data()]);
  for ( int i = 1, sz = list.size() - 1; i < sz; ++i ) {
    if ( false == val->HasMember( list[i].toUtf8().data() ) ) {
      rapidjson::StringBuffer sb;
      rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
      val->Accept(writer);
      error_log << QObject::tr("В Json-документе %1 не найдено поле %2")
        .arg( sb.GetString() )
        .arg( list[i] );
      return *val;
    }
    val = &((*val)[ list[i].toUtf8().data() ]);
  }
  return *val;
}

void DbiQuery::parseUninstalledArgsInQueryString()
{
  uninstalledargs_.clear();
  int indx = 0;
  QRegularExpressionMatch m = rx_.match( query_, indx );
  while ( true == m.hasMatch() ) {
    QString capstr = m.captured(0);
    indx = m.capturedEnd() + 1;
    uninstalledargs_.insert( capstr, m.capturedStart() );
    m = rx_.match( query_, indx );
  }
}

}
