#ifndef METEO_COMMONS_SERVICES_MAP_APPSTATUS_H
#define METEO_COMMONS_SERVICES_MAP_APPSTATUS_H

#include <memory>

#include <qobject.h>
#include <qdatetime.h>
#include <qpair.h>
#include <qmap.h>
#include <qmutex.h>

#include <meteo/commons/proto/state.pb.h>


class QTimer;


namespace meteo {

namespace rpc {
  class Channel;
}
namespace map {

class MethodBag;

class StatusSender : public QObject
{
  Q_OBJECT
  public:
    StatusSender( MethodBag* handler );
    ~StatusSender();

    bool init();

    /*! 
     * \brief Инкрементировать счетчики по карте title
     * \param title - название карты
     * \param mapsize - инкремент созданных карт
     * \param mapsucsize - инкремент успешно созданных карт
     */
    void incMap( const QString& title, int mapsize, int mapsucsize  );

    /*! 
     * \brief Инкрементировать счетчики по заданию title
     * \param title - назварние задания
     * \param jobsize - инкремент запущенных заданий
     * \param jobsucsize - инкремент успешно выполненных заданий
     */
    void incJob( const QString& title, int jobsize, int jobsucsize );

    /*! 
     * \brief - Инкрементировать счетчики по созданным документам
     * \param createdsize - инкремент созданных документов
     * \param delsize - инкремент удаленных документов
     */
    void incDoc( int createdsize, int delsize );

    /*! 
     * \brief - Обновить информацию об очереди заданий
     * \param running - инкремент запущенных
     * \param queue - инкремент ожидающих в очереди
     */
    void incJobSize( int running, int queue );

    app::OperationStatus getStatus() const;

  public slots:
    void slotTimeout();

  private:
    MethodBag* handler_;
    meteo::rpc::Channel* appctrl_;
    QTimer* timer_;
    mutable QMutex mutex_;

    /*! 
     * \brief Размер очереди заданий ( Дата-время - Выполняется/в очереди )
     */
    QPair< QDateTime, QPair< int, int > > jobsize_;
    /*! 
     * \brief статистика по заданиям ( Название задания - Дата-время - Всего/успешно )
     */
    QMap< QString, QPair< QDateTime, QPair< int, int > > > jobstat_;

    /*! 
     * \brief статистика по картам ( Дата-время - Название карты - Всего/успешно )
     */
    QMap< QString, QPair< QDateTime, QPair< int, int > > > mapstat_;

    /*! 
     * \brief статистика по документам ( Дата-время - создано/удалено )
     */
    QPair< QDateTime, QPair< int, int > > docstat_;

    

  private slots:
    void slotClientConnected();
    void slotClientDisconnected();
};

}
}

#endif
