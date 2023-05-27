#ifndef CROSS_COMMONS_APP_TERROR_H
#define CROSS_COMMONS_APP_TERROR_H

#include <qstring.h>

template<typename T, T no_error_code>
class TError
{
public:
  TError() :  _noErrorCode( no_error_code ),
              _code( no_error_code)
  {}

  T code() const         { return _code; }
  QString text() const   { return _text; }

  inline TError& operator << ( const T& err )       { _code = err; return *this; }
  inline TError& operator << ( const QString& s )   { _text.append( s ); return *this; }

  inline operator T() const       { return _code; }
  inline operator T()             { return _code; }
  inline operator QString() const { return _text; }
  inline operator QString()       { return _text; }
  inline operator bool() const    { return _code == _noErrorCode; }
  inline operator bool()          { return _code == _noErrorCode; }

  inline void reset()   { _code = _noErrorCode; _text = QString(); }

private:
  const T _noErrorCode;
  T       _code;
  QString _text;
};

#endif // CROSS_COMMONS_APP_TERROR_H
