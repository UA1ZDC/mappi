#ifndef METEO_COMMONS_UI_CONDITIONMAKER_TEMPLATES_CONDITIONGENERATOR_H
#define METEO_COMMONS_UI_CONDITIONMAKER_TEMPLATES_CONDITIONGENERATOR_H

#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

namespace meteo {

class ConditionGenerator
{
public:
  virtual Condition getCondition() const = 0;
};

}

#endif
