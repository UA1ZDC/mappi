#include "themalg.h"
#include "channel.h"
#include "datastore.h"
#include <commons/textproto/tprototext.h>

#include <commons/mathtools/mnmath.h>


#include <qsharedpointer.h>
#include <QFile>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/services/sessiondataservice/sessiondataservice.h>

#include <mappi/pretreatment/images/colors.h>
#include <mappi/pretreatment/images/filters.h>
#include <mappi/pretreatment/images/geomcorr.h>
#include <mappi/pretreatment/images/image.h>


using namespace mappi;
using namespace to;

ThemAlg::ThemAlg(conf::ThemType type, const std::string &them_name, QSharedPointer<to::DataStore>& ds): _ds(ds)
{
  _themType = type;
  _them_name = them_name;
}


ThemAlg::~ThemAlg()
{
  debug_log << QObject::tr("Очистка памяти %1").arg(QString::fromStdString(name()));
  clear();
  delete projection_;
  delete landmask_;
}

void ThemAlg::clear() {
  clearChannels();
  data_.clear();
  bitmap_.clear();
}

void ThemAlg::clearChannels() {
  QMapIterator<std::string, QSharedPointer<Channel>> iter(_ch);
  while(iter.hasNext()){
    iter.next();
    Channel().swap(*iter.value());
    _ch[iter.key()].clear();
  }
  _ch.clear();
}

bool ThemAlg::readConfig()
{
  conf::ThematicProcs conf;
  conf.CopyFrom(mappi::inter::Settings::instance()->thematicsCalc());
  if (!conf.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла настроек тематических обработок");
    return false;
  }
  instruments_.CopyFrom(mappi::inter::Settings::instance()->instruments());
  if (!instruments_.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла настроек инструментов");
    return false;
  }

  them_coef_.CopyFrom(mappi::inter::Settings::instance()->coef());
  if (!them_coef_.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла коэффициентов для методов тематической обработки");
    return false;
  }

  them_thresholds_.CopyFrom(mappi::inter::Settings::instance()->thres());
  if (!them_thresholds_.IsInitialized()) {
    error_log << QObject::tr("Ошибка файла настроек пороговых значений для методов тематической обработки");
    return false;
  }

  gradient_ = mappi::inter::Settings::instance()->palette(name());


  for(const auto &them : conf.thematics()) {
    if (name() == them.name()) {
      config_.CopyFrom(them);
      return true;
    }
  }

  return false;
}


bool ThemAlg::init(const QDateTime& start, const QString& satname)
{
  if (!readConfig())  return false;

  start_ = start;
  satname_ = satname;
  getVars();
  return true;
}

void ThemAlg::getVars()
{
  for(const ::mappi::conf::ThematicVariable &var: config_.vars())
  {
    std::string var_name = var.name();
    QMultiMap<mappi::conf::InstrumentType,std::string> map;
    for( const ::mappi::conf::ThematicChannel &channel :var.channel())
    {
      if(satname_ == QString::fromStdString(channel.satellite()))
      {
        map.insert(channel.instrument(),channel.channel());
      }
    }
    variables_.insert(var_name, map);
  }
}


bool ThemAlg::loadData() {
  if (_ds.isNull()) {
    return false;
  }
  QMap <std::string, QString> chfiles; //название канала, название файла
  QSet<QString> loaded_files;
  // QMap<std::string, QPair< std::string, mappi::conf::InstrumentType> > variables_; //имя переменной , <  прибор, название канала >
  QList <std::string> variables = variables_.keys();
  for (const auto &varname: qAsConst(variables)) {
    if (_ch.contains(varname)){
      debug_log << QObject::tr("Канал %1 уже существует в массиве").arg(QString::fromStdString(varname));
      continue;
    }

    QMultiMap<mappi::conf::InstrumentType, std::string> map = variables_.value(varname);
    for (const auto &instr: map.keys()) {
      if (!_ds->getFiles(start_, satname_, instr, map.values(instr), &chfiles) || chfiles.keys().length() == 0){
        debug_log << QObject::tr("Нет файлов для %1(%2)").arg(satname_).arg(QString::fromStdString(InstrumentType_Name(instr)));
        continue;
      }

      for (const auto &chname: chfiles.keys()) {
        if(loaded_files.contains(chfiles.value(chname))){
          debug_log << QObject::tr("Файл для %1 уже открыт").arg(QString::fromStdString(chname));
          if(!_ch.contains(varname)){
            for(const auto &sch: _ch.values()){
              if(sch->name() == chname){
                _ch.insert(varname, sch);
                break;
              }
            }
          }
          continue;
        }
        //! Чтение данных каналов из файлов
        debug_log << QObject::tr("Чтение %1(%2) из файла: %3")
            .arg(QString::fromStdString(chname))
            .arg(QString::fromStdString(varname))
            .arg(chfiles.value(chname));
        QSharedPointer <Channel> sch = QSharedPointer<Channel>(new Channel());
        sch->setInstrument(instr);
        sch->setName(chname);
        if (sch->readData(chfiles.value(chname))) {
          debug_log << QObject::tr("Прочитан файл %1 размером: %2x%3. Мин-Макс: %4,%5")
              .arg(chfiles.value(chname))
              .arg(sch->columns())
              .arg(sch->rows())
              .arg(sch->min())
              .arg(sch->max());
          _ch.insert(varname, sch);
          loaded_files.insert(chfiles.value(chname));
          if (channel_size() < sch->size()) {
            channel_size_ = sch->size();
            cols_ = sch->columns();
            rows_ = sch->rows();
          }
        } else {
          warning_log << "Ошибка чтения файла";
        }
      }
    }
  }

  // Устанавливаем значения масштабирования для каждого канала
  for (auto channel: _ch.values()) channel->scale(rows(), columns());
  debug_log << QObject::tr("Размер выходных изображений: %1x%2(%3)").arg(columns()).arg(rows()).arg(channel_size());
  return columns() != 0 && rows() != 0;
}

void ThemAlg::fillPalette()
{
  //int alpha = 255;
}

void ThemAlg::normalizeImg(int min, int max, int nMin, int nMax)
{
  int cnt = bitmap_.size();
  float k =  float(nMax - nMin)/float(max - min);
  for (int i=0; i< cnt; i++) {
    if (bitmap_[i] > max) bitmap_[i] = nMax;
    else if (bitmap_[i] < min) bitmap_[i] = nMin;
    else bitmap_[i] = uint8_t(k * float(bitmap_[i] - min) + float(nMin));
  }
}

void ThemAlg::normalize(int min, int max)
{
  for(auto it : qAsConst(_ch)) {
    if (it->number() >= 4 && it->header().satellite == "METEOR-M 2") {  //TODO пока так для макета, и-за ИК.  Должно быть в настройках, зависит от калибровки
      MnMath::normalize(it->data(), it->size(), it->min(), it->max(), max, min);
    } else {
      MnMath::normalize(it->data(), it->size(), it->min(), it->max(), min, max);
    }
  }
}

//! путь для сохранения и шаблон имени (его начало)
void ThemAlg::getSavePath(QString* path, QString* templName)
{
  if (nullptr == path || nullptr == templName) {
    return;
  }
  
  *path = MnCommon::varPath() + "/thematics/" + start_.toString("yyyy-MM-dd/");
  QDir dir;
  dir.mkpath(*path);
  
  *templName = start_.toString("yyyyMMddmmhh_") + satname_;
  templName->remove(' ');
  *templName = path + *templName + "_" + QString::fromStdString( name());

}


bool ThemAlg::saveData()
{
  if(_ch.size() == 0) return false;
  QString path;
  QString aname;
  getSavePath(&path, &aname);
  saveData(aname);
  return true;
}


bool ThemAlg::saveData(const QString& baseName)
{
  if(( data_.isEmpty() && bitmap_.isEmpty() ) || channels().isEmpty()) {
    error_log << QObject::tr("Ошибка сохранения данных: %1").arg(baseName);
    return false;
  }
  auto src = channels().first();
  if (nullptr == src) {
    return false;
  }

  std::string format = "png";
  QString typeext = ".png";
  QString filename = baseName + typeext;

  if(!data_.isEmpty())
  {
    typeext = ".to";
    format = "bin";
    filename = baseName + typeext;
    debug_log << QObject::tr("Cохраняем массив _data в %1").arg(filename);

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
      error_log << QObject::tr("Невозможно открыть файл: %1").arg(filename);
      return false;
    }
    meteo::global::PoHeader poh =  src->header();
    poh.short_name = QString::fromStdString( config_.name() );
    poh.name = QString::fromStdString( config_.description() );
    poh.unit_name = QString::fromStdString( config_.unit() );

    QDataStream stream(&file);
    stream << poh;
    for(int i=0; i<data_.size(); ++i){
      stream << data_.at(i);
    }
    file.close();
  }

  store()->save(src->header(), type() ,name(), filename, format);

  return true;
}

bool ThemAlg::saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format)
{
  if(data_.isEmpty() ) {
    error_log << QObject::tr("Ошибка сохранения изображения(нет данных): %1").arg(name);
    return false;
  }
  const ::mappi::conf::ImageTransform& transform = config_.image();

  if(channel.isNull()) {
    error_log << QObject::tr("Ошибка сохранения изображения(канал пуст): %1").arg(name);
    return false;
  }
  if(!transform.IsInitialized()){
    error_log << QObject::tr("Ошибка сохранения изображения(нет трансформации): %1").arg(name);
    return false;
  }

  const meteo::global::PoHeader& header = channel->header();
  if(transform.geocoding())
  {
    if(nullptr == projection_ && !initProjection(channel)) {
      error_log << QObject::tr("Проекция не создана");
      return false;
    }
  }else{
    sat_view_.setDateTime(header.start, header.stop);
    sat_view_.setTLEParams(header.tle);
  }

  po::GeomCorrection geom(sat_view_);
  int instrindx = instrConfIndex(header.instr);
  if(0 <= instrindx )
  {
    const ::mappi::conf::Instrument &instr = instruments_.instrs(instrindx);
    float gridStep = MnMath::deg2rad(transform.geostep());
    geom.createCoords(channel->rows(), gridStep, MnMath::deg2rad(instr.scan_angle()), MnMath::deg2rad(instr.fovstep()));
  }

  if(QImage::Format_RGB32 == format || QImage::Format_ARGB32 == format || QImage::Format_RGBA8888 == format){
    size_t size = data_.size();
    bitmap_.resize(size * 4);
    for(size_t i = 0; i < size; i++)
    {
      const QColor& color = gradient_.color(data_[i]).rgba();
      bitmap_[i * 4]     = color.red();// pixels_[Color::kRed  ].at(i);
      bitmap_[i * 4 + 1] = color.green();//pixels_[Color::kGreen].at(i);
      bitmap_[i * 4 + 2] = color.blue();//pixels_[Color::kBlue ].at(i);
      bitmap_[i * 4 + 3] = color.alpha();//alpha;
    }
    format = QImage::Format_RGBA8888;
  }else{
    debug_log << QObject::tr("Применяем фильтры к изображению: %1.png").arg(name);
    for(const auto &filter: qAsConst(transform.filter()))
    {
      switch(filter){
        case conf::kInvert:
          colors::invert(bitmap_.data(), bitmap_.size());
          break;
        case conf::kStretchHist:
          colors::stretchHist(bitmap_.data(), bitmap_.size());
          break;
        case conf::kEqualization:
          colors::equalization(bitmap_.data(), bitmap_.size(), columns(), rows());
          break;
        case conf::kFillEmpty:
          //TODO
          break;
        case conf::kMedian3:
          meteo::median3(rows(), columns(), &bitmap_);
          break;
        case conf::kMedian5:
          meteo::median5(rows(), columns(), &bitmap_);
          break;
        case conf::kNagaoMacuyamaModify:
          meteo::nagaoMacuyamaModify(rows(), columns(), &bitmap_);
          break;
        default: {}
      }
    }
  }

  mappi::Image *img = new Image(bitmap_,header,nullptr );
  bool ok = img->save(name, transform, geom, format);
  delete img;

  if(ok) debug_log << QObject::tr("Сохранено изображение: %1.png").arg(name);
  else error_log << QObject::tr("Ошибка сохранения изображения: %1.png").arg(name);
  return ok;
}

bool ThemAlg::saveImg()
{
  if(_ch.size() == 0 || _ch.first().isNull()) return false;
  QString path;
  QString aname;
  getSavePath(&path, &aname);
  QImage::Format format = QImage::Format_RGB32;
  switch (config_.image().format()) {
    case ::mappi::conf::kGrayscale8:
      format = QImage::Format_Grayscale8;
    break;
    case ::mappi::conf::kIndexed8:
      format = QImage::Format_Indexed8;
    break;
    case ::mappi::conf::kRGB32:
      format = QImage::Format_RGB32;
    break;
    case ::mappi::conf::kARGB32:
      format = QImage::Format_ARGB32;
    break;
  }
  saveImage(aname, _ch.first(), format);
  return true;
}

bool ThemAlg::isValid(const conf::ThematicProc &thematic)
{
  bool ok = thematic.has_name();
  ok &= thematic.has_red() && thematic.has_enabled();
  for(const auto &themVar : thematic.vars()) {
    if(!ok) break;
    ok &= themVar.has_name();
    for(const auto &ch : themVar.channel()) {
      if(!ok) break;
      ok &= ch.has_satellite() && ch.has_instrument() && ch.has_channel();
    }
  }
  return ok;
}

bool ThemAlg::initLandMask()
{
  if(nullptr == landmask_){
    landmask_ = new LandMask;
  }
  return landmask_->initLandSea();
}


bool ThemAlg::initProjection(const QSharedPointer<Channel>& channel)
{
  const meteo::global::PoHeader &header = channel->header();
  int instrindx = instrConfIndex(header.instr);
  if(instrindx < 0){
    error_log << QObject::tr("Проекция не создана, инструмент не найден: %1").arg(header.instr);
    return false;
  }
  const ::mappi::conf::Instrument &instr = instruments_.instrs(instrindx);

  if (nullptr != projection_){
    delete projection_;
    projection_ = nullptr;
  }

  projection_ = new meteo::POSproj;
  sat_view_.setDateTime(header.start, header.stop);
  sat_view_.setTLEParams(header.tle);
  projection_->setSatellite(header.start, header.stop, header.tle,
                            instr.scan_angle()*meteo::DEG2RAD,
                            instr.samples(), instr.velocity());

  meteo::GeoVector corners(4);
  if(!projection_->countGridCorners(corners.data())) {
    error_log << error_log << QObject::tr("Невозможно определить координаты углов, будут ошибки");
    return false;
  }
  debug_log << QObject::tr("Точки углов в начале сканирования %1 -> ")
      .arg(header.start.toString("dd.MM.yy hh:mm:ss.zzz")) << corners.at(0) << corners.at(1);
  debug_log << QObject::tr("Точки углов в конце сканирования %1 -> ")
      .arg(header.stop.toString("dd.MM.yy hh:mm:ss.zzz")) << corners.at(2) << corners.at(3);
  return true;
}

int ThemAlg::instrConfIndex(mappi::conf::InstrumentType instrtype)
{
  int kol_instr = instruments_.instrs_size();

  for(int i = 0; i < kol_instr; ++i) {
    const ::mappi::conf::Instrument &instr = instruments_.instrs(i);
    if( instrtype == instr.type()){
      return i;
    }
  }
  return -1;
}


ThemAlg::pixelType ThemAlg::cloudTestWithLandMask(float a1, float a2, float t3,float t4, int landmask)
{
  float ndv = (a2-a1)/(a2+a1);
  const float thresholdSurface2 = them_thresholds_.cloudsurface2();
  const float thresholdTempr = them_thresholds_.cloudtempr();

  pixelType pixtype = OPEN_LAND;
  pixtype = openWaterTest(a1, a2, landmask);
  if(UNKNOW == pixtype){
    pixtype = openSurfaceTest(a1, ndv, landmask);
  }
  if(UNKNOW != pixtype){
    //проверяем на лед. уже точно не облака
    pixtype = snowIceTest(t4, ndv, pixtype);
    return pixtype; //
  }
  if(a1 >thresholdSurface2 || (t3-t4) > thresholdTempr){
    //уже точно облака
    pixtype = CLOUD;
  } else {
    pixtype = OPEN_LAND;
  }


  return pixtype;
}

ThemAlg::pixelType ThemAlg::cloudTestWithLandMask2(float a1, float a2, float t3,float t4, int landmask)
{
  const float thresholdSurface2 = them_thresholds_.cloudsurface2();
  const float thresholdTempr = them_thresholds_.cloudtempr();
  pixelType pixtype =OPEN_LAND;
  pixtype = openWaterTest(a1, a2,landmask);
  if(UNKNOW != pixtype){
    return pixtype;
  }
  if(a1 >thresholdSurface2 || (t3-t4) > thresholdTempr){
    //уже точно облака
    pixtype = CLOUD;
  } else {
    pixtype = OPEN_LAND;
  }


  return pixtype;
}

ThemAlg::pixelType ThemAlg::snowIceTest(float t4, float ndv, pixelType pixtype)
{
  const float thresholdIce1 = them_thresholds_.cloudice1();
  const float thresholdIce2 = them_thresholds_.cloudice2();
  if((t4 < thresholdIce1) && (ndv < thresholdIce2)){
    if(OPEN_WATER == pixtype ||  OPEN_HYDRO == pixtype ) { //вода, значит лед
      pixtype = ICE;
    }
    if(OPEN_LAND == pixtype ) { //суша, значит снег
      pixtype = SNOW;
    }
  }
  return pixtype;
}
//Идентификация открытой поверхности суши:
ThemAlg::pixelType ThemAlg::openSurfaceTest(float a1, float ndv, int landmask)
{
  pixelType pixtype = UNKNOW;
  const float thresholdSurface1 = them_thresholds_.cloudsurface1();
  const float thresholdSurface2 = them_thresholds_.cloudsurface2();
  if(LandMask::LANDMASK == landmask || -1 == landmask) {
    if((ndv) > thresholdSurface1 && a1 < thresholdSurface2){
      pixtype = OPEN_LAND;
    }
  }
  return pixtype;
}


//Идентификация открытой водной поверхности:
ThemAlg::pixelType ThemAlg::openWaterTest(float a1, float a2, int landmask)
{
  pixelType pixtype = UNKNOW;
  const float thresholdWater1 = them_thresholds_.cloudwater1();
  const float thresholdWater2 = them_thresholds_.cloudwater2();
  if( (a2 < thresholdWater1) &&
      ( (a1-a2) > thresholdWater2)){
    if(LandMask::SEAMASK == landmask || -1 == landmask) {
      pixtype = OPEN_WATER;
    }else {
      pixtype = OPEN_HYDRO;
    }
  }

  return pixtype;
}

LandMask *landmask_ = nullptr;


