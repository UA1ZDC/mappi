#ifndef METEO_COMMONS_GLOBAL_FORMALDOCSAVER_H
#define METEO_COMMONS_GLOBAL_FORMALDOCSAVER_H

#include <qmap.h>
#include <qobject.h>
#include <qprocess.h>
#include <qtemporaryfile.h>
#include <qfilesystemwatcher.h>

#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/proto/web.pb.h>
#include <meteo/commons/proto/taskservice.pb.h>

namespace meteo {

namespace rpc {
class Channel;
}

class FormalDocSaver : public QObject
{
  Q_OBJECT
  public:
    FormalDocSaver( QObject* parent = 0 );
    ~FormalDocSaver();

  public slots:
    void slotCreateFormalDoc( const map::proto::Map& map, const QStringList& users = QStringList() );

    web::UserListResponce getUserList( bool* ok = nullptr );

    int setTimeout(int timeout);

  private:
    void openEmptyDoc( rpc::Channel* ch );
    void openTemp( rpc::Channel* ch, const QStringList& users );
    void createOfficeProc( rpc::Channel* ch );

  private slots:
    void slotClientDisconnected( rpc::Channel* ch );
    void slotDocumentChanged( const QString& filename );
    void slotProcFinished();
    void sendFilesToUsers( rpc::Channel* ch );
    bool uploadFile( rpc::Channel* ch );
    void sendMessages( rpc::Channel* ch );
    QList<QByteArray> chunksFromFile( const QByteArray& file );

  private:
    QFileSystemWatcher* watcher_ = nullptr;
    /*! 
     * \brief ClientInfo - Содержит все необходимые данные клиента
     */
    struct ClientInfo {
      QString scriptname_;   //!< название файла сценария
      QString docname_;      //!< название вида документа (Например: Дневник погоды)
      QString template_;     //! название файла шаблона
      map::proto::Map map_;  //!< Параметры задания
      map::proto::ExportRequest req_export_map_;        //!< параметры экспортирования документа
      QTemporaryFile* file_ = nullptr;                  //!< временный файл с документом
      QProcess* proc_ = nullptr;                        //!< процесс опенофиса, в которм открыт запрошенный документ

      map::proto::Response* resp_createdoc_ = nullptr;  //!< ответ на запрос создания документа
      map::proto::Response* resp_createmap_ = nullptr;  //!< ответ на запрос построения документа на сервере
      map::proto::Response* resp_savedocum_ = nullptr;  //!< ответ на запрос сохранения документа
      map::proto::ExportResponse* resp_exportmap_ = nullptr; //!< ответ на запрос экспорта документа
      QStringList users_;
      QString fileid_;
      QString filename_;

      void clear();
    };
    web::UserListResponce protousers_;


  QMap< rpc::Channel*, ClientInfo > clients_;
  QMap< QString, meteo::map::proto::Map > formals_;
  int timeout_;
};

}

#endif
