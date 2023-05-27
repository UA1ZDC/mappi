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

  if(!pixels_.contains(color_channel)){
      debug_log << QObject::tr("Цветовой канал %1 добавлен в хешмап pixels_").arg(static_cast<std::underlying_type<Color>::type>(color_channel));
      pixels_.insert(color_channel, QVector<uchar>());
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

  for(uchar pix: data){
    uchar pixel = coef * pow(pix - min,  gamma);
    pixels_[color_channel].append(pixel);
  }
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
  if(config_.vars_size() != chs.size()){
    error_log << QObject::tr("Количество каналов в конфигурационном файле(%1) и выбранном потоке(%2) не совпадают.").arg(config_.vars_size()).arg(chs.size()) ;
    return false;
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



bool RgbAlg::saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format )
{
  const auto chs = channels();
  if (chs.isEmpty()) {
    debug_log << "empty";
    return false;
  }

  bitmap_.clear();

  if (pixels_.size() < 3) {
    debug_log << "Невозможно сохранить изображение. Размер хешмапа pixels_:" << pixels_.size();
    return false;
  }

  int size = pixels_.first().size();
  if(0 == size){
      debug_log << "Невозможно сохранить изображение. Размер первого канала равен 0:";
      return false;
  }
  if(pixels_[Color::kBlue].size() != pixels_[Color::kGreen].size() || pixels_[Color::kGreen].size() != pixels_[Color::kRed].size()){
      debug_log << QObject::tr("Невозможно сохранить изображение. Размеры каналов не совпадают (%1,%2,%3)")
                                .arg(pixels_[Color::kBlue].size())
                                .arg(pixels_[Color::kGreen].size())
                                .arg(pixels_[Color::kRed].size());
      return false;
  }
  bitmap_.resize(size * 4);
  uchar alpha = 255;
  for(int i = 0; i < size; ++i)
  {
    bitmap_[i * 4]     = pixels_[Color::kBlue ].at(i);
    bitmap_[i * 4 + 1] = pixels_[Color::kGreen].at(i);
    bitmap_[i * 4 + 2] = pixels_[Color::kRed  ].at(i);
    bitmap_[i * 4 + 3] = alpha;
  }
  QString imageFileName = name + ".png";
  QImage im(bitmap_.data(), channel->columns(), channel->rows(), channel->columns()*4, QImage::Format_ARGB32);
  bool ok = im.save(imageFileName, "PNG");
  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения файла %1").arg(imageFileName);
  }
  return ok;
}

//bool RgbAlg::saveData(const QString& /*baseName*/)
//{
//  // Сохранить QMap data_ в каком-нибудь формате
//  return true;
//}
