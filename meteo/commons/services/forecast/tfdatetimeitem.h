#ifndef METEO_COMMONS_FORECAST_TFDATETIMEITEM_H
#define METEO_COMMONS_FORECAST_TFDATETIMEITEM_H
#include "tfitem.h"
#include <meteo/commons/proto/field.pb.h>
#include <cross-commons/debug/tlog.h>

class TFDateTimeItem : public TFItem
{
public:
  TFDateTimeItem(const QString &aitemText,const QString &aitemName,TFModel *aparentI);

//    virtual void reset();

    virtual void setData(const QVariant& adata) ;
    virtual void setMarker(const QColor& );
    virtual QVariant getItemDataM();
    virtual QVariant getItemData();
    virtual QString getItemInfo();


private:
};

#endif // METEO_COMMONS_FORECAST_TFCOMBOITEM_H
