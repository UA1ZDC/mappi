#ifndef METEO_COMMONS_OCEAN_OCEAN_H
#define METEO_COMMONS_OCEAN_OCEAN_H

#include <meteo/commons/ocean/turoven.h>
#include <commons/meteo_data/meteo_data.h>


namespace ocean {

  class Ocean  {
  public:

    Ocean();
    ~Ocean();
    
    void print() const;
    //    bool setData(const meteo::surf::OneOceanValueOnPoint& data);
    int countValid(ValueType) const;
    
    bool setData(TMeteoData &data);
    //   bool setDataPoH(const TMeteoData& data);
    bool haveData() const {return isData_;}
    const QMap<float, Uroven> & urovenList() const {return urList_;}
  
    ocean::Uroven getUrPoKey(float key)const;
    bool isEmpty();
    void clear();  
  
  private:
    //    void fromProtoUroven(const meteo::surf::Uroven_S& res, ocean::Uroven *ur_);

  private:
    bool isData_; //!< Загружены или нет данные
    QMap<float, ocean::Uroven> urList_; //!< данные по уровням
    ValueType keyType_;
  };
}

#endif
