#include <qtextcodec.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/msgparser/rawparser.h>
#include <meteo/novost/msgstream/core/unimas.h>

#include "test.pb.h"

int main(int argc, char** argv)
{
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
  QCoreApplication app(argc,argv);

  QByteArray hexIn = "0100000000000000008e000055414e5439394b44444c000002000000";

  /*

  Пример описания протоструктуры и конфигурации сервисного пакета (tcp)
  для протокола SocketSpecial.

  // unimas_tcp.conf
  bin_format {
    begin { "01" path: "type" }
    end   { "04" }
    chunk { size: 4   path: "ijp" }
    chunk { size: 2   path: "len" }
    chunk { size: 2   path: "num" }
    chunk { size: 12  path: "ahd" }
    chunk { size: 1   path: "pri" }
  }

  */
  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinMarker* beg = format.add_begin();
  beg->set_hex_data("01");
  meteo::msgparser::RawBinChunk* chunk = 0;
  chunk = format.add_chunk();
  chunk->set_size(3);
  chunk = format.add_chunk();
  chunk->set_size(4);
  chunk->set_path("ijp");
  chunk = format.add_chunk();
  chunk->set_size(2);
  chunk->set_path("len");
  chunk = format.add_chunk();
  chunk->set_size(2);
  chunk->set_path("num");
  chunk = format.add_chunk();
  chunk->set_size(12);
  chunk->set_path("ahd");
  chunk = format.add_chunk();
  chunk->set_size(1);
  chunk->set_path("pri");
  chunk = format.add_chunk();
  chunk->set_size(3);

  meteo::RawParser parser;
  parser.setFormat(format);
  parser.appendData(QByteArray::fromHex(hexIn));

  meteo::msgparser::SockSpec_ServicePacket pack;
  bool res = parser.parse(&pack, true);

  var(res);
  var(format.Utf8DebugString());
  var(pack.Utf8DebugString());

  meteo::TServicePacket servPack(meteo::TServicePacket::TCP, QByteArray::fromHex(hexIn).data());
  servPack.printDebug();

  return 0;
}
