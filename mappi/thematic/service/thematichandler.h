#ifndef MAPPI_THEMATIC_PROCESSOR_THEMATICHANDLER_H
#define MAPPI_THEMATIC_PROCESSOR_THEMATICHANDLER_H

#include <qobject.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qqueue.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qsharedpointer.h>

#include <mappi/proto/thematic.pb.h>
#include <mappi/proto/sessiondataservice.pb.h>


namespace mappi {

  namespace to {
    class DataStore;
  }
  
// struct ThematicTask {
//   proto::SessionData          session;
//   QQueue<conf::ThematicProc> task;
//   QMap<QString, QString>      channelFiles_;    //!< канал : файл
// };

  struct ThematicTask {
    QDateTime start;
    QString satellite;
    QList<conf::InstrumentType> instruments;
//    QList<conf::ThemType> types;
     QMap< std::string,conf::ThemType> types;
  };
  
class ThematicHandler : public QObject
{
  Q_OBJECT
public:
  ThematicHandler(QObject* parent = 0);
  ~ThematicHandler();

  void addTask(ThematicTask& task);
  void stop();
  void interrupt();

public:
  Q_SLOT void run();
  Q_SLOT bool isWorking();

 Q_SIGNAL void finished();
 Q_SIGNAL void complete();

private:
  Q_SLOT void dataProcess(const ThematicTask& task);

private:
  QQueue<ThematicTask>        taskQueue_;
  QMutex                      addMutex_;

  QWaitCondition              waitFile_;
  QMutex                      waitFileMutex_;

  QMutex                      processMutex_;
  QWaitCondition              waitProcess_;
  //  QMap<conf::ThemType, int>  themCount_;

  // bool                        isLocal_   = false;
  bool                        working_   = false;
  bool                        stop_      = false;
  bool                        interrupt_ = false;

  QSharedPointer<to::DataStore> _store;
  
// private:
//   static QString getFileName(const proto::SessionData& session, const QString& themName);
//   static bool saveThematic(const proto::SessionData& session, const conf::ThematicProc& them, conf::InstrumentType instrType,
//                            const QString& fileName, int ch_number = 0);
//   static bool checkThematics(const proto::SessionData& session);

//   static bool isChInSession(const conf::ThematicChannel& ch, const QString& satName, conf::InstrumentType instrumentType);
};

} // mappi
#endif // MAPPI_THEMATIC_PROCESSOR_THEMATICHANDLER_H
