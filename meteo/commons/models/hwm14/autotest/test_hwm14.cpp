#include <cstdlib>
#include <cstring>

#include <qstring.h>
#include <qbytearray.h>

#include "autotest.h"

//
//
//

class Test_Hwm14 : public CppUnit::TestCase
{
private:
  CPPUNIT_TEST_SUITE( Test_Hwm14 );
  CPPUNIT_TEST( heightProfile );
  CPPUNIT_TEST( latitudeProfile );
  CPPUNIT_TEST( localTimeProfile );
  CPPUNIT_TEST( longitudeProfile );
  CPPUNIT_TEST( dayOfYearProfile );
  CPPUNIT_TEST( magneticActivityProfile );
  CPPUNIT_TEST( dwmMagneticLatitudeProfile );
  CPPUNIT_TEST( dwmMagneticLocalTimeProfile );
  CPPUNIT_TEST( dwmKpProfile );
  CPPUNIT_TEST_SUITE_END();

public:
  Test_Hwm14()
  {
    ::setenv("HWMPATH", "../", 1);
  }

  void heightProfile();
  void latitudeProfile();
  void localTimeProfile();
  void longitudeProfile();
  void dayOfYearProfile();
  void magneticActivityProfile();
  void dwmMagneticLatitudeProfile();
  void dwmMagneticLocalTimeProfile();
  void dwmKpProfile();
};

T_REGISTER_TEST( Test_Hwm14, "hwm" );

//
//
//

#include <commons/mathtools/mnmath.h>
#include <meteo/commons/models/hwm14/funcs.h>


/* ORIGIN in checkhwm14.f90
!******************************************************************************
!
!PERSHIFT
!JOHN EMMERT   9/12/03
!TRANSLATED TO FORTRAN-90 10/4/06. FORTRAN VERSION ONLY ALLOWS SCALAR INPUTS
!SHIFTS INPUT VALUES INTO A SPECIFIED PERIODIC INTERVAL
!
!CALLING SEQUENCE:   Result = PERSHIFT(x, range)
!
!ARGUMENTS
!      x:        The value to be shifted
!      perint:   2-element vector containing the start and end values
!                of the desired periodic interval.  The periodicity is
!                determined by the span of the range.
!
!ROUTINES USED THAT ARE NOT IN THE STANDARD FORTRAN-90 LIBRARY
!      None

function pershift(x, perint)

  real(4), parameter :: tol=1e-4
  real(4)            :: x, perint(0:1)
  real(4)            :: a, span, offset, offset1, pershift

  pershift = x
  a = perint(0)
  span = perint(1) - perint(0)
  if (span .ne. 0) then
    offset = x-a
    offset1 = mod(offset,span)
    if (abs(offset1) .lt. tol) offset1 = 0
  endif
  pershift = a + offset1
  if ((offset .lt. 0) .and. (offset1 .ne. 0)) pershift = pershift + span

  return

end function pershift
*/
double pershift(double x, double lower, double upper)
{
  double tol = 1e-4;
  double a = 0;
  double span = 0;
  double offset = 0;
  double offset1 = 0;
  double pershift_ = 0;

  pershift_ = x;
  a = lower;
  span = upper - lower;
  if ( !MnMath::isZero(span) ) {
    offset = x - a;
    offset1 = std::fmod(offset, span);
    if ( qAbs(offset1) < tol ) {
      offset1 = 0;
    }
  }
  pershift_ = a + offset1;
  if ( (offset < 0) && !MnMath::isZero(offset1) ) {
    pershift_ = pershift_ + span;
  }
  return pershift_;
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! height profile
  day = 150
  iyd = 95000 + day
  ut = 12.0
  sec = ut * 3600.0
  glat = -45.0
  glon = -85.0
  stl = pershift(ut + glon/15.0, (/0.0, 24.0/) )
  ap(2) = 80.0
  apqt(2) = -1.0
  print *, 'height profile'
  print '(a5,i3, a5,f4.1, a7,f5.1, a7,f6.1, a6,f4.1, a5,f5.1)', &
            'day=',day, ', ut=',ut, ', glat=',glat,  &
            ', glon=',glon, ', stl=',stl, ', ap=',ap(2)
  print '(6x,3a22)', 'quiet', 'disturbed', 'total'
  print '(a6,3(a12,a10))', 'alt', 'mer','zon', 'mer','zon', 'mer','zon'
  do ialt = 0, 400, 25
    alt = float(ialt)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
    call dwm07(iyd,sec,alt,glat,glon,ap,dw)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
    print '(f6.0,3(f12.3,f10.3))', alt, qw, dw, w
  end do
  print *
  print *
*/
void Test_Hwm14::heightProfile()
{
  float alt = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};
  float apqt[2] = {0,0};
  int ialt = 0;

  int day = 150;
  int iyd = 95000 + day;
  float ut = 12.0;
  float sec = ut * 3600.0;
  float glat = -45.0;
  float glon = -85.0;
  float stl = pershift(ut + glon/15.0, 0.0, 24.0);

  ap[1] = 80.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("height profile");
  results << QString("day=%1, ut=%2, glat=%3, glon=%4, stl=%5, ap=%6")
                .arg(day, 3, 10, QChar(' '))
                .arg(ut, 4, 'f', 1, QChar(' '))
                .arg(glat, 5, 'f', 1, QChar(' '))
                .arg(glon, 6, 'f', 1, QChar(' '))
                .arg(stl, 4, 'f', 1, QChar(' '))
                .arg(ap[1], 5, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
          + QString("quiet").rightJustified(22, ' ')
          + QString("disturbed").rightJustified(22, ' ')
          + QString("total").rightJustified(22, ' ')
              ;
  results << QString("alt").rightJustified(6, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
              ;

  for ( ialt=0; ialt<=400; ialt+=25 ) {
    alt = float(ialt);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(alt, 6, 'f', 0, QChar(' '))
            + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
            + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
            + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
            ;
  }
  results << QString("");

  QFile file(":/files/hwm14_height_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

  ! latitude profile
    day = 305
    iyd = 95000 + day
    ut = 18.0
    sec = ut * 3600.0
    alt = 250.0
    glon = 30.0
    stl = pershift(ut + glon/15.0, (/0.0, 24.0/) )
    ap(2) = 48.0
    apqt(2) = -1.0
    print *, 'latitude profile'
    print '(a5,i3, a5,f4.1, a6,f5.1, a7,f6.1, a6,f4.1, a5,f5.1)', &
              'day=',day, ', ut=',ut, ', alt=',alt,  &
              ', glon=',glon, ', stl=',stl, ', ap=',ap(2)
    print '(6x,3a22)', 'quiet', 'disturbed', 'total'
    print '(a6,3(a12,a10))', 'glat', 'mer','zon', 'mer','zon', 'mer','zon'
    do ilat = -90, 90, 10
      glat = float(ilat)
      call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
      call dwm07(iyd,sec,alt,glat,glon,ap,dw)
      call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
      print '(f6.1,3(f12.3,f10.3))', glat, qw, dw, w
    end do
*/
void Test_Hwm14::latitudeProfile()
{
  int iyd = 0;
  float sec = 0;
  float alt = 0;
  float glat = 0;
  float glon = 0;
  float stl = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};
  float ut = 0;
  float apqt[2] = {0,0};
  int day = 0;
  int ilat = 0;

  day = 305;
  iyd = 95000 + day;
  ut = 18.0;
  sec = ut * 3600.0;
  alt = 250.0;
  glon = 30.0;
  stl = pershift(ut + glon/15.0, 0.0, 24.0);
  ap[1] = 48.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("latitude profile");
  results << QString("day=%1, ut=%2, alt=%3, glon=%4, stl=%5, ap=%6")
              .arg(day, 3, 10, QChar(' '))
              .arg(ut, 4, 'f', 1, QChar(' '))
              .arg(alt, 5, 'f', 1, QChar(' '))
              .arg(glon, 6, 'f', 1, QChar(' '))
              .arg(stl, 4, 'f', 1, QChar(' '))
              .arg(ap[1], 5, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
          + QString("quiet").rightJustified(22, ' ')
          + QString("disturbed").rightJustified(22, ' ')
          + QString("total").rightJustified(22, ' ')
            ;
  results << QString("glat").rightJustified(6, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
          + QString("mer").rightJustified(12, ' ')
          + QString("zon").rightJustified(10, ' ')
            ;

  for ( ilat=-90; ilat<=90; ilat+=10 ) {
    glat = float(ilat);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(glat, 6, 'f', 0, QChar(' '))
            + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
            + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
            + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
            + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/hwm14_lat_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

  ! local time profile
    day = 75
    iyd = 95000 + day
    alt = 125.0
    glat = 45.0
    glon = -70.0
    ap(2) = 30.0
    apqt(2) = -1.0
    print *, 'local time profile'
    print '(a5,i3, a6,f5.1, a7,f5.1, a7,f6.1, a5,f5.1)', &
              'day=',day, ', alt=',alt, ', glat=',glat,  &
              ', glon=',glon, ', ap=',ap(2)
    print '(5x,3a22)', 'quiet', 'disturbed', 'total'
    print '(a5,3(a12,a10))', 'stl', 'mer','zon', 'mer','zon', 'mer','zon'
    do istl = 0,16
      stl = 1.5 * float(istl)
      sec = (stl - glon/15.0) * 3600.0
      call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
      call dwm07(iyd,sec,alt,glat,glon,ap,dw)
      call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
      print '(f5.1,3(f12.3,f10.3))', stl, qw, dw, w
    end do
*/
void Test_Hwm14::localTimeProfile()
{
  int iyd = 0;
  float sec = 0;
  float alt = 0;
  float glat = 0;
  float glon = 0;
  float stl = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};

  float apqt[2] = {0,0};
  int day = 0;
  int istl = 0;

  day = 75;
  iyd = 95000 + day;
  alt = 125.0;
  glat = 45.0;
  glon = -70.0;
  ap[1] = 30.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("local time profile");
  results << QString("day=%1, alt=%2, glat=%3, glon=%4, stl=%5, ap=%6")
              .arg(day, 3, 10, QChar(' '))
              .arg(alt, 4, 'f', 1, QChar(' '))
              .arg(glat, 5, 'f', 1, QChar(' '))
              .arg(glon, 6, 'f', 1, QChar(' '))
              .arg(stl, 4, 'f', 1, QChar(' '))
              .arg(ap[1], 5, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
           + QString("quiet").rightJustified(22, ' ')
           + QString("disturbed").rightJustified(22, ' ')
           + QString("total").rightJustified(22, ' ')
              ;
  results << QString("stl").rightJustified(6, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
              ;

  for ( istl=0; istl<=16; istl+=1 ) {
    stl = 1.5 * float(istl);
    sec = (stl - glon/15.0) * 3600.0;
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(stl, 6, 'f', 1, QChar(' '))
        + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/hwm14_time_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! longitude profile
  day = 330
  iyd = 95000 + day
  ut = 6.0
  sec = ut * 3600.0
  alt = 40.0
  glat = -5.0
  ap(2) = 4.0
  apqt(2) = -1.0
  print *, 'longitude profile'
  print '(a5,i3, a5,f4.1, a6,f5.1, a7,f5.1, a7,f6.1, a5,f5.1)', &
            'day=',day, ', ut=',ut, ', alt=',alt, ', glat=',glat,  &
            ', glon=',glon, ', ap=',ap(2)
  print '(6x,3a22)', 'quiet', 'disturbed', 'total'
  print '(a6,3(a12,a10))', 'glon', 'mer','zon', 'mer','zon', 'mer','zon'
  do ilon = -180, 180, 20
    glon = float(ilon)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
    call dwm07(iyd,sec,alt,glat,glon,ap,dw)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
    print '(f6.0,3(f12.3,f10.3))', glon, qw, dw, w
  end do
*/
void Test_Hwm14::longitudeProfile()
{
  int iyd = 0;
  float sec = 0;
  float alt = 0;
  float glat = 0;
  float glon = 0;
  float stl = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};

  float ut = 0;
  float apqt[2] = {0,0};
  int day = 0;
  int ilon = 0;

  day = 330;
  iyd = 95000 + day;
  ut = 6.0;
  sec = ut * 3600.0;
  alt = 40.0;
  glat = -5.0;
  ap[1] = 4.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("longitude profile");
  results << QString("day=%1, ut=%2, alt=%3, glat=%4, glon=%5, ap=%6")
         .arg(day, 3, 10, QChar(' '))
         .arg(ut, 4, 'f', 1, QChar(' '))
         .arg(alt, 5, 'f', 1, QChar(' '))
         .arg(glat, 6, 'f', 1, QChar(' '))
         .arg(glon, 4, 'f', 1, QChar(' '))
         .arg(ap[1], 5, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
      + QString("quiet").rightJustified(22, ' ')
      + QString("disturbed").rightJustified(22, ' ')
      + QString("total").rightJustified(22, ' ')
         ;
  results << QString("glon").rightJustified(6, ' ')
      + QString("mer").rightJustified(12, ' ')
      + QString("zon").rightJustified(10, ' ')
      + QString("mer").rightJustified(12, ' ')
      + QString("zon").rightJustified(10, ' ')
      + QString("mer").rightJustified(12, ' ')
      + QString("zon").rightJustified(10, ' ')
         ;

  for ( ilon=-180; ilon<=180; ilon+=20 ) {
    glon = float(ilon);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(glon, 6, 'f', 0, QChar(' '))
        + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/hwm14_lon_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! day of year profile
  ut = 21.0
  sec = ut * 3600.0
  alt = 200.0
  glat = -65.0
  glon = -135.0
  stl = pershift(ut + glon/15.0, (/0.0, 24.0/) )
  ap(2) = 15.0
  apqt(2) = -1.0
  print *, 'day of year profile'
  print '(a4,f4.1, a6,f5.1, a7,f5.1, a7,f6.1, a6,f4.1, a5,f5.1)', &
            'ut=',ut, ', alt=',alt, ', glat=',glat,  &
            ', glon=',glon, ', stl=',stl, ', ap=',ap(2)
  print '(6x,3a22)', 'quiet', 'disturbed', 'total'
  print '(a6,3(a12,a10))', 'day', 'mer','zon', 'mer','zon', 'mer','zon'
  do day = 0, 360, 20
    iyd = 95000 + day
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
    call dwm07(iyd,sec,alt,glat,glon,ap,dw)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
    print '(i6,3(f12.3,f10.3))', day, qw, dw, w
  end do
*/
void Test_Hwm14::dayOfYearProfile()
{
  int iyd = 0;
  float sec = 0;
  float alt = 0;
  float glat = 0;
  float glon = 0;
  float stl = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};
  float ut = 0;
  float apqt[2] = {0,0};
  int day = 0;

  ut = 21.0;
  sec = ut * 3600.0;
  alt = 200.0;
  glat = -65.0;
  glon = -135.0;
  stl = pershift(ut + glon/15.0, 0.0, 24.0);
  ap[1] = 15.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("day of year profile");
  results << QString("ut=%1, alt=%2, glat=%3, glon=%4, stl=%5, ap=%6")
              .arg(ut, 4, 'f', 1, QChar(' '))
              .arg(alt, 5, 'f', 1, QChar(' '))
              .arg(glat, 6, 'f', 1, QChar(' '))
              .arg(glon, 6, 'f', 1, QChar(' '))
              .arg(stl, 4, 'f', 1, QChar(' '))
              .arg(ap[1], 5, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
           + QString("quiet").rightJustified(22, ' ')
           + QString("disturbed").rightJustified(22, ' ')
           + QString("total").rightJustified(22, ' ')
              ;
  results << QString("day").rightJustified(6, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
              ;

  for ( day=0; day<=360; day+=20 ) {
    iyd = 95000 + day;
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(day, 6, 'f', 0, QChar(' '))
        + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/hwm14_doy_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! ap profile
  day = 280
  iyd = 95000 + day
  ut = 21.0
  sec = ut * 3600.0
  alt = 350.0
  glat = 38.0
  glon = 125.0
  stl = pershift(ut + glon/15.0, (/0.0, 24.0/) )
  ap(2) = 48.0
  apqt(2) = -1.0
  print *, 'magnetic activity profile'
  print '(a5,i3, a5,f4.1, a6,f5.1, a7,f5.1, a7,f6.1, a6,f4.1)', &
            'day=',day, ', ut=',ut, ', alt=',alt,  &
            ', glat=',glat, ', glon=',glon, ', stl=',stl
  print '(6x,3a22)', 'quiet', 'disturbed', 'total'
  print '(a6,3(a12,a10))', 'ap', 'mer','zon', 'mer','zon', 'mer','zon'
  do iaptemp = 0, 260, 20
    ap(2) = float(iaptemp)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,apqt,qw)
    call dwm07(iyd,sec,alt,glat,glon,ap,dw)
    call hwm14(iyd,sec,alt,glat,glon,stl,f107a,f107,ap,w)
    print '(f6.1,3(f12.3,f10.3))', ap(2), qw, dw, w
  end do
*/
void Test_Hwm14::magneticActivityProfile()
{
  int iyd = 0;
  float sec = 0;
  float alt = 0;
  float glat = 0;
  float glon = 0;
  float stl = 0;
  float f107a = 0;
  float f107 = 0;
  float ap[2] = {0,0};
  float w[2] = {0,0};
  float qw[2] = {0,0};
  float dw[2] = {0,0};
  float ut = 0;
  float apqt[2] = {0,0};
  int day = 0;
  int iaptemp = 0;

  day = 280;
  iyd = 95000 + day;
  ut = 21.0;
  sec = ut * 3600.0;
  alt = 350.0;
  glat = 38.0;
  glon = 125.0;
  stl = pershift(ut + glon/15.0, 0.0, 24.0);
  ap[1] = 48.0;
  apqt[1] = -1.0;

  QStringList results;

  results << QString("magnetic activity profile");
  results << QString("day=%1, ut=%2, alt=%3, glat=%4, glon=%5, stl=%6")
              .arg(day, 3, 10, QChar(' '))
              .arg(ut, 4, 'f', 1, QChar(' '))
              .arg(alt, 4, 'f', 1, QChar(' '))
              .arg(glat, 5, 'f', 1, QChar(' '))
              .arg(glon, 6, 'f', 1, QChar(' '))
              .arg(stl, 4, 'f', 1, QChar(' '));
  results << QString("").rightJustified(6, ' ')
           + QString("quiet").rightJustified(22, ' ')
           + QString("disturbed").rightJustified(22, ' ')
           + QString("total").rightJustified(22, ' ')
              ;
  results << QString("ap").rightJustified(6, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
           + QString("mer").rightJustified(12, ' ')
           + QString("zon").rightJustified(10, ' ')
              ;
  for ( iaptemp=0; iaptemp<=260; iaptemp+=20 ) {
    ap[1] = float(iaptemp);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,apqt,qw);
    dwm07_(&iyd,&sec,&alt,&glat,&glon,ap,dw);
    hwm14_(&iyd,&sec,&alt,&glat,&glon,&stl,&f107a,&f107,ap,w);
    results << QString("%1").arg(ap[1], 6, 'f', 1, QChar(' '))
        + QString("%1").arg(qw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(qw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(dw[1], 10, 'f', 3, QChar(' '))
        + QString("%1").arg(w[0], 12, 'f', 3, QChar(' '))
        + QString("%1").arg(w[1], 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/hwm14_ap_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! dwm: mlat profile
  kp = 6.0
  mlt = 3.0
  print *, 'dwm: magnetic latitude profile'
  print '(a5,f4.1, a5,f3.1)', 'mlt=',mlt, ', kp=',kp
  print '(a6,a12,a10)', 'mlat', 'mag mer', 'mag zon'
  do imlat = -90, 90, 10
    mlat = float(imlat)
    call dwm07b(mlt, mlat, kp, mmpwind, mzpwind)
    print '(f6.1,f12.3,f10.3)', mlat, mmpwind, mzpwind
  end do
*/
void Test_Hwm14::dwmMagneticLatitudeProfile()
{
  float mlt = 0;
  float mlat = 0;
  float kp = 0;
  float mmpwind = 0;
  float mzpwind = 0;
  int imlat = 0;

  QStringList results;

  kp = 6.0;
  mlt = 3.0;
  results << QString("dwm: magnetic latitude profile");
  results << QString("mlt=%1, kp=%2")
              .arg(mlt, 4, 'f', 1, QChar(' '))
              .arg(kp, 3, 'f', 1, QChar(' '));
  results << QString("mlat").rightJustified(6, ' ')
           + QString("mag mer").rightJustified(12, ' ')
           + QString("mag zon").rightJustified(10, ' ')
           ;
  for ( imlat=-90; imlat<=90; imlat+=10 ) {
    mlat = float(imlat);
    dwm07b_(&mlt,&mlat,&kp,&mmpwind,&mzpwind);
    results << QString("%1").arg(mlat, 6, 'f', 1, QChar(' '))
             + QString("%1").arg(mmpwind, 12, 'f', 3, QChar(' '))
             + QString("%1").arg(mzpwind, 10, 'f', 3, QChar(' '))
             ;
  }
  results << QString("");

  QFile file(":/files/dwm_latitude_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! dwm: mlt profile
  kp = 6.0
  mlat = 45.0
  print *, 'dwm: magnetic local time profile'
  print '(a6,f5.1, a5,f3.1)', 'mlat=',mlat, ', kp=',kp
  print '(a6,a12,a10)', 'mlt', 'mag mer', 'mag zon'
  do imlt = 0, 16
    mlt = float(imlt)*1.5
    call dwm07b(mlt, mlat, kp, mmpwind, mzpwind)
    print '(f6.1,f12.3,f10.3)', mlt, mmpwind, mzpwind
  end do
*/
void Test_Hwm14::dwmMagneticLocalTimeProfile()
{
  float mlt = 0;
  float mlat = 0;
  float kp = 0;
  float mmpwind = 0;
  float mzpwind = 0;
  int imlt = 0;

  QStringList results;

  kp = 6.0;
  mlat = 45.0;
  results << QString("dwm: magnetic local time profile");
  results << QString("mlt=%1, kp=%2")
              .arg(mlt, 4, 'f', 1, QChar(' '))
              .arg(kp, 3, 'f', 1, QChar(' '));
  results << QString("mlat").rightJustified(6, ' ')
           + QString("mag mer").rightJustified(12, ' ')
           + QString("mag zon").rightJustified(10, ' ')
              ;
  for ( imlt=0; imlt<=16; imlt+=1 ) {
    mlt = float(imlt) * 1.5;
    dwm07b_(&mlt,&mlat,&kp,&mmpwind,&mzpwind);
    results << QString("%1").arg(mlt, 6, 'f', 1, QChar(' '))
        + QString("%1").arg(mmpwind, 12, 'f', 3, QChar(' '))
        + QString("%1").arg(mzpwind, 10, 'f', 3, QChar(' '))
        ;
  }
  results << QString("");

  QFile file(":/files/dwm_local_time_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

/* ORIGIN in checkhwm14.f90

  integer            :: iyd
  real(4)            :: sec,alt,glat,glon,stl,f107a,f107,ap(2)
  real(4)            :: w(2), qw(2), dw(2)
  real(4)            :: mlt, mlat, kp, mmpwind, mzpwind
  real(4)            :: ut, apqt(2)
  real(4),external   :: pershift
  integer            :: day
  integer            :: ialt,istl,ilat,ilon,iaptemp
  integer            :: imlat,imlt,ikp

! dwm: kp profile
  mlat = -50.0
  mlt = 3.0
  print *, 'dwm: kp profile'
  print '(a6,f5.1, a6,f4.1)', 'mlat=',mlat, ', mlt=',mlt
  print '(a6,a12,a10)', 'kp', 'mag mer', 'mag zon'
  do ikp = 0, 18
    kp = float(ikp)*0.5
    call dwm07b(mlt, mlat, kp, mmpwind, mzpwind)
    print '(f6.1,f12.3,f10.3)', kp, mmpwind, mzpwind
  end do
*/
void Test_Hwm14::dwmKpProfile()
{
  float mlt = 0;
  float mlat = 0;
  float kp = 0;
  float mmpwind = 0;
  float mzpwind = 0;
  int ikp = 0;

  QStringList results;

  mlat = -50.0;
  mlt = 3.0;
  results << QString("dwm: kp profile");
  results << QString("mlat=%1, mlt=%2")
              .arg(mlat, 4, 'f', 1, QChar(' '))
              .arg(mlt, 3, 'f', 1, QChar(' '));
  results << QString("kp").rightJustified(6, ' ')
           + QString("mag mer").rightJustified(12, ' ')
           + QString("mag zon").rightJustified(10, ' ')
              ;
  for ( ikp=0; ikp<=18; ikp+=1 ) {
    kp = float(ikp) * 0.5;
    dwm07b_(&mlt,&mlat,&kp,&mmpwind,&mzpwind);
    results << QString("%1").arg(kp, 6, 'f', 1, QChar(' '))
            + QString("%1").arg(mmpwind, 12, 'f', 3, QChar(' '))
            + QString("%1").arg(mzpwind, 10, 'f', 3, QChar(' '))
            ;
  }
  results << QString("");

  QFile file(":/files/dwm_kp_profile_cpp.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}
