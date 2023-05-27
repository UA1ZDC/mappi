#include "tfcomboitem.h"
#include <cross-commons/debug/tlog.h>
#include "tdataprovider.h"

TFComboItem::TFComboItem(const QString &aitemText,const QString &aitemName,TFModel *aparentI)
: TFItem(aitemText,aitemName,aparentI)
{
  itemRole_ = meteo::forecast::ForecastMethodItemRole::kComboData;
  //cur_item = 0;
  itemData_ =  0;
  //coord_item = nullptr;
 // value_item = nullptr;
}

int TFComboItem::currentIndex()
{

  return itemData_.toInt();
}



void TFComboItem::setData(const QVariant& adata)
{
  itemData_ = adata;
  resetChilds();
  //  resetDeps();
}

void TFComboItem::setMarker(const QColor & mrk)
{ //trc;
  marker_=mrk;
 // if(coord_item)  coord_item->setMarker(mrk);
 // if(value_item)  value_item->setMarker(mrk);
}


QVariant TFComboItem::getItemData() {
  return getItemDataM();
}

QVariant TFComboItem::getItemDataM() {
  bool ok = false;
  int i = itemData_.toInt(&ok);
  if(ok && -1<i && i< items_.count()){
    return items_.at(i);
  }
  return QVariant();
}


QString TFComboItem::getItemInfo(){

  return getItemData().toString();
}


