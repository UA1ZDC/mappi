#ifndef MAPPI_UI_PLUGIN_SESSION_RECEIVERIMAGESCENE_H
#define MAPPI_UI_PLUGIN_SESSION_RECEIVERIMAGESCENE_H

#include <memory>

#include <qgraphicsscene.h>
#include <qgraphicsitem.h>
#include <qvector.h>
#include <qmap.h>

#include <sat-commons/satellite/satellitebase.h>
#include <commons/geobasis/geovector.h>
#include <mappi/proto/satellite.pb.h>

#include <mappi/schedule/session.h>
#include <meteo/commons/rpc/rpc.h>


// namespace rpc {
//   class TController;
// } // rpc

namespace meteo {
  namespace map {
    class PosDocument;
  } // map
} // meteo

namespace mappi {
  namespace conf {
    class Reception;
    class DataReply;
  }

class ReceiverImageScene : public QGraphicsScene
{
  Q_OBJECT
public:
  ReceiverImageScene(QGraphicsView* parent);
  ~ReceiverImageScene();

  void setSession(std::shared_ptr<schedule::Session> session, const conf::Instrument& instrument,
                  const QDateTime& dt, const MnSat::STLEParams &stle);

  void removeTemp();
  void resize(const QSize& size);
  void subscribe(meteo::rpc::Channel* ctrl);
  bool showLayers() const;

public slots:
  void slotSwitchChannel(const QString& channel);
  void slotToggleMap(bool show);

signals:
  void readySatChannels(const QStringList& channels);
  void sigResize(const QSize& size);

protected:
  void drawBackground(QPainter *painter, const QRectF &rect);
  void drawForeground(QPainter *painter, const QRectF &rect);

private slots:
  void callbackStreamReceiv(mappi::conf::DataReply* reply);

private:
  bool initDocument(const QSize& size, float scan_angle, int samples,
                    float velocity, const MnSat::STLEParams &stle);
  void loadImages(QMap<QString, QString> paths);
  void loadImage(const QString& alias, const QString& filename);
  void printImage(const QByteArray& data);
  void clearPictures();

private:
  QMap<int, QString>                  channelAliases_;      //!< Номер канала    : Псевдоним канала
  QMap<QString, QGraphicsPixmapItem*> pictures_;            //!< Псевдоим канала : Картинка

  QGraphicsTextItem*                  noDataItem_;          //!< Объект текст
  QString                             noDataText_;          //!< Информационный текст

  QSize                               size_;                //!< Размер снимка
  double                              scale_;               //!< Масштаб снимка по X

  std::shared_ptr<schedule::Session>  session_;             //!< Текущий сеанс
  float                               scan_angle_;          //!< Угол сканирования прибора
  int                                 samples_;             //!< Ширина сканирования
  float                               velocity_;            //!< Скорость приема прибора
  meteo::map::PosDocument*            document_;            //!< Документ - карта

  QString                             currentChannel_;      //!< Выбраный канал

  //  mappi::conf::DataReply*             replyReceiver_;       //!< Ответ от сервиса приёма
  QMap<int, QByteArray>               data_;                //!< Данные отсервиса приема
  QMap<int, int>                      rows_;                //!< Количество строк у полученного изображения

  bool                                useMap_;              //!< Отоброжать береговую черту
  bool                                showLayer_;           //!< Показывать/скрывать контуры
  bool                                sessionActive_;       //!< Принимаемый ли текущий сеанс

  bool                                readySatChannels_;    //!< Сформирован список каналов для принимаемого сеанса
  QStringList                         satChannels_;         //!< Полученные со спутника каналы

private:
  static MnSat::TLEParams getTLE(const schedule::Session& session);
};

}

#endif // MAPPI_UI_PLUGIN_SESSION_RECEIVERIMAGESCENE_H
