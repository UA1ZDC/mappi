#include "rgb.h"
#include "channel.h"
#include "themformat.h"

#include <qimage.h>
#include <qfile.h>

#include <mappi/pretreatment/images/colors.h>

using namespace mappi;
using namespace to;

namespace {

mappi::to::ThemAlg* RgbAlg(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::RgbAlg(type, them_name, ds);
}

static const bool resRgb = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kFalseColor, RgbAlg);
}

RgbAlg::RgbAlg(mappi::conf::ThemType type, const std::string &them_name, QSharedPointer<DataStore> &ds)
  :ThemAlg(type,them_name,ds)
{

}

RgbAlg::~RgbAlg()
{
}

void RgbAlg::setData(Color color_channel, const QVector<uchar> &data )
{
  float gamma =1., coef = 1.;
  float min = 0;
  float max = 255;

  if(pixels_.contains(color_channel)){
    warning_log << QObject::tr("Цветовой канал %1 уже существует хешмапе pixels_")
                      .arg(static_cast<std::underlying_type<Color>::type>(color_channel));
  }

  switch (color_channel) {
    case Color::kRed:
      if(config_.has_red_gamma()){
        gamma = config_.red_gamma();
      }
      if(config_.has_red_min()){
        min = config_.red_min();
      }
      if(config_.has_red_max()){
        max = config_.red_max();
      }
    break;
    case Color::kBlue:
      if(config_.has_blue_gamma()){
        gamma = config_.blue_gamma();
      }
      if(config_.has_blue_min()){
        min = config_.blue_min();
      }
      if(config_.has_blue_max()){
        max = config_.blue_max();
      }
    break;
    case Color::kGreen:
      if(config_.has_green_gamma()){
        gamma = config_.green_gamma();
      }
      if(config_.has_green_min()){
        min = config_.green_min();
      }
      if(config_.has_green_max()){
        max = config_.green_max();
      }
    break;
  }
  coef = 255. / pow(max - min, gamma);

  pixels_[color_channel].resize(data.size());
  for(int i = 0; i < data.size(); i++)
    pixels_[color_channel][i] = static_cast<uchar>(coef * pow(data.at(i) - min,  gamma));
}


//! Обработка данных спутника
bool RgbAlg::process()
{
  pixels_.clear();
  const auto chs = channels();

  if(!isValid(config_)){
      error_log << QObject::tr("Неверный конфигурационный файл.");
      return false;
  }
  for(const auto &config_val : qAsConst(config_.vars())){
    if(!chs.contains(config_val.name())){
      error_log << QObject::tr("В потоке не найден канал %1.").arg(QString::fromStdString(config_val.name())) << chs.keys() ;
      return false;
    }
  }

//  normalize(0, 255);

  thematic::ThematicCalc calc;
  auto keys = chs.keys();

  for(const auto &themVar : qAsConst(keys) )
  {
    QString varName = QString::fromStdString(themVar);
    calc.bindArray( varName, *chs.value(themVar) );
  }

  calc.parsingExpression(QString::fromStdString(config_.red()));
  setData(Color::kRed, calc.dataProcessing());

  calc.parsingExpression(QString::fromStdString(config_.green()));
  setData(Color::kGreen, calc.dataProcessing());

  calc.parsingExpression(QString::fromStdString(config_.blue()));
  setData(Color::kBlue, calc.dataProcessing());
  return !pixels_.isEmpty();
}

bool RgbAlg::saveImage(const QString& name, const QSharedPointer<Channel>& /*channel*/, QImage::Format /*format*/)
{
  if (pixels_.size() < 3) {
    debug_log << "Невозможно сохранить изображение. Размер хешмапа pixels_:" << pixels_.size();
    return false;
  }

  if(channel_size_ != pixels_[Color::kBlue].size() ||
     channel_size_ != pixels_[Color::kGreen].size() ||
     channel_size_ != pixels_[Color::kRed].size()){
      debug_log << QObject::tr("Невозможно сохранить изображение. Размеры каналов не совпадают %1!=(%2,%3,%4)")
                                .arg(channel_size_)
                                .arg(pixels_[Color::kRed].size())
                                .arg(pixels_[Color::kGreen].size())
                                .arg(pixels_[Color::kBlue].size());
      return false;
  }
  debug_log << QObject::tr("Изображение: %1(%2x%3) -> %4")
                  .arg(channel_size_)
                  .arg(rows_)
                  .arg(cols_)
                  .arg(name);

  bitmap_.clear();
  bitmap_.resize(channel_size_ * 4);
  uchar alpha = 255;
  for(int i = 0; i < channel_size_; ++i)
  {
    bitmap_[i * 4]     = pixels_[Color::kRed ].at(i);
    bitmap_[i * 4 + 1] = pixels_[Color::kGreen].at(i);
    bitmap_[i * 4 + 2] = pixels_[Color::kBlue  ].at(i);
    bitmap_[i * 4 + 3] = alpha;
  }
  QString imageFileName = name + ".png";
  QImage im(bitmap_.data(), columns(), rows(), columns()*4, QImage::Format_RGBA8888);
  bool ok = im.save(imageFileName, "PNG");

  if(ok) debug_log << QObject::tr("Сохранено изображение: %1(%2x%3)").arg(imageFileName).arg(rows()).arg(columns());
  else error_log << QObject::tr("Ошибка сохранения изображения: %1(%2x%3)").arg(imageFileName).arg(rows()).arg(columns());
  return ok;
}
