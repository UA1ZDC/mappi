#ifndef JULLIAN_H
#define JULLIAN_H

#include <qdatetime.h>

//! определение юлианской даты
class JullianDate {
 public:
  JullianDate();
  JullianDate( uint year, double day );
  JullianDate( const QDateTime& dt );

  ~JullianDate();

  void setfromJan1_12h_1900(double day);
  double fromJan1_12h_1900() const;
  double getDate() const;
  double getModif() const;

  double toGMST( ) const;
  double toLGMST( double longitude ) const;

  void addMin( double );

 private:
  void init( uint year, double day );
  
 private:
  double _jDate;

};

#endif //JULLIAN_H
