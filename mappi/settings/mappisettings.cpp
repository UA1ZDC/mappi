#include "mappisettings.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>

template<> ::mappi::MappiSettings* mappi::inter::Settings::_instance = 0;

using namespace mappi;

static const QString kReceptionConf = "reception.conf";
//static const QString kThematicsConf = "thematics.conf";
static const QString kThematicsCalcConf = "thematics_calc.conf";
static const QString kSatInstrConf = "satinstr.conf";
static const QString kPaletteConf = "palette.conf";
static const QString kThematicThresholds = "thresholds.conf";
static const QString kThematicCoef = "coefficients.conf";

MappiSettings::MappiSettings()
{
  //appendConf(kThematicsConf, MnCommon::etcPath("mappi"), &thematics_, QObject::tr("Предустановленные тематические обработки"));
  appendConf(kThematicsCalcConf, MnCommon::etcPath("mappi"), &thematics_calc_, QObject::tr("Предустановленные тематические обработки (калькулятор)"));
  appendConf(kReceptionConf, MnCommon::etcPath("mappi"), &reception_, QObject::tr("Конфигурация приёмника"));
  appendConf("satpretr.conf", MnCommon::etcPath("mappi"), &satellites_, QObject::tr("Параметры спутников"));
  appendConf(kSatInstrConf, MnCommon::etcPath("mappi"), &instruments_, QObject::tr("Параметры инструментов"));
  
  // appendConf("satframe.conf", MnCommon::etcPath("mappi"), &satframe_, QObject::tr("описание фрейма спутниковых данных"));
  appendConf(kPaletteConf, MnCommon::etcPath("mappi"), &palettes_, QObject::tr("Палитра тематических обработок"));
  appendConf(kThematicThresholds, MnCommon::etcPath("mappi"), &them_thresholds_,
             QObject::tr("Пороговые значения для методов тематической обработки"));
  appendConf(kThematicCoef, MnCommon::etcPath("mappi"), &them_coef_, QObject::tr("Коэффициенты для методов тематической обработки"));
}


MappiSettings::~MappiSettings()
{
  
}


TColorGradList MappiSettings::palette(const std::string & themname) const
{
  TColorGradList grad;

  for ( int i = 0, isz = palettes_.palette_size(); i < isz; ++i ) {
    const ::mappi::conf::ThematicPalette& p = palettes_.palette(i);
    if ( p.name() != themname ) {
      continue;
    }

    for ( int j = 0, jsz = p.color_size(); j < jsz; ++j ) {
      const ::meteo::map::proto::ColorGrad& c = p.color(j);
      double min = c.min_value();
      double max = min;
      if ( c.has_max_value() ) {
        max = c.max_value();
      }
      QColor ncolor = QColor::fromRgba( c.min_color() );
      QColor xcolor = ncolor;
      if ( c.has_max_color() ) {
        xcolor = QColor::fromRgba( c.max_color() );
      }

      grad.append(TColorGrad(min, max, ncolor, xcolor ));
      if(c.has_title()){
        grad.last().setTitle( QString::fromStdString(c.title()));
      }
    }

    break;
  }

  return grad;
}

void MappiSettings::setPalette(const std::string & themname, const TColorGradList& grad)
{
  int idx = -1;
  for ( int i = 0, isz = palettes_.palette_size(); i < isz; ++i ) {
    const ::mappi::conf::ThematicPalette& p = palettes_.palette(i);
    if ( p.name() == themname ) {
      idx = i;
      break;
    }
  }

  ::mappi::conf::ThematicPalette* p = nullptr;
  if ( -1 == idx ) {
    p = palettes_.add_palette();
    p->set_name(themname);
  }
  else {
    p = palettes_.mutable_palette(idx);
    p->clear_color();
  }

  for ( int i = 0, isz = grad.size(); i < isz; ++i ) {
    const TColorGrad& cg = grad.at(i);
    ::meteo::map::proto::ColorGrad* c = p->add_color();
    c->set_min_value(cg.begval());
    if ( !cg.oneval() ) {
      c->set_max_value(cg.endval());
    }
    c->set_min_color(cg.begcolor().rgba());
    if ( !cg.onecolor() ) {
      c->set_max_color(cg.endcolor().rgba());
    }
    if(false == cg.title().isEmpty())
    {
      c->set_title(cg.title().toStdString());
    }
  }
  if (true == saveFile(MnCommon::etcPath("mappi") + kPaletteConf, &palettes_)) {

  }


}

bool MappiSettings::satellite(const QString& satelliteName, ::mappi::conf::PretrSat* satellite)
{
  if (nullptr == satellite) {
    return false;
  }
  
  bool result = false;
  
  for (int i = 0; i < satellites_.satellite_size(); ++i) {
    if (satellites_.satellite(i).has_name() &&
        QString::fromStdString(satellites_.satellite(i).name()) == satelliteName) {
      satellite->CopyFrom(satellites_.satellite(i));
      result = true;
      break;
    }
  }
  
  return result;
}

bool MappiSettings::radiometer(const QString& satelliteName, ::mappi::conf::Instrument* instr)
{
  if (nullptr == instr) {
    return false;
  }
  
  ::mappi::conf::PretrSat sat;
  if(false == satellite(satelliteName, &sat)) {
    error_log << QObject::tr("Не найдены настройки спутника") << satelliteName;
    return false;
  }


  if(!sat.has_radiometer()) {
    return false;
  }

  ::mappi::conf::InstrumentType type = sat.radiometer();
  return instrument(type, instr);
}

bool MappiSettings::instrumentByName(const std::string& instrName, ::mappi::conf::Instrument* instrument)
{
  for ( int i = 0, sz = instruments_.instrs_size(); i < sz; ++i ) {
    if (true == instruments_.instrs(i).has_name() &&
        instruments_.instrs(i).name() == instrName) {
      instrument->CopyFrom(instruments_.instrs(i));
      return true;
    }
  }
  
  return false;
}

bool MappiSettings::instrument(::mappi::conf::InstrumentType type, ::mappi::conf::Instrument* instrument)
{
  if (nullptr == instrument) {
    return false;
  }
  
  for ( int i = 0, sz = instruments_.instrs_size(); i < sz; ++i ) {
    if (true == instruments_.instrs(i).has_type() &&
        instruments_.instrs(i).type() == type) {
      instrument->CopyFrom(instruments_.instrs(i));
      return true;
    }
  }
  
  return false;
}


bool MappiSettings::saveReception(::mappi::conf::Reception* reception)
{
  if (true == saveFile(MnCommon::etcPath("mappi") + kReceptionConf, reception)) {
    reception_.CopyFrom(*reception);
    return true;
  }

  return false;
}

bool MappiSettings::saveThematics(::mappi::conf::ThematicProcs* thematics)
{
  if (true == saveFile(MnCommon::etcPath("mappi") + kThematicsCalcConf, thematics)) {
    thematics_calc_.CopyFrom(*thematics);
    //thematics_.CopyFrom(*thematics);
    return true;
  }
  return false;
}

bool MappiSettings::saveThresholds(::mappi::conf::ThematicThresholds* thematics)
{
  if (true == saveFile(MnCommon::etcPath("mappi") + kThematicThresholds, thematics)) {
    them_thresholds_.CopyFrom(*thematics);
    return true;
  }
  return false;
}

bool MappiSettings::saveCoef(::mappi::conf::ThematicCoef* thematics)
{
  if (true == saveFile(MnCommon::etcPath("mappi") + kThematicCoef, thematics)) {
    them_coef_.CopyFrom(*thematics);
    return true;
  }
  return false;
}


//TODO копия из meteo::Settings, там private
bool MappiSettings::saveFile(const QString& fileName, google::protobuf::Message* message)
{
  QString data = TProtoText::toText(*message);

  bool status = false;
  if ( false == data.isEmpty() ) {
    meteo::SettingsFile sf(fileName);
    status = sf.write( data.toUtf8() );
    if(true == status){
      //   info_log<<QObject::tr("Настройки сохранены успешно (%1)").arg(fileName);
    }
  }
  else {
    error_log << QString("Получены пустые значения настройки %1").arg( fileName );
  }

  return status;
}
