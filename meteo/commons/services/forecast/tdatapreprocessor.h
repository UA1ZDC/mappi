#ifndef TDATAPREPROCESSOR_H
#define TDATAPREPROCESSOR_H

#include <qobject.h>
#include <qscriptvalue.h>

#include "tdataprovider.h"

#include <commons/sun_moon/src/func_sun_moon.h>

class TDataPreprocessor: public QObject
{
  Q_OBJECT

public:

  TDataPreprocessor(TDataProvider* adataProvider): _dataProvider(adataProvider) {};

  Q_INVOKABLE QDateTime getSunRise(int year, int month, int day, double d_fi0, double d_la0, int i_N, const QScriptValue& typeSun);


private:

  TDataProvider* _dataProvider;
  // zond::Zond _dataProcessor;

};


#endif