#include <qtextcodec.h>
#include <qcoreapplication.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/proto/forecast.pb.h>

#include <meteo/commons/global/global.h>

#include <sql/nosql/nosqlmanager.h>

#include <meteo/commons/services/forecast/tforecastdb.h>
#include <meteo/bank/settings/banksettings.h>


#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt   = QStringList() << "?" << "help";
const QStringList kClearOpt  = QStringList() << "c" << "clear";
const QStringList kForecastOpt  = QStringList() << "f" << "forecast";
const QStringList kDataOpt  = QStringList() << "d" << "data";
const QStringList kAccuracyOpt  = QStringList() << "a" << "acuracy";


const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kClearOpt, "Очистить БД перед наполнением.")
  << HELP(kForecastOpt, "Генерация прогнозов.")
  << HELP(kDataOpt, "Генерация данных.")
  << HELP(QStringList(), "")
  << HELP(kAccuracyOpt, "Запросить оправдиваемость прогнозов (формат: punkt_id,method_name)")
     ;



struct NotUsedCol {};

template<class T1,
         class T2,
         class T3=NotUsedCol,
         class T4=NotUsedCol,
         class T5=NotUsedCol,
         class T6=NotUsedCol,
         class T7=NotUsedCol,
         class T8=NotUsedCol,
         class T9=NotUsedCol,
         class T10=NotUsedCol>
class Table
{
public:

  struct Row {
    T1 t1;
    T2 t2;
    T3 t3;
    T4 t4;
    T5 t5;
    T6 t6;
    T7 t7;
    T8 t8;
    T9 t9;
    T10 t10;
  };

  uint64_t insert(const T1& t1 = T1(),
                  const T2& t2 = T2(),
                  const T3& t3 = T3(),
                  const T4& t4 = T4(),
                  const T5& t5 = T5(),
                  const T6& t6 = T6(),
                  const T7& t7 = T7(),
                  const T8& t8 = T8(),
                  const T9& t9 = T9(),
                  const T10& t10 = T10())
  {
    Row row;
    row.t1 = t1;
    row.t2 = t2;
    row.t3 = t3;
    row.t4 = t4;
    row.t5 = t5;
    row.t6 = t6;
    row.t7 = t7;
    row.t8 = t8;
    row.t9 = t9;
    row.t10 = t10;

    ++rowIdCounter_;

    rows_[rowIdCounter_] = row;

    return rowIdCounter_;
  }

  QList<Row> select() const { return rows_.values(); }

private:
  uint64_t rowIdCounter_;
  QHash<uint64_t,Row> rows_;
};


typedef Table<QString,int,int,double,int,QString, int> TableData;
typedef Table<QString,int,int,double,int,int,QString,QString, int> TableForecast;


int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");

  TLog::setMsgHandler(TLog::clearStdOut);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  TAPPLICATION_NAME("meteo");
  ::meteo::gSettings(meteo::bank::Settings::instance());
  if ( false == meteo::bank::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  TArg args(argc, argv);

  if ( args.contains(kHelpOpt, TArg::Flag_Any)  ) {
    kHelp.print();
    return 0;
  }


  if ( args.contains(kClearOpt) ) {
    //TODO
    //info_log << "CLEAR:" << q.exec("SELECT clear_db_forecast(now()::timestamp);");
  }

  // list<location, descr, center, value, hours>
  TableForecast forecasts;
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.5, 21600, 1000, "2016-01-01 00:00", "2016-01-01 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.5, 21600, 1000, "2016-01-02 00:00", "2016-01-02 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.5, 21600, 1000, "2016-01-03 00:00", "2016-01-03 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.5, 21600, 1000, "2016-01-04 00:00", "2016-01-04 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.5, 21600, 1000, "2016-01-05 00:00", "2016-01-05 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 21600, 1000, "2016-01-06 00:00", "2016-01-06 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 21600, 1000, "2016-01-07 00:00", "2016-01-06 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 21600, 1000, "2016-01-08 00:00", "2016-01-06 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 21600, 1000, "2016-01-09 00:00", "2016-01-06 12:00", 100);
  forecasts.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 21600, 1000, "2016-01-10 00:00", "2016-01-06 12:00", 100);

  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-01 00:00", "2016-01-01 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-02 00:00", "2016-01-02 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-03 00:00", "2016-01-03 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-04 00:00", "2016-01-04 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-05 00:00", "2016-01-05 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-06 00:00", "2016-01-06 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-07 00:00", "2016-01-07 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-08 00:00", "2016-01-08 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-09 00:00", "2016-01-09 12:00", 200);
  forecasts.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 21600, 0, "2016-01-10 00:00", "2016-01-10 12:00", 200);

  if ( args.contains(kForecastOpt) ) {
  debug_log << "Inserting data";

    foreach ( const TableForecast::Row& row, forecasts.select() ) {
      meteo::forecast::TForecastDb::add_forecast(row.t1,
                                                 QDateTime::currentDateTime(),
                                                 QDateTime::fromString(row.t7, Qt::ISODate),
                                                 QDateTime::fromString(row.t8, Qt::ISODate),
                                                 row.t2, row.t4, row.t6, row.t9, row.t3, row.t5, "", "");
      info_log << "ADD FORECAST[ ??? ]:"
               << "loc[" << row.t1 << "]"
               << "center[" << row.t2 << "]"
               << "descr[" << row.t3 << "]"
               << "value[" << row.t4 << "]"
               << "hours[" << row.t5 << "]"
               << "level[" << row.t6 << "]"
                  ;
    }
  }

  TableData reals;
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250,  9.9, 1000, "2016-01-01 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 10.0, 1000, "2016-01-02 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 11.0, 1000, "2016-01-03 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 12.0, 1000, "2016-01-04 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 13.0, 1000, "2016-01-05 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 14.0, 1000, "2016-01-06 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 15.0, 1000, "2016-01-07 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 16.0, 1000, "2016-01-08 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 17.0, 1000, "2016-01-09 03:00", 100);
  reals.insert("5b583268068f28e73b0fa27b", 12101, 250, 18.0, 1000, "2016-01-10 03:00", 100);


  reals.insert("5b583268068f28e73b0fa27b", 20013, 250,  20.0, 0, "2016-01-01 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250,  40.0, 0, "2016-01-02 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250,  60.0, 0, "2016-01-03 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250,  80.0, 0, "2016-01-04 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 100.0, 0, "2016-01-05 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 120.0, 0, "2016-01-06 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 140.0, 0, "2016-01-06 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 160.0, 0, "2016-01-07 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 180.0, 0, "2016-01-08 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 200.0, 0, "2016-01-09 03:00", 200);
  reals.insert("5b583268068f28e73b0fa27b", 20013, 250, 220.0, 0, "2016-01-10 03:00", 200);

  if ( args.contains(kDataOpt) ) {   
    foreach ( const TableData::Row& row, reals.select() ) {
      meteo::forecast::TForecastDb::add_forecast_accuracy(row.t1, QDateTime::fromString(row.t6, Qt::ISODate), row.t2, row.t4, row.t5, row.t7, row.t3 );
      //bool r = q.exec(sql.arg(row.t1).arg(row.t2).arg(row.t3).arg(row.t4).arg(row.t5).arg(row.t6));
      info_log << "ADD DATA[ ?? ]:"
               << "loc[" << row.t1 << "]"
               << "center[" << row.t2 << "]"
               << "descr[" << row.t3 << "]"
               << "value[" << row.t4 << "]"
               << "level[" << row.t5 << "]";
    }
  }

  if ( args.contains(kAccuracyOpt) ) {
    QString loc = args.value(kAccuracyOpt).section(",", 0, 0);
    QString method = args.value(kAccuracyOpt).section(",", 1, 1);

    meteo::forecast::AccuracyRequest req;
    req.set_method(method.toUtf8().constData());
    req.mutable_punkt()->set_index(loc.toStdString());
    debug_log << req.Utf8DebugString();

    meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
    if ( 0 == ch ) {
      error_log << QObject::tr("Не удалось подключиться к сервису.");
      return -1;
    }

    meteo::forecast::AccuracyReply* reply = ch->remoteCall(&meteo::forecast::ForecastData::GetAccuracy, req, 30000);
    if ( 0 == reply ) {
      error_log << QObject::tr("Первышено время ожидания ответа от сервиса.");
      return -1;
    }

    var(reply->Utf8DebugString());
  }

  return 0;
}
