#include "tfdatetimeitem.h"
#include <cross-commons/debug/tlog.h>
#include "tdataprovider.h"

TFDateTimeItem::TFDateTimeItem(const QString &aitemText,const QString &itemIndex,TFModel *aparentI)
: TFItem(aitemText,itemIndex,aparentI)
{
  itemRole_ = meteo::forecast::ForecastMethodItemRole::kDatetimeData;
  itemData_ =  QTime(0,0);
}


void TFDateTimeItem::setData(const QVariant& adata)
{
  itemData_ = adata;
  resetChilds();
}

void TFDateTimeItem::setMarker(const QColor & mrk)
{ //trc;
  marker_=mrk;
}


QVariant TFDateTimeItem::getItemData() {
  float reth = getItemDataM().toTime().hour();
  reth += getItemDataM().toTime().minute()/60.;
  return MnMath::ftoi_norm(reth);
}

QVariant TFDateTimeItem::getItemDataM() {
  return itemData_;
}


QString TFDateTimeItem::getItemInfo(){
  return getItemDataM().toTime().toString("Время hh:mm");
}


