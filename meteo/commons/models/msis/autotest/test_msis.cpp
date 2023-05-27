#include <cstdlib>
#include <cstring>

#include <qstring.h>
#include <qbytearray.h>

#include "autotest.h"

//
//
//

class Test_Msis : public CppUnit::TestCase
{
private:
  CPPUNIT_TEST_SUITE( Test_Msis );
  CPPUNIT_TEST( test_gtd7 );
  CPPUNIT_TEST_SUITE_END();

public:
  Test_Msis()
  {
  }

  void test_gtd7();
};

T_REGISTER_TEST( Test_Msis, "msis" );

//
//
//

#include <meteo/commons/models/msis/nrlmsise-00.h>
#include <meteo/commons/models/msis/funcs.h>

void Test_Msis::test_gtd7()
{
  QStringList results;

  struct nrlmsise_output output[17];
  struct nrlmsise_input input[17];
  struct nrlmsise_flags flags;
  struct ap_array aph;

  /* input values */
  for ( int i=0; i<7; ++i ) {
    aph.a[i]=100;
  }
  flags.switches[0]=0;
  for ( int i=1; i<24; ++i ) {
    flags.switches[i]=1;
  }
  for ( int i=0; i<17; ++i ) {
    input[i].doy=172;
    input[i].year=0; /* without effect */
    input[i].sec=29000;
    input[i].alt=400;
    input[i].g_lat=60;
    input[i].g_long=-70;
    input[i].lst=16;
    input[i].f107A=150;
    input[i].f107=150;
    input[i].ap=4;
  }
  input[1].doy=81;
  input[2].sec=75000;
  input[2].alt=1000;
  input[3].alt=100;
  input[10].alt=0;
  input[11].alt=10;
  input[12].alt=30;
  input[13].alt=50;
  input[14].alt=70;
  input[16].alt=100;
  input[4].g_lat=0;
  input[5].g_long=0;
  input[6].lst=4;
  input[7].f107A=70;
  input[8].f107=180;
  input[9].ap=40;
  input[15].ap_a=&aph;
  input[16].ap_a=&aph;

  /* evaluate 0 to 14 */
  for ( int i=0; i<15; ++i ) {
    gtd7(&input[i], &flags, &output[i]);
  }

  /* evaluate 15 and 16 */
  flags.switches[9]=-1;
  for ( int i=15; i<17; ++i ) {
    gtd7(&input[i], &flags, &output[i]);
  }

  /* output type 1 */
  for ( int i=0; i<17; ++i ) {
    results << QString("");

    QString line;
    for ( int j=0; j<9; ++j ) {
      line += QString("%1 ").arg(output[i].d[j], 11, 'E', 6);
    }
    line += QString("%1 ").arg(output[i].t[0], 11, 'E', 6);
    line += QString("%1").arg(output[i].t[1], 11, 'E', 6);
    results << line;
    /* DL omitted */
  }

  results << QString("");
  /* output type 2 */
  for ( int i=0; i<3; ++i ) {
    results << QString("");
    QString line = QString("DAY").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].doy, 12, 10, QChar(' '));
    }
    results << line;
    line = QString("UT").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].sec, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("ALT").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].alt, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("LAT").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].g_lat, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("LONG").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].g_long, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("LST").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].lst, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("F107A").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].f107A, 12, 'f', 0, QChar(' '));
    }
    results << line;
    line = QString("F107").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(input[i*5+j].f107, 12, 'f', 0, QChar(' '));
    }
    results << line;
    results << QString("");
    results << QString("");
    line = QString("TINF").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].t[0], 12, 'f', 2, QChar(' '));
    }
    results << line;
    line = QString("TG").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].t[1], 12, 'f', 2, QChar(' '));
    }
    results << line;
    line = QString("HE").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[0], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("O").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[1], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("N2").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[2], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("O2").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[3], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("AR").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[4], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("H").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[6], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("N").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[7], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("ANM 0").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[8], 12, 'e', 3, QChar(' '));
    }
    results << line;
    line = QString("RHO").leftJustified(6, ' ');
    for ( int j=0; j<5; ++j ) {
      line += QString("%1").arg(output[i*5+j].d[5], 12, 'e', 3, QChar(' '));
    }
    results << line;
    results << QString("");
  }

  QFile file(":/files/output.txt");
  CPPUNIT_ASSERT( file.open(QFile::ReadOnly) );

  QString actual = results.join("\n");
  QString expected = QString::fromUtf8(file.readAll());

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}

