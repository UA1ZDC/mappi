#ifndef METEO_ML_METHODS_KRLS_ML_H
#define METEO_ML_METHODS_KRLS_ML_H

#include <stdlib.h>
#include <QString>

#include <dlib/svm.h>
// #include <dlib/dnn.h>
// #include <dlib/mlp.h>
#include <dlib/data_io.h>
#include <vector>

#include <meteo/commons/services/ml/analyser/analyser.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
// #include <meteo/ml/settings/mlsettings.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/forecast.pb.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/proto/surface.pb.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <commons/obanal/func_obanal.h>

#include <commons/geobasis/geodata.h>
//#include <meteo/commons/services/fieldata/fieldservicedb.h>





namespace meteo {
namespace ml {

/**
 * Класс восстановления полей по прогностическим данным
 *
 *
 *
 *  Пример использования:
 *
 *  meteo::ml::ForecastAnalyzer* analyser = new meteo::ml::ForecastAnalyzer();
 *        // устанаваливаем отладку
 *        analyser->setDebugOn()
 *        // устанавливаем множитель десктриптора под которым будем сохранять
 *        // в 0 (т.е. не используем)
 *        .setFieldDescription(0)
 *        .setParam(11002);
 *
 *  forecastml->setAnalyzer(analyser)
 *          .setDebugOn()
 *          .setPoint( (double)0.851429, (double)0.774345 )
 *          .predictKRLSModel();
 *          .trainKRLSTodayModel();
 *
 *
 *
 */
class ForecastMLKRLS : public ForecastML
{

public:
  ForecastML();
  ~ForecastML();


  /**
   * Обучаем кумулятивно (за сегодня )
   * модель KRLS
   * dlib/docs/ml.html#krls
   * обязательно должен быть задан ForecastAnalyzer с параметрами
   *
   * @return [ForecastML]
   */
  ForecastML& trainKRLSTodayModel();
  //
  // получаем прогностическое значение в точке
  // предварительно необходимо загрузить параметры через setAnalyzer
  // ForecastAnalyzer
  //
  double      predictKRLSModel();
  double      predictKRLSModel(double lat, double lon);

  //
  // прогнозируем поле по станциям и записываем его в БД
  //
  ForecastML& predictOnStations();

  ForecastML& trainModel();

  // устанавливаем список параметров, по которым будем считать модель
  ForecastML& setPredictParamList(QList<int> paramList);
  // запускаем прогнозирование по станциям
  ForecastML& trainOnStations(QList<QString> stantions, QList<QString> stantionspdc);
  ForecastML& trainOnStations();


  /**
   *
   *  Частные методы
   *  которые выполняют одну функцию
   *
   */

  /**
   * Загружаем модель из базы
   * @return [description]
   */
  ForecastML& loadModel();
  /**
   * Сохраняем модель в базу
   * @return [description]
   */
  ForecastML& saveModel();


  /**
   * Получаем аналайзер
   * @return [description]
   */
  ForecastAnalyzer& getAnalyzer()   { return *this->analyser; }

  /**
   * Задаем аналайзер
   * @return [description]
   */
  ForecastML&       setAnalyzer(ForecastAnalyzer* m_analyzer) { this->analyser = m_analyzer; return *this; }
  // устанавливаем дату/время
  ForecastML&       setDateTime(  QDateTime dt );
  // Устанавливаем текущую точку
  ForecastML&       setPoint(  meteo::GeoPoint point );
  // в радианах
  ForecastML&       setPoint( double lat, double lon );




  ForecastML&       dnnTrain();




  // отправляем сообщение в дебаг, если надо
  ForecastML&       debugMSG( QString txt );
  ForecastML&       setDebugOn();
  ForecastML&       setDebugOff();

private:

  // количество дней, после которых начинаем забывать, что
  // мы тренировали
  int max_trained_days = 5;
  // максимальный порог ошибки
  float max_loss = 0.1;

  typedef dlib::matrix<double,1,1>             KRLS_type; // декларируем тип переменной, в которой будем содержать данные
  typedef dlib::radial_basis_kernel<KRLS_type> KRLS_kernel_type; // инициализируем ядро алгоритма
  dlib::krls<KRLS_kernel_type>    *krls_model  = nullptr; //модель, с которой работаем


  // список параметров по которому будем производить обучение
  QList<int>         paramList;
  // станции, по которым будем прогнозировать (которые нужны)
  QList<QString>     stantions;
  // станции, которые используются для анализа и обучения
  QList<QString>     stantionspdc;

  // матрица значений
  // типа двумерный массив, в котором храним значения
  QList< QList<double> > valueMatrix;

  // матрица ответов
  // то, что хотим получить в ответе
  QList< double > answerMatrix;

  /**
   * Объект, который работает с базой и получает все необходимые поля
   */
  ForecastAnalyzer *analyser        = nullptr;

  meteo::GeoPoint*   current_point  = nullptr; // текущая точка, по которой считаем модель

  meteo::GeoData*    predicted_data = nullptr; // спрогнозированные данные

  bool isDebug                      = false;   // включаем или отключаем отладку
  QDateTime *datetime               = nullptr; // текущее время и дата
  QString *errormsg                 = nullptr; // сообщение об ошибке

};

}
}

#endif
