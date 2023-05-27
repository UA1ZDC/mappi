#ifndef MAPPI_THEMATIC_PROCESSOR_THEMATICSERVICE_H
#define MAPPI_THEMATIC_PROCESSOR_THEMATICSERVICE_H

#include <mappi/proto/thematic.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <qobject.h>
#include <qmap.h>

namespace mappi {

  class ThematicHandler;
  
  class ThematicService : public QObject, public ::mappi::conf::ThematicService
  {
    Q_OBJECT
  public:
    explicit ThematicService(QObject* parent = 0);
    ~ThematicService();

    void PerformThemProcess(::google::protobuf::RpcController* ctrl,
			    const mappi::conf::PerformThemRequest* req,
			    mappi::conf::ThemResponse* resp,
			    ::google::protobuf::Closure* done);
    
  private:
    void init();
    bool addTask(const QString& satname, const QDateTime& start, const QList<conf::InstrumentType>& instruments);    
    
  private:
    // static bool getChannelForThematic(ThematicTask& task, const conf::ThematicProc& them);
    // static bool getChannelForFalseColor(ThematicTask& task);
    // static bool getChannelForGrayScale(ThematicTask& task);
    
    // static bool isValid(const conf::ThematicProc& thematic);
    // static bool isValid(const conf::ThematicChannel& channel);
    // static bool hasThematic(const proto::SessionData& session, const conf::ThematicProc& thematic);
    
  private:
    // QMap<QString, conf::ThematicProc> thematics_;      //!< Доступные предустановленные тематические обработки
    // QList<proto::SessionData>          unprocsessions_; //!< Необработанные сеансы
    ThematicHandler*                   handler_;        //!< Поток выполняющий предварительные обработки
   // QMap<QString, QList<conf::ThemType>> _conf; //!< спутник, список обработок
    QMap<QString, QMap< std::string,conf::ThemType>> _conf; //!< спутник, список обработок

  };

} 

#endif
