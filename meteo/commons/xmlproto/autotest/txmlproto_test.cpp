#include "txmlproto_test.h"
CPPUNIT_TEST_SUITE_REGISTRATION( TXmlProto_Test );

#include "test.pb.h"

#include <cross-commons/debug/tlog.h>
#include <commons/testhelper/tcppunit.h>
#include <meteo/commons/xmlproto/txmlproto.h>
#include <commons/testhelper/ttesthelper.h>

#include <QtXml>


void TXmlProto_Test::toXml()
{
  test::Application msg;
  msg.set_id( 12501 );
  msg.set_name( "test.exe" );
  msg.add_args( "--no-gui" );
  msg.add_args( "--loop=5" );
  msg.set_platform( test::WINDOWS );
  msg.set_size( 1024 );

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXml.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( msg );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlWithMessage()
{
  test::Book book;
  book.mutable_card()->set_id( 100 );
  book.set_name( QObject::tr("Война и мир").toStdString() );
  book.set_pages( 1368 );

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlWithMessage.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( book );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlWithRepeatedMessage()
{
  test::Book book1;
  book1.mutable_card()->set_id( 100 );
  book1.set_name( QObject::tr("Война и мир").toStdString() );
  book1.set_pages( 1368 );

  test::Book book2;
  book2.mutable_card()->set_id( 101 );
  book2.set_name( QObject::tr("Собрание сочинений").toStdString() );
  book2.set_pages( 325 );

  test::Bookcase bookcase;
  bookcase.set_color( test::BLACK );
  *bookcase.add_books() = book1;
  *bookcase.add_books() = book2;

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlWithRepeatedMessage.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( bookcase );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlEscape()
{
  test::Book book1;
  book1.set_name( QObject::tr("Xml движок сам экранирует спец. символы: $,#,%,^,?,\\,',\",>,<.").toStdString() );

  test::Book book2;
  book2.set_name( QObject::tr("Ушастый нинзь \">_<\"").toStdString() );

  test::Bookcase bookcase;
  *bookcase.add_books() = book1;
  *bookcase.add_books() = book2;

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlEscape.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( bookcase );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlMultiMessages()
{
  test::Planet p[2];
  p[0].set_name( QObject::tr("Земля").toStdString() );
  p[1].set_name( QObject::tr("Марс").toStdString() );

  test::SolarSystem s;
  *s.add_planets() = p[0];
  *s.add_planets() = p[1];

  test::Galaxy g;
  *g.add_systems() = s;

  test::Planet pp[2];
  pp[0].set_name( QObject::tr("Планета-1").toStdString() );
  pp[1].set_name( QObject::tr("Планета-2").toStdString() );

  test::SolarSystem ss;
  *ss.add_planets() = pp[0];
  *ss.add_planets() = pp[1];

  test::Galaxy gg;
  gg.set_name( "g-234" );
  *gg.add_systems() = ss;

  test::Universe u;
  *u.add_galaxies() = g;
  *u.add_galaxies() = gg;

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlMultiMessages.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( u );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlSeveralSimilarFields()
{
  test::BookBox box;
  box.mutable_book1()->set_name( std::string("Book 1") );
  box.mutable_book2()->set_name( std::string("Book 2") );
  box.mutable_book3()->set_name( std::string("Book 3") );

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlSeveralSimilarFields.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( box );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::toXmlGroup()
{
  test::Settings opt;
  opt.set_id( 100 );
  opt.mutable_db()->set_host("localhost");
  opt.mutable_db()->set_name("db_name");

  QString expectedXml;
  CPPUNIT_ASSERT( test::helper::readFile("expected/toXmlGroup.xml", &expectedXml) );

  QDomDocument xml = TXmlProto::toXml( opt );
  CPPUNIT_ASSERT_EQUAL( expectedXml, xml.toString(2) );
}

void TXmlProto_Test::fromXml()
{
  QString inputXml;
  CPPUNIT_ASSERT( test::helper::readFile("input/fromXml.xml", &inputXml) );

  QDomDocument xml;
  CPPUNIT_ASSERT( xml.setContent(inputXml) );

  test::Application msg;
  CPPUNIT_ASSERT( TXmlProto::fillProto( &msg, xml.documentElement() ) );

  CPPUNIT_ASSERT_EQUAL( 12501, msg.id() );
  CPPUNIT_ASSERT_EQUAL( std::string("test.exe"), msg.name() );
  CPPUNIT_ASSERT_EQUAL( 2, msg.args_size() );
  CPPUNIT_ASSERT_EQUAL( std::string("--no-gui"), msg.args(0) );
  CPPUNIT_ASSERT_EQUAL( std::string("--loop=25"), msg.args(1) );
  CPPUNIT_ASSERT_EQUAL( test::WINDOWS, msg.platform() );
  CPPUNIT_ASSERT_EQUAL( u_int64_t(1024), msg.size() );
}

void TXmlProto_Test::fromXmlWithMessage()
{
  QString inputXml;
  CPPUNIT_ASSERT( test::helper::readFile("input/fromXmlWithMessage.xml", &inputXml) );

  QDomDocument xml;
  CPPUNIT_ASSERT( xml.setContent(inputXml) );

  test::Book book;
  CPPUNIT_ASSERT( TXmlProto::fillProto( &book, xml.documentElement() ) );

  CPPUNIT_ASSERT_EQUAL( 1368u, book.pages() );
  CPPUNIT_ASSERT_EQUAL( QObject::tr("Война и мир").toStdString(), book.name() );
  CPPUNIT_ASSERT_EQUAL( u_int64_t(25), book.card().id() );
}

void TXmlProto_Test::fromXmlWithRepeatedMessage()
{
  QString inputXml;
  CPPUNIT_ASSERT( test::helper::readFile("input/fromXmlWithRepeatedMessage.xml", &inputXml) );

  QDomDocument xml;
  CPPUNIT_ASSERT( xml.setContent(inputXml) );

  test::Bookcase msg;
  CPPUNIT_ASSERT( TXmlProto::fillProto( &msg, xml.documentElement() ) );

  CPPUNIT_ASSERT_EQUAL( test::BLACK, msg.color() );
  CPPUNIT_ASSERT_EQUAL( 2, msg.books_size() );
  // book 1
  CPPUNIT_ASSERT_EQUAL( u_int64_t(100), msg.books(0).card().id() );
  CPPUNIT_ASSERT_EQUAL( QObject::tr("Война и мир").toStdString(), msg.books(0).name() );
  CPPUNIT_ASSERT_EQUAL( 1368u, msg.books(0).pages() );
  // book 2
  CPPUNIT_ASSERT_EQUAL( u_int64_t(101), msg.books(1).card().id() );
  CPPUNIT_ASSERT_EQUAL( QObject::tr("Собрание сочинений").toStdString(), msg.books(1).name() );
  CPPUNIT_ASSERT_EQUAL( 325u, msg.books(1).pages() );
}

void TXmlProto_Test::fromXmlSeveralSimilarFields()
{
  QString inputXml;
  CPPUNIT_ASSERT( test::helper::readFile("input/fromXmlSeveralSimilarFields.xml", &inputXml) );

  QDomDocument xml;
  CPPUNIT_ASSERT( xml.setContent(inputXml) );

  test::BookBox msg;
  CPPUNIT_ASSERT( TXmlProto::fillProto( &msg, xml.documentElement() ) );

  CPPUNIT_ASSERT_EQUAL( QObject::tr("Книга 1").toStdString(), msg.book1().name() );
  CPPUNIT_ASSERT_EQUAL( QObject::tr("Книга 2").toStdString(), msg.book2().name() );
  CPPUNIT_ASSERT_EQUAL( QObject::tr("Книга 3").toStdString(), msg.book3().name() );
}
