#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITION_WIDGETS_LONGINTVALIDATOR_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITION_WIDGETS_LONGINTVALIDATOR_H

#include <qvalidator.h>

class LongIntValidator : public QValidator
{
public:
  LongIntValidator( long int min, long int max);

  virtual State validate(QString &, int &) const;

private:
  long int min_;
  long int max_;
};

#endif
