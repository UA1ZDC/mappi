#ifndef MAPPI_DEVICE_RECEIVER_CTRL_H
#define MAPPI_DEVICE_RECEIVER_CTRL_H

#include <qobject.h>
#include <qprocess.h>

#include <commons/geobasis/coords.h>

class QMutex;
class QTimer;

class Satellite;

namespace MnSat {
  class STLEParams;
}

namespace mappi {
namespace receive {
  
/*!
 * \brief The Ctrl class - Управление ресивером
 *  Перед стартом в отдельном потоке перемести объект в отдельный поток
 *  Вызови setAdjustFreq??? и init
 *  Вызови функцию start() она запустит QProcess и установит working_ = true,
 * кроме этого она заблокирует внешнее редактирование данных
 *  Функция stop() принудительно прервет исполнение QProcess и разблокирует редактирование данных,
 * кроме этого она снимет флаг working_ и испустит сигнал finished()
 *  Сигнал finished() испускает при завершении работы QProcess в рабочем режиме.
 *  Функции startControl() и stopControl() зыпускаются по сигналам испускающимся из start(), stop()
 *
 *
 *
 */
class Ctrl : public QObject {
  Q_OBJECT
public:
  Ctrl(const QString& app);
  virtual ~Ctrl();

  //!<  Часть инициализации рассмотреть возможность переноса в init
  //! Вызывается до старта
  bool setAdjustFreq(const Coords::GeoCoord& site);

  bool init(const MnSat::STLEParams& stle, float freq, float gain, float rate);

  bool isWorking();

public slots:
  bool start();
  void stop();

signals:
  void starSignal();
  void stopSignal();
  void finished();

private slots:
  bool startControl();
  void stopControl();

  void adjustFreq();
  void readOutput();
  void appFinished(int, QProcess::ExitStatus);

private:
  QMutex*          editMutex_;
  QProcess*        pyapp_;
  QTimer*          timer_; //!< Проверка необходимости подстройки частоты
  Satellite*       satellite_;

  float            freq_      = 0.0f;    //!< Частота, Гц
  float            gain_      = 0.0f;    //!< Усиление
  float            rate_      = 0.0f;    //!< Частота дискретизации, Гц
  QString          app_;                 //!< Программа для управления прибором
  bool             adjustFreq_ = false;  //!< true - подстраивать частоту во время приёма
  Coords::GeoCoord site_;                //!< Пункт приёма
  bool             working_   = false;
};
    
} // receive
} // mappi

#endif
