#include "document.h"
#include <cross-commons/debug/tlog.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

namespace meteo {

static int countbson = 0;

Document::Document()
  : DbiEntry(),
    own_(nullptr),
    alien_(nullptr),
    refcnt_( new int32_t )
{
  Q_UNUSED(countbson);
  *refcnt_ = 1;
//  ++countbson;
//    debug_log << "CNTBSON =" << countbson;
}

Document::Document( const Document& doc )
  : DbiEntry(),
    refcnt_( doc.refcnt_ )
{
  *refcnt_ = *refcnt_ + 1;
  own_ = doc.own_;
  alien_ = doc.alien_;
}

Document::Document( bson_t* bson )
  : DbiEntry(),
    own_(bson),
    alien_(bson),
    refcnt_(new int32_t)
{
  *refcnt_ = 1;
//  ++countbson;
//    debug_log << "CNTBSON =" << countbson;
}

Document::Document( const bson_t* bson )
  : DbiEntry(),
    own_(nullptr),
    alien_(nullptr),
    refcnt_(new int32_t)
{
  own_ = bson_copy(bson);
//    debug_log << "CNTBSON =" << countbson;
  alien_ = own_;
  *refcnt_ = 1;
//  ++countbson;
}

Document::Document( bson_iter_t* iterator )
  : DbiEntry(),
    own_(nullptr),
    alien_(nullptr),
    refcnt_(new int32_t)
{
  *refcnt_ = 1;
  uint32_t length;
  const uint8_t* arr;
  bson_iter_document( iterator, &length, &arr );
  own_ = bson_new_from_data( arr, length );
  //  debug_log << "CNTBSON =" << ++countbson;
  alien_ = own_;
//  ++countbson;
//    debug_log << "CNTBSON =" << countbson;
}

Document::Document( const QString& json )
  : refcnt_( new int32_t )
{
  *refcnt_ = 0;
  bson_error_t e;
  QByteArray arr = json.toUtf8();
//  own_ = new bson_t(); //bson_new();;
//  ++countbson;
//  debug_log << "CNTBSON =" << countbson;

  own_ = bson_new_from_json( (uint8_t*)arr.data(), arr.length(), &e );
  if ( nullptr == own_ ) {
    error_log << QObject::tr("Ошибка = %1. (Код ошибки: %3)").arg(e.message).arg(e.code);
    error_log << QObject::tr("JSON с ошибкой =") << json;
//    return;
  }
  alien_ = own_;
  *refcnt_ = 1;
}

Document::Document( const rapidjson::Document& doc )
  : Document( Document::rapid2string(doc) )
{
}

Document::~Document()
{
  delete locdoc_; locdoc_ = nullptr;
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if ( nullptr != own_ ) {
      bson_destroy(own_);
      own_ = nullptr;
      //  debug_log << "CNTBSON =" << --countbson;
    }
    delete refcnt_;
//    --countbson;
  }
  refcnt_ = nullptr;
  own_ = nullptr;
  alien_ = nullptr;
}

QString Document::rapid2string( const rapidjson::Document& doc )
{
  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  doc.Accept(writer);
  return QString( sb.GetString() );
}

void Document::setDocument( bson_t* doc )
{
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if (  nullptr != own_ ) {
      bson_destroy(own_);
      own_ = nullptr;
      //  debug_log << "CNTBSON =" << --countbson;
    }
//    --countbson;
    delete refcnt_;
  }
  refcnt_ = new int32_t;
//  ++countbson;
  *refcnt_ = 1;

  own_ = doc;
  alien_ = doc;
}

void Document::setDocument( const bson_t* doc )
{
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if (  nullptr != own_ ) {
      bson_destroy(own_);
      own_ = nullptr;
      //  debug_log << "CNTBSON =" << --countbson;
    }
//    --countbson;
    delete refcnt_;
  }
  refcnt_ = new int32_t;
//  ++countbson;
  *refcnt_ = 1;

  own_ = nullptr;
  alien_ = doc;
}

int32_t Document::valueInt32( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return 0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return bson_iter_int32(&iter);
}

int64_t Document::valueInt64( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return 0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  if ( true == BSON_ITER_HOLDS_INT64(&iter) ) {
    return bson_iter_int64(&iter);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iter) ) {
    return bson_iter_int32(&iter);
  }
  return bson_iter_double(&iter);
}

double Document::valueDouble( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return 0.0;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  double val = 0.0;
  if ( true == BSON_ITER_HOLDS_DOUBLE(&iter) ) {
    val = bson_iter_double(&iter);
  }
  else if ( true == BSON_ITER_HOLDS_INT32(&iter) ) {
    val = bson_iter_int32(&iter);
  }
  return val;
}

QString Document::valueString( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return QString();
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return QString( bson_iter_utf8(&iter, NULL) );
}

QList<meteo::GeoVector> Document::valueGeoVector( const QString& field, bool* res ) const
{
  QList<meteo::GeoVector> veclist;
  if ( nullptr != res ) {
    *res = false;
  }
  QString type = valueString("type");
  if(0 != type.compare("Polygon")){
    debug_log << QObject::tr("Тип %1 не поддерживается").arg(type);
    return veclist;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return veclist;
  }
  if ( nullptr != res ) {
    *res = false;
  }
  Array ar = valueArray("coordinates");
  while ( true == ar.next() ) {
    Array coordarr;
    ar.valueArray(&coordarr);
    meteo::GeoVector vector;
    while ( true == coordarr.next() ) {
      Array a;
      coordarr.valueArray(&a);
      auto str = a.jsonExtendedString();
      Document d(str);
      bool res1;
      double lon = d.valueDouble("0",&res1);
      if(false == res1){
        continue;
      }
      double lat = d.valueDouble("1",&res1);
      if(false == res1){
        continue;
      }
      GeoPoint point = meteo::GeoPoint::fromDegree( lat, lon);
      vector.append(point);
    }
    if ( 0 != vector.size() ) {
      veclist.append(vector);
    }
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return veclist;
}

meteo::GeoPoint Document::valueGeo( const QString& field, bool* res ) const
{
  meteo::GeoPoint point;
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return point;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  uint32_t length;
  const uint8_t* arr;
  bson_iter_document( &iter, &length, &arr );
  bson_t* doc = bson_new_from_data( arr, length );
  if ( false == find_doc_field( "coordinates.0", &iter, doc ) ) {
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
  if ( false == find_doc_field( "coordinates.1", &iter, doc ) ) {
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
  if ( false == find_doc_field( "alt", &iter, doc ) ) {
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
  point = meteo::GeoPoint::fromDegree( lat, lon, alt );
  
  return point;
}

QDateTime Document::valueDt( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return QDateTime();
  }
  if ( nullptr != res ) {
    *res = true;
  }

  QDateTime qdt;
  
  if ( true == BSON_ITER_HOLDS_DATE_TIME(&iter) ) {
    int64_t dt = bson_iter_date_time(&iter);
    qdt = QDateTime::fromMSecsSinceEpoch((unsigned long int)dt, Qt::OffsetFromUTC );
  }
  else {
    qdt = QDateTime::fromString(bson_iter_utf8(&iter, NULL), Qt::ISODate);
  }
  

  qdt.setTimeSpec(Qt::LocalTime);
  return qdt;
}

QString Document::valueOid( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return QString();
  }
  const bson_oid_t* oid = bson_iter_oid(&iter);
  if ( nullptr == oid && true == BSON_ITER_HOLDS_UTF8(&iter) ) { //на случай, если oid сохранили, как строку
    return QString( bson_iter_utf8(&iter, NULL) );
  }
  if ( nullptr == oid && true == BSON_ITER_HOLDS_INT32(&iter) ) { //на случай, если oid сохранили, как строку
    return QString::number( bson_iter_int32(&iter) );
  }
  if ( nullptr == oid ) {
    error_log << QObject::tr("Не удалось получить объект типа Oid из поля '%1'")
                 .arg(field);
    return QString();
  }
  if ( nullptr != res ) {
    *res = true;
  }
  char arr[25];
  bson_oid_to_string( oid, arr );
  return QString::fromUtf8(arr);
}

const Document& Document::valueDocument( const QString& field, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr == locdoc_ ) {
      locdoc_ = new Document();
    }
    *locdoc_ = Document();
    return *locdoc_;
  }
  uint32_t length;
  const uint8_t* array;
  bson_iter_document( &iter, &length, &array );
  bson_t* chdoc = bson_new_from_data( array, length );
  if ( nullptr != ok ) {
    *ok = true;
  }

  if ( nullptr == locdoc_ ) {
    locdoc_ = new Document();
  }
  *locdoc_ = Document(chdoc);
  return *locdoc_;
}

bool Document::valueBool( const QString& field, bool* res ) const
{
  if ( nullptr != res ) {
    *res = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    if ( nullptr != res ) {
      *res = res;
    }
    return false;
  }
  if ( nullptr != res ) {
    *res = true;
  }
  return bson_iter_bool(&iter);
}

const Array& Document::valueArray( const QString& field, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    locarr_ = Array();
    return locarr_;
  }
  uint32_t length;
  const uint8_t* array;
  bson_iter_array( &iter, &length, &array );
  if ( 0 == array ) {
    error_log << QObject::tr("Не удалось получть поле типа array");
    locarr_ = Array();
    return locarr_;
  }
  bson_t* arr = bson_new_from_data( array, length );
  if ( nullptr != ok ) {
    *ok = true;
  }

  locarr_ = Array(arr);
  return locarr_;
}

bool Document::hasField( const QString& field ) const
{
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    return false;
  }
  return true;
}

bson_type_t Document::typeField( const QString& field ) const
{
  bson_iter_t iter;
  if ( false == find_doc_field( field, &iter, alien_ ) ) {
    return BSON_TYPE_UNDEFINED;
  }

  return bson_iter_type(&iter);
}

void Document::removeField( const QString& field )
{
  if ( nullptr == own_ ) {
    return;
  }
  auto tmp = bson_new();
  bson_copy_to_excluding_noinit( own_, tmp, field.toUtf8().data(), NULL );
  bson_destroy(own_);
  own_ = tmp;
  alien_ = own_;
}

bool Document::isEmpty() const
{
  if ( nullptr == alien_ ) {
    return true;
  }
  bson_iter_t it;
  if ( false == bson_iter_init( &it, alien_ ) ) {
    return true;
  }
  if ( true == bson_iter_next(&it) ) {
    return false;
  }
  return true;
}

QString Document::jsonString() const
{
  if ( nullptr == alien_ ) {
    return QString();
  }
  char* data = bson_as_json( alien_, NULL);
  QString str(data);
  bson_free(data);
  return str;
}

QString Document::jsonExtendedString() const
{
  if ( nullptr == alien_ ) {
    return QString();
  }
  char* data = bson_as_canonical_extended_json( alien_, NULL);
  QString str(data);
  bson_free(data);
  return str;
}

Document& Document::operator=( const Document& doc )
{
  if ( this == &doc ) {
    return *this;
  }
  *refcnt_ = *refcnt_ - 1;
  if ( 0 == *refcnt_ ) {
    if ( nullptr != own_ ) {
      bson_destroy(own_);
      //  debug_log << "CNTBSON =" << --countbson;
      own_ = nullptr;
      alien_ = nullptr;
    }
//    --countbson;
    delete refcnt_;
  }
  refcnt_ = doc.refcnt_;
  own_ = doc.own_;
  alien_ = doc.alien_;
  *refcnt_ += 1;
  return *this;
}

bool Document::find_doc_field( const QString& field, bson_iter_t* iter, const bson_t* doc )
{
  bool res = false;
  if ( nullptr == doc ) {
    return false;
  }
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

QString Document::jsonPostgresql() const
{
  bson_iter_t it;
  if ( nullptr == alien_ || false == bson_iter_init( &it, alien_ ) ) {
    warning_log << QObject::tr("Не удалось инициализировать итератор");
    return QString("{}");
  }
  QString str;
  if ( false == bson_iter_next(&it) ) {
    warning_log << QObject::tr("Пустой документ!");
    return QString("{}");
  }
  QString nam = bson_iter_key(&it);
  QString val = Array::postgresqlValue(&it);
  str += QString("\"%1\" : %2").arg(nam).arg(val);
  while ( true == bson_iter_next(&it) ) {
    nam = bson_iter_key(&it);
    val = Array::postgresqlValue(&it);
    str += QString(",\"%1\" : %2").arg(nam).arg(val);
  }
  str = QString("{%1}").arg(str);
  return str;
}

rapidjson::Document Document::jsonRapid() const
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

}
