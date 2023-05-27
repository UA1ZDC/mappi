#ifndef TDEBUG_H
#define TDEBUG_H

#include "tlog.h"

#if TQT_VERSION == 3
  #include <qglobal.h>
  inline TLog qDebug() { return TLog(); }
#endif
#if TQT_VERSION == 4
  #include <QtDebug>
#endif

#endif // TDEBUG_H
