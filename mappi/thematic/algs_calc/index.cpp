#include "index.h"
#include "channel.h"
#include "themformat.h"

#include <qimage.h>
#include <qfile.h>


using namespace mappi;
using namespace to;

namespace {

mappi::to::ThemAlg* createIndex(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new Index(type,them_name, ds);
}

static const bool res8 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kIndex8, createIndex);
static const bool resgr8 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kGrayScale, createIndex);
//static const bool resNdvi = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kNdvi, createIndex);
}

Index::Index(mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds)
  :ThemAlg(type,them_name,ds)
{
}

Index::~Index()
{
}

//! Обработка данных спутника
bool Index::process()
{
  data_.clear();
  bitmap_.clear();
  const auto chs = channels();
  if(chs.size() == 0) return false;

  if(config_.vars_size() != chs.size() || false == isValid(config_))
  {
    error_log << QObject::tr("Не все данные получены. настроено:")<<" Получено "<<chs.keys() ;
    return false;
  }

  thematic::ThematicCalc calc;
  auto chlist = chs.keys();
  for(const auto &themVar : qAsConst(chlist))
  {
    QString varName = QString::fromStdString(themVar);
    calc.bindArray(varName, *chs.value(themVar));
  }

  bool res = calc.parsingExpression(QString::fromStdString(config_.red()));
  if(false == res){
    return false;
  }
  bitmap_ = calc.dataProcessing();


  //palette_.clear();
  data_ = calc.getResult();
  return !data_.isEmpty();
}


//bool Index::saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format)
//{

//}

