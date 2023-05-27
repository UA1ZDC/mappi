#include "test.pb.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/weather.pb.h>

#include <google/protobuf/message.h>

#include <qapplication.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qdebug.h>

#include <iostream>

void testLoadFill()
{
  test::app::Applications apps;
  QFile file("./proto.test");
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    std::cerr << "error open file with text proto =" << file.fileName().toLocal8Bit().data();
    return;
  }
  bool res = TProtoText::fromFile( "./proto.test", &apps, true );
  if ( false == res ) {
    std::cerr
      << QObject::tr("Не удалось загрузить текст в proto-сообщение.").toStdString();
    return;
  }
  QByteArray output = TProtoText::toText(apps);
  std::cout
    << QObject::tr("Содержимое proto-файла в текстовом виде =\n").toLocal8Bit().data()
    << output.data();

  test::app::Applications inputoproto;
  res = TProtoText::fillProto( output, &inputoproto );
  if ( false == res ) {
    std::cerr
      << QObject::tr("Не удалось загрузить текст в proto-сообщение. Текст =").toLocal8Bit().data()
      << output.data();
    return;
  }

  std::cout
    << QObject::tr("Отладочная строка заполненного proto-файла = \n").toLocal8Bit().data()
    << QObject::tr(inputoproto.Utf8DebugString().c_str()).toLocal8Bit().data();
}
//
//void loadProtosFromDir()
//{
//  QMap< QString, meteo::weather::proto::Map> protos;
//  bool res = TProtoText::loadProtosFromDirectory( meteo::global::kWeatherPatternPath(), &protos );
//  if ( false == res ) {
//    error_log << QObject::tr("Не удалось загрузить прото из директории %1")
//      .arg( meteo::global::kWeatherPatternPath() );
//  }
//  else {
//    info_log << QObject::tr("Загружено %1 протофайлов из директории %2:")
//      .arg( protos.size() )
//      .arg( meteo::global::kWeatherPatternPath() );
//    QMapIterator<QString, meteo::weather::proto::Map> it(protos);
//    int num = 0;
//    while ( true == it.hasNext() ) {
//      it.next();
//      qDebug() << "\t" << QObject::tr("№%1 - %2").arg(++num).arg( it.key() );
//    }
//  }
//}

int main( int argc, char** argv )
{
  QCoreApplication app(argc, argv);
  testLoadFill();
//  loadProtosFromDir();

  return EXIT_SUCCESS;
}
