#include "objectmimedata.h"

namespace meteo {
namespace map {

ObjectMimeData::ObjectMimeData() : QMimeData()
{

}

QList< QPair< PrimitiveTypes, std::string > > ObjectMimeData::serialObjectsList() const
{
  return serialObjectsList_;
}

void ObjectMimeData::setSerialObjectsList(const QList<QPair<PrimitiveTypes, std::string> > &serialObjectsList)
{
  serialObjectsList_ = QList< QPair< PrimitiveTypes, std::string > >(serialObjectsList);
  hasSerialObjectsList_ = true;
}

bool ObjectMimeData::hasSerialObjectsList() const
{
  return hasSerialObjectsList_;
}

void ObjectMimeData::clear()
{
  serialObjectsList_.clear();
  hasSerialObjectsList_ = false;
  QMimeData::clear();
}
}
}

