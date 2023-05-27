#ifndef METEO_COMMONS_UI_MAP_OBJECTMIMEDATA_H
#define METEO_COMMONS_UI_MAP_OBJECTMIMEDATA_H

#include <qmimedata.h>
#include <meteo/commons/global/common.h>

namespace meteo {
namespace map {

class ObjectMimeData : public QMimeData
{
  Q_OBJECT
public:
  ObjectMimeData();
  ~ObjectMimeData() {}
  QList< QPair< PrimitiveTypes, std::string > > serialObjectsList() const;
  void setSerialObjectsList(const QList< QPair< PrimitiveTypes, std::string > >& serialObjectsList);
  bool hasSerialObjectsList() const;
  void clear();

private:
  QList< QPair< PrimitiveTypes, std::string > > serialObjectsList_;
  bool hasSerialObjectsList_ = false;
};


}
}


#endif //METEO_COMMONS_UI_MAP_OBJECTMIMEDATA_H
