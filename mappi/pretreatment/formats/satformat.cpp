#include "satformat.h"

namespace mappi {
  namespace po {
    namespace singleton {
      template<> mappi::po::SatFormat *SatFormat::_instance = 0;
    }
  }
}

using namespace mappi;
using namespace po;

SatFormat::SatFormat() {
  bool ok = true;
  ok &= readReceptionConf();
  ok &= readInstrsConf();
  ok &= readSatsConf();

  _isValid = ok;
}

SatFormat::~SatFormat() {}

bool SatFormat::readReceptionConf() {
  QFile file(RECEPTION_FILE);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(RECEPTION_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (!TProtoText::fillProto(text, &_receptionConf)) {
    error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(RECEPTION_FILE);
    return false;
  }
  _tleStoragePath = pbtools::toQString(_receptionConf.file_storage().tle());
  _receptionStoragePath = pbtools::toQString(_receptionConf.file_storage().root());
  _deleteInputFile = _receptionConf.file_storage().delete_input();
  _copyInputFile = _receptionConf.file_storage().copy_input();
  _copyCaduFile = _receptionConf.file_storage().copy_cadu();

  for (int idx = 0; idx < _receptionConf.satellite_size(); idx++) {
    _satReception.insert(
        QString::fromStdString(_receptionConf.satellite(idx).name()),
        _receptionConf.satellite(idx)
    );
  }
  return true;
}

//! Чтение настроек приборов
bool SatFormat::readInstrsConf() {
  QFile file(INSTRCONF_FILE);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка при загрузке параметров приборов из файла '%1'").arg(INSTRCONF_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (!TProtoText::fillProto(text, &_instrConf)) {
    error_log << QObject::tr("Ошибка в структуре файла параметров приборов '%1'").arg(INSTRCONF_FILE);
    return false;
  }

  for (int idx = 0; idx < _instrConf.instrs_size(); idx++) {
    _instrConfCol.insert(_instrConf.instrs(idx).type(), _instrConf.instrs(idx));
  }
  return true;
}

//! Чтение настроек спутников
bool SatFormat::readSatsConf() {
  QFile file(SATCONF_FILE);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка при загрузке параметров спутника из файла '%1'").arg(SATCONF_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (!TProtoText::fillProto(text, &_pretrConf)) {
    error_log << QObject::tr("Ошибка в структуре файла параметров спутника '%1'").arg(SATCONF_FILE);
    return false;
  }

  for (int idx = 0; idx < _pretrConf.satellite_size(); idx++) {
    _satConf.insertMulti(QString::fromStdString(_pretrConf.satellite(idx).name()), _pretrConf.satellite(idx));
    _satPipelines.insert(
        QString::fromStdString(_pretrConf.satellite(idx).name()),
        QString::fromStdString(_pretrConf.satellite(idx).pipeline())
    );
  }

  return true;
}

const mappi::conf::PretrSat* SatFormat::getPretreatmentFor(const QString &name, conf::RateMode mode) {
  QMap<QString, mappi::conf::PretrSat>::const_iterator it = _satConf.constFind(name);
  while (it != _satConf.constEnd() && it.key() == name) {
    if (it.value().mode() == mode) {
      return &it.value();
    }
    ++it;
  }
  return nullptr;
}

QString SatFormat::getWeatherFilePath(const QDateTime dateTime) {
  return meteo::global::findWeatherFile(_tleStoragePath, dateTime);
}

QString SatFormat::getCompositesSettingsFilePath(){
  return COMPCONF_FILE;
}

QString SatFormat::getPipelineFor(const QString &satName) {
  return _satPipelines.contains(satName) ? _satPipelines.value(satName) : "";
}

const mappi::conf::ReceptionParam SatFormat::getReceptionFor(const QString &satName) {
  return _satReception.contains(satName) ? _satReception.value(satName) : mappi::conf::ReceptionParam();
}

QVector<conf::InstrumentType> SatFormat::getInstrumentTypesBy(const QString &instrName){
  std::string name = instrName.toStdString();
  QVector<conf::InstrumentType> result;
  for (int idx = 0; idx < _instrConf.instrs_size(); idx++) {
    if(_instrConf.instrs(idx).name() == name) {
      result.push_back(_instrConf.instrs(idx).type());
      continue;
    }else {
      for(int j = 0; j < _instrConf.instrs(idx).alias_size(); j++){
        if(_instrConf.instrs(idx).alias(j) == name) result.push_back(_instrConf.instrs(idx).type());
        continue;
      }
    }
  }
  return result;
}

bool SatFormat::hasInstrument(const conf::InstrumentType &instr_type){
  return _instrConfCol.contains(instr_type);
}

conf::Instrument SatFormat::getInstrumentBy(const conf::InstrumentType &instr_type){
  return _instrConfCol.value(instr_type);
}

QString SatFormat::getSiteName() const {
  return QString::fromStdString(_receptionConf.site().name().c_str());
};
meteo::GeoPoint SatFormat::getSiteCoord() const {
  meteo::GeoPoint point;
  meteo::sprinf::GeoPoint p = _receptionConf.site().point();
  point.setPoint(p.lat_radian(), p.lon_radian(), p.height_meters());
  return point;
};