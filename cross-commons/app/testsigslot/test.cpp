#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/tsigslot.h>
#include <cross-commons/debug/tlog.h>

class TestSignal
{
  public:
    sigslot::signal0<> testsignal;
};

class TestSlot : public sigslot::has_slots<>
{
  public:
    TestSlot( TestSignal* sgn )
    {
      sgn->testsignal.connect( this, &TestSlot::slotTestSlot );
//      sgn->testsignal.disconnect_all();
    }
    void slotTestSlot() { debug_log << "test slot"; }
};

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  TestSignal sgn;
  for ( int i = 0;; ++i) {
    TestSlot slot(&sgn);
    debug_log << "here" << i;
//    sgn.testsignal.disconnect_all();
  }
  return EXIT_SUCCESS;
}
