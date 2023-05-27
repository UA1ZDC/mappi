#include "longintvalidator.h"
#include <cross-commons/debug/tlog.h>

LongIntValidator::LongIntValidator(long int min, long int max):
  min_(min),
  max_(max)
{

}


QValidator::State LongIntValidator::validate(QString &input, int &) const
{
  if ( true == input.isEmpty() ) {
    return QValidator::State::Acceptable;
  }
  if ( min_ < 0 && 0 == input.compare(QObject::tr("-")) ){
    return QValidator::State::Intermediate;
  }

  bool ok = false;
  long int value = input.toLong(&ok);
  if ( false == ok ) {
    return QValidator::State::Invalid;
  }
  else if ( min_ <= value && value <= max_ ) {
    return QValidator::State::Acceptable;
  }
  else {
    while ( INT64_MIN / 10 <= value && value <= INT64_MAX / 10 ) {
      if ( min_ <= value && value <= max_ ){
        return QValidator::State::Intermediate;
      }
      else {
        value *= 10;
      }
    }
    return QValidator::State::Invalid;
  }
}
