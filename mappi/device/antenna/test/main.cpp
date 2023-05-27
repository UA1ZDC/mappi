#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellite.h>
#include <mappi/global/global.h>
#include <mappi/schedule/schedule.h>
#include <mappi/device/antenna/antenna.h>
#include <mappi/device/antenna/scriptchain.h>
#include <qcoreapplication.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>

#define SCHEDULE_FILE (MnCommon::varPath("mappi") + "schedule.txt")
#define TLE_FILE (MnCommon::varPath("mappi") + "weather.txt")
#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")


using namespace mappi;
using namespace antenna;


static struct option long_options[] = {
  { "script",   required_argument, nullptr, 's' },
  { "azimut",   required_argument, nullptr, 'a' },
  { "dsa",      required_argument, nullptr, 'a' },
  { "elevat",   required_argument, nullptr, 'e' },
  { "dse",      required_argument, nullptr, 'e' },
  { "help",     no_argument,       nullptr, 'h' },
  { "fh",       required_argument, nullptr, 'f' },
  { "fh_out",   required_argument, nullptr, 'o' },
  { nullptr,    0,                 nullptr,  0 }
};


void usage(const QString& app);
bool initTrack(const Configuration& conf, QList<MnSat::TrackTopoPoint>& track);


int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  bool showUsage = false;
  QString scriptName;
  float azimut = 0;
  float elevat = 0;
  QList<MnSat::TrackTopoPoint> track;

  bool fhTurnOn = false;
  int fhOutputN = -1;

  int value = 0;
  while ((value = ::getopt_long(argc, argv, ":s:a:e:f:o:h", long_options, 0)) != -1) {
    switch (value) {
      case 's' : scriptName = ::optarg;        break;
      case 'h' : showUsage = true;             break;
      case 'a' : azimut = ::atof(::optarg);    break;
      case 'e' : elevat = ::atof(::optarg);    break;
      case 'f' : fhTurnOn = ::atoi(::optarg);  break;
      case 'o' : fhOutputN = ::atoi(::optarg); break;

      case ':' :
        error_log << QObject::tr("Не задан аргумент для -%1, выход").arg((char)::optopt);
        return EXIT_FAILURE;

      case '?' :
      default :
        info_log << QObject::tr("Неизвестный опция -%1, ошибка игнорируется").arg((char)::optopt);
        break;
    }
  };

  if (showUsage) {
    usage(app.applicationName());
    return EXIT_SUCCESS;
  }

  info_log << QObject::tr("Конфигурационный файл: %1").arg(CONF_FILE);
  Configuration conf;
  if (conf.load(CONF_FILE) == false) {
    error_log << QObject::tr("Не удалость загрузить конфигурационный файл");
    return EXIT_FAILURE;
  }
  debug_log << conf.toString();

  Antenna antenna;
  if (antenna.init(conf) == false) {
    error_log << QObject::tr("Ошибка инициализации антенны");
    return false;
  }

  if (antenna.open() == false) {
    error_log << QObject::tr("Не удалость открыть порт");
    return false;
  }

  ScriptChain chain(&antenna);
  QObject::connect(&chain, &ScriptChain::completed, qApp, [&antenna]() {
    Drive azimut;
    Drive elevat;
    Feedhorn feedhorn;

    antenna.snapshot(&azimut, &elevat, &feedhorn);
    info_log << azimut.toString() << elevat.toString();
    info_log << feedhorn.toString();

    qApp->quit();
  });

  QObject::connect(&chain, &ScriptChain::failed, []() {
    error_log << QObject::tr("Текущее положение антенны не опредлено");
    qApp->quit();
  });

  // feedhorn
  if (fhTurnOn)
    antenna.feedhorn(true);

  // feedhorn output
   if (0 < fhOutputN)
    antenna.feedhornOutput(static_cast<Feedhorn::output_t>(fhOutputN), true);

  chain.appendCatcher();
  if (scriptName == "direct")
    chain.appendDirect(azimut, elevat, false);

  else if (scriptName == "tracker") {
    if (initTrack(conf, track))
      chain.appendTracker(track);

  } else if (scriptName == "monkeytest")
    chain.appendMonkeyTest(azimut, elevat);

  chain.run();

  return app.exec();
}


void usage(const QString& app)
{
  QStringList list;
  list.append(QObject::tr("использование: %1 [option]\nопции:").arg(app));
  list.append(QObject::tr("\t-s, --script сценарий работы (tracker|direct|monkeytest);"));
  list.append(QObject::tr("\t-a, --azimut азимут, градусы;"));
  list.append(QObject::tr("\t-a, --dsa(degree per second azimut) азимут, градусы;"));
  list.append(QObject::tr("\t-e, --elevat угол места, градусы;"));
  list.append(QObject::tr("\t-e, --dse(degree per second elevat) угол места, градусы;"));
  list.append(QObject::tr("\t-f, --fh(feedhorn) управление питанием облучателя (1 -вкл/0 - выкл);"));
  list.append(QObject::tr("\t-o, --fh_out(feedhorn output) активировать дополнительный выход (1 - 4);"));
  list.append(QObject::tr("\t-h, --help показать справку и выйти.\n"));

  std::cout << list.join('\n').toStdString();
}

bool initTrack(const Configuration& conf, QList<MnSat::TrackTopoPoint>& track)
{
  // QByteArray buf;
  // while (std::cin)
  //   buf.append(std::cin.get());

  schedule::Schedule schedule;
  if (schedule.load(SCHEDULE_FILE) == false) {
    error_log << QObject::tr("Не удалось загрузить расписание: %1").arg(SCHEDULE_FILE);
    return false;
  }
  info_log << schedule.toNumberString().prepend('\n');

  int idx = -1;
  std::cout << "Введите номер сеанса: ";
  std::cin >> idx;

  schedule::Session& session = schedule.getSession(idx);
  debug_log << session.data().toString();

  Satellite satellite;
  if (satellite.readTLE(session.data().satellite, TLE_FILE) == false) {
    error_log << QObject::tr("Не удалось прочитать tle-файл: %1").arg(TLE_FILE);
    return false;
  }

  bool isOk = satellite.trajectory(session.data().aos,
    session.data().los,
    conf.profile.timeSlot,
    conf.point,
    track
  );

  if (!isOk) {
    error_log << QObject::tr("Ошибка расчета траектории");
    return false;
  }

  Quarter::sequence_t seq = Quarter::ident(track);
  debug_log << "original" << Quarter::info(track, seq);

  Quarter::correction(track, MnMath::deg2rad(conf.azimutCorrect));
  seq = Quarter::ident(track);
  debug_log << "after correction" << Quarter::info(track, seq);

  isOk = Quarter::adaptation(track, seq);
  if (!isOk) {
    error_log << QObject::tr("Недопустимая траектория сопровождения");
    return false;
  }

  debug_log << "after adaptation" << Quarter::info(track, seq);

  return true;
}
