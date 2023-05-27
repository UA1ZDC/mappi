#ifndef METEO_ML_METHODS_ML_H
#define METEO_ML_METHODS_ML_H

#include <stdlib.h>
#include <QString>
#include <QElapsedTimer>

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
 *  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();
 *        // устанаваливаем отладку
 *        analyser->setDebugOn()
 *        // устанавливаем множитель десктриптора под которым будем сохранять
 *        // в 0 (т.е. не используем)
 *        .setFieldDescription(0)
 *        .setParam(11002);
 *
 *  MethodML->setAnalyzer(analyser)
 *          .setDebugOn()
 *          .setPoint( (double)0.851429, (double)0.774345 )
 *          .predictKRLSModel();
 *          .trainKRLSTodayModel();
 *
 *
 *
 */
class MethodML
{

public:
  MethodML();
  ~MethodML();


  /**
   * Обучаем кумулятивно (за сегодня )
   * модель KRLS
   * dlib/docs/ml.html#krls
   * обязательно должен быть задан Analyser с параметрами
   *
   * @return [MethodML]
   */
  MethodML& trainKRLSTodayModel();

  // обучаем на фактических данных
  MethodML& trainKRLSTodayFactModel();

  //
  // получаем прогностическое значение в точке
  // предварительно необходимо загрузить параметры через setAnalyzer
  // Analyser
  //
  double      predictKRLSModel();
  double      predictKRLSModel(double lat, double lon);


  //
  // прогнозируем поле по станциям и записываем его в БД
  //
  MethodML& predictOnStations();

  //
  // получаем значения по всем обученным моделям и записывае поле в БД
  //
  MethodML& predictOnAllModels();

  MethodML& trainModel();

  // устанавливаем список параметров, по которым будем считать модель
  MethodML& setForPredictParamList(QList<int> paramList);
  // список параметров, которые хотим спрогнозировать
  MethodML& setPredictParamList(QList<int> paramList);

  // запускаем прогнозирование по станциям
  MethodML& fillMatrixForTrain(QList<QString> stantions, QList<QString> stantionspdc);
  MethodML& fillMatrixForTrain();


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
  MethodML& loadModel();
  MethodML& loadModelByFid( QString file_id );
  // вспомогательный метод загрузки модели из файловой системы
  MethodML& loadModelFromByteArray( QByteArray ba );

  /**
   * загружаем список моджелей из базы
   * @return [description]
   */
  void loadAllModels(QList<meteo::GeoPoint>* models, QList<QString>* file_ids );
  /**
   * Сохраняем модель в базу
   * @return [description]
   */
  MethodML& saveModel();

  // получаем сериализованную модель
  QByteArray  getModel();
  // устанавливаем модель
  MethodML& setModel(QByteArray ba){ this->model = ba; return *this; };

  /**
   * Получаем аналайзер
   * @return [description]
   */
  Analyser& getAnalyzer()   { return *this->analyser; }

  /**
   * Задаем аналайзер
   * @return [description]
   */
  MethodML&       setAnalyzer(Analyser* m_analyzer) { this->analyser = m_analyzer; return *this; }
  // устанавливаем дату/время
  MethodML&       setDateTime(  QDateTime dt );
  // Устанавливаем текущую точку
  MethodML&       setPoint(  meteo::GeoPoint point );
  // в радианах
  MethodML&       setPoint( double lat, double lon );




  MethodML&       dnnTrain();




  // отправляем сообщение в дебаг, если надо
  MethodML&       debugMSG( QString txt );
  MethodML&       setDebugOn();
  MethodML&       setDebugOff();

private:

  // количество дней, после которых начинаем забывать, что
  // мы тренировали
  int max_trained_days = 20;
  // максимальный порог ошибки
  float max_loss = 0.1;

  typedef dlib::matrix<double,1,1>             KRLS_type; // декларируем тип переменной, в которой будем содержать данные
  typedef dlib::radial_basis_kernel<KRLS_type> KRLS_kernel_type; // инициализируем ядро алгоритма
  dlib::krls<KRLS_kernel_type>    *krls_model  = nullptr; //модель, с которой работаем

  //
  // Сериализованная модель
  // которая хранится в классе в виде байт-последовательности
  //
  QByteArray         model;

  // список параметров по которому будем производить обучение
  QList<int>         paramList;
  // список параметров по которому будем производить обучение
  QList<int>         parampdcList;
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
  Analyser   *analyser      = nullptr;

  /**
   * @brief Объект подключения к ьазе данных
   * 
   */
  TObanalDb *obanal_db_ = nullptr;

  meteo::GeoPoint*   current_point  = nullptr; // текущая точка, по которой считаем модель

  meteo::GeoData*    predicted_data = nullptr; // спрогнозированные данные

  bool isDebug                      = false;   // включаем или отключаем отладку
  QDateTime *datetime               = nullptr; // текущее время и дата
  QString *errormsg                 = nullptr; // сообщение об ошибке

};

}
}

#endif
