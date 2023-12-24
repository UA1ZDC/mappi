#include <mappi/pretreatment/handler/handler.h>
#include <mappi/global/streamheader.h>

#include <mappi/pretreatment/formats/satpipeline.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <sat-commons/satellite/satellite.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <fstream>

#include <QFile>

using namespace mappi;
using namespace po;
using namespace TSatPretr;

Handler::~Handler(){
  if(!_tempFileName.isNull()){
    QFile::remove(_tempFileName);
    debug_log << QObject::tr("Временный файл удален: %1").arg(_tempFileName);
  }

  _pipeline.clear();
  debug_log << QObject::tr("Временная папка удалена: %1").arg(_pipeline.params().output_dir);

  if(_deleteInput && singleton::SatFormat::instance()->getFlagDeleteInputFile()) {
    QFile::remove(_fileName);
    if (QFileInfo::exists(_fileName + ".header")) QFile::remove(_fileName + ".header");
    debug_log << QObject::tr("Удален исходный файл: %1").arg(_fileName);
  }
}


void Handler::clear(){
  _pipeline = SatPipeline();
  _mode = conf::kUnkRate;
  _header = meteo::global::StreamHeader();
  _headerType = kNoHeader;
  _level = mappi::conf::kUnkLevel;
  _tempFileName.clear();
  _fileName.clear();
  _path.clear();
  _suffix.clear();
}

//! запуск обработки данных
bool Handler::process(SaveNotify* notify)
{
  if (_fileName.isEmpty()) {
    error_log << QObject::tr("Не задан файл с потоком");
    return false;
  }

  QString headerType = "kNoHeader";
  switch(_headerType){
    case kPreOldHeader:
      headerType = "kPreOldHeader"; break;
    case kPostOldHeader:
      headerType = "kPostOldHeader"; break;
    case kMappiHeader:
      headerType = "kMappiHeader"; break;
    case kSeparateHeader:
      headerType = "kSeparateHeader"; break;
    default: //kNoHeader
      headerType = "kNoHeader"; break;
  }
  debug_log << QObject::tr("Обработка файла: %1").arg(_fileName);
  debug_log << QObject::tr("Тип хедера: %1").arg(headerType);

  if(!readHeader()) {
    error_log << QObject::tr("Невозможно прочитать header");
    return false;
  }

  if (_header.satellite.isEmpty()) {
    error_log << QObject::tr("Не определено название спутника");
    return false;
  }

  debug_log << QObject::tr("Header:");
  debug_log << QObject::tr("  site: %1").arg(_header.site);
  debug_log << QObject::tr("  start: %1").arg(_header.start.toString("dd.MM.yyyy hh:mm"));
  debug_log << QObject::tr("  stop: %1").arg(_header.stop.toString("dd.MM.yyyy hh:mm"));
  debug_log << QObject::tr("  direction: %1").arg(_header.direction);
  debug_log << QObject::tr("  rate_mode: %1").arg(QString::fromStdString(RateMode_Name(_header.mode)));
  debug_log << QObject::tr("  level: %1").arg(QString::fromStdString(DataLevel_Name(_level)));
  debug_log << QObject::tr("  swap: %1").arg(QString::fromStdString(ByteSwap_Name(_swap)));

  if (!setupPipeline(notify)) {
    error_log << QObject::tr("Невозможно настроить пайплайн");
    return false;
  }

  debug_log << QObject::tr("Параметры Pipeline:");
  debug_log << QObject::tr("  input_file: %1").arg(_pipeline.params().input_file);
  debug_log << QObject::tr("  input_level: %1").arg(_pipeline.params().input_level);
  debug_log << QObject::tr("  pipeline_name: %1").arg(_pipeline.params().pipeline_name);
  debug_log << QObject::tr("  path: %1").arg(_pipeline.path());
  debug_log << QObject::tr("  output_dir: %1").arg(_pipeline.params().output_dir);
  debug_log << QObject::tr("  additional_params: %1").arg(_pipeline.params().additional_params);

  if (!_pipeline.run()){
    error_log << QObject::tr("Ошибка запуска пайплайна");
    return false;
  }

  if (!_pipeline.dtStart().isValid() || !_pipeline.dtEnd().isValid()) {
    warning_log << QObject::tr("Неверное время получения информации, используем данные из header");
    _pipeline.setDateTime(_header.start, _header.stop);
  }

  if(_useOnlyTimePipeline){
    _header.start.setTime(_pipeline.dtStart().time());
    _header.stop.setTime(_pipeline.dtEnd().time());
    _pipeline.setDateTime(_header.start, _header.stop);
  }else{
    _header.start = _pipeline.dtStart();
    _header.stop = _pipeline.dtEnd();
  }
  debug_log << QObject::tr("Pipeline:");
  debug_log << QObject::tr("  satName: %1").arg(_pipeline.satName());
  debug_log << QObject::tr("  dtStart: %1").arg(_pipeline.dtStart().toString("dd.MM.yyyy hh:mm"));
  debug_log << QObject::tr("  dtEnd: %1").arg(_pipeline.dtEnd().toString("dd.MM.yyyy hh:mm"));
  debug_log << QObject::tr("  instruments:");
  debug_log << _pipeline.instruments(",", " ");

  if (nullptr != notify) notify->rawNotify(_header, _fileName);
  else error_log << QObject::tr("Неверный указатель на saveNotify");

  if (!_pipeline.save(_header)) {
    error_log << QObject::tr("Ошибка сохранения данных");
    return false;
  }
  debug_log << QObject::tr("Сохранено в %1").arg(_pipeline.path());

  _pipeline.saveComposites(_header);

  if(singleton::SatFormat::instance()->getFlagCopyCaduFile()) {
    QDir::root().mkpath(_path + "/" + "copied_files");
    QString extensions[3] = {".cadu", ".frm", ".bin"};
    for(auto ext : extensions){
      QString caduFileName = QString("%1/%2%3").arg(
          _pipeline.params().output_dir,
          _pipeline.params().pipeline_name,
          ext
      );
      debug_log << QObject::tr("Попытка копирования cadu файла: %1").arg(caduFileName);
      if(!QFileInfo::exists(caduFileName)) continue;

      QFileInfo fileInfo(caduFileName);
      QString copiedFileName = QString("%1/%2/%3").arg(
          _path,
          "copied_files",
          _pipeline.dtStart().toString("ddMMyyyy_hhmm") + "_" + fileInfo.completeBaseName()
      );
      QFile::copy(_fileName, copiedFileName);
      debug_log << QObject::tr("Скопирован cadu файл: %1 -> %2").arg(fileInfo.completeBaseName()).arg(copiedFileName);
    }
  }

  if(singleton::SatFormat::instance()->getFlagCopyInputFile()) {
    QDir::root().mkpath(_path + "/" + "copied_files");
    QFileInfo fileInfo(_fileName);
    QString copiedFileName = QString("%1/%2/%3").arg(
        _path,
        "copied_files",
        fileInfo.completeBaseName()
    );
    QFile::copy(_fileName, copiedFileName);
    if (QFileInfo::exists(_fileName + ".header")) QFile::copy(_fileName + ".header", copiedFileName + ".header");
    debug_log << QObject::tr("Скопирован исходный файл: %1 -> %2").arg(fileInfo.completeBaseName()).arg(copiedFileName);
  }

  if (nullptr != notify) notify->finish();
  else error_log << QObject::tr("Неверный указатель на saveNotify");
  return true;
}

bool Handler::setupPipeline(SaveNotify* notify)
{
  if (_mode == conf::kUnkRate) {
    _mode = conf::kHiRate;
  }

  auto pretrConf = singleton::SatFormat::instance()->getPretreatmentFor(_header.satellite, _mode);
  if (nullptr == pretrConf) {
    error_log << QObject::tr("Ошибка загрузки настроек предварительной обработки для спутника") << _header.satellite;
    return false;
  }

  auto receptionConf = singleton::SatFormat::instance()->getReceptionFor(_header.satellite);
  if (receptionConf.receiver().empty()) {
    error_log << QObject::tr("Ошибка загрузки настроек приема для спутника") << _header.satellite;
    return false;
  }
  
  _pipeline.setSatName(_header.satellite);
  _pipeline.setTLE(_header.tle);
  _pipeline.setRecvMode(_mode);
  _pipeline.setDataLevel(_level == mappi::conf::kUnkLevel ? pretrConf->level() : _level);
  _pipeline.setPath(_path + _suffix);
  _pipeline.setInstruments(pretrConf->instr());
  _pipeline.setSaveNotify(notify);
  _pipeline.setParams(
      QString("{\"samplerate\":%1,\"baseband_format\":\"%2\"}")
          .arg(receptionConf.receiver().at(0).rate())
          .arg(receptionConf.receiver().at(0).format().c_str())
  );
  
  return true;
}

//! чтение заголовка
bool Handler::readHeader()
{
  QFile file(_fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(_fileName);
    return false;
  }
  uint64_t dataSize = file.size();
  QDataStream in(&file);

  switch (_headerType) {
    case kPreOldHeader:  //заголовок в начале старого формата
    {
      meteo::global::StreamHeaderOld sHeadOld;
      in >> sHeadOld;
      sHeadOld.fill(&_header);
      dataSize -= meteo::global::StreamHeaderOld::size();

      QFileInfo fileInfo(file.fileName());
      _tempFileName = QString("%1/%2%3%4").arg(
          _path,
          fileInfo.baseName(),
          "_noheader.",
          fileInfo.completeSuffix()
      );
      QDir dir;
      dir.mkpath(_tempFileName.left(_tempFileName.lastIndexOf("/")));

      std::ifstream src(file.fileName().toStdString(), std::ios::binary);
      std::ofstream dst(_tempFileName.toStdString(),    std::ios::binary);
      src.seekg(meteo::global::StreamHeaderOld::size());
      dst << src.rdbuf();
      _pipeline.setFileName(_tempFileName);
    }
    break;
    case kPostOldHeader:  //заголовок в конце старого формата
    {
      dataSize -= meteo::global::StreamHeaderOld::size();
      in.skipRawData(dataSize);
      meteo::global::StreamHeaderOld sHeadOld;
      in >> sHeadOld;
      sHeadOld.fill(&_header);
      file.seek(0);

      QFileInfo fileInfo(file.fileName());
      _tempFileName = QString("%1/%2%3%4").arg(
          _path,
          fileInfo.baseName(),
          "_noheader.",
          fileInfo.completeSuffix()
      );
      QDir dir;
      dir.mkpath(_tempFileName.left(_tempFileName.lastIndexOf("/")));

      std::ifstream src(file.fileName().toStdString(), std::ios::binary);
      std::ofstream dst(_tempFileName.toStdString(),    std::ios::binary);
      char buf[1024];
      unsigned int readCount = 0;
      while(src.read(&buf[0], 1024) || (readCount = src.gcount()) != 0)  dst.write(&buf[0], readCount);
      _pipeline.setFileName(_tempFileName);
    }
    break;
    case kMappiHeader:  //заголовок в начале нового формата
    {
      meteo::global::PreHeader preHeader;
      in >> preHeader;
      if (preHeader.type != meteo::global::kRawFile) {
        error_log << QObject::tr("Тип файла не соответствует сырому потоку");
        return false;
      }

      QByteArray ar(preHeader.offset, 0);
      in.readRawData(ar.data(), ar.size());
      meteo::global::fromBuffer(ar, _header);
      dataSize = file.size() - preHeader.size() - preHeader.offset;

      QFileInfo fileInfo(file.fileName());
      _tempFileName = QString("%1/%2%3%4").arg(
          _path,
          fileInfo.baseName(),
          "_noheader.",
          fileInfo.completeSuffix()
      );
      QDir dir;
      dir.mkpath(_tempFileName.left(_tempFileName.lastIndexOf("/")));

      std::ifstream src(file.fileName().toStdString(), std::ios::binary);
      std::ofstream dst(_tempFileName.toStdString(), std::ios::binary);
      src.seekg(preHeader.size() + preHeader.offset);
      dst << src.rdbuf();
      _pipeline.setFileName(_tempFileName);
    }
    case kSeparateHeader:  //заголовок отдельно
    {
      QFile headerFile(_fileName + ".header");
      if (!headerFile.open(QIODevice::ReadOnly)) {
        error_log << QObject::tr("Ошибка открытия файла c заголовком %1").arg(_fileName);
        return false;
      }
      meteo::global::PreHeader preHeader;
      QDataStream headerStream(&headerFile);
      headerStream >> preHeader;
      if (preHeader.type != meteo::global::kRawFile) {
        error_log << QObject::tr("Тип файла не соответствует сырому потоку");
        return false;
      }
      QByteArray ar(preHeader.offset, 0);
      headerStream.readRawData(ar.data(), ar.size());
      meteo::global::fromBuffer(ar, _header);
      _pipeline.setFileName(_fileName);
    }
    break;
    default: //kNoHeader
    break;
  }
  file.close();
  if (dataSize <= 0) {
    error_log << QObject::tr("В файле нет данных");
    return false;
  }

  if(_swap == mappi::conf::kSwap) {
    debug_log << QObject::tr("Переставляем четные и нечетные байты местами");
    QString fileToSwapPath = _tempFileName.isNull() ? file.fileName() : _tempFileName;
    QFile fileToSwap(fileToSwapPath);
    fileToSwap.open(QIODevice::ReadOnly);
    QByteArray data = fileToSwap.readAll();
    fileToSwap.close();

    int even_data_size = data.size() - data.size() % 2;
    for(int i=0; i<even_data_size; i+=2){
      char temp = data[i];
      data[i] = data[i+1];
      data[i+1] = temp;
    }

    QFileInfo fileInfo(fileToSwapPath);
    _tempFileName = QString("%1/%2%3%4").arg(
        _path,
        fileInfo.baseName(),
        "_swapped.",
        fileInfo.completeSuffix()
    );
    QDir dir;
    dir.mkpath(_tempFileName.left(_tempFileName.lastIndexOf("/")));

    QFile saveFile(_tempFileName);
    saveFile.open(QIODevice::WriteOnly);
    saveFile.write(data);
    saveFile.close();
    _pipeline.setFileName(_tempFileName);
  }

  return true;
}

/*! 
 * \brief Создание даты приёма из опций
 * \param opt  опции
 * \param date дата приёма
 */
void Handler::dateFromOpt(TSatPretr::PretrOpt &opt, QDate &date) {
  bool passedDate = (opt.year != 0 || opt.month != 0 || opt.day != 0);
  if (opt.year == 0) { opt.year = date.year(); }
  if (opt.month == 0) { opt.month = date.month(); }
  if (opt.day == 0) { opt.day = date.day(); }

  if (QDate::isValid(opt.year, opt.month, opt.day)){
    date.setDate(opt.year, opt.month, opt.day);
    if(passedDate) _useOnlyTimePipeline = true;
  }
}

void Handler::parseStream(const QString &fileName, const QString &weatherFile, const QString &path, PretrOpt &opt) {
  Satellite sat;
  sat.readTLE(opt.satName, weatherFile);
  setTle(sat.getTLEParams());

  setPath(path);
  setSatName(opt.satName);
  setFile(fileName);
  setPipelineFile(fileName);
  setPipelineName(opt.pipelineName);
  setPipelineParams(opt.json_params);

  //  QString stream;
  if (opt.mode != mappi::conf::kUnkRate) {
    setRecvMode(opt.mode);
  }

  setSwap(opt.swap);

  if (opt.manchester) {
    setDataLevel(opt.invert ? mappi::conf::kManchesterInvertLevel : mappi::conf::kManchesterLevel);
  } else if (opt.deframer) {
    setDataLevel(opt.invert ? mappi::conf::kDeframerIvertLevel : mappi::conf::kDeframerLevel);
  } else {
    setDataLevel(mappi::conf::kFrameLevel);
  }

  if (opt.headerExist) {
    if(QFileInfo::exists(fileName + ".header")) {
      setHeaderType(mappi::po::Handler::kSeparateHeader);
    }else {
      setHeaderType(mappi::po::Handler::kMappiHeader);
    }
  } else if (opt.oldHeaderExist) {
    if (opt.deframer) { //с такими параметрами виндовый cif формат
      setHeaderType(mappi::po::Handler::kPreOldHeader);
    } else {
      setHeaderType(mappi::po::Handler::kPostOldHeader);
    }
  } else {
    QDateTime dt = QDateTime::currentDateTimeUtc();
    QDate date = dt.date();
    dateFromOpt(opt, date);
    dt.setDate(date);
    setDateTime(dt);
    setSite(opt.siteName);
  }
}
