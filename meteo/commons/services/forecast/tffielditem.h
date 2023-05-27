#ifndef METEO_COMMONS_FORECAST_TFFIELDITEM_H
#define METEO_COMMONS_FORECAST_TFFIELDITEM_H

#include "tfitem.h"
#include <meteo/commons/proto/field.pb.h>
#include <cross-commons/debug/tlog.h>


enum neededVal{
  fValue,
  fGradX,
  fGradY,
  fRadKriv,  
  fLapl  
};

class TFFieldItem : public TFItem
{
public:
  TFFieldItem(const QString &aitemText,const QString &aitemname,TFModel  *aparentI);

    
    virtual void setData(const QVariant& adata);
    virtual QString getItemInfo();
    virtual QVariant getAddItemData() ;
    virtual QStringList listData( );
    virtual int currentIndex( );
    virtual void setMarker(const QColor&);
    virtual void appendChild(TFItem *child);
    
    void reset();
    void needValue();
    void setFieldProp(const QString &sparam, const QString &asrok,const QString &alevel,const QString &atypelevel, const QString &atime);
    void setFieldPropAdd(const QString &sparam,const QString &typeAdd, const QString &asrok,const QString &alevel,const QString &atypelevel, const QString &atime);
 
    const QList< meteo::field::DataDesc>& getFieldlist() const  {return fieldlist_;}
    void setFieldlist(const QList< meteo::field::DataDesc> &fl) { fieldlist_ = fl;}  
    void setCurrentIndex( int index );
    
private:  
  void needAdvection(const QVariant &);
  bool getAdvectPoint();
  bool getValue();
  QString getComboText(const meteo::field::DataDesc &d);
      
    meteo::field::DataRequest fieldProp;
    bool is_need_advection_ =false;
    bool isNeedValue;

    int cur_field;
    QList< meteo::field::DataDesc> fieldlist_;
    fieldValue fval_;
    int needed_val_;
};


#endif // METEO_COMMONS_FORECAST_TFFIELDITEM_H
