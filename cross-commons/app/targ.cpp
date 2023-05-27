#include "targ.h"
#include <cross-commons/debug/tlog.h>


namespace  {
enum KeyType{
  KeyType_One,    //!< односимвольный ключ
  KeyType_Multi   //!< многосимвольный ключ
};
}


TArg::TArg(int argc, char** argv)
{
  QStringList list;
  for( int i=0; i<argc; ++i ){
    list += argv[i];
  }
  parse(list);
}

TArg::TArg(const QStringList& args)
{
  parse(args);
}

QString TArg::value( const QString& key, const QString& defaultValue ) const
{
  if( singleValues_.contains(key) ){
    return singleValues_[key];
  }
  else if( multiValues_.contains(key) ){
    return multiValues_[key].first();
  }

  return defaultValue;
}

QString TArg::value(const QStringList& keys, const QString& defaultValue) const
{
  foreach ( const QString& key, keys ) {
    if( singleValues_.contains(key) ){
      return singleValues_[key];
    }
    else if( multiValues_.contains(key) ){
      return multiValues_[key].first();
    }
  }

  return defaultValue;
}

QStringList TArg::values( const QString& key, const QStringList& defaultValue ) const
{
  if( singleValues_.contains(key) ){
    return QStringList( singleValues_[key] );
  }
  else if( multiValues_.contains(key) ){
    return multiValues_[key];
  }

  return defaultValue;
}

QStringList TArg::values(const QStringList& keys, const QStringList& defaultValue) const
{
  QStringList values;
  foreach ( const QString& key, keys ) {
    if( singleValues_.contains(key) ){
      values.append(singleValues_[key]);
    }
    else if( multiValues_.contains(key) ){
      values += multiValues_[key];
    }
  }

  return values.isEmpty() ? defaultValue : values;
}

QStringList TArg::keys() const
{
  QStringList list;
  list += singleValues_.keys();
  list += multiValues_.keys();
  return list;
}

bool TArg::contains( const QString& key ) const {
  return singleValues_.contains(key) || multiValues_.contains(key);
}

bool TArg::contains( const QStringList& keys, CompareFlag flag  ) const {
  if( Flag_Any == flag ){
    foreach( QString key, keys ){
      if( singleValues_.contains(key) || multiValues_.contains(key) ){
        return true;
      }
    }
  }
  else {
    int counter = 0;
    foreach( QString key, keys ){
      if( singleValues_.contains(key) || multiValues_.contains(key) ){
        counter++;
      }
    }

    return keys.size() == counter;
  }

  return false;
}

int TArg::keyCount() const
{
  return singleValues_.size() + multiValues_.size();
}

void TArg::parse(const QStringList& list)
{
  QRegExp argRx("^([-]{1,2})([\\-a-zA-z0-9?.]+)(=(.*))?");
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    const QString& arg = list.at(i);

    argRx.indexIn(arg);
    QString key   = argRx.cap(2);
    QString value = argRx.cap(4);
    if( key.isEmpty() ){
      continue;
    }

    KeyType keyType = (argRx.cap(1).size()) == 2 ? KeyType_Multi : KeyType_One;

    if( (i+1) < isz && KeyType_One == keyType && !argRx.exactMatch(list.at(i+1)) ){
      value = list.at(++i);
    }

    if( key.length() > 1 && KeyType_One == keyType ){
      for( int j=0,jsz=key.size(); j<jsz; ++j ){
        append( key.at(j) );
      }
    }
    else {
      value.replace( QRegExp("^['\"](.*)['\"]$"), "\\1" );
      append( key, value );
    }
  }
}

void TArg::append(const QString& key, const QString& value)
{
  if( singleValues_.contains(key) ){
    multiValues_[key].append( singleValues_[key] );
    multiValues_[key].append( value );

    singleValues_.remove( key );
  }
  else if ( multiValues_.contains(key) ) {
    multiValues_[key].append( value );
  }
  else {
    singleValues_[key] = value;
  }
}
