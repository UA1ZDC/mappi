#include "tfieldparams.h"

#include <qfile.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include "tfield.h"

static const QString kFieldSettingsPath = MnCommon::etcPath("meteo") + "/field.conf.d";

namespace obanal {

namespace singleton {
  template<> obanal::TFieldParams* TFieldParams::_instance = nullptr;
}

TFieldParams::TFieldParams() { loadParams(); }

TFieldParams::~TFieldParams() { }

void TFieldParams::setFieldParams(TField* field)
{
  if(nullptr == field) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }

//  loadParams(); Без мьютекса нельзя просто так взять и загрузить параметры, потому что многопоточность, а тут синглтон

  int descr = field->getDescr();
  int level = field->getLevel();
  int type_level = field->getLevelType();

  proto::TField fp;
  if(false == params_.contains(descr)) {
    return;
//    if ( false == params_.contains(-1) ) {
//      return;
//    }
//    else {
//      fp.CopyFrom( params_[-1] );
//    }
  }
  else {
    fp.CopyFrom(params_[descr]);
  }
  QString name  = QString::fromStdString(fp.name());
  QString unit  = QString::fromStdString(fp.unit());
  float minval = fp.minvalue();
  float maxval = fp.maxvalue();
  float step   = fp.step();

  for ( const proto::TFieldLevel& lvl : fp.level() ) {
    if ( lvl.level() == level ) {
      if ( true == lvl.has_name())     { name = QString::fromStdString(lvl.name());}
      if ( true == lvl.has_minvalue()) { minval = lvl.minvalue(); }
      if ( true == lvl.has_maxvalue()) { maxval = lvl.maxvalue(); }
      if ( true == lvl.has_step())     { step = lvl.step();       }

      for ( const proto::TFieldTypeLevel& tplvl : lvl.type_level() ) {
        if ( tplvl.type_level() == type_level) {
          if ( true == tplvl.has_minvalue()) { minval = tplvl.minvalue(); }
          if ( true == tplvl.has_maxvalue()) { maxval = tplvl.maxvalue(); }
          if ( true == tplvl.has_step())     { step = tplvl.step();       }
        }
      }
    }
  }

  field->name_     = name;
  field->unit_     = unit;
  field->min_val_  = minval;
  field->max_val_  = maxval;
  field->step_val_ = step;
}

void TFieldParams::loadParams()
{
  QMap<QString, proto::TField> paramfiles;
  if(false == TProtoText::loadProtosFromDirectory(kFieldSettingsPath, &paramfiles)) {
    error_log << QObject::tr("Не удалось загрузить параметры полей");
    return;
  }

  for(auto it = paramfiles.cbegin(); it != paramfiles.cend(); ++it) {
    const proto::TField& f = it.value();
    if(false == f.has_descr()) {
      warning_log << QObject::tr("В файле настроки поля %1 обнаружены параметры без указания дескриптора.").arg(it.key());
      continue;
    }
    params_.insert(f.descr(), f);
  }
}

float TFieldParams::minvalue(const int64_t descr, const int level, const int type_level, bool* ok) const
{
  bool   result_ok = false;
  float result    = -9999.0;

  if(false == hasDescriptor(descr)) {
    if(nullptr != ok) { *ok = result_ok; }
    return result;
  }

  const proto::TField& fp = params_[descr];
  for(const proto::TFieldLevel& lvl : fp.level()) {
    if(lvl.level() == level) {
      for(const proto::TFieldTypeLevel& tplvl : lvl.type_level()) {
        if(tplvl.type_level() == type_level) {
          if(tplvl.has_minvalue()) {
            result_ok = true;
            result    = tplvl.minvalue();
            break;
          }
        }
      }
      if(false == result_ok && lvl.has_minvalue()) {
        result_ok = true;
        result    = lvl.minvalue();
        break;
      }
      else {
        break;
      }
    }
  }

  if(false == result_ok && fp.has_minvalue()) {
    result_ok = true;
    result    = fp.minvalue();
  }

  if(nullptr != ok) { *ok = result_ok; }
  return result;
}

float TFieldParams::maxvalue(const int64_t descr, const int level, const int type_level, bool* ok) const
{
  bool   result_ok = false;
  float result    = -9999.0;

  if(false == hasDescriptor(descr)) {
    if(nullptr != ok) { *ok = result_ok; }
    return result;
  }

  const proto::TField& fp = params_[descr];
  for(const proto::TFieldLevel& lvl : fp.level()) {
    if(lvl.level() == level) {
      for(const proto::TFieldTypeLevel& tplvl : lvl.type_level()) {
        if(tplvl.type_level() == type_level) {
          if(tplvl.has_maxvalue()) {
            result_ok = true;
            result    = tplvl.maxvalue();
            break;
          }
        }
      }
      if(false == result_ok && lvl.has_maxvalue()) {
        result_ok = true;
        result    = lvl.maxvalue();
        break;
      }
      else {
        break;
      }
    }
  }

  if(false == result_ok && fp.has_maxvalue()) {
    result_ok = true;
    result    = fp.maxvalue();
  }

  if(nullptr != ok) { *ok = result_ok; }
  return result;
}

float TFieldParams::step(const int64_t descr, const int level, const int type_level, bool* ok) const
{
  bool   result_ok = false;
  float result    = -9999.0;

  if(false == hasDescriptor(descr)) {
    if(nullptr != ok) { *ok = result_ok; }
    return result;
  }

  const proto::TField& fp = params_[descr];
  for(const proto::TFieldLevel& lvl : fp.level()) {
    if(lvl.level() == level) {
      for(const proto::TFieldTypeLevel& tplvl : lvl.type_level()) {
        if(tplvl.type_level() == type_level) {
          if(tplvl.has_step()) {
            result_ok = true;
            result    = tplvl.step();
            break;
          }
        }
      }
      if(false == result_ok && lvl.has_step()) {
        result_ok = true;
        result    = lvl.step();
        break;
      }
      else {
        break;
      }
    }
  }

  if(false == result_ok && fp.has_step()) {
    result_ok = true;
    result = fp.step();
  }

  if(nullptr != ok) { *ok = result_ok; }
  return result;
}

}
