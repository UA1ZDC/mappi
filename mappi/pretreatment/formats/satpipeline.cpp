#include <mappi/pretreatment/formats/satpipeline.h>
#include <QtConcurrent/QtConcurrent>

using namespace meteo;

namespace mappi::po {

/*!
 * \brief Расчёт угла между меридианом,проходящим через начальную точку и линией, соединяющей начальную и конечную точку, относительно начальной точки (угол поворота изображения относительно начальной/верхней точки)
 * \param geoB начальная точка в географических координатах
 * \param geoE конечная точка  в географических координатах
 * \return Угол поворота, радиан
 */
  float rotateAngle(Coords::GeoCoord geoB, Coords::GeoCoord geoE) {
    // printf("beg: lat=%f lon=%f\nEnd: lat=%f lon=%f\n", rad2deg(geoB.lat), rad2deg(geoB.lon),
    // 	 rad2deg(geoE.lat), rad2deg(geoE.lon)
    // 	 );
    float d = Coords::geoDistance(geoB, geoE);
    return MnMath::spAcos((cos(geoE.lat) * cos(geoB.lon - geoE.lon) - cos(d) * cos(geoB.lat)) / sin(d) / sin(geoB.lat));
  }

  void SatPipeline::clear() {
    QDir dir(_params.output_dir);
    dir.removeRecursively();
  }

//! Установка даты/времения первой и последней строк сканирования
/*!
  \param dtBeg  дата/время первой строки сканирования
  \param dtEnd  дата/время последней строки сканирования
*/
  void SatPipeline::setDateTime(const QDateTime &dtBeg, const QDateTime &dtEnd) {
    if (nullptr == _sat) {
      error_log << QObject::tr("Ошибка установки даты/времени. Кадр сформирован без создания спутника");
      return;
    }

    debug_log << "dtBeg" << dtBeg.toString("dd.MM.yy hh:mm:ss.zzz")
              << "dtEnd" << dtEnd.toString("dd.MM.yy hh:mm:ss.zzz");

    _sat->setDateTime(dtBeg, dtEnd);

    //определение положения начальной и конечной точек сканирования
    Coords::GeoCoord geoB;
    Coords::GeoCoord geoE;
    if (!_sat->getPosition(_sat->timeFromTLE(dtBeg), &geoB) ||
        !_sat->getPosition(_sat->timeFromTLE(dtEnd), &geoE)) {
      error_log << QObject::tr("Ошибка определения положения спутника");
    }

    var(geoB.lat * 180 / 3.14);
    var(geoB.lon * 180 / 3.14);
    var(geoE.lat * 180 / 3.14);
    var(geoE.lon * 180 / 3.14);

    _rAngle = rotateAngle(geoB, geoE); //TODO
    debug_log << "rangle=" << _rAngle * 180 / 3.14;
  }

//! Загрузка орбитальных параметров
/*!
  \param satName название спутника
  \param tleFile файл с орбитальными параметрами
*/
  void SatPipeline::setTLE(const QString &satName, const QString &tleFile) {
    if (nullptr == _sat) {
      error_log << QObject::tr("Ошибка загрузки орбитальных параметров. Кадр сформирован без создания спутника");
      return;
    }

    if (!_sat->readTLE(satName, tleFile)) {
      error_log << QObject::tr("Ошибка чтения TLE %1. Спутник %2").arg(tleFile).arg(satName);
    }
  }

//!
/*! Установка орбитальных параметров
  \param tle орбитальные параметры
*/
  void SatPipeline::setTLE(const MnSat::TLEParams &tle) {
    if (nullptr == _sat) {
      error_log << QObject::tr("Ошибка загрузки орбитальных параметров. Кадр сформирован без создания спутника");
      return;
    }

    _sat->setTLEParams(tle);
  }

//! Установка приборов, данные которых будут обрабатываться для текущего формата
/*!
  \param instrs список приборов (название, настройка обработки)
*/
  void SatPipeline::setInstruments(const google::protobuf::RepeatedPtrField <mappi::conf::PretrInstr> &instrs) {
    for (int idx = 0; idx < instrs.size(); idx++) {
      _instrPretr.push_back(instrs.Get(idx));
    }
  }

  QString SatPipeline::invertBytesInFile(QString filePath){
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return filePath;
    QByteArray data = file.readAll();
    file.close();

    QtConcurrent::blockingMap(data, [](char &c){ c = ~c; });

    QFileInfo fileInfo(filePath);
    QString newFilePath = QString("%1%2%3%4").arg(_params.output_dir, fileInfo.baseName(), "_inverted.", fileInfo.completeSuffix());
    QFile saveFile(newFilePath);
    saveFile.open(QIODevice::WriteOnly);
    saveFile.write(data);
    saveFile.close();

    return newFilePath;
  }

  void SatPipeline::setDataLevel(conf::DataLevel level) {
    _level = level;

    switch (_level) {
      case conf::kDeframerIvertLevel:
        _params.input_file = invertBytesInFile(_params.input_file);
      case conf::kDeframerLevel:
        _params.input_level = "soft";
        break;
      case conf::kFrameLevel:
        _params.input_level = _satDumpWrapper->pipelineHasCadu(_params.pipeline_name) ? "cadu" : "frames";
        break;
      case conf::kManchesterLevel:
      case conf::kManchesterInvertLevel:
      default:
        _params.input_level = "baseline";
    }
  }

  bool SatPipeline::run() {
    auto pipeline = _satDumpWrapper->setupPipeline(_params);
    if (!_satDumpWrapper->runPipeline(pipeline, _params)) return false;
    _instrProducts = _satDumpWrapper->getProducts(_params.output_dir);

    conf::InstrCollect _instrConf = singleton::SatFormat::instance()->getInstruments();
    debug_log << "Инструменты, полученные из конфига:";
    for (int idx = 0; idx < _instrConf.instrs_size(); idx++) {
      for (auto instr: _instrPretr) {
        if (instr.type() == _instrConf.instrs(idx).type()) {
          debug_log << QObject::tr("  %1 (%2)").arg(
              QString::fromStdString(_instrConf.instrs(idx).name()),
              QString::fromStdString(InstrumentType_Name(_instrConf.instrs(idx).type()))
          );
          break;
        }
      }
    }

    debug_log << "Инструменты, полученные из pipeline:";
    for (auto instrName: _instrProducts.keys()) debug_log << "  " + instrName;

    filterOutInstrs();
    if (_instrCheckedTypes.count() == 0) {
      error_log << "В ходе выполнения pipeline получено 0 инструментов";
      return false;
    }

    QDateTime dtStartAll;
    QDateTime dtEndAll;
    for (auto instrName: _instrCheckedTypes.keys()) {
      QDateTime dtStart = _instrProducts[instrName].time.first;
      QDateTime dtEnd = _instrProducts[instrName].time.second;
      if (!dtStartAll.isValid() || dtStart < dtStartAll) dtStartAll = dtStart;
      if (!dtEndAll.isValid() || dtEnd > dtEndAll) dtEndAll = dtEnd;
    }
    if (dtStartAll.isValid()) _sat->setDtStart(dtStartAll);
    if (dtEndAll.isValid()) _sat->setDtEnd(dtEndAll);
    return true;
  }

  void SatPipeline::filterOutInstrs() {
    for (QString instrName: _instrProducts.keys()) {
      auto instrTypes = singleton::SatFormat::instance()->getInstrumentTypesBy(instrName);
      if (instrTypes.size() <= 0) {
        debug_log << QObject::tr("Инструмент %1 не существует в общем конфиге инструментов").arg(instrName);
        continue;
      }

      for (auto instrType: instrTypes) {
        for (auto instr: _instrPretr) {
          if (instrType != conf::InstrumentType::kInstrUnk && instr.type() == instrType) {
            _instrCheckedTypes.insert(instrName, instrType);
            break;
          }
        }
      }
    }
  }

  bool SatPipeline::save(const meteo::global::StreamHeader &header) {
    QString dateTemplate = "yyyyMMddhhmm";
    QString startStr = _sat->dtStart().toString(dateTemplate);
    QString endStr = _sat->dtEnd().toString(dateTemplate);

    QDir dir;
    QString outputPath = QString("%1/%2_%3_%4").arg(_path, startStr, endStr, satName().remove(' '));
    info_log << QObject::tr("Сохранение данных для %1 по пути: %2").arg(_satName, outputPath);
    for (auto instrName: _instrCheckedTypes.keys()) {
      int32_t channel_num = 1;
      conf::InstrumentType instr_type = _instrCheckedTypes.value(instrName);
      if(!singleton::SatFormat::instance()->hasInstrument(instr_type)) continue;

      mappi::conf::Instrument curInstr = singleton::SatFormat::instance()->getInstrumentBy(instr_type);
      mappi::conf::PretrInstr pretrConfig;
      for (auto instr: _instrPretr) {
        if (instr.type() == curInstr.type()) {
          pretrConfig = instr;
          break;
        }
      }

      for (auto imageProduct: _instrProducts[instrName].images) {
        QString imgFullPath = imageProduct.file_path;

        QDateTime dtStart = _instrProducts[instrName].time.first;
        QDateTime dtEnd = _instrProducts[instrName].time.second;
        if (!dtStart.isValid() || !dtEnd.isValid()) {
          dtStart = _sat->dtStart();
          dtEnd = _sat->dtEnd();
        }

        if (!QFile::exists(imgFullPath)) {
          error_log << QObject::tr("Файл не существует: %1").arg(imgFullPath);
          continue;
        }
        debug_log << QObject::tr("Получено изображение %1 (channel %2)").arg(instrName).arg(channel_num);
        auto image_8bit = imageProduct.image.to8bits();
        QVector<uchar> im_data(image_8bit.data(), image_8bit.data() + image_8bit.size());
        meteo::global::PoHeader pohead = createPoHeader<uint16_t>(
            header,
            imageProduct.image,
            instr_type,
            channel_num,
            dtStart,
            dtEnd
        );
        Image im(im_data, pohead, _notify);

        QString filePath = QString("%1/%2_%3_%4_%5_%6").arg(
            outputPath,
            dtStart.toString(dateTemplate),
            dtEnd.toString(dateTemplate),
            _satName,
            QString::number(instr_type).rightJustified(2, '0'),
            QString::number(channel_num).rightJustified(2, '0')
        );
        mappi::conf::ImageTransform imtransform;
        if(passImageTransform(channel_num, pretrConfig, imtransform)){
          if(imtransform.geocoding()){
            debug_log << QObject::tr("Сохранение PNG и GeoTiff");
          }else{
            debug_log << QObject::tr("Сохранение PNG файла");
          }
          po::GeomCorrection geomCorr = createGeomCorrection(curInstr, pretrConfig, pohead, imtransform);
          im.save(filePath, imtransform, geomCorr);
        }else{
          QString outputPNG = filePath + ".png";
          dir.mkpath(outputPNG.left(outputPNG.lastIndexOf("/")));
          QFile::remove(outputPNG);
          QFile::copy(imgFullPath, outputPNG);
          warning_log << QObject::tr("Отсутствует ImageTransform. Копируем изображение из папки SatDump");
        }

        QString outputPO = filePath + ".po";
        meteo::global::PreHeader pre = createPreHeader(pohead);
        QVector<uint16_t> full_data(imageProduct.image.data(), imageProduct.image.data() + imageProduct.image.size());
        savePO(outputPO, pohead, pre, full_data);
        channel_num++;
      }

      QString productJSON = QString("%1/%2/product.json").arg(_params.output_dir, instrName);
      if (QFile::exists(productJSON)) {
        debug_log << QObject::tr("product.json для %1 скопирован").arg(instrName);
        QFile::copy(productJSON, QString("%1/%2.json").arg(outputPath, instrName));
      }
    }

    QString datasetJSON = QString("%1/dataset.json").arg(_params.output_dir);
    if (QFile::exists(datasetJSON)) {
      debug_log << QObject::tr("dataset.json скопирован");
      QFile::copy(datasetJSON, QString("%1/dataset.json").arg(outputPath));
    }

    return true;
  }

  bool SatPipeline::savePO(
      QString outputFilePath,
      meteo::global::PoHeader &pohead,
      meteo::global::PreHeader &pre,
      QVector<uint16_t> &data
  ) {
    QFile file(outputFilePath);
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    ds << pre;
    ds << pohead;
    ds << data;
    file.close();
    if (nullptr != _notify) _notify->dataNotify(pohead, outputFilePath);
    return true;
  }

  meteo::global::PreHeader SatPipeline::createPreHeader(meteo::global::PoHeader &pohead, QDate date){
    QByteArray poheadArray;
    pohead >> poheadArray;

    meteo::global::PreHeader pre;
    pre.type = meteo::global::kPretrFile;
    pre.year = date.year();
    pre.month = date.month();
    pre.day = date.day();
    pre.offset = poheadArray.size();
    return pre;
  }

  template<typename T>
  meteo::global::PoHeader SatPipeline::createPoHeader(
      const meteo::global::StreamHeader &header,
      const SatDump::Image <uint16_t> &image,
      const conf::InstrumentType &instr_type,
      int32_t channel_num,
      QDateTime dtStart,
      QDateTime dtEnd
  ) {
    global::PoHeader pohead;
    pohead.site = header.site;
    pohead.siteCoord = header.siteCoord;
    pohead.start = dtStart;
    pohead.stop = dtEnd;
    pohead.satellite = header.satellite;
    pohead.direction = header.direction;
    pohead.tle = header.tle;
    pohead.instr = instr_type;
    pohead.procflag = false;
    pohead.channel = channel_num;
    pohead.lines = image.height();
    pohead.samples = image.width();
    pohead.type_hash = typeid(T).hash_code();
    pohead.min = std::numeric_limits<T>::min();
    pohead.max = std::numeric_limits<T>::max();
    pohead.undef = -9999;
    return pohead;
  }

  bool SatPipeline::passImageTransform(
      int32_t channel_num,
      const mappi::conf::PretrInstr &pretrConfig,
      mappi::conf::ImageTransform &transform
  ) {
    QString channel_name = QString::number(channel_num).rightJustified(2, '0');

    for (mappi::conf::ImageTransform imtransform: pretrConfig.image()) {
      QRegExp rx(QString::fromStdString(imtransform.ch_rx()));
      if (rx.exactMatch(channel_name)){
        transform = imtransform;
        return true;
      }
    }
    return false;
  }

  po::GeomCorrection SatPipeline::createGeomCorrection(
      const mappi::conf::Instrument &instr,
      const mappi::conf::PretrInstr &pretrConfig,
      meteo::global::PoHeader pohead,
      mappi::conf::ImageTransform imtransform
  ) {
    po::GeomCorrection geomCorr(*_sat);
    float gridStep = MnMath::deg2rad(pretrConfig.geostep());
    if (pretrConfig.geostep() < 0.005) {
      gridStep = MnMath::deg2rad(0.005);
      warning_log << QObject::tr("Маленький шаг сетки %1 заменен на %2 градусов").arg(pretrConfig.geostep()).arg(0.005);
    }

    if (imtransform.geocoding()) {
      geomCorr.createCoords(
          pohead.lines,
          gridStep,
          MnMath::deg2rad(instr.scan_angle()),
          MnMath::deg2rad(instr.fovstep())
      );
    }
    return geomCorr;
  }

  QString SatPipeline::instruments(QString delimeter, QString prefix) {
    QString s;
    bool isFirst = true;
    for (QString instrName: _instrCheckedTypes.keys()) {
      if (!isFirst) s += delimeter;
      else isFirst = false;

      s += QString("%1%2 (%3)").arg(
          prefix,
          instrName,
          QString::fromStdString(InstrumentType_Name(_instrCheckedTypes.value(instrName)))
      );
    }
    return s;
  }
}