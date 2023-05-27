#ifndef METEO_COMMONS_FORECAST_TFCOMBOITEM_H
#define METEO_COMMONS_FORECAST_TFCOMBOITEM_H
#include "tfitem.h"
#include <meteo/commons/proto/field.pb.h>
#include <cross-commons/debug/tlog.h>

class TFComboItem : public TFItem
{
public:
  TFComboItem(const QString &aitemText,const QString &aitemName,TFModel *aparentI);

//    virtual void reset();

    virtual void setData(const QVariant& adata) ;

    void setItems(const QStringList& items) { items_ = items; }

    virtual QStringList listData() { return items_;  }
    virtual int currentIndex();
    virtual void setMarker(const QColor& );
    virtual QVariant getItemDataM();
    virtual QVariant getItemData();
    virtual QString getItemInfo();

    //TFItem *coord_item;
    //TFItem *value_item;

private:
};

#endif // METEO_COMMONS_FORECAST_TFCOMBOITEM_H
