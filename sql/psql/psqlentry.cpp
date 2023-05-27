#include "psqlentry.h"
#include "psqlquery.h"

#include <pg_config.h>
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

namespace meteo {

PsqlEntry::PsqlEntry( PsqlQuery* query )
  : DbiEntry(),
  result_(query->res_),
  rowindex_(query->currentrow_)
{
  if ( nullptr == result_ ) {
    warning_log << QObject::tr("nullptr == result_");
    return;
  }
  if ( 0 > rowindex_ || rowindex_ >= query->size_ ) {
    warning_log << QObject::tr("Некорректные параметры для записи в результатах запроса");
    return;
  }
  int clmn = 0;
  for ( auto key : query->columns_ ) {
    if ( 0 == PQgetisnull( result_, rowindex_, clmn ) ) { //Есть результат!
      values_.insert( key, QString::fromUtf8( PQgetvalue( result_, rowindex_, clmn ) ) );
    }
    ++clmn;
  }
}

PsqlEntry::PsqlEntry( const QHash<QString, QString>& values )
  : DbiEntry(),
  values_(values)
{
}

PsqlEntry::PsqlEntry()
  : DbiEntry()
{
}

PsqlEntry::PsqlEntry( const PsqlEntry& entry )
  : DbiEntry(entry),
  result_( entry.result_ ),
  rowindex_( entry.rowindex_ ),
  values_( entry.values_ )
{
}

PsqlEntry::~PsqlEntry()
{
}

int32_t PsqlEntry::valueInt32( const QString& field, bool* res ) const
{
  bool locres = false;
  int32_t val = 0;
  if ( true == values_.contains(field) ) {
    locres = true;
    val = values_.value(field).toInt();
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}


int64_t PsqlEntry::valueInt64( const QString& field, bool* res ) const
{
  bool locres = false;
  int64_t val = 0;
  if ( true == values_.contains(field) ) {
    locres = true;
    val = values_.value(field).toLongLong();
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}

double PsqlEntry::valueDouble( const QString& field, bool* res ) const
{
  bool locres = false;
  double val = 0;
  if ( true == values_.contains(field) ) {
    locres = true;
    val = values_.value(field).toDouble();
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}

QString PsqlEntry::valueString( const QString& field, bool* res ) const
{
  bool locres = false;
  QString val = 0;
  if ( true == values_.contains(field) ) {
    locres = true;
    val = values_.value(field);
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}


QList<meteo::GeoVector> PsqlEntry::valueGeoVector( const QString& field, bool* res ) const
{
  if ( false == values_.contains(field) ) {
    if ( nullptr != res ) {
      *res = false;
      return QList<GeoVector>();
    }
  }
  return DbiQuery::json2geovector( values_.value(field), res );
}

meteo::GeoPoint PsqlEntry::valueGeo( const QString& field, bool* res ) const
{
  if ( false == values_.contains(field) ) {
    if ( nullptr != res ) {
      *res = false;
      return GeoPoint();
    }
  }
  return DbiQuery::json2geopoint( values_.value(field), res );
}


QDateTime PsqlEntry::valueDt( const QString& field, bool* res ) const
{
  bool locres = false;
  QDateTime val;
  if ( true == values_.contains(field) ) {
    locres = true;
    val = QDateTime::fromString( values_.value(field), Qt::ISODate );
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}

QString PsqlEntry::valueOid( const QString& field, bool* ok ) const
{
  return valueString( field, ok );
}

bool PsqlEntry::valueBool( const QString& field, bool* res ) const
{
  bool locres = false;
  bool val = false;
  if ( true == values_.contains(field) ) {
    locres = true;
    auto v = values_.value(field);
    val = ( 0 < v.length() && QChar('t') == v[0] ) ? true : false ;
  }
  if ( nullptr != res ) {
    *res = locres;
  }
  return val;
}

const Document& PsqlEntry::valueDocument( const QString& field, bool* res ) const
{
  if ( true == values_.contains(field) ) {
    locdoc_ = Document( values_.value(field) );
    if ( nullptr != res ) {
      *res = true;
    }
    return locdoc_;
  }
  if ( nullptr != res ) {
    *res = false;
  }
  locdoc_ = Document();
  return locdoc_;
}

const Array& PsqlEntry::valueArray( const QString& field, bool* res ) const
{
  if ( true == values_.contains(field) ) {
    QByteArray a = values_.value(field).toUtf8();
    locarr_ = Array(values_.value(field));
    if ( nullptr != res ) {
      *res = true;
    }
    return locarr_;
  }
  if ( nullptr != res ) {
    *res = false;
  }
  locarr_ = Array();
  return locarr_;
}

bool PsqlEntry::hasField( const QString& field ) const
{
  return values_.contains(field);
}

bson_type_t PsqlEntry::typeField( const QString& field ) const
{
  Q_UNUSED(field);
  warning_log << QObject::tr("Не реализовано!");
  return BSON_TYPE_UNDEFINED;
}

void PsqlEntry::removeField( const QString& field )
{
  values_.remove(field);
}

QString PsqlEntry::jsonExtendedString() const
{
  QStringList list;
  for ( auto it = values_.constBegin(), end = values_.constEnd(); it != end; ++it ) {
    list.append( it.key() + ": " + it.value() );
  }
  return QString("{%1}").arg( list.join(",") );
}

}
