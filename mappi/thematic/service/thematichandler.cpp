#include "thematichandler.h"
//#include "filegetter.h"


#include <qthread.h>

#include <meteo/commons/global/global.h>

//#include <mappi/services/fileservice/fileserviceclient/filegetter.h>
//#include <mappi/settings/mappisettings.h>
#include <mappi/thematic/algs_calc/themformat.h>
#include <mappi/thematic/algs_calc/themalg.h>
//#include <mappi/global/streamheader.h>        //! readPoHeader();

#include <mappi/thematic/algs_calc/dataservstore.h>

using namespace mappi;

ThematicHandler::ThematicHandler(QObject* parent)
  : QObject(parent)
{
  //  isLocal_ = true; // ::mappi::inter::Settings::instance()->serviceAddress(meteo::settings::proto::kFile).isLocal(); TODO
  _store = QSharedPointer<to::DataStore>(new to::DataServiceStore());
}

ThematicHandler::~ThematicHandler()
{
}

void ThematicHandler::run()
{
  while (!stop_ && !interrupt_)
  {
    var(working_);
    waitProcess_.wait(&processMutex_);
    working_ = true;
    while (!taskQueue_.empty() && !interrupt_)
    {
      debug_log << QObject::tr("Осталось %1 сеансов").arg(taskQueue_.size());
      addMutex_.lock();
      ThematicTask task = taskQueue_.takeFirst();
      addMutex_.unlock();
      dataProcess(task);
      processMutex_.unlock();
      var(taskQueue_.size());
    }
    working_ = false;
    debug_log << QObject::tr("Все сеансы обработаны");
    Q_EMIT(complete());
  }
  while (!taskQueue_.empty()) {
    taskQueue_.pop_front();
  }
  working_ = false;
  Q_EMIT(finished());
}

void ThematicHandler::addTask(ThematicTask &task)
{
  trc;
  addMutex_.lock();
  bool contains = false;
  for(const auto &tsk : qAsConst(taskQueue_)) {
    //   if(tsk.sessionId == task.sessionId) {
    if(tsk.satellite == task.satellite &&
       tsk.start == task.start) {
      contains = true;
      break;
    }
  }
  if(!contains) {
    taskQueue_.push_back(std::move(task));
    debug_log << QObject::tr("Добавлен сеанс %1 %2").arg(task.satellite).arg(task.start.toString(Qt::ISODate));
  } else {
    debug_log << QObject::tr("Сеанс %1 %2 уже находится в очереди на обработку").arg(task.satellite).arg(task.start.toString(Qt::ISODate));
  }

  addMutex_.unlock();
  waitProcess_.wakeOne();
}

void ThematicHandler::dataProcess(const ThematicTask& task)
{
  debug_log << QObject::tr("Обработка сеанса %1 %2").arg(task.satellite).arg(task.start.toString(Qt::ISODate));


  QMapIterator<std::string,conf::ThemType> i(task.types);
  while (i.hasNext()) {
    i.next();
    auto alg = std::unique_ptr<to::ThemAlg>(to::singleton::ThemFormat::instance()->createThemAlg(i.value(),i.key(), _store));
    if (nullptr == alg) {
      continue;
    }
    try {
        alg->init(task.start, task.satellite);
        alg->process();
        alg->saveImg();
        alg->saveData();
    } catch (const std::exception& ex) {
        error_log << QObject::tr("Необработанное исключение %1").arg(ex.what());
    }catch (...) {
        error_log << QObject::tr("Необработанное исключение неизвестного типа");
    }
  }


//  for (auto type : task.types) {
//    var(type);
//    auto alg = std::unique_ptr<to::ThemAlg>(to::singleton::ThemFormat::instance()->createThemAlg(type, _store));
//    if (nullptr == alg) {
//      continue;
//    }
//    debug_log << task.start << task.satellite << task.instruments;
//    alg->process(task.start, task.satellite, task.instruments);
  
//  }
  
}

// void ThematicHandler::dataProcess(ThematicTask task)
// {
//   debug_log << QObject::tr("Обработка сеанса %1").arg(task.session.id());
//   //error_log << "TODO: commented code";
//   // if(false == isLocal_) TODO для удаленных сервисов
//   // {
//   //   for(auto fileIt = task.channelFiles_.begin(); fileIt != task.channelFiles_.end(); ++fileIt)
//   //   {

//   //     FileGetter* fileGetter  = new FileGetter();
//   //     fileGetter->setFilename(fileIt.value());

//   //     QThread *thread = new QThread();
//   //     fileGetter->moveToThread(thread);
//   //     connect(thread,     &QThread::started,      fileGetter, &FileGetter::run);

//   //     connect(fileGetter, &FileGetter::complete,  this,
//   //             [&](QString filename){
//   //               task.channelFiles_[fileIt.key()] = filename;
//   //               waitFile_.wakeOne();
//   //             }, Qt::DirectConnection);

//   //     connect(fileGetter, &FileGetter::finished,  fileGetter, &FileGetter::deleteLater);
//   //     connect(fileGetter, &FileGetter::destroyed, thread, &QThread::quit);
//   //     connect(thread,     &QThread::finished,     thread, &QThread::deleteLater);
//   //     thread->start();

//   //     waitFile_.wait(&waitFileMutex_);
//   //   }
//   // }

//   if(task.channelFiles_.isEmpty()) {
//     debug_log << QObject::tr("Сеанс %1, нет связанных файлов").arg(task.session.id());
//     if(false == task.task.isEmpty()) {
//       checkThematics(task.session);
//     }
//     return;
//   }
//   meteo::global::PoHeader poHeader = meteo::global::readPoHeader(task.channelFiles_.first());
//   conf::Instrument instrument;
//   mappi::inter::Settings::instance()->instrument(poHeader.instr, &instrument);

//   bool taskOk = true;
//   for(auto them : task.task)
//   {
//     bool ok = true;
//     auto alg = std::unique_ptr<to::ThemAlg>(to::singleton::ThemFormat::instance()->createThemAlg(them.type()));
//     if (nullptr == alg) {
//       continue;      
//     }
//     if(them.type() == proto::kGrayScale)
//     {
//       for(auto ch : task.channelFiles_)
//       {
//         bool grayScaleOk = true;
//         poHeader = meteo::global::readPoHeader(ch);
//         grayScaleOk &= alg->readChannelData(ch);
//         if(grayScaleOk) {
// 	  ok &= alg->process();
// 	}
//         if(grayScaleOk)
//         {
//           task.session.set_date_start(poHeader.start.toString(Qt::ISODate).toStdString());
//           task.session.set_date_end(poHeader.stop.toString(Qt::ISODate).toStdString());
//           ++themCount_[them.type()];
//           auto fileName = getFileName(task.session, QString(them.name().data()));
//           grayScaleOk &= alg->saveImage(fileName);
//           if(grayScaleOk) ok &= saveThematic(task.session, them, instrument.type(), alg->getImageFileName(), poHeader.channel);
//         }
//         ok &= grayScaleOk;
//       }
//     }
//     else
//     {
//       QList<int> ch_numbers;

//       for(auto themVar : them.vars())
//       {
//         for(auto ch : themVar.channel())
//         {
//           if(isChInSession(ch, QString::fromStdString(task.session.sat_name().data()), instrument.type()))
//           {
//             auto chIt = task.channelFiles_.find(QString::fromStdString(ch.channel().data()));
//             ok &= (task.channelFiles_.end() != chIt);
//             if(ok) {	      
//               ok &= alg->readChannelData(chIt.value(), chIt.key());
//               if(ok) {
//                 poHeader = meteo::global::readPoHeader(chIt.value());
//                 ch_numbers.append(poHeader.channel);
//                 break;
//               }
//               else
//                 warning_log << QObject::tr("Не удалось прочитать файл канал %1 переменной %2").arg(chIt.value()).arg(chIt.key());
//             }
//           }
//         }
//         if(!ok) break;
//       }
//       if(ok) {
// 	alg->normalize(0, 255); //TODO смотри внутри
// 	ok &= alg->process(them);
//       }
//       if(ok)
//       {
//         task.session.set_date_start(poHeader.start.toString(Qt::ISODate).toStdString());
//         task.session.set_date_end(poHeader.stop.toString(Qt::ISODate).toStdString());
//         ++themCount_[them.type()];
//         auto fileName = getFileName(task.session, QString::fromStdString(them.name().data()));
//         if(proto::kFalseColor == them.type() ||
// 	   proto::kDayMicrophisicsColor == them.type() ||
// 	   proto::kCloudsColor == them.type() ||
// 	   proto::kNaturalColor == them.type() ||
// 	   proto::kNightMicrophysicsColor == them.type()	   
// 	   ) {
//           ok &= alg->saveImage(fileName);
//         }
//         else {
//           alg->saveImage(fileName);
//           ok &= alg->saveData(fileName);
//         }

//         if(ok) {
//           if(proto::kFalseColor == them.type() ||
// 	     proto::kDayMicrophisicsColor == them.type() ||
// 	     proto::kCloudsColor == them.type() ||
// 	     proto::kNaturalColor == them.type() ||
// 	     proto::kNightMicrophysicsColor == them.type()) {
//             ok &= saveThematic(task.session, them, instrument.type(), alg->getImageFileName(), ch_numbers.first());
//           }
//           else {
//             ok &= saveThematic(task.session, them, instrument.type(), alg->getDataFileName(), ch_numbers.first());
//           }
//         }
//         else {
//           warning_log << QObject::tr("Не удалось сохранить файлы %1, %2").arg(alg->getImageFileName()).arg(alg->getDataFileName());
//         }
//       }
//       else {
//         warning_log << QObject::tr("Обработка %1 выполнена неудачно").arg(them.name().data());
//       }
//     }
//     taskOk &= ok;
//   }
//   if(taskOk) {
//     checkThematics(task.session);
//   }
//   else {
//     debug_log << QObject::tr("Сеанс %1, не помечен как завершенный").arg(task.session.id());
//   }
// }

/*!
 * \brief ThematicProcessor::saveThematic сохранить тематическую обработку
 * \param session   - сеанс
 * \param name      - тип обработки
 * \param instrType - тип прибора
 * \param fileName  - имя файла
 * \param ch_number  - псевдоним канала
 * \return успех операции
 */
// bool ThematicHandler::saveThematic(const proto::SessionData& session, const proto::ThematicProc& them, conf::InstrumentType instrType,
//                                    const QString& fileName, int ch_number)
// {
//   bool ok = false;
//   auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
//   if(nullptr != channel)
//   {
//     debug_log << QObject::tr("канал %1").arg(ch_number);
//     proto::ThematicData req;
//     req.set_name(them.name());
//     req.set_type(them.type());
//     req.set_title(them.description());
//     req.set_date_start(session.date_start());
//     req.set_date_end(session.date_end());
//     req.set_session_id(session.id());
//     req.set_path(fileName.toStdString());
//     req.set_instrument_type(instrType);
//     // if (req.instrument_type() == mappi::conf::kAvhrr3Cadu) { //TODO пока так для макета
//     //   req.set_instrument_type(mappi::conf::kAvhrr3);
//     // }
//     req.set_channel_number(ch_number);

//     auto* resp = channel->remoteCall(&mappi::proto::SessionDataService::SaveThematic, req, 10000, true);
//     if(nullptr != resp)
//     {
//       ok = (resp->has_result() ? resp->result() : false);
//       if(false == ok && resp->has_comment()) {
//         error_log << QString::fromStdString(resp->comment());
//       }
//       delete resp;
//     }
//     delete channel;
//   }
//   return ok;
// }

/*!
 * \brief ThematicHandler::checkThematics отметить сеанс как сеанс с выполнеными тематическими обработками
 * \param session - сеанс
 * \return успех операции
 */
// bool ThematicHandler::checkThematics(const proto::SessionData& session)
// {
//   auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
//   bool ok = true;
//   if(nullptr != channel)
//   {
//     proto::SessionData sreq;
//     sreq.set_id(session.id());
//     sreq.set_thematic_done(true);
//     auto* thems = channel->remoteCall(&mappi::proto::SessionDataService::MarkSessionThematicDone, sreq, 10000, true);
//     if(nullptr != thems)
//     {
//       ok &= thems->has_result() ? thems->result() : false;
//       if(!ok && thems->has_comment()) {
//         error_log << QString::fromStdString(thems->comment());
//       }
//       delete thems;
//     }
//     delete channel;
//   }
//   return ok;
// }

// QString ThematicHandler::getFileName(const proto::SessionData& session, const QString& themName)
// {
//   QDateTime dateTime = QDateTime::fromString(QString::fromStdString(session.date_start()), Qt::ISODate);
//   QString satdata = QString::fromStdString(::mappi::inter::Settings::instance()->reception().satdata());
//   QString dir = satdata + "/thematics/" + dateTime.toString("yyyy-MM-dd/");
//   QDir mkdir; mkdir.mkpath(dir);
//   QString filename = dir + QObject::tr("%1_%2_%3_%4")
//       .arg(dateTime.toString("yyyyMMdd_hhmm"))
//       .arg(themName)
//       .arg(session.id())
//       .arg(QString::fromStdString(session.sat_name()));
//   return filename;
// }

bool ThematicHandler::isWorking() { return working_; }
void ThematicHandler::stop()      { stop_      = true; waitProcess_.wakeOne(); }
void ThematicHandler::interrupt() { interrupt_ = true; waitProcess_.wakeOne(); }

// bool ThematicHandler::isChInSession(const proto::ThematicChannel& ch, const QString& satName, conf::InstrumentType instrumentType)
// {
//   return QString(ch.satellite().data()) == satName && ch.instrument() == instrumentType;
// }


