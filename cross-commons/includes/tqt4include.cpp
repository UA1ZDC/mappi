#include "tqt4include.h"

//#include "tcommoninclude.h"

#include <QDateTime>


namespace TCommon
{
  QDateTime datetimeFromString( const QString& str, const QString& pattern )
  {
    return QDateTime::fromString( str, pattern );
  }
}
