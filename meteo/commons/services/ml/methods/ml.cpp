#include "ml.h"

using namespace dlib;

namespace meteo {
namespace ml {



MethodML::MethodML()
{

  analyser      = new meteo::ml::Analyser();
  obanal_db_    = new TObanalDb();
  // подключаемся к базе данных файловой системы
  obanal_db_->connectToGridFs(QString("mlmodel"));
  // время
  datetime      = new QDateTime(QDateTime::currentDateTimeUtc());
  current_point = new meteo::GeoPoint();

}

MethodML::~MethodML()
{
  delete analyser;
  delete datetime;
  delete obanal_db_;
  delete current_point;
}


/**
 * Обучаем кумулятивно (за сегодня )
 * модель KRLS
 * dlib/docs/ml.html#krls
 * обязательно должен быть задан Analyser с параметрами
 * @return [description]
 */
MethodML& MethodML::trainKRLSTodayModel(){

  // setDateTime( datetime->addSecs( -1*60*60*24 ) );

  // функция расчета c порогом ошибки
  krls_model = new dlib::krls<KRLS_kernel_type>( KRLS_kernel_type(max_loss), 0.1, max_trained_days );
  // массив с данными
  KRLS_type m;

                // очищаем данные
  analyser->clearData()
                // .setDebugOn()
                // получаем поле с данными
                .setDateTime( *datetime )
                .setHour(0)
                .getFactDataFromSRV()
                .getGribDataFromSRV();


  // вытаскиваем данные за нули (это будет ответ, Y)
  meteo::GeoData all_data = analyser->getAllData();

  if ( isDebug ){
    debugMSG( QString("Полученные данные: %1").arg(all_data.size()) );
  }

  // получаем поле прогнозов на 00
  // будем по нему считать, это  X
  obanal::TField grib_data = analyser->getAllgribField();
  if ( isDebug ){
    debugMSG( QString("Полученные грибы: %1").arg(grib_data.kolData()) );
  }

  // если мы получили данные
  // и еще не задали точку
  // то задаем ее
  if ( all_data.size() >0 ){
      //
      // получаем станцию, по которой будем считать
      //
      // проходимся по списку всех станций
      // и пытаемся найти по координатам станцию которая пришла
      for ( int j=0; j<all_data.size() ; j++ ){

        // устанавливаем текущую точку
        current_point->setLat( all_data.at(j).point.lat() );
        current_point->setLon( all_data.at(j).point.lon() );

        debugMSG("Загружаем предобученную модель из базы данных");

        // загружаем базу
        loadModel();

        // if (grib_data.pointValue( all_data.at(j).point ) == 0 ){
        //   debug_log<<"Данные гриб равны нулю, пропускаем";
        //   continue;
        // }

        m(0) = grib_data.pointValue( all_data.at(j).point );

        // тренируем
        krls_model->train(m, all_data.at(j).data);

        if ( isDebug ){
          debug_log << "Тренируем модель: grib="<< m(0) << "; val="<<all_data.at(j).data
                    << " Точка: lat=" << all_data.at(j).point.lat()
                    << "lon=" << all_data.at(j).point.lon() << "gribs:"<<grib_data.kolData()  ;
        }

        debugMSG("Сохраняем модель в базу данных");
        // сохраняем модель
        saveModel();

      }//endfor


  }else{
    debugMSG("Ноль данных, не продолжаем.");
    delete krls_model;
  }

  return *this;
}




/**
 * Обучаем кумулятивно (за сегодня )
 * на данных приземный набллюдений
 * модель KRLS
 * dlib/docs/ml.html#krls
 * обязательно должен быть задан Analyser с параметрами
 * @return [description]
 */
MethodML& MethodML::trainKRLSTodayFactModel(){

  // setDateTime( datetime->addSecs( -1*60*60*24 ) );

  // функция расчета c порогом ошибки
  krls_model = new dlib::krls<KRLS_kernel_type>( KRLS_kernel_type(max_loss), 0.1, max_trained_days );
  // массив с данными
  KRLS_type m;

                // очищаем данные
  analyser->clearData()
                // .setDebugOn()
                // получаем поле с данными
                // .setDateTime( *datetime )
                // .setHour(0)
                .setForecasthour(0)
                .setCenter(250)
                .setModel(250)
                .setSynopStationType()
                // забираем  за текущий срок
                .getFactDataFromSRV()
                // забираем  за предшествующий срок
                .setHourStep(3)
                .prevTime()
                .getFieldDataFromSRV();


  // вытаскиваем данные за нули (это будет ответ, Y)
  meteo::GeoData all_data = analyser->getAllData();

  if ( isDebug ){
    debugMSG( QString("Полученные данные: %1").arg(all_data.size()) );
  }

  // получаем поле 
  // будем по нему считать, это  X
  obanal::TField field_data = analyser->getAllgribField();
  if ( isDebug ){
    debugMSG( QString("Полученные поля: %1").arg(field_data.kolData()) );
  }

  // если мы получили данные
  // и еще не задали точку
  // то задаем ее
  if ( all_data.size() >0 ){
      //
      // получаем станцию, по которой будем считать
      //
      // проходимся по списку всех станций
      // и пытаемся найти по координатам станцию которая пришла
      for ( int j=0; j<all_data.size() ; j++ ){

        // устанавливаем текущую точку
        current_point->setLat( all_data.at(j).point.lat() );
        current_point->setLon( all_data.at(j).point.lon() );

        debugMSG("Загружаем предобученную модель из базы данных");

        // загружаем базу
        loadModel();

        m(0) = field_data.pointValue( all_data.at(j).point );

        // тренируем
        krls_model->train(m, all_data.at(j).data);

        if ( isDebug ){
          debug_log << "Тренируем модель: surf_field="<< m(0) << "; all_data_val="<<all_data.at(j).data
                    << " Точка: lat=" << all_data.at(j).point.lat()
                    << "lon=" << all_data.at(j).point.lon() << "all_field:"<<field_data.kolData()  ;
        }

        debugMSG("Сохраняем модель в базу данных");
        // сохраняем модель
        saveModel();

      }//endfor


  }else{
    debugMSG("Ноль данных, не продолжаем.");
    delete krls_model;
  }

  return *this;
}

/**
 * получаем прогностическое значение в точке
 * предварительно необходимо загрузить параметры через setAnalyzer
 * Analyser
 * должна быть задана точка, по которой строим прогноз
 * current_point
 * должно быть загружено поле грибов
 *
 * @return [description]
 */
double MethodML::predictKRLSModel(){
  double val=0;

  // функция расчета c порогом ошибки
  krls_model = new dlib::krls<KRLS_kernel_type>( KRLS_kernel_type(max_loss), 0.1, max_trained_days );

  // массив с данными
  KRLS_type m;

  // получаем поле прогнозов на 00
  // будем по нему считать, это  X
  obanal::TField grib_data = analyser->getAllgribField();
  // если грибы не загружены, пробуем загрузить их
  if ( grib_data.kolData()==0 ){
                  // очищаем данные
    grib_data = analyser->clearData()
                    // получаем поле с данными
                    .setDateTime( *datetime )
                    .setHour(0)
                    .getGribDataFromSRV()
                    .getAllgribField();
  }


  if ( isDebug ){
    debug_log<< "Полученные грибы: " << grib_data.kolData();
  }


  debugMSG("Загружаем предобученную модель из базы данных");
  // загружаем базу
  loadModel();

  // задаем значение
  m(0) = grib_data.pointValue( *current_point );
  // прогнозируем
  val = (*krls_model)(m);

  if ( isDebug ){
    debug_log << m(0) << val;
  }

  return val;
}

/**
 * тоже самое только точку, по которой прогнозируем передаем в параметрах
 * @return [description]
 */
double MethodML::predictKRLSModel(double lat, double lon){
  // устанавливаем текущую точку
  current_point->setLat( lat );
  current_point->setLon( lon );

  return predictKRLSModel();
}


/**
 * ================================ Частные методы ==========================
 */

/**
 * @brief загружаем модель из файловой системы
 * 
 * @param file_id 
 * @return MethodML& 
 */
MethodML& MethodML::loadModelByFid( QString file_id ){
  
  QByteArray ba;
  obanal_db_->readFileByFileId( ba, file_id  );

  loadModelFromByteArray(ba);
  return *this;
}

/**
 * @brief загрузка 
 * 
 * @param ba 
 * @return MethodML& 
 */
MethodML& MethodML::loadModelFromByteArray( QByteArray ba ){

  std::istringstream inn(ba.toStdString(), std::ios::binary);
  std::istream input(inn.rdbuf());


  std::string filename="/tmp/mlfunction_predict.dat";
  // debugMSG("Создан временный файл для модели");
  // считываем файл
  QFile *file = new QFile( QString::fromStdString(filename) );

  if ( file->open(QFile::ReadWrite) ){
    file->write(ba);
    // удаляем и очищаем файл
    file->close();

    krls_model = new dlib::krls<KRLS_kernel_type>( KRLS_kernel_type(max_loss), 0.1, max_trained_days );
    // debugMSG("Десериализуем модель в существующую");

    try {
      dlib::deserialize( filename )>> *krls_model;
    }
    catch (dlib::serialization_error& e) {
      error_log<<"Ошибка десериализации модели из базы, пропускаем.";
    }

    file->remove();
    delete file;
  }

  // dlib::deserialize( ker, input );

  // krls_model->function = ker;
  // krls_model->set_trainer(ker);
  // krls_model = new dlib::krls<KRLS_kernel_type>( ker, 0.1, 5 );

  debugMSG("Модель загружена из базы данных");

  return *this;
}

/**
 * Загружаем модель из базы
 * @return [description]
 */
MethodML& MethodML::loadModel(){
  // байтаррэй
  QByteArray ba;

  // загружаем
  if ( false == obanal_db_->loadlMLModel(ba, //сериализованный байтаррэй
                          analyser->getFilledDescr(),
                          QString("mlmodel"),
                          QString("krls"),
                          *current_point
                         ) ){
    debugMSG("Нет файла с моделью в базе данных. Загрузка отменена.");
    return *this;
  }

  loadModelFromByteArray(ba);

  return *this;
}

/**
 * Сохраняем модель в базу
 * @return [description]
 */
MethodML& MethodML::saveModel(){

  //
  // Оставим это на случай, если найдем как сериализовать не в файл, а в поток
  //
  // std::ostringstream out(std::ios::binary);
  // получаем главную функцию с расчетами
  // decision_function<KRLS_kernel_type> funct = krls_model->get_decision_function();
  // сериализуем
  // dlib::serialize( funct, out );

  std::string filename="/tmp/mlfunction_predict.dat";
  dlib::serialize( filename )<< *krls_model;

  // считываем файл
  QFile *file = new QFile( QString::fromStdString(filename) );


  // преобразуем в байтаррэй
  // QByteArray ba( out.str().data(), out.str().size() );
  if ( file->open(QFile::ReadWrite) ){
    QByteArray ba( file->readAll() );

    // удаляем и очищаем файл
    file->close();
    file->remove();
    delete file;

    // сохраняем
    obanal_db_->saveMLModel(  ba, //сериализованный байтаррэй
                            analyser->getFilledDescr(),
                            QString("mlmodel"),
                            QString("krls"),
                            *current_point
                           );

    debugMSG("Модель сохранена в базу данных");
  }

  return *this;
}

/**
 * загружаем все модели из базы
 * @return [description]
 */
void MethodML::loadAllModels( QList<meteo::GeoPoint> *points, QList<QString> *file_ids ){
  // загружаем
  if ( false == obanal_db_->loadAllMLModels(
                          points, //сериализованный байтаррэй
                          file_ids, //сериализованный байтаррэй
                          analyser->getFilledDescr(),
                          "krls" ) ){
    debugMSG("Не удалось загрузить обученные модели");
  }

  return ;
}

/**
 * Прогнозируем значения на всех станциях, которые находим
 * @return [description]
 */
MethodML& MethodML::predictOnStations(){
  QElapsedTimer timer;
  timer.start();

  // загружаем все станции
  meteo::GeoData stations = analyser->getStationsCoords();


  // загружаем все модели
  QList<meteo::GeoPoint> modelList;
  QList<QString> fileIdsList;

  loadAllModels(&modelList, &fileIdsList);

  // подготовленные для объективного анализа данные
  meteo::GeoData prepared_data;

  // будем по нему считать, это  X
  obanal::TField field_data = analyser->getAllgribField();

  // сколько моделей не нашли, а хотели бы
  int lost_models = 0;
  // сколько моделей нашли в базе
  int find_models = 0;

  //
  // проверяем, получили ли грибы
  //
  if ( field_data.kolData() == 0 ){
    // если грибов нет, то и делать нечего
    debugMSG( QString("Нет данных полей %1").arg(field_data.kolData()) );
    return *this;
  }

  // загружаем все существующие модели по найденным станциям
  for (int i = 0; i < stations.size(); ++i) {
    // задаем координаты текущей точки
    current_point->setLat(  stations.at(i).point.lat()  );
    current_point->setLon(  stations.at(i).point.lon()  );

    krls_model=nullptr;

    // search for element and if found change data
    auto itr = std::find_if(modelList.begin(), modelList.end(), [&](meteo::GeoPoint &pnt_) { return pnt_==stations.at(i).point; });
    if(itr != modelList.end()) {
        int j = itr - modelList.begin();
        // debugMSG( QString("Нашли модель через быстрый поиск. j = %1 ").arg(j) );
        // увеличиваем количество найденных моделей
        find_models++;
        loadModelByFid(fileIdsList.at(j));
    } 

    // если не загрузилось с первого раза - пробуем другой вариант, 
    // долгий, через поиск в базе данных по координатам
    // == upd == этот поиск не работает, если файл не найден в списке моделей, то его нет и в базе ==
    // if ( krls_model==nullptr ){
    //   debugMSG( QString("Не получилось найти модель через быстрый поиск. Пробуем через запрос к базе. %1 %2").arg(current_point->lat()).arg(current_point->lon()) );
      // загружаем модель по точке
    //   loadModel();
    // }

    if ( krls_model==nullptr ){
      debugMSG( QString("Нет модели по станции %1 %2").arg(current_point->lat()).arg(current_point->lon()) );
      // Увеличиваем количество ненайденных моделей
      lost_models++;
      continue;
    }

    KRLS_type m;
    //
    // запрашиваем прогнозы по модели станциям
    //
    // задаем значение
    m(0) = field_data.pointValue( *current_point );
    // прогнозируем
    double val;
    val = (*krls_model)(m);

    // prepared_data должно быть заполнено для объективного анализа
    // щзаполняем массив с данными
    prepared_data.append(
            meteo::MData(
                current_point->lat(),
                current_point->lon(),
                val,
                TRUE_DATA
            ) );
  }

  debugMSG( QString("Данные по станциям заполнены скорректированными прогнозами. Всего %1").arg(prepared_data.size()) );

  debugMSG( QString("Найдено моделей ИНС: %1 ; Не найдено: %2 ; Количество станций по которым искали: %3").arg(find_models).arg(lost_models).arg(stations.size()) );

  debugMSG( QString("Выполнение операции завершено за %1 с").arg( timer.elapsed()/1000 ) );

  //
  // выполняем объективный анализ
  // и сохраняем поле в базе
  //
  analyser->setPreparedData(prepared_data);

  debugMSG( "Заполнили данные и установили Analyser" );

  return *this;
}



/**
 * получаем значения по всем обученным моделям и записывае поле в БД
 * @return [description]
 */
MethodML& MethodML::predictOnAllModels(){

  // загружаем все модели
  QList<meteo::GeoPoint> modelList;
  QList<QString> fileIdsList;

  loadAllModels(&modelList, &fileIdsList);

  // подготовленные для объективного анализа данные
  meteo::GeoData prepared_data;

  // будем по нему считать, это  X
  obanal::TField grib_data = analyser->getAllgribField();

  //
  // проверяем, получили ли грибы
  //
  if ( grib_data.kolData() == 0 ){
    // если грибов нет, то и делать нечего
    debugMSG( QString("Нет данных грибов %1").arg(grib_data.kolData()) );
    return *this;
  }

  // загружаем все существующие модели по найденным станциям
  for (int i = 0; i < modelList.size(); ++i) {
    // задаем координаты текущей точки
    current_point->setLat(  modelList.at(i).lat()  );
    current_point->setLon(  modelList.at(i).lon()  );

    krls_model=nullptr;

    // загружаем модель по точке
    if ( i < fileIdsList.count() ){
      loadModelByFid(fileIdsList.at(i));
    }else{
      loadModel();
    }

    if ( krls_model==nullptr ){
      debugMSG( QString("Нет модели по станции %1 %2").arg(current_point->lat()).arg(current_point->lon()) );
      continue;
    }

    KRLS_type m;
    //
    // запрашиваем прогнозы по модели станциям
    //
    // задаем значение
    m(0) = grib_data.pointValue( *current_point );
    // прогнозируем
    double val;
    val = (*krls_model)(m);

    // prepared_data должно быть заполнено для объективного анализа
    // щзаполняем массив с данными
    prepared_data.append(
            meteo::MData(
                current_point->lat(),
                current_point->lon(),
                val,
                TRUE_DATA
            ) );
  }

  debugMSG( QString("Данные по станция заполнены скорректированными прогнозами. Всего %1").arg(prepared_data.size()) );
  //
  // выполняем объективный анализ
  // и сохраняем поле в базе
  //
  analyser->setPreparedData(prepared_data);

  debugMSG( "Заполнили данные и установили Analyser" );

  return *this;
}


/**
 * Обучаем модель
 * @return [description]
 */
MethodML& MethodML::trainModel(){


  // задаем начальные условия
  analyser->setDebugOff()
                .setParam(12108);

  // проходимся по датам и
  // загружаме поля для обучения

  meteo::GeoPoint stpoint;
  stpoint.setLat( 0.0 );
  stpoint.setLon( 0.0 );


  // декларируем тип переменной, в которой будем содержать данные
  typedef dlib::matrix<double,3,1> sample_type;
  // инициализируем ядро алгоритма
  typedef dlib::radial_basis_kernel<sample_type> kernel_type;
  // функция расчета и порогом ошибки
  dlib::krls<kernel_type> test(kernel_type(0.1),0.1,5);
  // массив с данными
  sample_type m;

  // время
  QDateTime *dt = new QDateTime(QDateTime::currentDateTimeUtc());

  for (int i = 15; i > 0; --i)
  {
    // очищаем данные
    analyser->clearData();

    if ( isDebug ){
      debug_log<<i;

      debug_log << dt->addSecs( -i*60*60*24 ).toString(Qt::ISODate).toStdString();
    }

    m(1) = dt->addSecs( -i*60*60*24 ).date().dayOfYear() / 365;
    m(2) = dt->addSecs( -i*60*60*24 ).date().month() / 12;

    // получаем поле с данными
    analyser->setDateTime( dt->addSecs( -i*60*60*24 ) )
            .setHour(0)
            .getFactDataFromSRV()
            .getGribDataFromSRV();

    // вытаскиваем данные за нули (это будет ответ, Y)
    meteo::GeoData all_data = analyser->getAllData();
    if ( isDebug ){
      debug_log<< "Полученные данные: " <<all_data.size();
    }

    // получаем поле прогнозов на 00
    // будем по нему считать, это  X
    obanal::TField grib_data = analyser->getAllgribField();

    // если мы получили данные
    // и еще не задали точку
    // то задаем ее
    if ( all_data.size() >0 && MnMath::isEqual( stpoint.lat(), 0 ) ){
      stpoint.setLat( all_data.at(36).point.lat() );
      stpoint.setLon( all_data.at(36).point.lon() );
    }

    //
    // получаем станцию, по которой будем считать
    //
    // проходимся по списку всех станций
    // и пытаемся найти по координатам станцию которая пришла
    for ( int j=0; j<all_data.size() ; j++ ){

      // если находим станцию в списке станций - то
      // возвращаем ОК и прерываемся
      // debug_log << all_data.at(j).point.lat() << stpoint.lat();
      // debug_log << MnMath::isEqual(all_data.at(j).point.lat(), stpoint.lat() );
      if ( MnMath::isEqual(all_data.at(j).point.lat(), stpoint.lat() )
           &&
           MnMath::isEqual( all_data.at(j).point.lon(), stpoint.lon() ) ){

        m(0) = grib_data.pointValue( all_data.at(j).point );

        test.train(m, all_data.at(j).data);

        if ( isDebug ){
          debug_log<<"Тренируем модель: grib="<< m(0) << "; val="<<all_data.at(j).data ;
        }

        // debug_log << all_data.at(j).point.lat()  << all_data.at(j).point.lon();
      }


    }//endfor


    // return *this;

  }


  analyser->clearData();
  // получаем поле с данными
  analyser->setDateTime( *dt )
          .setHour(0)
          .getFactDataFromSRV()
          .getGribDataFromSRV();

  // вытаскиваем данные за нули (это будет ответ, Y)
  meteo::GeoData all_data = analyser->getAllData();


  // получаем поле прогнозов на 00
  // будем по нему считать, это  X
  obanal::TField grib_data = analyser->getAllgribField();

  m(1) = dt->date().dayOfYear() / 365;
  m(2) = dt->date().month() / 12;

  for ( int j=0; j< all_data.size(); j++ ){
    // если находим станцию в списке станций - то
    // возвращаем ОК и прерываемся
    if (  MnMath::isEqual(all_data.at(j).point.lat(), stpoint.lat() )
           &&
           MnMath::isEqual( all_data.at(j).point.lon(), stpoint.lon() ) ){

      m(0) = grib_data.pointValue( all_data.at(j).point );
      
      if ( isDebug ){
        debug_log<<"Проверяем модель: grib="<< m(0) << "; " << " val=" << all_data.at(j).data <<"; model="<<test(m);
      }

      // debug_log << all_data.at(j).point.lat()  << all_data.at(j).point.lon();
    }
  }

  delete dt;

  return *this;
}


/**
 * [MethodML::setPredictParamList description]
 * @param  paramList [description]
 * @return           [description]
 */
MethodML& MethodML::setPredictParamList(QList<int> paramList){
  paramList = paramList;
  return *this;
}

/**
 * [MethodML::setPredictParamList description]
 * @param  parampdcList [description]
 * @return           [description]
 */
MethodML& MethodML::setForPredictParamList(QList<int> parampdcList){
  parampdcList = parampdcList;
  return *this;
}

/**
 * Прогнозируем по станциям
 * stantions - станции, по которым будем прогнозировать
 * stantionspdc - станции, которые будем использовать для анализа
 * @param  paramList [description]
 * @return           [description]
 */
MethodML& MethodML::fillMatrixForTrain(QList<QString> stantions, QList<QString> stantionspdc){
  stantions    = stantions;
  stantionspdc = stantionspdc;
  fillMatrixForTrain();
  return *this;
}

/**
 * Заполняем матрицы для прогнозов
 * @return [description]
 */
MethodML& MethodML::fillMatrixForTrain(){

  valueMatrix.clear();
  answerMatrix.clear();

  // необходимо получить две матрицы значений
  // матрица со списком значений параметров со станций stantionspdc (которые будем загонять) (x)
  // и матрица со списком значений параметров со станций, по которым надо получить ответ (Y)
  // valueMatrix;
  //
  // если список параметров не пуст
  if ( !parampdcList.isEmpty() ){
    // проходимся по параметрам (температру,а давление, влажность ...)
    for (int i = 0; i < parampdcList.count(); ++i)   {
      // очищаем данные
      analyser->clearData()
              // устанавливаем нужнй параметр (все остальное уже задано)
              .setParam(parampdcList.at(i))
              // получаем фактические данные
              .getFactDataFromSRV()
              // заполняем структуры из списка станций
              .fillDataFromStationList( stantionspdc );

      debugMSG( QString("Загружены данные с параметром %1").arg( parampdcList.at(i)) );

      // получаем список значений с координатами
      meteo::GeoData* data = new meteo::GeoData();
      data = &analyser->getPreparedData();

      // заполняем массив с данными наблюдений
      for ( int j=0; j<data->size(); j++ ){
        if ( valueMatrix.count()>=(j+1) ){
          valueMatrix[j].append(data->at(j).data);
        }else{
          valueMatrix.append( QList<double>() << data->at(j).data );
        }
      }

      // заполняем массив для станций с ответами
      analyser->clearPreparedData()
              .clearSubstractedData()
              .fillDataFromStationList( stantions );

      data = new meteo::GeoData();
      data = &analyser->getPreparedData();

      // заполняем массив с данными наблюдений для ответа
      //
      // если это темпетарутра
      if ( paramList.indexOf( parampdcList.at(i) ) != -1 ) {
        if ( data->size()==0 ) {
          debugMSG("Данные для ответов по станциям пусты. Пропускаем обучение");
          valueMatrix.clear();
          answerMatrix.clear();
          return *this;
        }
        for ( int j=0; j<data->size(); j++ ){
          // if ( answerMatrix.count()>=(j+1) ){
          answerMatrix.append(data->at(j).data);
          // }else{
          //   answerMatrix.append( QList<double>() << data->at(j).data );
          // }

        }
      }

      delete data;
    }

  }


  for (int i = 0; i < valueMatrix.count(); ++i){
    if ( isDebug ){
      debug_log<< "Массив с данными по станциям для прогноза №"<< i;
    }
    // for (int j = 0; j < valueMatrix.at(i).count(); ++j)  {

      // debug_log<< "Данные "<< valueMatrix.at(i).at(j);

    // }
  }

  for (int i = 0; i < answerMatrix.count(); ++i){
    if ( isDebug ){
      debug_log<< "Массив с данными по станциям для ответа №"<< i << " : "<< answerMatrix.at(i);
    }
    // for (int j = 0; j < answerMatrix.at(i).count(); ++j)  {

    // debug_log<< "Данные "<< answerMatrix.at(i);

    // }
  }

  // answerMatrix;
  //
  // закончили заполнение матриц данными
  // теперь надо прогнозировать

  // добавляем время
  for (int i = 0; i < valueMatrix.count(); ++i)  {
    valueMatrix[i].append( double(analyser->getHour()) ); // нормализовать будем потом
  }

  return *this;
}

/**
 * Обучаем модель
 * @return [description]
 */
MethodML& MethodML::dnnTrain(){

  typedef matrix<double,20,1> params_type;


  params_type sample;

  std::vector<params_type> samples;
  std::vector<double> labels;

  typedef radial_basis_kernel<params_type> kernel_type;
  krr_trainer<kernel_type> trainer;





  //
  // если модель есть, то пытаемся ее десериализовать
  //
  // if ( model.size()>0 ){
  //   decision_function<kernel_type> test;
  //   std::istringstream inn( model.toStdString(), std::ios::binary);
  //   std::istream input(inn.rdbuf());

  //   dlib::deserialize( test, inn );



  //   // делаем следующий шаг по времени (чтобы проверить)
  //   analyser->nextTime();
  //   // загружаем новые данные
  //   fillMatrixForTrain();
  //   debug_log << "=========== Прогнозируем ===========";
  //   debug_log << "исходные данные ";
  //   for (int k = 0; k < valueMatrix.count() ; ++k) {
  //     for (int l = 0; l < valueMatrix[k].count() ; ++l) {
  //       sample(k+k*l) = valueMatrix.at(k).at(l) / 500;

  //       debug_log<< valueMatrix.at(k).at(l) ;
  //     }
  //   }

  //   debug_log << "Что спрогнозировали " << test(sample) << " что получилось " <<answerMatrix.at(0);


  //   return *this;
  // }












  //    ------------------------------    //

  for (size_t i = 0; i < 57; ++i)
  {
    if ( answerMatrix.count()>0 && valueMatrix.count()>0 ){
      // training_images[i].set_size(row_i,col_i);
      // проходимся по значениям
      for (int k = 0; k < valueMatrix.count() ; ++k) {
        for (int l = 0; l < valueMatrix[k].count() ; ++l) {
          sample(k+k*l) = valueMatrix.at(k).at(l) / 500;
        }
      }

      samples.push_back(sample);

      // training_images[i].set_size(row_o,col_o);
      // проходимся по значениям
      for (int k = 0; k < answerMatrix.count() ; ++k) {
        // training_labels[i] = answerMatrix.at(k);
        // for (int l = 0; l < answerMatrix[k].count() ; ++l) {
        //   training_labels[i](k,l) = answerMatrix.at(k).at(l);
        // }
        // net.train( sample, answerMatrix.at(k) );
      }

      labels.push_back(answerMatrix.at(0) / 500 );
    }
    analyser->nextTime();
    fillMatrixForTrain();
  }
  //    ------------------------------    //


  const double gamma = 0.001;//3.0/compute_mean_squared_distance(randomly_subsample(samples, 10));

  trainer.set_kernel(kernel_type(gamma));

  // now train a function based on our sample points
  decision_function<kernel_type> test = trainer.train(samples, labels);

  // делаем следующий шаг по времени (чтобы проверить)
  analyser->nextTime();
  // загружаем новые данные
  fillMatrixForTrain();

  serialize("/tmp/saved_function.dat") << test;
  QFile file( "/tmp/saved_function.dat" );
  if ( file.open(QFile::ReadWrite) ){
    model= file.readAll();
    file.close();
    debugMSG("Модель считана из файла, будем сохранять");
  }else{
    debugMSG("Не смогли считать модель из файла. Сохранять нечего.");
  }

  if ( isDebug ){
    debug_log << "=========== Прогнозируем ===========";
    debug_log << "исходные данные ";
  }
  for (int k = 0; k < valueMatrix.count() ; ++k) {
    for (int l = 0; l < valueMatrix[k].count() ; ++l) {
      sample(k+k*l) = valueMatrix.at(k).at(l) / 500;
      if ( isDebug ){
        debug_log<< valueMatrix.at(k).at(l) ;
      }
    }
  }
  if ( isDebug ){
    debug_log << "Что спрогнозировали " << test(sample)*500 << " что получилось " <<answerMatrix.at(0);
  }

  return *this;
}


/**
 * получаем сериализованную модель
 * @return [description]
 */
QByteArray  MethodML::getModel(){
  return model;
}

/**
 * Установка значения переменной
 * @param  date [description]
 * @return          [description]
 */
MethodML& MethodML::setDateTime(  QDateTime dt ){
  datetime = new QDateTime( dt );
  return *this;
}

/**
 * Отправляем сообщение в дебаг, если надо
 * @param  txt [description]
 * @return     [description]
 */
MethodML&  MethodML::debugMSG( QString txt ){
  if ( isDebug ){
    debug_log << txt;
  }
  return *this;
}


/**
 * Включение/выключение режима отладки
 * @return [description]
 */
MethodML&  MethodML::setDebugOn(){
  isDebug = true;
  return *this;
}

/**
 * Включение/выключение режима отладки
 * @return [description]
 */
MethodML&  MethodML::setDebugOff(){
  isDebug = false;
  return *this;
}


/**
 * Задаем координаты текущей точки
 * @param  point [description]
 * @return       [description]
 */
MethodML&  MethodML::setPoint(  meteo::GeoPoint point ){
  current_point->setLat(point.lat());
  current_point->setLon(point.lon());
  return *this;
}

/**
 * Задаем координаты текущей точки
 * @param  point [description]
 * @return       [description]
 */
MethodML&  MethodML::setPoint( double lat, double lon ){
  current_point->setLat(lat);
  current_point->setLon(lon);
  return *this;
}



} // namespace ml
} //namespace meteo
