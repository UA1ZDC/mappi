#include <qstringlist.h>
#include <qcoreapplication.h>
#include <qfile.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <sql/nosql/document.h>
#include <sql/nosql/array.h>
#include <sql/rapidjson/prettywriter.h>
#include <sql/rapidjson/stringbuffer.h>

static QString json ="{\"type\":\"MultiPolygon\",\"coordinates\":[[[[31.5056,69.619],[31.508,69.6162],[31.5132,69.6152],[31.5182,69.617]]]]}";


int main(int argc, char** argv)
{
  QCoreApplication* app = new QCoreApplication(argc,argv);

  meteo::Document doc(json);

  debug_log << "doc = " << doc.valueGeoVector("coordinates");

  return EXIT_SUCCESS;
}
