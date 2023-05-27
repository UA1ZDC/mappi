#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <cross-commons/app/paths.h>
#include <tdebug.h>

//int test_dt_from_string( int argc, char* argv[] )
//{
//  if ( 3 > argc ) {
//    qDebug() << "err params for parse dt string";
//    return EXIT_FAILURE;
//  }
//  QDateTime dt = TCommon::datetimeFromString( argv[1], argv[2] );
//  qDebug() << "dt =" << dt.toString("dd.MM.yyyy hh:mm:ss");
//  return EXIT_SUCCESS;
//}

void testSplitStr()
{
  QString str("a;b;c;");
  QString split(";");
  QStringList list = QSTRINGLIST_FROM_QSTRING( str, split );
  qDebug() << "str =" << str << "split =" << split;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    qDebug() << "list[" << i << "] =" << list[i];
  }
}

void test_stlstruuid()
{
  qDebug() << "uuid =" << string_guid_generation();
}

void test_qvaluelist()
{
  QValueList<QString> strlist;
  strlist.append("ololo1");
  strlist.append("ololo2");
  strlist.append("ololo3");
  for ( int i = 0; i < strlist.size(); ++i ) {
    qDebug() << "strlist[" << i << "] =" << strlist[i];
  }
}

TAPPLICATION_NAME("test")

int main( int , char**  )
{
  testSplitStr();
  test_stlstruuid();
  test_qvaluelist();
//  return test_dt_from_string( argc, argv );
}
