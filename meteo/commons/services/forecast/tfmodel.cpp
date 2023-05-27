#include <QtGui>
#include <QStyledItemDelegate>

#include "tfitem.h"
#include "tfmodel.h"
#include "tdataprovider.h"
#include "tffielditem.h"
#include "tfcomboitem.h"
#include "tforecast.h"
#include "tfdatetimeitem.h"


//! [0]
TFModel::TFModel( TDataProvider *adataProvider,  TForecast *amain_method, const QString &aname )
  : QAbstractItemModel(amain_method), rootItem_(nullptr),dataProvider_(adataProvider),main_method_(amain_method)
{
  // debug_log<<"TFModel";

  if(!rootItem_){
    rootItem_ = new TFItem(QObject::tr("Величина"),aname,this);
    rootItem_->setData(QObject::tr("Значение"));
    //rootItem_->setAddData(QObject::tr("Поле"));
    rootItem_->setCurCoord(amain_method->getStationCoord());
  }
}

TFModel::~TFModel()
{
  // debug_log<<"~TFModel";

  delete rootItem_;
}

TFItem *TFModel::rootItem(){
  return rootItem_;
}

bool TFModel::setData( const QModelIndex & index,
                       const QVariant &value , int role )
{

  TFItem* item = static_cast<TFItem*>(index.internalPointer());
  if(!item) return false;
  beforeRepaint();
  if(1 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
      case meteo::forecast::ForecastMethodItemRole::kSrcData:
      default:
        if(Qt::EditRole == role ){
          item->setItemData(value);
        } else item->setData(value);
      break;
    }
  } else {
    item->setData(value);
  }
  repaint();

  return true;
}

int TFModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<TFItem*>(parent.internalPointer())->columnCount();
  else
    return rootItem_->columnCount();
}

QVariant TFModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();


  TFItem *item = static_cast<TFItem*>(index.internalPointer());
  if(!item) {
    return QVariant();
  }

  if( role == Qt::ToolTipRole) {
    return  item->getItemInfo();
  }

  if (role == Qt::DecorationRole && 0 == index.column())
  {
    QPixmap marker_vs = QPixmap(15,15);
    marker_vs.fill(Qt::transparent);
    QPainter p(&marker_vs);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setBrush( QBrush( item->marker() ) );
    p.setPen(  item->marker().darker(150) );
    p.drawEllipse( 1,1,13,13 );
    return marker_vs;
  }
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  return item->data(index.column());
}

Qt::ItemFlags TFModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  return  Qt::ItemIsEnabled | Qt::ItemIsSelectable|Qt::ItemIsEditable;

}

QVariant TFModel::headerData(int section, Qt::Orientation orientation,
                             int role) const
{ //QHeaderView::Stretch
  if (orientation == Qt::Horizontal ){
    //
    // устанавливаем заголовок таблицы (колонок)
    //
    if ( role == Qt::DisplayRole ){
      switch(section){
        case 0:
        return QObject::tr("Величина");
        case 1:
        return QObject::tr("Значение параметра");
        case 2:
        return QObject::tr("Источник параметра");
      }
    }


  }
  // устанавливаем размер колонок
  if ( role == Qt::SizeHintRole){
    return QSize(500,30);
  }



  return QVariant();
}

QModelIndex TFModel::index(int row, int column, const QModelIndex &parent)
const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  TFItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<TFItem*>(parent.internalPointer());

  TFItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex TFModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  TFItem *childItem = static_cast<TFItem*>(index.internalPointer());
  TFItem *parentItem = childItem->parent();

  if (parentItem == nullptr ){
    debug_log << "parentItem is NULL!";
    return QModelIndex();
  }
  if (parentItem == rootItem_ )
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int TFModel::rowCount(const QModelIndex &parent) const
{
  TFItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<TFItem*>(parent.internalPointer());

  return parentItem->childCount();
}

void TFModel::beforeRepaint(){
  emit(layoutAboutToBeChanged() );

}


void TFModel::repaint(){
  emit(layoutChanged() );

}

void TFModel::resetModel(){
  beforeRepaint();
  if(nullptr != rootItem_){
    rootItem_->resetAll();
  }
  repaint();

}

/**
 * проверяем все ли маркеры включены
 */
void TFModel::slotCheckMarkers(){
  bool all_good_state=true;
  if ( !rootItem_ ){
    return;
  }
  // проверяем на корректность введенных данных все подчиненные айтемы
  foreach(TFItem *aitt, rootItem_->getChilds() ){
    if ( aitt->childCount()>0  ){
      foreach(TFItem *ait, aitt->getChilds() ){

        // debug_log << ait->getText() << ait->isAllGood();

        if( !ait->isAllGood() ){
          // если есть хоть один невведенный - все плохо
          all_good_state=false;
        }
      }
    }else{
      // debug_log << aitt->getText() << aitt->isAllGood();
      if( !aitt->isAllGood() ){
        // если есть хоть один невведенный - все плохо
        all_good_state=false;
      }
    }
  }

  // проверяем есть ли изменения по отношению к предыдующему состоянию
  if ( all_good_state!=all_good ){
    // если есть - меняем и испускаем сигнал
    this->all_good = all_good_state;
    emit( allGoodChanges() );
  }

}

/**
 * подключаемся к сигналу
 * @param item [description]
 */
void TFModel::connectNewItem( TFItem *item ){
  // подключаемcz к сигналу, испускаемому при изменении маркера
  if(nullptr != main_method_->getWidget()){
    QObject::connect( item, SIGNAL(markerChanged()), this, SLOT( slotCheckMarkers() ) );
  }
  return;
}

void TFModel::addCalcItem(const QString & text, const QString & descrName, const QString & funcName,
                          const QString &parentName)
{
  Q_UNUSED(descrName);
  Q_UNUSED(text);
  Q_UNUSED(funcName);
  Q_UNUSED(parentName);
}

TFItem * TFModel::addDataItem(const QString & text, const QString & descr,
                              const QString &parentName, const QString & alvl,
                              const QString & atypelevel,
                              const QString & asrok, const QString & aname)
{
  Q_UNUSED(aname);

  TFItem *pit = getItem(parentName);
  if(nullptr == pit){
    debug_log << QObject::tr("не найден родитель!");
    return nullptr;
  }
  TFItem *item = new TFItem(text,descr,this);
  // подключаем сигнал к новому айтему
  connectNewItem( item );
  pit->appendChild(item);
  item->setLevel(alvl);
  item->setTypeLevel(atypelevel);
  item->setSrok(asrok);
  item->setRole(meteo::forecast::ForecastMethodItemRole::kSrcData);
  return item;
}


void TFModel::addSimpleItem(const QString & text,const QString&descr, const QString &parentName, const QString &val)
{

  if(nullptr == getItem(parentName)){
    debug_log << QObject::tr("не найден родитель!");
    return;
  }
  TFItem* item = new TFItem(text,descr,this);

  // подключаем сигнал к новому айтему
  connectNewItem( item );

  item->setRole(meteo::forecast::ForecastMethodItemRole::kSimpleData);
  if(getItem(parentName)){
    getItem(parentName)->appendChild(item);
  }

  item->setItemData(val);
  // item->reset();
}

//
TFItem* TFModel::addComboFuncItem(const QScriptValue& func, const  QScriptValue& args,const QString& parentName)
{
  QStringList vs = args.toVariant().toString().split("//");
  if(vs.length() < 2){
    debug_log << "bad data in script "<<vs;
    return nullptr;
  }
  QString text = vs.at(0);
  QString descr = vs.at(1);
  TFItem* item = new TFComboItem(text,descr,this);
  TFItem* par_item = getItem(parentName);
  if(nullptr == par_item){
    delete item;
    if(main_method_){
      debug_log << QObject::tr("Ошибка в скрипте метода ") << main_method_->methodName();
    } else {
      debug_log << QObject::tr("Неизвестная ошибка в скрипте");
    }
    return nullptr;
  }
  par_item->appendChild(item);
  item->setRole(meteo::forecast::ForecastMethodItemRole::kScriptableComboData);
  item->setFunction(func,vs);
  return item;
}


TFItem* TFModel::addFuncItem(const QScriptValue& func,
                             const  QScriptValue& args,const QString& parentName)
{

  QStringList vs = args.toVariant().toString().split("//");
  if(vs.length() < 2){
    debug_log << "bad data in script "<<vs;
    return nullptr;
  }
  QString text = vs.at(0);
  QString descr = vs.at(1);
  TFItem* item = new TFItem(text,descr,this);
  TFItem* par_item = getItem(parentName);
  if(nullptr == par_item){
    delete item;
    if(main_method_){
      debug_log << QObject::tr("Ошибка в скрипте метода ") << main_method_->methodName();
    } else {
      debug_log << QObject::tr("Неизвестная ошибка в скрипте");
    }
    return nullptr;
  }
  par_item->appendChild(item);

  // подключаем сигнал к новому айтему
  connectNewItem( item );

  item->setRole(meteo::forecast::ForecastMethodItemRole::kScriptableData);
  item->setFunction(func,vs);
  return item;
}



TFItem* TFModel::getItem(const QString& itemName){
  TFItem* aItemvs = rootItem_;
  QStringList descriptors = itemName.split(".");
  if(1 < descriptors.count()&& nullptr != main_method_){
    aItemvs = main_method_->getMainItem(descriptors.at(0));
  }

  for(int i =0; i< descriptors.count(); ++i){
    QString itemvs = descriptors.at(i);
    if(nullptr == aItemvs) return nullptr;
    aItemvs = aItemvs->getPoDescriptor(itemvs);
  }


  return aItemvs;
}

void TFModel::addFieldItem(const QString& parentName, const QString& text,const QString& aname,
                           const QString & sparam, const QString &  asrok,
                           const QString &alevel, const QString &atypelevel,
                           const QString &  atime)
{
  TFFieldItem* item = new TFFieldItem(text,aname,this);
  if(getItem(parentName)){
    getItem(parentName)->appendChild(item);
  }
  item->setFieldProp(sparam,  asrok, alevel,atypelevel,  atime);
  // item->reset();


}

void TFModel::addFieldAdd(const QString& parentName,const QString& addn, const QString& text,const QString& aname,
                          const QString & sparam, const QString &asrok,const QString &alevel, const QString &atypelevel, const QString &atime)
{
  TFFieldItem* item = new TFFieldItem(text,aname,this);
  if(getItem(parentName)){
    getItem(parentName)->appendChild(item);
  }
  item->setFieldPropAdd(sparam,addn,asrok, alevel,atypelevel,atime);
  item->needValue();
  // item->reset();
}

void TFModel::addFieldValueItem(const QString& parentName, const QString& text,
                                const QString & sparam, const QString &asrok,
                                const QString &alevel,const QString &atypelevel,const QString& aName)
{
  TFFieldItem* item = new TFFieldItem(text,aName,this);
  if(getItem(parentName)){
    getItem(parentName)->appendChild(item);
  }
  item->setFieldProp(sparam, asrok, alevel,atypelevel, "0");
  item->needValue();
  //item->reset();
}

void TFModel::addComboItem(const QString &text, const QString &descr, const QStringList &items, const QString &parentName)
{

  TFComboItem* item = new TFComboItem(text,descr,this);
  if(getItem(parentName)){
    getItem(parentName)->appendChild(item);
  }

  item->setItems(items);
  //item->setLevel(lvl);
  //item->reset();

}
TFItem* TFModel::addDTItem(const QScriptValue& func,
                           const  QScriptValue& args,
                           const QString& parentName)
{
  QStringList vs = args.toVariant().toString().split("//");
  if(vs.length() < 2){
    debug_log << "bad data in script "<<vs;
    return nullptr;
  }
  QString text = vs.at(0);
  QString itemIndex = vs.at(1);

  TFItem* item = new TFDateTimeItem(text,itemIndex,this);


  //TFItem* item = new TFComboItem(text,descr,this);
  TFItem* par_item = getItem(parentName);
  if(nullptr == par_item){
    delete item;
    if(main_method_){
      debug_log << QObject::tr("Ошибка в скрипте метода ") << main_method_->methodName();
    } else {
      debug_log << QObject::tr("Неизвестная ошибка в скрипте");
    }
    return nullptr;
  }
  par_item->appendChild(item);
  item->setRole(meteo::forecast::ForecastMethodItemRole::kDatetimeData);
  item->setFunction(func,vs);
  return item;
}

QString TFModel::methodFullName(){
  if(nullptr == main_method_) return "";
  return main_method_->methodFullName();
}

QString TFModel::methodName(){
  if(nullptr == main_method_) return "";
  return main_method_->methodName();
}

StationData TFModel::station() const{
  if(nullptr == main_method_) return StationData();
  return main_method_->getStationData();
}


