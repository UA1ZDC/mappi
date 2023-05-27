#include <qstring.h>



#include "mn_funcs.h"

namespace MnCommon {
  
  QString splitDigits( int digit, int grouplength, QChar delim )
  {
    QString number = QString::number(digit);
    int counter = 0;
    QString result;
    for ( int i = number.size() - 1, sz = -1; i > sz; --i ) {
      result.prepend( number[i] );
      ++counter;
      if ( counter == grouplength && 0 != i ) {
	counter = 0;
	result.prepend(delim);
      }
    }
    return result;
  }
  
}
