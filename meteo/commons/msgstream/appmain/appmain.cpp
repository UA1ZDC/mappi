#include <qthread.h>
#include <qcoreapplication.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/msgstream/streamapp.h>
#include <meteo/commons/msgstream/plugins/ftpstream/ftpstreamin.h>
#include <meteo/commons/msgstream/plugins/ftpstream/ftpstreamout.h>
#include <meteo/commons/msgstream/plugins/filestream/filestreamin.h>
#include <meteo/commons/msgstream/plugins/filestream/filestreamout.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512streamclient.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512streamserver.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512receiver.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512sender.h>

#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2gstreamclient.h>
#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2gstreamserver.h>

#include <meteo/commons/msgstream/plugins/socketspecial/unimasstreamserver.h>
#include <meteo/commons/msgstream/plugins/socketspecial/unimasstreamclient.h>
//#include <meteo/commons/msgstream/plugins/socketspecial/unimasserver.h>
#include <meteo/commons/msgstream/plugins/ptkpp/ptkppstreamrecv.h>
#include <meteo/commons/msgstream/plugins/udpstream/udpstreamin.h>
#include <meteo/commons/msgstream/plugins/udpstream/udpstreamout.h>

#include <meteo/commons/msgstream/plugins/cliwarestream/cliwarestream.h>
#include <meteo/commons/msgstream/plugins/esimostream/esimostream.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt      = QStringList() << "h" << "help";

const QStringList kIdOpt        = QStringList() << "id";
const QStringList kProtocolOpt  = QStringList() << "protocol";
const QStringList kReceiverOpt  = QStringList() << "receiver";
const QStringList kSenderOpt    = QStringList() << "sender";
const QStringList kSenderReceiverOpt    = QStringList() << "senderReceiver";
const QStringList kReconnectOpt = QStringList() << "reconnect_timeout";
const QStringList kCenterWindowOpt = QStringList() << "center_window";
const QStringList kMaxQueueOpt  = QStringList() << "max_queue";
const QStringList kRuleOpt      = QStringList() << "rule";
const QStringList kParseOpt   = QStringList() << "file_parse_mode";

const QStringList kPredefT1  = QStringList() << "predef_t1";
const QStringList kPredefT2  = QStringList() << "predef_t2";
const QStringList kPredefA1  = QStringList() << "predef_a1";
const QStringList kPredefA2  = QStringList() << "predef_a2";
const QStringList kPredefii  = QStringList() << "predef_ii";
const QStringList kPredefCCCC  = QStringList() << "predef_cccc";
const QStringList kPredefMsgType  = QStringList() << "predef_msgtype";
const QStringList kSplitStucked   = QStringList() << "split_stucked";

const QList<QStringList> kFileRecv = QList<QStringList>()
 << HELP("file_recv.path",             "Путь к директории c файлами")
 << HELP("file_recv.file_filter",      "Шаблон имени файлов для обработки (например, *.tlg)")
 << HELP("file_recv.path_filter",      "Шаблон имени пути для обработки (например, /pub/data/grib.*)")
 << HELP("",                           " ? - один любой символ")
 << HELP("",                           " * - любой символ встречающийся 0 или более раз")
 << HELP("file_recv.delete_files",     "Удаляnm обработанные файлы")
 << HELP("file_recv.meta_template",    "Шаблон для извлечения метаинформации из имени файла")
 << HELP("",                           " YYYY - года (4 цифры)")
 << HELP("",                           " YY - текущий года (последние 2 цифры)")
 << HELP("",                           " MM - месяц с ведущим нулём (01-12)")
 << HELP("",                           " DD - день с ведущим нулём (01-31)")
 << HELP("",                           "  J  - день года (001-366)")
 << HELP("",                           " hh - час с ведущим нулём (00-23)")
 << HELP("",                           " mm - минуты с ведущим нулём (00-59)")
 << HELP("",                           " YYGGgg - срок формирования")
 << HELP("",                           " T1 - ")
 << HELP("",                           " T2 - ")
 << HELP("",                           " A1 - ")
 << HELP("",                           " A2 - ")
 << HELP("",                           " ii - ")
 << HELP("",                           " CCCC - ")
 << HELP("",                           " Символ '*' заменяет один любой символ.")
 << HELP("",                           "Пример: /path/**BG/{YYYY}{MM}{DD}")
 << HELP("file_recv.wait_before_read", "Ожидание между временем последней модификации файла и началом чтения (сек.)")
;

const QList<QStringList> kFileSend = QList<QStringList>()
 << HELP("file_send.path",         "Путь к директории куда будут сохраняться файлы")
 << HELP("file_send.mask",         "Шаблон имени создаваемых файлов")
 << HELP("",                       " (по умолчанию равен '{YYYY}{MM}{DD}_{ID}.tlg')")
 << HELP("",                       " ID - идентификатор")
 << HELP("",                       " YYYY - текущий года (4 цифры)")
 << HELP("",                       " MM - месяц с ведущим нулём (01-12)")
 << HELP("",                       " DD - день с ведущим нулём (01-31)")
 << HELP("",                       " hh - час с ведущим нулём (00-23)")
 << HELP("",                       " mm - минуты с ведущим нулём (00-59)")
 << HELP("",                       " ss - секунды с ведущим нулём (00-59)")
 << HELP("file_send.msg_limit",    "Ограничение на количество телеграмм в одном файле")
 << HELP("file_send.size_limit",   "Ограничение на размер файла (в байтах)")
 << HELP("file_send.time_limit",   "Ограничение на время, в течении которого, производится запись в файла (в секундах)")
 << HELP("file_send.content_only", "Удалять из телеграммы начальную строку, заголовок, завершающие символы")
;

const QList<QStringList> kFtpRecv = QList<QStringList>()
<< HELP("ftp_recv.server.host",     "Адрес ftp-сервера")
<< HELP("ftp_recv.server.port",     "Порт ftp-сервера")
<< HELP("ftp_recv.server.user",     "Имя пользователя")
<< HELP("ftp_recv.server.password", "Пароль")
<< HELP("ftp_recv.server.mode",     "Режим подключения к ftp-серверу (kActiveFtpMode|kPassiveFtpMode)")
<< HELP("ftp_recv.server.transfer_type",     "Режим передачи ftp-сервера (kBinaryTransferType|kAsciiTransferType)")
<< HELP("ftp_recv.path",            "Путь к директории c файлами")
<< HELP("ftp_recv.file_filter",     "Шаблон имени файлов, которые необходимо обрабатывать")
<< HELP("ftp_recv.path_filter",     "Маска путей, которые необходимо обрабатывать")
<< HELP("ftp_recv.delete_files",    "Флаг удаления обработанных файлов")
<< HELP("ftp_recv.meta_template",   "Шаблон для извлечения метаинформации из имени файла")
<< HELP("ftp_recv.wait_before_read","Ожидание, в секундах, между временем последней модификации файла и началом чтения")
<< HELP("ftp_recv.session_timeout", "Ожидание, в секундах, между сеансами работы")
<< HELP("ftp_recv.file_size_limit", "Файлы больше заданного размера не будут загружаться (Мбайт)")
<< HELP("ftp_recv.skip_older",      "Не загружать файлы, дата создания которых старше указанной (ISO)")
;

const QList<QStringList> kFtpSend = QList<QStringList>()
<< HELP("ftp_send.server.host",     "Адрес ftp-сервера")
<< HELP("ftp_send.server.port",     "Порт ftp-сервера")
<< HELP("ftp_send.server.user",     "Имя пользователя")
<< HELP("ftp_send.server.password", "Пароль")
<< HELP("ftp_send.server.mode",     "Режим подключения к ftp-серверу (kActiveFtpMode|kPassiveFtpMode)")
<< HELP("ftp_recv.server.transfer_type",     "Режим передачи ftp-сервера (kBinaryTransferType|kAsciiTransferType)")
<< HELP("ftp_send.path",            "Путь сохранения файлов")
<< HELP("ftp_send.mask",            "Шаблон имени создаваемых файлов")
<< HELP("",                         " (по умолчанию равен '{YYYY}{MM}{DD}_{ID}.tlg')")
<< HELP("",                         " ID - идентификатор")
<< HELP("",                         " YYYY - текущий года (4 цифры)")
<< HELP("",                         " MM - месяц с ведущим нулём (01-12)")
<< HELP("",                         " DD - день с ведущим нулём (01-31)")
<< HELP("",                         " hh - час с ведущим нулём (00-23)")
<< HELP("",                         " mm - минуты с ведущим нулём (00-59)")
<< HELP("",                         " ss - секунды с ведущим нулём (00-59)")
<< HELP("ftp_send.msg_limit",       "Ограничение на количество телеграмм в одном файле")
<< HELP("ftp_send.size_limit",      "Ограничение на размер файла (в байтах)")
<< HELP("ftp_send.time_limit",      "Ограничение на время, в течении которого, производится запись в файла (в секундах)")
<< HELP("ftp_send.content_only",    "Удалять из телеграммы начальную строку, заголовок, завершающие символы")
;

const QList<QStringList> kSockSpec = QList<QStringList>()
<< HELP("sockspec.mode",            "Режим информационного обмена (SOCKSPEC_TCP,SOCKSPEC_UTCP)")
<< HELP("sockspec.host",            "Адрес сервера")
<< HELP("sockspec.port",            "Порт сервера")
<< HELP("sockspec.session_timeout", "Ожидание между сеансами работы (в секундах)")
<< HELP("sockspec.ack_timeout",     "Время ожидания подтвердения получения телеграммы")
<< HELP("sockspec.compress_mode",   "Режим сжатия (COMPRESS_NONE,COMPRESS_OPTIM,COMPRESS_ALWAYS)")
<< HELP("sockspec.client",          "Запуск в режиме клиента (true,false)")
;

const QList<QStringList> kSriv512 = QList<QStringList>()
<< HELP("sriv512.host",           "Адрес сервера (при запуске в режиме клиента)")
<< HELP("sriv512.port",           "Порт сервера")
<< HELP("sriv512.ack_waiting",    "Время ожидания подтвердения получения телеграммы")
<< HELP("sriv512.window",         "Количество сообщений передаваемых без ожидания подтверждения")
<< HELP("sriv512.mode_client",    "Запуск в режиме клиента (true,false)")
<< HELP("sriv512.compress_optim", "Включение сжатия передаваемых данных")
<< HELP("sriv512.max_len",        "Максимальный размер отправляемого сообщения")
;

const QList<QStringList> kSocket2G = QList<QStringList>()
<< HELP("socksp2g.host",           "Адрес сервера (при запуске в режиме клиента)")
<< HELP("socksp2g.port",           "Порт сервера")
<< HELP("socksp2g.ack_waiting",    "Время ожидания подтвердения получения телеграммы")
<< HELP("socksp2g.window",         "Количество сообщений передаваемых без ожидания подтверждения")
<< HELP("socksp2g.mode_client",    "Запуск в режиме клиента (true,false)")
<< HELP("socksp2g.compress_optim", "Включение сжатия передаваемых данных")
<< HELP("socksp2g.max_len",        "Максимальный размер отправляемого сообщения")
<< HELP("socksp2g.path",            "Путь сохранения файлов")
;

const QList<QStringList> kUdp = QList<QStringList>()
<< HELP("udp.host",           "Адрес сервера (при запуске в режиме клиента)")
<< HELP("udp.port",           "Порт сервера")
<< HELP("udp.meta_template",   "Шаблон для извлечения метаинформации из имени файла")
<< HELP("",                       " (по умолчанию равен '{YYYY}{MM}{DD}_{ID}.tlg')")
<< HELP("",                       " ID - идентификатор")
<< HELP("",                       " YYYY - текущий года (4 цифры)")
<< HELP("",                       " MM - месяц с ведущим нулём (01-12)")
<< HELP("",                       " DD - день с ведущим нулём (01-31)")
<< HELP("",                       " hh - час с ведущим нулём (00-23)")
<< HELP("",                       " mm - минуты с ведущим нулём (00-59)")
<< HELP("",                       " ss - секунды с ведущим нулём (00-59)")
<< HELP("udp.compressionMode",    "Метод сжатия: COMPRESS_NONE, COMPRESS_OPTIM, COMPRESS_ALWAYS")
<< HELP("udp.chunk_size",    "Размер UDP пакета (1024)")
<< HELP("udp.need_fec",    "Необходимость избыточного кодирования true/false")
<< HELP("udp.expire_time",    "Время устаревания сообщения. По истечению expire_time пакет удаляется из очереди (600 сек)")
<< HELP("udp.state_interval",    "Время перепроверки соединения (60 сек)")
<< HELP("udp.read_interval",    "Время перепроверки очереди входящих сообщений (1000 мсек)")
<< HELP("udp.time_to_last_activity",    "")
<< HELP("udp.msg_limit",    "Ограничение на количество телеграмм в одном файле")
<< HELP("udp.size_limit",   "Ограничение на размер файла (в байтах)")
<< HELP("udp.time_limit",   "Ограничение на время, в течении которого, производится запись в файл (в секундах)")
<< HELP("udp.content_only", "Удалять из телеграммы начальную строку, заголовок, завершающие символы")
<< HELP("udp.send_try", "Количество повторов при отправке сообщения (1)")
;

const QList<QStringList> kPtkpp = QList<QStringList>()
//<< HELP("ptkpp.","")
;

const QList<QStringList> kCliWareRecv = QList<QStringList>()
<< HELP("mcd_recv.server.host",     "Адрес сервера")
;

const QList<QStringList> kESIMORecv = QList<QStringList>()
<< HELP("file_recv.path",             "Путь к директории c файлами netCDF")
<< HELP("esimo_recv.server.host",     "Адрес сервера")
;

const HelpFormatter kHelp = HelpFormatter()
    << HELP(kHelpOpt, "Эта справка.")
    << HELP(QStringList(), "")
    << HELP(kReceiverOpt, "Запуск в режиме приёма данных.")
    << HELP(kSenderOpt, "Запуск в режиме передачи данных.")
    << HELP(kProtocolOpt, "Один из следующих типов протоколов: file, ftp, socketspecial, sriv512, udp, ptkpp, socksp2g.")
    << HELP(kIdOpt, "Идентификатор потока.")
    << HELP(kReconnectOpt, "Время ожидания между попытками подключения к ЦКС (секунды, по умолчанию 5)")
    << HELP(kMaxQueueOpt, "При достижении максимального размера очереди отправки в msgcenter приём приостанавливается.")
    << HELP(kRuleOpt, "Название правила для парсера")
    << HELP(kParseOpt, "Режим выделения сообщений из потока")
    << HELP("", "PARSE - выделять отдельные сообщения из потока данных")
    << HELP("", "NO_PARSE - не выделять отдельные сообщения из потока данных(ожидается, что каждое сообщение располагается в отдельном файле")
    << HELP("", "SPLIT - разбивать файл на отдельные сообщения без декодирования(ожидается, что на вход приходят отдельные файлы)")
    << HELP(kCenterWindowOpt, "Количество сообщений передаваемых в ЦКС без ожидания подтверждения.")
    << HELP("", "")
    << HELP(kPredefT1, "Устанавливает поле T1, если оно не заполнено")
    << HELP(kPredefT2, "Устанавливает поле T2, если оно не заполнено")
    << HELP(kPredefA1, "Устанавливает поле A1, если оно не заполнено")
    << HELP(kPredefA2, "Устанавливает поле A2, если оно не заполнено")
    << HELP(kPredefii, "Устанавливает поле ii, если оно не заполнено")
    << HELP(kPredefCCCC, "Устанавливает поле CCCC, если оно не заполнено")
    << HELP(kPredefMsgType, "Устанавливает поле msgtype")
    << HELP(kSplitStucked, "Разделять склееные сводки GRIB/BUFR на отдельные телеграммы (не работает в sriv512)")
    << HELP("", "")
    << HELP("", "")
    << HELP("", "---------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА FILE (приём):")
    << kFileRecv
    << HELP("", "")
    << HELP("", "")
    << HELP("", "------------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА FILE (передача):")
    << kFileSend
    << HELP("", "")
    << HELP("", "")
    << HELP("", "--------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА FTP (приём):")
    << kFtpRecv
    << HELP("", "")
    << HELP("", "")
    << HELP("", "-----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА FTP (передача):")
    << kFtpSend
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА SOCKETSPECIAL:")
    << kSockSpec
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА SOCKETSPECIAL 2 Gen:")
    << kSocket2G
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА SRIV512:")
    << kSriv512
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА PTKPP:")
    << kPtkpp
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА UDP:")
    << kUdp
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА CliWare ВНИИГМИ-МЦД:")
    << HELP("sockspec.session_timeout", "Ожидание между запросами (в секундах)")
    << HELP(kMaxQueueOpt, "При достижении этого значения в очереди передачи запросов, передача запросов приостанавливается.")
    << kCliWareRecv
    << HELP("", "")
    << HELP("", "")
    << HELP("", "----------------------------------")
    << HELP("", "ПАРАМЕТРЫ ПРОТОКОЛА ESIMO:")
    << HELP("sockspec.session_timeout", "Ожидание между запросами (в секундах)")
    << HELP(kMaxQueueOpt, "При достижении этого значения в очереди передачи запросов, передача запросов приостанавливается.")
    << kESIMORecv
    ;


const QStringList kProtocolList = { "file", "ftp", "socketspecial", "socksp2g", "sriv512", "ptkpp", "udp", "cliware", "esimo" };

int appMain( int argc, char** argv )
{
  try {
    TAPPLICATION_NAME( "meteo" );
    meteo::global::setLogHandler();
    ::meteo::gSettings(meteo::global::Settings::instance());
    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    QCoreApplication* app = new QCoreApplication(argc, argv);
    QObjectDeleter deleter(app,false);

    TArg args( argc, argv );

    if ( args.contains(kHelpOpt) ) {
      kHelp.print();
      return EXIT_SUCCESS;
    }

    if ( !args.contains(kProtocolOpt) ) {
      error_log << QObject::tr("Необходимо указать протокол.");
      return EXIT_FAILURE;
    }

    QString protocol = args.value(kProtocolOpt);
    if ( !kProtocolList.contains(protocol) ) {
      error_log << QObject::tr("Протокол '%1' не поддерживается.").arg(protocol);
      return EXIT_FAILURE;
    }

    ::meteo::msgstream::Options opt;
    PbHelper helper(&opt);
    QStringList keys = args.keys();
    // удаляем ключи, которых нет в proto-структуре
    keys.removeAll("sender");
    keys.removeAll("receiver");
    for ( const QString& o : keys ) {
      QString s = args.value(o);
      helper.setValue(o.split("."), s);
    }

    bool isReceiver = args.contains(kReceiverOpt);
    bool isSender = args.contains(kSenderOpt);
    bool oneOf = (isReceiver || isSender)/*&& !(isReceiver && isSender)*/;
    if( !oneOf ){
      error_log << QObject::tr("Необходимо указать --receiver или --sender");
      return EXIT_FAILURE;
    }
    if ( true == args.contains(kSplitStucked) ) {
      opt.set_split_stucked(true);
    }
    if ( args.contains(kSenderReceiverOpt) ) {
        opt.set_direction(::meteo::msgstream::kSenderReceiver);
      } else{
        ::meteo::msgstream::Direction direct = isSender ? ::meteo::msgstream::kSender : ::meteo::msgstream::kReceiver;
        opt.set_direction(direct);
      }
    // определяем имя модуля
    if ( isSender ) {
      if ( protocol == "file"               ) { opt.set_module("file-send");      }
      else if ( protocol == "ftp"           ) { opt.set_module("ftp-send");       }
      else if ( protocol == "socketspecial" ) { opt.set_module("socketspecial");  }
      else if ( protocol == "sriv512"       ) { opt.set_module("sriv512");        }
      else if ( protocol == "socksp2g"       ) { opt.set_module("socksp2g");        }
      else if ( protocol == "udp"       ) { opt.set_module("udp-send");        }
      else if ( protocol == "ptkpp"         )
      {
        error_log << QObject::tr("Протокол PTKPP не поддерживает передачу телеграмм");
        return EXIT_FAILURE;
      }
      else if ( protocol == "cliware"         ) {
        error_log << QObject::tr("Протокол CliWare не поддерживает передачу телеграмм");
        return EXIT_FAILURE;
      }
      else if ( protocol == "esimo"         ) {
        error_log << QObject::tr("Протокол ESIMO не поддерживает передачу телеграмм");
        return EXIT_FAILURE;
      }
    }
    else {
      if ( protocol == "file"               ) { opt.set_module("file-recv");      }
      else if ( protocol == "ftp"           ) { opt.set_module("ftp-recv");       }
      else if ( protocol == "socketspecial" ) { opt.set_module("socketspecial");  }
      else if ( protocol == "sriv512"       ) { opt.set_module("sriv512");        }
      else if ( protocol == "socksp2g"       ) { opt.set_module("socksp2g");        }

      else if ( protocol == "ptkpp"         ) { opt.set_module("ptkpp-recv");     }
      else if ( protocol == "udp"           ) { opt.set_module("udp-recv");       }
      else if ( protocol == "cliware"       ) { opt.set_module("cliware");        }
      else if ( protocol == "esimo"         ) { opt.set_module("esimo");          }
    }

    if ( args.contains(kIdOpt) ) {
      opt.set_id(args.value(kIdOpt).toStdString());
    }

    ::meteo::StreamModule* module = nullptr;

    QString mid = QString::fromStdString(opt.module());

    if ( ::meteo::FtpStreamIn::moduleId() == mid ) {
      module = new ::meteo::FtpStreamIn;
    }
    else if ( ::meteo::FtpStreamOut::moduleId() == mid ) {
      module = new ::meteo::FtpStreamOut;
    }
    else if ( ::meteo::FileStreamIn::moduleId() == mid ) {
      module = new ::meteo::FileStreamIn;
    }
    else if ( ::meteo::FileStreamOut::moduleId() == mid ) {
      module = new ::meteo::FileStreamOut;
    }
    else if ( ::meteo::Sriv512StreamClient::moduleId() == mid ) {
      if ( opt.sriv512().mode_client() ) {
       module = new meteo::Sriv512StreamClient;
       //module->setHost(QString::fromUtf8(opt.sriv512().host().c_str()));
      }
      else {
        module = new meteo::Sriv512StreamServer;
      }
    }
    else if ( ::meteo::Socksp2gStreamClient::moduleId() == mid ) {
      if ( opt.socksp2g().mode_client() ) {
       module = new meteo::Socksp2gStreamClient;
      }
      else {
        module = new meteo::Socksp2gStreamServer;
      }
    }
    else if ( ::meteo::UnimasStreamClient::moduleId() == mid ) {
      if ( true == opt.sockspec().client() ) {
        module = new ::meteo::UnimasStreamClient;
      }
      else {
        module = new ::meteo::UnimasStreamServer;
      }
    }
    else if ( ::meteo::PtkppStreamRecv::moduleId() == mid ) {
      module = new ::meteo::PtkppStreamRecv;
    }
    else if ( ::meteo::UdpStreamOut::moduleId() == mid ) {
      module = new ::meteo::UdpStreamOut;
    }
    else if ( ::meteo::UdpStreamIn::moduleId() == mid ) {
      module = new ::meteo::UdpStreamIn;
    }
    else if ( ::meteo::CliWareStream::moduleId() == mid ) {
      module = new ::meteo::CliWareStream;
    }
    else if ( ::meteo::ESIMOStream::moduleId() == mid ) {
      module = new ::meteo::ESIMOStream;
    }
    else {
      error_log << QObject::tr("Модуль '%1' не входит в состав %2.").arg(mid,qApp->applicationName());
      return EXIT_FAILURE;
    }


    none_log << QObject::tr("\nПАРАМЕТРЫ ЗАПУСКА:\n") << opt.Utf8DebugString();


    QThread* thread = new QThread;
    QObject::connect( thread, &QThread::started, module, &meteo::StreamModule::slotInit );

    module->setOptions(opt);
    module->moveToThread(thread);

    ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;
    status->setUpdateLimit(1);
    status->setSendLimit(50);

    ::meteo::StreamApp* stream = new ::meteo::StreamApp;
    stream->setStatus(status);
    module->setStatus(status);


    if ( !stream->init(opt) ) {
      delete stream;
      delete status;
      delete thread;
      delete module;
      return EXIT_FAILURE;
    }
    TSList<meteo::tlg::MessageNew> incoming;
    stream->setIncoming(&incoming);
    module->setIncoming(&incoming);
    QObject::connect( module, &::meteo::StreamModule::newIncomingMsg, stream, &meteo::StreamApp::slotSendToCenter );

    TSList<meteo::tlg::MessageNew> outgoing;
    stream->setOutgoing(&outgoing);
    module->setOutgoing(&outgoing);
    QObject::connect( stream, &::meteo::StreamApp::newOutgoingMsg, module, &meteo::StreamModule::slotNewIncoming );

    status->start();
    thread->start();

    app->exec();

    thread->quit();
    thread->wait(5000);
    thread->terminate();

    status->terminate();

    delete thread;
    delete stream;
    delete status;
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

