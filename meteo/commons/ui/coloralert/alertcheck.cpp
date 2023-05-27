#include "alertcheck.h"

#include <commons/textproto/pbtools.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/global/global.h>

namespace meteo {

AlertCheck::AlertCheck()
{  
}

void AlertCheck::addAlert(const tablo::ColorAlert& alert)
{
  QString descr = pbtools::toQString(alert.descrname());
  if ( descr.isEmpty() ) {
    return;
  }

  for ( int i=0,isz=alert.condition_size(); i<isz; ++i ) {
    data_[descr].append(alert.condition(i));
  }
}

tablo::Color AlertCheck::color(const QHash<QString, double>& values) const
{
  tablo::Color color = tablo::kNoColor;

  QHashIterator<QString,double> it(values);
  while ( it.hasNext() ) {
    it.next();

    color = max(color, check(it.key(), it.value()));
  }

  return color;
}

tablo::Color AlertCheck::check(const QString& descr, QVariant value) const
{
  if ( !data_.contains(descr) ) { return tablo::kNoColor; }

  tablo::Color color = tablo::kNoColor;

  foreach ( const tablo::Condition& cond, data_[descr] ) {
    switch ( cond.operation() ) {
      case tablo::kRange:
        if ( value.toDouble() < cond.operand_a() || value.toDouble() > cond.operand_b() ) { continue; }
        break;
      case tablo::kMore:
        if ( value.toDouble() <= cond.operand_a() ) { continue; }
        break;
      case tablo::kLess:
        if ( value.toDouble() >= cond.operand_a() ) { continue; }
        break;
      case tablo::kEqual:
        if (true != MnMath::isEqual(value.toDouble(), cond.operand_a()) ) { continue; }
        break;
      case tablo::kNotEqual:
        if (MnMath::isEqual(value.toDouble(), cond.operand_a()) ) { continue; }
        break;
    case tablo::kCifrCode:
      if ( value.toString() != QString::fromStdString( cond.operandcode()) ) {
        continue;
      }
      break;
      case tablo::kNoOperation:
        continue;
        break;
    }
    color = max(color, cond.color());
  }

  return color;
}

tablo::Color AlertCheck::max(tablo::Color c1, tablo::Color c2) const
{
  if ( tablo::kRed == c1    || tablo::kRed == c2 )    { return tablo::kRed; }
  if ( tablo::kYellow == c1 || tablo::kYellow == c2 ) { return tablo::kYellow; }
  if ( tablo::kGreen == c1  || tablo::kGreen == c2 )  { return tablo::kGreen; }

  return tablo::kNoColor;
}

} // meteo
