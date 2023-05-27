#include <cstring>
#include <limits.h>
#include <arpa/inet.h>

#include <qstring.h>
#include <qbytearray.h>

#include <commons/textproto/tprototext.h>
#include <meteo/commons/msgparser/rawparser.h>
#include <meteo/commons/msgparser/rawparser.pb.h>

#include "autotest.h"
#include "test_rawparser.pb.h"

//
//
//

class Test_RawParser : public CppUnit::TestCase
{
private:
  CPPUNIT_TEST_SUITE( Test_RawParser );
  CPPUNIT_TEST( netorder_16 );
  CPPUNIT_TEST( netorder_32 );
  CPPUNIT_TEST( one_begin );
  CPPUNIT_TEST( many_begin );
  CPPUNIT_TEST( one_end );
  CPPUNIT_TEST( many_end );
  CPPUNIT_TEST_SUITE_END();

public:
  void netorder_16();
  void netorder_32();
  void one_begin();
  void many_begin();
  void one_end();
  void many_end();
};

T_REGISTER_TEST( Test_RawParser, "rawparser" );

//
//
//

//! CASE: С преобразование порядка байт и без.
void Test_RawParser::netorder_16()
{
  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinChunk* chunk = format.add_chunk();
  chunk->set_size(2);
  chunk->set_path("len");

  meteo::RawParser parser;
  parser.setFormat(format);

  uint16_t i16 = std::numeric_limits<uint16_t>::max();
  const char* raw_i16 = (char*)&i16;
  QByteArray ba = QByteArray::fromRawData(raw_i16, sizeof(uint16_t));

  meteo::SockSpec_ServicePacket pack;

  parser.setByteOrder(meteo::RawParser::kHostOrder);
  parser.setData(ba);
  CPPUNIT_ASSERT( parser.parse(&pack) );
  CPPUNIT_ASSERT_EQUAL( uint32_t(i16), pack.len() );

  pack.Clear();

  uint16_t i16_net = htons(i16);
  const char* raw_i16_net = (char*)&i16_net;
  QByteArray netba = QByteArray::fromRawData(raw_i16_net, sizeof(uint16_t));

  parser.setByteOrder(meteo::RawParser::kNetOrder);
  parser.setData(netba);
  CPPUNIT_ASSERT( parser.parse(&pack) );
  CPPUNIT_ASSERT_EQUAL( uint32_t(i16), pack.len() );
}

//! CASE: С преобразование порядка байт и без.
void Test_RawParser::netorder_32()
{
  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinChunk* chunk = format.add_chunk();
  chunk->set_size(4);
  chunk->set_path("ijp");

  meteo::RawParser parser;
  parser.setFormat(format);

  int32_t i32 = std::numeric_limits<int32_t>::max();
  const char* raw_i32 = (char*)&i32;
  QByteArray ba = QByteArray::fromRawData(raw_i32, sizeof(int32_t));

  meteo::SockSpec_ServicePacket pack;

  parser.setByteOrder(meteo::RawParser::kHostOrder);
  parser.setData(ba);
  CPPUNIT_ASSERT( parser.parse(&pack) );
  CPPUNIT_ASSERT_EQUAL( i32, pack.ijp() );

  pack.Clear();

  int32_t i32_net = htonl(i32);
  const char* raw_i32_net = (char*)&i32_net;
  QByteArray netba = QByteArray::fromRawData(raw_i32_net, sizeof(int32_t));

  parser.setByteOrder(meteo::RawParser::kNetOrder);
  parser.setData(netba);
  CPPUNIT_ASSERT( parser.parse(&pack) );
  CPPUNIT_ASSERT_EQUAL( i32, pack.ijp() );
}

//! TEST:
void Test_RawParser::one_begin()
{
  const QString strBeg = "DEV_ID";

  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinMarker* beg = format.add_begin();
  beg->set_hex_data(strBeg.toUtf8().toHex());
  beg->set_path("ahd");

  QByteArray raw = QByteArray("DEV_ID\r\n...data...");

  meteo::RawParser parser;
  parser.setFormat(format);
  parser.setData(raw);

  meteo::SockSpec_ServicePacket pack;
  CPPUNIT_ASSERT( parser.parse(&pack) );
  CPPUNIT_ASSERT_EQUAL( strBeg, QString::fromUtf8(pack.ahd().c_str()) );
}

void Test_RawParser::many_begin()
{
  const QString strBeg1 = "D_ID_1";
  const QString strBeg2 = "D_ID_2";

  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinMarker* beg = 0;
  beg = format.add_begin();
  beg->set_hex_data(strBeg1.toUtf8().toHex());
  beg->set_path("data");
  beg = format.add_begin();
  beg->set_hex_data(strBeg2.toUtf8().toHex());
  beg->set_path("data");

  QByteArray raw1 = QByteArray("D_ID_1\r\n...data...");
  QByteArray raw2 = QByteArray("D_ID_2\r\n...data...");

  meteo::RawParser parser;
  parser.setFormat(format);
  parser.setData(raw1);

  meteo::Test_Format tf;
  CPPUNIT_ASSERT( parser.parse(&tf) );
  CPPUNIT_ASSERT_EQUAL( strBeg1, QString::fromUtf8(tf.data().c_str()) );

  tf.Clear();
  parser.setData(raw2);

  CPPUNIT_ASSERT( parser.parse(&tf) );
  CPPUNIT_ASSERT_EQUAL( strBeg2, QString::fromUtf8(tf.data().c_str()) );
}

void Test_RawParser::one_end()
{
  const QString strEnd = "\r\n\r\n";

  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinMarker* end = format.add_end();
  end->set_hex_data(strEnd.toUtf8().toHex());
  end->set_path("data");
  meteo::msgparser::RawBinChunk* chunk = format.add_chunk();
  chunk->set_size(18);

  QByteArray raw = QByteArray("DEV_ID\r\n...data...\r\n\r\n");

  meteo::RawParser parser;
  parser.setFormat(format);
  parser.appendData(raw);

  meteo::Test_Format tf;
  CPPUNIT_ASSERT( parser.parse(&tf) );
  CPPUNIT_ASSERT_EQUAL( strEnd, QString::fromUtf8(tf.data().c_str()) );
}

void Test_RawParser::many_end()
{
  const QString strEnd1 = "end_1";
  const QString strEnd2 = "end_2";

  meteo::msgparser::RawBinFormat format;
  meteo::msgparser::RawBinMarker* end = 0;
  end = format.add_end();
  end->set_hex_data(strEnd1.toUtf8().toHex());
  end->set_path("data");
  end = format.add_end();
  end->set_hex_data(strEnd2.toUtf8().toHex());
  end->set_path("data");
  meteo::msgparser::RawBinChunk* chunk = format.add_chunk();
  chunk->set_size(18);

  QByteArray raw1 = QByteArray("DEV_ID ...data... end_1");
  QByteArray raw2 = QByteArray("DEV_ID ...data... end_2");

  meteo::RawParser parser;
  parser.setFormat(format);
  parser.setData(raw1);

  meteo::Test_Format tf;
  CPPUNIT_ASSERT( parser.parse(&tf) );
  CPPUNIT_ASSERT_EQUAL( strEnd1, QString::fromUtf8(tf.data().c_str()) );

  tf.Clear();
  parser.setData(raw2);

  CPPUNIT_ASSERT( parser.parse(&tf) );
  CPPUNIT_ASSERT_EQUAL( strEnd2, QString::fromUtf8(tf.data().c_str()) );
}

