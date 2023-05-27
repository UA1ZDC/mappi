#ifndef TCROSSDEFINES_H
#define TCROSSDEFINES_H

#include <qdatetime.h>
#include <qglobal.h>

//using QT_VERSION macros
#define MAX_QT3 0x30308
#include <qstring.h>
#if QT_VERSION > MAX_QT3
  #define TQT_VERSION    4
#else
  #define TQT_VERSION    3
#endif



#if TQT_VERSION == 4
#include "tqt4include.h"
//#include "tqt4mocinclude.h"
#else
#include "tqt3include.h"
//#include "tqt3mocinclude.h"
#endif


#endif
