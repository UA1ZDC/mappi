#ifndef TCOMMONINCLUDE_H
#define TCOMMONINCLUDE_H

#include "tcrossdefines.h"
#include <string>

#include <qdatetime.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#define COMMON_SPLIT_REGEXP     QRegExp("( |\\t)")

#undef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                           \
  void operator=(const TypeName&)

namespace TCommon
{
  QDateTime datetimeFromString( const QString& str, const QString& pattern );
  QDateTime datetimeFromDb( const QString& str );

}


extern "C" char* string_guid_generation();

#endif
