#include "thematicservice.h"
#include "thematichandler.h"

#include <qthread.h>
#include <qtimer.h>

#include <cross-commons/app/paths.h>
#include <meteo/commons/rpc/rpc.h>


#include <mappi/settings/mappisettings.h>
// #include <mappi/services/sessiondataservice/sessiondataservice.h>
// #include <mappi/proto/thematic.pb.h>
// #include <mappi/proto/satellite.pb.h>
// #include <mappi/proto/sessiondataservice.pb.h>

// #include <mappi/thematic/algs/themformat.h>
// #include <mappi/thematic/algs/themalg.h>

//static const QString kThematicConf = MnCommon::etcPath("mappi") + "thematics.conf";
// static const QString kThematicPath = MnCommon::varPath("mappi") + "thematics/%1";
// static const QString kPreatrementPath = MnCommon::varPath("mappi") + "thematics/%1";

using namespace mappi;

ThematicService::ThematicService(QObject* parent):
  QObject(parent),
  handler_(new ThematicHandler())
{
  QThread* thread = new QThread;
  handler_->moveToThread(thread);
  QObject::connect(thread,   &QThread::started,           handler_, &ThematicHandler::run);
  //connect(handler_, &ThematicHandler::complete,  this,     &ThematicProcessor::slotInit);
  QObject::connect(handler_, &ThematicHandler::finished,  handler_, &ThematicHandler::deleteLater);
  QObject::connect(handler_, &ThematicHandler::destroyed, thread,   &QThread::quit);
  QObject::connect(thread,   &QThread::finished,  thread,   &QThread::deleteLater);
  thread->start();

  init();
}

ThematicService::~ThematicService()
{
}

/*!
 * \brief ThematicProcessor::slotInit считывает тематические обработки и необработанные сеансы
 */
void ThematicService::init()
{
  conf::ThematicProcs conf;
  //conf.CopyFrom(mappi::inter::Settings::instance()->thematics());
  conf.CopyFrom(mappi::inter::Settings::instance()->thematicsCalc());
  if (!conf.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла настроек тематических обработок");
    return;
  }
  
 /* for(const auto &them : conf.thematics()) {
    QList<conf::ThemType> typelist;

    for (auto type: them.type()) {
      typelist.append(conf::ThemType(type));
    }
    
    _conf.insert(QString::fromStdString(them.satellite()), typelist);
    // debug_log << them.name() << them.Utf8DebugString();
  }*/

  QMap< std::string,conf::ThemType> typelist;

  for(const auto &them : conf.thematics()) {
    for(const auto &vars : them.vars()) {
      for(const auto &channel : vars.channel()) {
        //if (satname == QString::fromStdString(channel.satellite())) {
          //if(false == typelist->contains(them.type())){
            typelist.insert(them.name(),them.type());
            _conf.insert(QString::fromStdString(channel.satellite()), typelist);

          //}
        //  break;
        //}
      }
    }
  }
  //debug_log << satname << *typelist ;






  // proto::SessionData req;
  // //TODO пока так, чтоб старые данные тоже забрать
  // // req.set_date_start(QDateTime::currentDateTimeUtc().addSecs(-10 * 24 * 60 * 60).toString(Qt::ISODate).toStdString());
  // //  req.set_date_end(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString());
  // req.set_thematic_done(false);
  // req.set_processed(true);

  // meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
  // if (nullptr == ch) return;
  
  // auto* sessions = ch->remoteCall(&mappi::proto::SessionDataService::GetAvailableSessions, req, 60 * 1000, true);
  // if(nullptr != sessions)
  //   {
  //     for(auto session : sessions->sessions()) {
  //       if(session.has_sat_name()) {
  //         unprocsessions_.append(session);
  //       }
  //     }
  //     delete sessions;
  //   }
  
  // delete ch;
  

  // if(unprocsessions_.isEmpty()) {
  //   QTimer::singleShot(1 * 60 * 1000, this,  SLOT(slotInit()));
  // }
  // else {
  //   processing();
  // }
}

void ThematicService::PerformThemProcess(::google::protobuf::RpcController* ctrl,
					 const mappi::conf::PerformThemRequest* req,
					 mappi::conf::ThemResponse* res,
					 ::google::protobuf::Closure* done)
{
  trc;
  Q_UNUSED(ctrl);

  if ((!req->has_sat_name() || !req->has_date_start() ||
       req->sat_name().size() == 0 || req->date_start().size() == 0
       /*||   req->instr_size() == 0*/)) {
    res->set_comment(QObject::tr("Неверно указаны параметры запуска").toStdString());
    res->set_result(false);
    done->Run();
    return;
  }

  var(req->Utf8DebugString());

  QString satname = QString::fromStdString(req->sat_name());
  QDateTime start = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
  QList<conf::InstrumentType> instruments;
  for (auto instr : req->instr()) {
    instruments.append(conf::InstrumentType(instr));
  }
    
  if (!addTask(satname, start, instruments)) {
    res->set_comment(QObject::tr("Нет настроек для выполнения тематической обработки. Спутник %1").arg(satname).toStdString());
    res->set_result(false);
    done->Run();
    return;
  }
  
  res->set_result(true);
  done->Run();
}

/*!
 * \brief ThematicProcessor::processing вычисление тематических обработак
 */
bool ThematicService::addTask(const QString& satname, const QDateTime& start, const QList<conf::InstrumentType>& instruments)
{
  if (!_conf.contains(satname)) {
    warning_log << QObject::tr("Для спутника %1 нет настроек для тематической обработки").arg(satname);
    return false;
  }
  
  ThematicTask task;
  task.satellite = satname;
  task.start = start;
  task.instruments = instruments;
  task.types = _conf.value(satname);
  handler_->addTask(task);
  return true; 
}

// bool ThematicProcessor::getChannelForThematic(ThematicTask& task, const conf::ThematicProc& them)
// {
//   mappi::conf::Instrument instrument;
//   ::mappi::inter::Settings::instance()->instrument(QString(task.session.sat_name().data()), &instrument);
//   QSet<QString> channels;
//   // Заполняем список каналов инструмента
//   for(const conf::ThematicVariable themVar : them.vars())
//   {
//     for(const conf::ThematicChannel ch  : themVar.channel())
//     {
//       if(isValid(ch) && ch.instrument() == instrument.type() && 0 == ch.satellite().compare(task.session.sat_name()))
//       {
//         QString chAlias(ch.channel().data());
//         if(false == task.channelFiles_.contains(chAlias)) {
//           channels.insert(chAlias);
//           break;
//         }
//       }
//     }
//   }


//   QMap<QString, QString> channelsFiles;
//   bool ok = true;
//   meteo::rpc::Channel* dcCh = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
//   if (nullptr == dcCh) return false;

//   for(auto ch : channels)
//     {
//       proto::Pretreatment req;
//       req.set_session_id(task.session.id());
//       req.set_instrument_type(instrument.type());
//       req.set_channel_alias(ch.toStdString());
//       auto* prets = dcCh->remoteCall(&mappi::proto::SessionDataService::GetAvailablePretreatment, req, 10000, true);
//       if(ok &= (nullptr != prets))
//       {
//         for(auto pretreatment : prets->pretreatments())
//         {
//           if(ok &= pretreatment.has_path()) {
//             channelsFiles.insert(ch, QString(pretreatment.path().data()));
//           }
//           else {
//             break;
//           }
//         }
//         delete prets;
//       }
//       if(false == ok) break;
//     }
//   delete dcCh;
  
//   if(channelsFiles.size() == channels.size()) {
//     task.channelFiles_ = task.channelFiles_.unite(channelsFiles);
//     ok = true;
//   }
//   else {
//     ok = false;
//   }
//   return ok;
// }

// bool ThematicProcessor::getChannelForGrayScale(ThematicTask& task)
// {
//   conf::Instrument instrument;
//   ::mappi::inter::Settings::instance()->instrument(QString(task.session.sat_name().data()), &instrument);
//   QSet<QString> channels;
//   // Заполняем список каналов инструмента
//   for(conf::Channel ch : instrument.channel())
//   {
//     if(ch.has_number())
//     {
//       if(ch.has_alias())
//       {
//         QString chAlias(ch.alias().data());
//         if(false == task.channelFiles_.contains(chAlias)) {
//           channels.insert(chAlias);
//         }
//       }
//       else {
//         error_log << QObject::tr("У канала %1 не установлен псевдоним").arg(ch.number());
//       }
//     }
//     else {
//       error_log << QObject::tr("У канала не установлен номер. %1").arg(QString(ch.DebugString().data()));
//     }
//   }

//   bool ok = false;
//   meteo::rpc::Channel* dcCh = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
//   if (nullptr == dcCh) return false;

//   // auto* ctrl = meteo::global::serviceController(meteo::settings::proto::kDataControl);
//   // if(nullptr != ctrl)
//   {
//     //Удаляем каналы для которых выполнена grayscale
//     proto::ThematicData themReq;
//     themReq.set_session_id(task.session.id());
//     themReq.set_name("grayscale");
//     auto* thems = dcCh->remoteCall(&proto::SessionDataService::GetAvailableThematic, themReq, 10000);
//     if(nullptr != thems) {
//       for(auto i = 0, sz = thems->themes_size(); i < sz; ++i) {
//         channels.remove(QString::fromStdString(thems->themes(i).channel_alias()));
//       }
//       delete thems;
//     }
//     //Добавляем в список файлов каналы для которых не выполнена grayscale
//     for(auto ch : channels)
//     {
//       proto::Pretreatment req;
//       req.set_session_id(task.session.id());
//       req.set_instrument_type(instrument.type());
//       req.set_channel_alias(ch.toStdString());
//       auto* prets = dcCh->remoteCall(&proto::SessionDataService::GetAvailablePretreatment, req, 10000);
//       if(nullptr != prets)
//       {
//         for(auto pretreatments : prets->pretreatments()) {
//           if(ok |= pretreatments.has_path()) {
//             task.channelFiles_.insert(ch, QString(pretreatments.path().data()));
//           }
//         }
//         delete prets;
//       }
//     }
//     delete dcCh;
//   }
//   return ok;
// }

/*!
 * \brief ThematicProcessor::hasThematic Проверка на существование тематической обработки
 * \param session  - Сеанс
 * \param thematic - Тип ТО
 * \return true если существует
 */
// bool ThematicProcessor::hasThematic(const proto::SessionData& session, const conf::ThematicProc& them)
// {
//   bool ok = true; //Предположим что ТО есть
//   meteo::rpc::Channel* dcCh = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
//   if (nullptr == dcCh) return false;

//   //  auto* ctrl = meteo::global::serviceController(meteo::settings::proto::kDataControl);
//   // if(nullptr != ctrl)
//   {
//     proto::ThematicData themReq;          //!< Запрос тематических обработак
//     themReq.set_session_id(session.id());
//     themReq.set_name(them.name());

//     auto* thems = dcCh->remoteCall(&mappi::proto::SessionDataService::GetAvailableThematic, themReq, 10000, true);

//     if(nullptr != thems)
//     {
//       if(conf::kGrayScale == them.type())
//       {
//         proto::Pretreatment pretReq;          //!< Запрос предварительных обработак
//         pretReq.set_session_id(session.id());

//         auto* prets = dcCh->remoteCall(&mappi::proto::SessionDataService::GetAvailablePretreatment, pretReq, 10000, true);
//         // true - если количество тематических обработак GrayScale совпадает с количеством предварительных обработак
//         if(ok &= (nullptr != prets)) {
//           ok &= prets->pretreatments_size() == thems->themes_size();
//         }
//       }
//       else {
//         bool themUseInSession = true;
//         for(auto themVar : them.vars())
//         {
//           bool useVariable = false;
//           for(auto themCh : themVar.channel())
//           {
//             useVariable = (0 == themCh.satellite().compare(session.sat_name()));
//             if(useVariable) {
//               break;
//             }
//           }
//           themUseInSession &= useVariable;
//         }
//         // Если используется: сложное условие, иначе true
//         ok &= themUseInSession
//             ? thems->themes_size() > 0 && thems->themes(0).has_path()
//             : true;
//       }
//       delete thems;
//     }
//     delete dcCh;
//   }
//   return ok;
// }

// bool ThematicProcessor::isValid(const conf::ThematicProc& thematic)
// {
//   bool result = thematic.has_name();
//   result &= thematic.has_red();
//   result &= thematic.has_type();
//   if(result && thematic.type() == conf::kFalseColor) {
//     result &= thematic.has_green() && thematic.has_blue();
//   }
//   if(result &= thematic.has_enabled()) {
//     result &= thematic.enabled();
//   }
//   for(auto _var : thematic.vars()) {
//     if(false == result) break;
//     result &= _var.has_name();
//   }
//   return result;
// }
// bool ThematicProcessor::isValid(const conf::ThematicChannel& channel)
// {
//   return channel.has_satellite() && channel.has_instrument() && channel.has_channel();
// }
