#include <qthread.h>
#include <qcoreapplication.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/msgstream/streamapp.h>
#include <meteo/commons/msgstream/plugins/ftpstream/ftpstreamin.h>
#include <meteo/commons/msgstream/plugins/ftpstream/ftpstreamout.h>
#include <meteo/commons/msgstream/plugins/filestream/filestreamin.h>
#include <meteo/commons/msgstream/plugins/filestream/filestreamout.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512stream.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512client.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512server.h>
#include <meteo/commons/msgstream/plugins/socketspecial/unimasstream.h>
#include <meteo/commons/msgstream/plugins/ptkpp/ptkppstreamrecv.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <meteo/sbor/settings/settings.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt  = QStringList() << "h" << "help";
const QStringList kHostOpt  = QStringList() << "H" << "host";
const QStringList kPortOpt  = QStringList() << "p" << "port";

const HelpFormatter kHelp = HelpFormatter()
    << HELP(kHelpOpt, "Эта справка.")
    << HELP(kHostOpt, "")
    << HELP(kPortOpt, "")
       ;


int main( int argc, char** argv )
{
  try {
    TAPPLICATION_NAME( "meteo" );

    QCoreApplication* app = new QCoreApplication(argc, argv);

    TArg args( argc, argv );

    if( args.contains(kHelpOpt) ){
      kHelp.print();
      delete app;
      return EXIT_SUCCESS;
    }

    ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;
    status->setUpdateLimit(1000);
    status->setSendLimit(5000);

    meteo::Sriv512Stream* s = 0;
    if ( args.contains(kHostOpt) ) {
      s = new meteo::Sriv512Client(status);
      s->setHost(args.value(kHostOpt));
    }
    else {
      s = new meteo::Sriv512Server(status);
    }

    s->setId("sriv.test");
    s->setPort(args.value(kPortOpt).toInt());

    s->slotInit();

    TSList<meteo::tlg::MessageNew> incoming;
    s->setIncoming(&incoming);

    TSList<meteo::tlg::MessageNew> outgoing;
    s->setOutgoing(&outgoing);

    if ( args.contains(kHostOpt) ) {
      meteo::tlg::MessageNew msg;
      msg.set_msg("TEST MESSAGE! TEST MESSAGE! TEST MESSAGE! TEST MESSAGE! TEST MESSAGE!");
      msg.set_format(meteo::tlg::kWMO);
      msg.mutable_header()->set_cccc("RUMS");
      msg.set_addrmode(meteo::tlg::kCircular);

      QByteArray ba = meteo::tlg::proto2tlg(msg);
      msg = meteo::tlg::tlg2proto(ba);

      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
      outgoing.append(msg);
    }

    app->exec();
    delete app;
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

