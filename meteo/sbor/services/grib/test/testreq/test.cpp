#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/decoders.pb.h>

#include <meteo/novost/global/global.h>
#include <meteo/commons/proto/tgribformat.pb.h>

#include <qobject.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qfile.h>

#include <stdlib.h>
#include <iostream>

/*!
  \file  test.cpp
  \brief Проверка запроса на раскодированое телеграммы (отправка телеграммы, получение раскодированных данных)
*/


class TestGetDecode {
public:
  TestGetDecode() {}
  ~TestGetDecode() {};

  void testgrib(meteo::decoders::DecodedMsg* resp)
  {
    if (resp->has_error()) {
      var(resp->error());
    }
    var(resp->data_size());
    for (int i=0; i< resp->data_size(); i++) {
      grib::TGribData grib;
      grib.ParseFromString(resp->data(i));
      var(grib.DebugString());
    }

  }
};

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  QString path;
  if (argc <= 1) {
    error_log<<"Need point file name\n";
    exit(-1);
  } else {
    path = argv[1];
  }

  QFile fl(path);
  if (!fl.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла '%1'").arg(path);
    return 0;
  }
  QByteArray bufAll = fl.readAll();
  fl.close();
  

  rpc::Address addr("192.168.52.39", meteo::global::kServicePorts[meteo::global::kGribService]);
  rpc::Channel* channel = rpc::ServiceLocation::instance()->serviceClient(addr);
  if ( 0 == channel ) {
    error_log << QObject::tr("Сервис grib не обнаружен");
    return EXIT_FAILURE;
  }
  
  meteo::decoders::DecodeRequest req;
  req.set_tlg(bufAll.data(), bufAll.size());
  meteo::decoders::DecodedMsg resp;

  rpc::TController ctrl(channel);
  TestGetDecode testdec;

  ctrl.remoteCall(&meteo::decoders::DecodersRpc::GetDecodedMsg,
		  req,  &resp, &testdec,  &TestGetDecode::testgrib);

  return app.exec();
}
