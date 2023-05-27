#ifndef TGRIB_IFACE_H
#define TGRIB_IFACE_H

#include <meteo/commons/proto/tgribformat.pb.h>

class TGridGrib;
class TProductGrib;
template<class T> class QVector;

namespace grib {
  class TGridDefinition;
  class TProductDefinition;
}



//! фасад для grib::TGribData
class TGribIface {
public:
  TGribIface(const grib::TGribData* grib);
  ~TGribIface();

  void setGrib(const grib::TGribData* grib);

  const grib::TGridDefinition& grid();
  const grib::TProductDefinition& product();

  //! true - если сетка регулярная
  bool isRegular() const { return !_grib->grid().has_points(); }
  //! true - если есть битовое отображение
  bool isBitmap() const { return _grib->has_bitmap(); }

  bool fillPoints(QVector<uint16_t>* points) const;
  bool fillPoints(::google::protobuf::RepeatedField< ::google::protobuf::int32 >* points) const;
  bool fillData(QVector<float>* data) const;
  bool fillData(::google::protobuf::RepeatedField< double >* data) const;
  
private:
  void clear();
  bool createGrid();
  bool createProduct();

  bool fillData(QVector<float>* data, const std::string& str) const;

private:
  const grib::TGribData* _grib; //!< структура GRIB
  TGridGrib* _grid; //!< описание сетки
  TProductGrib* _prod; //!< описание продукта
  int _vers; //!< номер версии GRIB
  bool _isDummy; //!< true - если создана пустышка, т.к. был передан нулевой указатель
};

#endif
