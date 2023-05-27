#include "tcommoninclude.h"

#include <quuid.h>

extern "C" char * string_guid_generation()
{
  QUuid uid = QUuid::createUuid();
  QString strUid = uid.toString();
  uint len = strUid.length();
  char *res = new char[len+1];
  memcpy(res, ASCII_FROM_QSTRING(strUid), len);
  res[len] = 0;
  return res;
}

namespace TCommon
{
  QDateTime datetimeFromDb( const QString& str )
  {
    return datetimeFromString( str, "yyyy-MM-dd hh:mm:ss" );
  }
}
