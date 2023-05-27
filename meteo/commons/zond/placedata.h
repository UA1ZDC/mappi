#ifndef PlaceData_H
#define PlaceData_H

#include <meteo/commons/proto/surface_service.pb.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/clouddata.h>

namespace zond {
  
class PlaceData
{
public:
  PlaceData();

  bool setData(const meteo::surf::OneZondValueOnStation &zond); 
  bool getMeteoData(double pur, TMeteoData *md)const;
  void clear();  

  int32_t dataSize() const ;
  int32_t serializeToArray( char* arr ) const ;
  int32_t parseFromArray( const char* data );

  const QString& index() const { return index_; }
  const QString& ruName() const { return ruName_; }
  const QString& enName() const { return enName_; }
  const meteo::GeoPoint& coord() const { return coord_; }
  const QDateTime& dt() const  { return dt_; }
  const QString& name() const   { return name_; }
  const int& model() const { return model_; }
  double xPos() const { return xPos_; }
  const zond::Zond& zond() const { return zond_; }
  int echelon() const { return echelon_; }
  bool helpPoint() const { return helpPoint_; }
  QString docName()const ;
  zond::Zond& zond() { return zond_; }

  void setIndex(const QString& index) { index_ = index; }
  void setIndex(int index) { index_ = QString::number(index); }
  void setRuName(const QString& name) { ruName_ = name; }
  void setEnName(const QString& name) { enName_ = name; }
  void setName(const QString& name)   { name_ = name; }
  void setModel(const int& model) { model_ = model; }
  void setCoord(double lat, double lon) { coord_.setLat(lat); coord_.setLon(lon); } //радианы
  void setCoord(const meteo::GeoPoint& coord) { coord_ = coord; }
  void setDt(const QDateTime& dt) { dt_= dt; }
  void setXPos(double xpos)  { xPos_ = xpos; }
  void setZond(const QByteArray& ba) { zond_.clear(); zond_ << ba; }
  void setEchelon(int echelon) { echelon_ = echelon; }
  void setHelpPoint(bool helpPnt) { helpPoint_ = helpPnt; }
  
  friend const PlaceData& operator>>(const PlaceData& data, QByteArray& out);
  friend PlaceData& operator<<(PlaceData& data, const QByteArray& ba);

private:
  
  QString index_;
  QString ruName_;
  QString enName_;
  meteo::GeoPoint coord_;
  QDateTime dt_; 
  QString name_;
  double    xPos_;
  zond::Zond zond_;
  int echelon_;
  int model_;
  bool helpPoint_; // флаг промежточной точки (используется в качестве дополнительных данных для расчётов)
  
};

  const PlaceData& operator>>(const PlaceData& data, QByteArray& out);
  PlaceData& operator<<(PlaceData& data, const QByteArray& ba);

}

#endif
