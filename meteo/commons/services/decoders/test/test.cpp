#include <qcoreapplication.h>
#include <qdir.h>

#include <meteo/commons/proto/sigwx.pb.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <unistd.h>

#define _DEBUG

using namespace meteo;

void testReadSigwx(const QString& fileName)
{
  QByteArray ba;
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return;
  }
  ba = file.readAll();
  file.close();

  std::string str(ba.data(), ba.size());
  sigwx::SigWx fsig;
  fsig.ParseFromString(str);

  var(fsig.DebugString());

}



int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("testmeteodecoder");
  // QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  int flag = 0, opt;
  QString fileName;

  while ((opt = getopt(argc, argv, "1:h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      fileName = optarg;
      var(fileName);
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1\n"
	"-1 <fileName> - чтение из файла sigwx\n";
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log<<"Need select test with option. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  }

  // QString fileName;
  // if (flag == 1) {
  //   if (optind >= argc) {
  //     error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
  //     exit(-1);
  //   } else {
  //     fileName = argv[optind];
  //   }
  // }

  QCoreApplication app(argc, argv);

  switch (flag) {
  case 1:
    testReadSigwx(fileName);
    break;

  default: {}
  }

  return 0;
}
