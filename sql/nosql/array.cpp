#include "array.h"
#include "document.h"
#include <cross-commons/debug/tlog.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

namespace meteo {

static int countarr = 0;

Array::Array()
  : array_(nullptr),
  refcnt_( new int32_t )
{
  Q_UNUSED(countarr);
//  ++countarr;
  *refcnt_ = 1;
//  debug_log << "CNTARR =" << countarr;
}

Array::Array( const QString& json )
  : refcnt_( new int32_t )
{
//  ++countarr;
  *refcnt_ = 0;
//  debug_log << "CNTARR =" << countarr;
  bson_error_t e;
  QByteArray arr = json.toUtf8();
  array_ = bson_new_from_json( (uint8_t*)arr.data(), arr.length(), &e );//bson_new();;
  if ( nullptr == array_ ) {
    error_log << QObject::tr("Ошибка = %1. (Код ошибки: %3)").arg(e.message).arg(e.code);
    error_log << QObject::tr("JSON с ошибкой =") << json;
//    return;
  }
  *refcnt_ = *refcnt_ + 1;
  initIterator();
}

Array::Array( const rapidjson::Document& doc )
  : Array( Document::rapid2string(doc) )
{
}

Array::Array( const Array& arr )
  : refcnt_( arr.refcnt_ )
{
  *refcnt_ = *refcnt_ + 1;
  array_ = arr.array_;
  initIterator();
}

Array::Array( bson_t* a )
  : array_(a),
  refcnt_( new int32_t )
{
//  ++countarr;
//  debug_log << "CNTARR =" << countarr;
  *refcnt_ = 1;
  initIterator();
}

Array::~Array()
{
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if ( nullptr != array_ ) {
      bson_destroy(array_);
//      delete array_;
    }
//    --countarr;
//  debug_log << "CNTARR =" << countarr;
    delete refcnt_;
  }
  refcnt_ = nullptr;
  array_ = nullptr;
}

Array& Array::operator=( const Array& arr )
{
  if ( this == &arr ) {
    return *this;
  }

  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if ( nullptr != array_ ) {
      bson_destroy(array_);
//      delete array_;
    }
//    --countarr;
//  debug_log << "CNTARR =" << countarr;
    delete refcnt_;
  }
  refcnt_ = arr.refcnt_;
  array_ = arr.array_;
  *refcnt_ += 1;
  return *this;
}

bool Array::initIterator()
{
  if ( nullptr == array_ ) {
    return false;
  }
  if ( false == bson_iter_init( &iterator_, array_ ) ) {
    error_log << QObject::tr("Не удалось инициализировать итератор");
    return false;
  }
  return true;
}

bool Array::next()
{
  if ( nullptr == array_ ) {
    return false;
  }
  return bson_iter_next(&iterator_);
}

bool Array::isEmpty() const
{
  if ( nullptr == array_ ) {
    return true;
  }
  bson_iter_t it;
  if ( false == bson_iter_init( &it, array_ ) ) {
    return true;
  }
  if ( true == bson_iter_next(&it) ) {
    return false;
  }
  return true;
}

int32_t Array::valueInt32( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return 0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return bson_iter_int32(&iterator_);
}

int64_t Array::valueInt64( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return 0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return bson_iter_int64(&iterator_);
}

double Array::valueDouble( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return 0.0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  double val = 0.0;
  if ( true == BSON_ITER_HOLDS_DOUBLE(&iterator_) ) {
    val = bson_iter_double(&iterator_);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iterator_) ) {
    val = bson_iter_int32(&iterator_);
  }
  return val;
}

QString Array::valueString( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return QString();
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return QString( bson_iter_utf8(&iterator_, NULL) );
}

meteo::GeoPoint Array::valueGeo( bool* res )
{
  meteo::GeoPoint point;
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return point;
  }
  uint32_t length;
  const uint8_t* arr;
  bson_iter_document( &iterator_, &length, &arr );
  bson_t* doc = bson_new_from_data( arr, length );
  bson_iter_t iter;
  bson_iter_init( &iter, doc );
  if ( false == Document::find_doc_field( "coordinates.0", &iter, doc ) ) {
    bson_destroy(doc);
    error_log << QObject::tr("Не удалось получить координаты");
    return point;
  }
  double lon = 0.0;
  if ( true == BSON_ITER_HOLDS_DOUBLE(&iter) ) {
    lon = bson_iter_double(&iter);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iter) ) {
    lon = bson_iter_int32(&iter);
  }
  if ( false == Document::find_doc_field( "coordinates.1", &iter, doc ) ) {
    bson_destroy(doc);
    error_log << QObject::tr("Не удалось получить координаты");
    return point;
  }
  double lat = 0.0;
  if ( true == BSON_ITER_HOLDS_DOUBLE(&iter) ) {
    lat = bson_iter_double(&iter);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iter) ) {
    lat = bson_iter_int32(&iter);
  }
  if ( false == Document::find_doc_field( "alt", &iter, doc ) ) {
    bson_destroy(doc);
    error_log << QObject::tr("Не удалось получить координаты");
    return point;
  }
  double alt = 0.0;
  if ( true == BSON_ITER_HOLDS_DOUBLE(&iter) ) {
    alt = bson_iter_double(&iter);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iter) ) {
    alt = bson_iter_int32(&iter);
  }
  bson_destroy(doc);
  if ( nullptr != res ) {
    *res = true;
  }
  point = meteo::GeoPoint::fromDegree( lat, lon, alt );
  
  return point;
}

QDateTime Array::valueDt( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return QDateTime();
  }
  int64_t dt = bson_iter_date_time(&iterator_);
  QDateTime qdt = QDateTime::fromMSecsSinceEpoch((unsigned long int)dt, Qt::OffsetFromUTC );
  if ( false == qdt.isValid() ) {
    return QDateTime();
  }

  if ( nullptr != res ) {
    *res = true;
  }
  qdt.setTimeSpec(Qt::LocalTime);
  return qdt;
}

QString Array::valueOid( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }

  const bson_oid_t* oid = bson_iter_oid(&iterator_);
  if ( nullptr == oid && true == BSON_ITER_HOLDS_UTF8(&iterator_) ) { //на случай, если oid сохранили, как строку
    return QString( bson_iter_utf8(&iterator_, NULL) );
  }
  if ( nullptr == oid && true == BSON_ITER_HOLDS_INT32(&iterator_) ) { //на случай, если oid сохранили, как строку
    return QString::number( bson_iter_int32(&iterator_) );
  }
  if ( nullptr == oid ) {
    error_log << QObject::tr("Не удалось получить объект типа Oid из поля");
    return QString();
  }
  if ( nullptr != res ) {
    *res = true;
  }
  char arr[25];
  bson_oid_to_string( oid, arr );
  return QString::fromUtf8(arr);
}

void Array::setArray( bson_t* array )
{
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
//    --countarr;
//  debug_log << "CNTARR =" << countarr;
    if (  nullptr != array_ ) {
      bson_destroy(array_);
//      delete array_;
      //  debug_log << "CNTBSON =" << --countbson;
    }
    delete refcnt_;
  }
  refcnt_ = new int32_t;
//  ++countarr;
//  debug_log << "CNTARR =" << countarr;
  *refcnt_ = 1;

  array_ = array;
  initIterator();
//  alien_ = doc;
}

bool Array::valueArray( Array* arr )
{
  if ( nullptr == array_ ) {
    return false;
  }
  uint32_t length;
  const uint8_t* array;
  bson_iter_array( &iterator_, &length, &array );
  if(0==length || nullptr == array ){
    return false;
  }
  bson_t* chjar = bson_new_from_data( array, length );
  arr->setArray( chjar );
  return true;
}


bool Array::valueDocument( Document* doc )
{
  if ( nullptr == array_ ) {
    return false;
  }
  uint32_t length;
  const uint8_t* array;
  bson_iter_document( &iterator_, &length, &array );
  bson_t* chdoc = bson_new_from_data( array, length );
  doc->setDocument( chdoc );
  return true;
}

bool Array::valueBool( bool* res )
{
  if ( nullptr != res ) {
    *res = false;
  }
  if ( nullptr == array_ ) {
    return 0.0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  bool val = bson_iter_int32(&iterator_);
  return val;
}

QString Array::jsonString() const
{
  if ( nullptr == array_ ) {
    return QString();
  }
  char* data = bson_as_json( array_, NULL);
  QString str(data);
  bson_free(data);
  return str;
}

QString Array::jsonExtendedString() const
{
  if ( nullptr == array_ ) {
    return QString();
  }
  char* data = bson_as_canonical_extended_json( array_, NULL);
  QString str(data);
  bson_free(data);
  return str;
}

rapidjson::Document Array::jsonRapid() const
{
  QString jsonstr = jsonExtendedString();
  QByteArray arr = jsonstr.toUtf8();
  rapidjson::Document doc;
  doc.Parse( arr.data() );
  if ( true == doc.HasParseError() ) {
    error_log << QObject::tr("Запрос не является json-документом. Ошибка = %1")
      .arg( doc.GetParseError() );
    return doc;
  }
  return doc;
}

QString Array::jsonPostgresql() const
{
  bson_iter_t it;
  if ( false == bson_iter_init( &it, array_ ) ) {
    error_log << QObject::tr("Не удалось инициализировать итератор");
    return QString();
  }
  QString sqlstr = "";
  if ( false == bson_iter_next(&it) ) {
    warning_log << QObject::tr("Пустой массив!");
    return "[]";
  }
  sqlstr = Array::postgresqlValue(&it);


  while ( true == bson_iter_next(&it) ) {
    sqlstr += "," + postgresqlValue(&it);
  }
  return QString("[%1]").arg(sqlstr);
}

QString Array::postgresqlValue( bson_iter_t* it )
{
  bson_type_t t = bson_iter_type(it);
  switch (t) {
    case BSON_TYPE_DOUBLE:
      return QString::number( bson_iter_double(it) );
      break;
    case BSON_TYPE_UTF8: {
        QString str = bson_iter_utf8(it, NULL);
        str.replace("'", "\"");
        str.replace("\"", "\\\"");
        return QString("\"%1\"").arg( str );
      }
      break;
    case BSON_TYPE_DOCUMENT: {
        Document doc;
        uint32_t length;
        const uint8_t* array;
        bson_iter_document( it, &length, &array );
        bson_t* chdoc = bson_new_from_data( array, length );
        doc.setDocument( chdoc );
        return QString("%1").arg(doc.jsonPostgresql());
      }
      break;
    case BSON_TYPE_OID: {
        const bson_oid_t* oid = bson_iter_oid(it);
        if ( nullptr == oid ) {
          error_log << QObject::tr("Не удалось получить объект типа Oid из поля '%1'");
          return QString();
        }
        char arr[25];
        bson_oid_to_string( oid, arr );
        return QString("\"%1\"").arg( QString::fromUtf8(arr) );
      }
      break;
    case BSON_TYPE_BOOL: {
        bool val = bson_iter_int32(it);
        return ( true == val ) ? "true" : "false" ;
      }
      break;
    case BSON_TYPE_DATE_TIME: {
        int64_t dt = bson_iter_date_time(it);
        QDateTime qdt = QDateTime::fromMSecsSinceEpoch((unsigned long int)dt, Qt::OffsetFromUTC );
        return QString("\"%1\"").arg( qdt.toString(Qt::ISODate) ); //!FIXME понять как записыать datetime в json в postgres'е
      }
      break;
    case BSON_TYPE_INT32:
      return QString::number( bson_iter_int32(it) );
      break;
    case BSON_TYPE_INT64:
      return QString::number( bson_iter_int64(it) );
      break;
    default:
      return "";
  }
  return "";
}


}
