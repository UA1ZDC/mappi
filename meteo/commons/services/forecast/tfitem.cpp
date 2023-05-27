#include <QStringList>
#include <QDebug>

#include "tfitem.h"
#include "tdataprovider.h"
#include "tffielditem.h"
#include "tfmodel.h"
#include "tforecast.h"

TFItem::TFItem(const QString & text,const QString & descr, TFModel *amodel):
    model_(amodel),
    itemScriptArgs_(nullptr),
    itemFunc_(nullptr),
    itemRole_(meteo::forecast::ForecastMethodItemRole::kNoData),
    itemText_(text),
    itemDescriptor_(descr),
    cur_value_index_(-1)
{
    setMarker(BAD_DATA);
    itemFunc_          = new QScriptValue();
    itemScriptArgs_    = new QStringList;
    meta_data_.station = amodel->station();
    need_reset_        = false;
}

TFItem::~TFItem()
{
    if(itemScriptArgs_ ){delete itemScriptArgs_ ; itemScriptArgs_ =nullptr;}
    if(itemFunc_ ){delete itemFunc_ ; itemFunc_ =nullptr;}
    qDeleteAll(childItems_);
}

void TFItem::setDataSourceType(dataSource sdt){
   meta_data_.data_source_type = sdt;
}

void TFItem::setParrent(const QString &ap){
    parentItem_ = ap;
}

void TFItem::appendDeps(TFItem *dit){
    if(!depItems_.contains(dit)){
        depItems_.append(dit);
    } else {
        debug_log <<dit->getDescr()<< "уже есть в списке! Вероятно, ошибка в скрипте!";
    }
}

const meteo::GeoPoint & TFItem::curCoord() const {
    if(cur_coord_.fi()<-999.||cur_coord_.la()<-998.){
        if(constParent()){
            return constParent()->curCoord();
        }
    }

    return cur_coord_;
}

void TFItem::setFunction(const  QScriptValue& afunc,const  QStringList& args){
    //setRole(scriptable_data);
    *itemFunc_ = afunc;
    *itemScriptArgs_ = args;
    for(int k = 2; itemScriptArgs_->length() > k; k++ )
    {
        QString itscr = itemScriptArgs_->at(k);
        if(itscr.isEmpty()) continue;
        setDependPoName(itscr);
    }
}

void TFItem::setCurCoord(const meteo::GeoPoint & ac){
    cur_coord_ = ac;
}

TDataProvider* TFItem::dataProvider(){
    if(nullptr != model()){
        return model()->dataProvider();
    }
    return nullptr;
}

void TFItem::setNoData(){
    itemData_ = QObject::tr("Нет данных");
    model_->beforeRepaint();
    setMarker( BAD_DATA);
    model_->repaint();

}

void TFItem::setErrInScript(){
    itemData_ = QObject::tr("Ошибка в скрипте");
    model_->beforeRepaint();
    setMarker( BAD_DATA);
    model_->repaint();
}

bool TFItem::getValue() {

    if(nullptr == dataProvider()) {
        return false;
    }
    bool ok = false;
    if (currentIndex() > meta_list_.size() - 1 || currentIndex() < 0) return false;
    if(field_src == meta_list_.value(currentIndex()).data_source_type){
        std::string findex = meta_list_.value(currentIndex()).field_id.toStdString();
        fieldValue fval;
        if (dataProvider()->getFiedValuePoIndex(parent()->curCoord(), findex, &fval)){
            itemData_ = QString::number( fval.value, 'f', checkRoundDigits() );
            ok = true;
          }
      } else {
        ok = true;
        itemData_ = QString::number( values_list_.value(currentIndex()), 'f', checkRoundDigits() );
      }
    if(ok) {setMarker(GOOD_DATA);}
     else { setNoData();}
    return ok;
}

void TFItem::setItemData(const QVariant& adata){
    if(0==adata.toString().compare(QObject::tr("Нет данных"),Qt::CaseInsensitive)){
        setNoData();
        resetChilds();
        return;
    }
    if(0==adata.toString().compare("nan",Qt::CaseInsensitive)){
        setErrInScript();
        resetChilds();
        return;
    }
    if(itemData_ == adata){
        return;
    }
    cur_value_index_ = -1;
    itemData_ = adata;
    setMarker(MANUAL_DATA);

    // если автоперерасчет включен, то рассчитывать прогноз не будем
    if ( disable_autoreset==false ){
        resetChilds();
    }
}

void TFItem::setData(const QVariant& adata)
{
    if(adata.toInt() == cur_value_index_){
        model_->repaint();
        return;
    }
    cur_value_index_ = adata.toInt();

    if (-1 < cur_value_index_) {
        setMarker( GOOD_DATA);
        if (getValue()) {
            resetChilds();
        }
    } else {
        setNoData();
        resetChilds();
    }
}

void TFItem::appendChild(TFItem *item)
{
    item->setParrent(itemDescriptor_);
    childItems_.append(item);
}

TFItem *TFItem::child(int row)
{
    return childItems_.value(row);
}


meteo::forecast::ForecastMethodItemRole TFItem::role(){
    return itemRole_;
}


int TFItem::childCount() const
{
    return childItems_.count();
}

QColor& TFItem::marker()  {

    if(0 == childItems_.size() ){
        return marker_;
    }
    else {

        QColor m=GOOD_DATA;
        foreach(TFItem *ait,childItems_){
            if(BAD_DATA == ait->marker_ ){
                m = BAD_DATA;
                break;
            }
        }
        if ( m!=marker_ ){
            setMarker(m);
        }
    }

    return marker_;
}

int TFItem::columnCount() const
{
    return 3;
}

/**
 * при установке маркера - оповещаем всех, что маркер ихменился
 * чтобы перерисовать стили
 * @param mrk [description]
 */
void TFItem::setMarker(const QColor& mrk){
    if ( mrk == BAD_DATA ){
        good_data_ = false;
    }else{
        good_data_ = true;
    }

    // испускаем сигнал только если маркер поменялся
    if ( mrk != marker_ ){
        marker_ = mrk;
        emit(markerChanged());
    }

}

void TFItem::resetData(){

    values_list_.clear();
    meta_list_.clear();
    if(nullptr == dataProvider() ||  nullptr == model()) {
        return;
    }
    TFItem* it_vs = model()->getItem(parentItem_);
    if(nullptr == it_vs ) return;
    model()->beforeRepaint();
    meta_data_.station.coord = it_vs->curCoord();
    meta_data_.srok =  getValuePoName(srok_).toFloat();
    meta_data_.level = getValuePoName(level_).toInt();
    meta_data_.type_level = getValuePoName(type_level_).toInt();

    if( true == isInAdvection()){
        meta_data_.station.index = "";
        meta_data_.station.name = "";
    }

    if(dataProvider()->getValue( meta_data_, itemDescriptor_, &meta_list_,&values_list_)){
        setCurCoord( meta_data_.station.coord );
        if(0 < meta_list_.size()){
            cur_value_index_ = meta_list_.keys().first();
            getValue();
            itemRole_ = meteo::forecast::ForecastMethodItemRole::kSrcData;
            setMarker(GOOD_DATA);
        }
    } else {
        setNoData();
    }
    model()->repaint();
}


void TFItem::reset()
{
    switch(itemRole_){
    case meteo::forecast::ForecastMethodItemRole::kSimpleData:
    case meteo::forecast::ForecastMethodItemRole::kCoordData:
        break;
    case meteo::forecast::ForecastMethodItemRole::kSrfData:
    case meteo::forecast::ForecastMethodItemRole::kAeroData:
    case meteo::forecast::ForecastMethodItemRole::kSrcData:
        resetData();
        break;
    case meteo::forecast::ForecastMethodItemRole::kScriptableData:
    case meteo::forecast::ForecastMethodItemRole::kScriptableComboData:
    case meteo::forecast::ForecastMethodItemRole::kResultData:
    case meteo::forecast::ForecastMethodItemRole::kDatetimeData:
        resetScript();
        break;
    case meteo::forecast::ForecastMethodItemRole::kFieldData:
    case meteo::forecast::ForecastMethodItemRole::kComboData:
        setMarker(GOOD_DATA);
        break;
    case meteo::forecast::ForecastMethodItemRole::kCalcData:
        setMarker(GOOD_DATA);
        break;
    case meteo::forecast::ForecastMethodItemRole::kNoData:
        setMarker(BAD_DATA);
    default:
        itemData_ = QVariant();
    }

}

TFItem * TFItem::setDependPoName(const QString &itscr){
    if(itscr.isEmpty()) return nullptr;
    bool ok;
    itscr.toFloat(&ok);
    TFItem* itvs = nullptr;
    if(!ok){//если это не число - значит указатель на итем
        itvs = model()->getItem(itscr);
        if(!itvs){
            setErrInScript();
            return itvs;
        }
        itvs->appendDeps(this);
    }
    return itvs;
}



QVariant TFItem::getValuePoName(const QString &itscr){

    if(itscr.isEmpty()) return "";
    bool ok;
    float fval = itscr.toFloat(&ok);
    if(!ok){//если это не число - значит указатель на итем
        TFItem* itvs = model()->getItem(itscr);
        if(!itvs){
            setErrInScript();
            return "";
        }
        // debug_log<<"getValuePoName "<<itvs->getItemData().toString();
        return itvs->getItemData();
    }
    return QString::number(fval);
}


void TFItem::resetScript()
{
    if(itemScriptArgs_->length() < 2) return ;
    QScriptValueList args;
    for(int k = 2; itemScriptArgs_->length() > k; k++ )
    {
        QString itscr = itemScriptArgs_->at(k);
        if(itscr.isEmpty()) continue;
        
        QVariant vsvs = getValuePoName(itscr);
        
        // debug_log<<"===== itemScriptArgs_" << itscr << " vsvs: "<<vsvs.toString();

        if(!vsvs.isValid() || vsvs.isNull()) return;
        if(!model_ || !model_->mainMethod() ||!model_->mainMethod()->engine()) return;
        QScriptValue scvs = model_->mainMethod()->engine()->newVariant(vsvs);
        // debug_log<<  scvs.toVariant().value<meteo::GeoPoint>().toString();
        args << scvs;

    }
    // QScriptValue ret_val1 = itemFunc_->call(QScriptValue(), args);
    QScriptValue ret_val = itemFunc_->call(QScriptValue(), args);

    if( ret_val.isError()
            || 0 == ret_val.toString().compare("undefined",Qt::CaseInsensitive)){ //FIXME пока так
        setErrInScript();
        return;
    }

    if(  ret_val.toString() == BAD_VALUE
         || 0 == ret_val.toString().compare("nan",Qt::CaseInsensitive)){
        setNoData();
    } else {
        setMarker(GOOD_DATA);
        if(ret_val.isArray()){
            //   debug_log<< ret_val.toVariant().toStringList();
            items_ = ret_val.toVariant().toStringList();
            return;
        }
        if(meteo::forecast::ForecastMethodItemRole::kDatetimeData == itemRole_){
            int hour = int(ret_val.toNumber());
            int minutes = (ret_val.toNumber() - hour)*60;
            itemData_ = QTime(hour,minutes);
            return;
          }
        if(!ret_val.isNumber() && ret_val.isString()){
            itemData_ = ret_val.toString();
        } else{
            //
            // назначем форматирование
            //
            // error_log<<"script: "<<itemDescriptor_;
            itemData_ = QString::number(ret_val.toNumber(),'f', checkRoundDigits() );
        }
    }

}

TFItem* TFItem::getItem(const QString &args)
{
    QStringList list = args.split(".");
    if (2 == list.size())
    {
        QString parentDescr = list[0];
        QString itemDescr = list[1];
        TFItem* aparentItem = getPoDescriptor(parentDescr);
        if(nullptr != aparentItem){
            return aparentItem->getPoDescriptor(itemDescr);
        }
    }
    else if (3 == list.size())
    {
        QString parentDescr = list[0];
        QString itemDescr = list[1];
        QString valueDescr = list[2];
        TFItem* aparentItem = getPoDescriptor(parentDescr);
        if(nullptr != aparentItem){
            TFItem* aItem = aparentItem->getPoDescriptor(itemDescr);
            if(nullptr != aItem){
                return aItem->getPoDescriptor(valueDescr);
            }
        }
    }
    return nullptr;
}

/**
 * возвращаем значение, которое содержится в экземпляре класса
 * @return [description]
 */
int TFItem::checkRoundDigits(){
    if(nullptr == dataProvider()) return 1;
    return dataProvider()->checkRoundDigits(itemDescriptor_);
}

/**
 * возвращаем значение, которое содержится в экземпляре класса
 * @return [description]
 */
QRegExp TFItem::getRegexpByParam(){
    // error_log<<"regexp"<<itemDescriptor_;
  if(nullptr == dataProvider()) return QRegExp();
  return dataProvider()->getRegexpByParam(itemDescriptor_);
}

QString TFItem::getComboText(const FMetaData & m){

QString svs = m.station.name;
if(!m.station.index.isEmpty()) svs = svs+ " ("+m.station.index+" )";

if ( 0 < m.srok) {
    svs += QString::fromUtf8(" (%1 ч)").arg(m.srok);
}
svs += QObject::tr(" ") + m.dt.toString("dd.MM.yyyy hh:mm") + " ";
if (m.level > 0 ) {
    svs += QString::number(m.level) + QObject::tr("гПа\n");
}
return svs;
}

QString TFItem::getItemInfo(){
    if (currentIndex() > listData( ).size() - 1 || currentIndex() < 0) return QString();
    QString lvs;
    FMetaData meta_data = meta_list_.value(currentIndex());
    if(field_src == meta_data.data_source_type){
        if (meta_data.type_level > 1 ) {
            lvs = QObject::tr("Данные на уровне %1 гПа\n").arg(meta_data.level);
        } else {
            lvs = QObject::tr("Приземные данные \n");
        }
        lvs += QObject::tr("центр ") + meta_data.station.name + " \n";
        lvs += QObject::tr("срок ") + meta_data.dt.toString("yyyy-MM-dd HH:mm") + " ";
        if (meta_data.srok > 0) {
            lvs += QObject::tr("прогноз на %1 ч\n").arg(meta_data.srok);
        } else {
            lvs += QObject::tr("анализ \n");
        }
        if(meta_data.station.coord.isValid()){
            lvs += "Координаты: "+ meta_data.station.coord.toString();
        }
        return lvs;
    }

    switch(meta_data.type_level){
    case 1:
        lvs += "Данные наблюдений";
        break;
    case 100:
        lvs += "Данные зондирования";
        break;
    default:
        break;
    }
    QString fullname = "" ;
    if(0!=meta_data.station.index.compare("-9999")){
        fullname = meta_data.station.name+" (" + meta_data.station.index + ")";
      }
    switch(meta_data.data_source_type){
    case cur_station_src:
        lvs += "\nна станции " +fullname;
        break;
    case near_station_src:
        lvs += "\nна ближайшей станции " +fullname;
        break;
    case cur_aero_station_src:
        lvs += "\nна станции " + fullname;
        break;
    case near_aero_station_src:
        lvs += "\nна ближайшей станции " +fullname;
        break;
    case field_src:
        lvs = "Данные анализа";
        break;
    default:
        return QString();
    }
    lvs += QObject::tr(" \nсрок ") +
            (meta_data.dt.toString("yyyy-MM-dd HH:mm")) + "\n";

    switch(meta_data.type_level){
    case 1:
        lvs += " (у поверхности)\n";
        break;
    case 100:
        lvs += " (уровень " +QString::number(meta_data.level) + " гПа)\n";
        break;
    default:
        return QString();
        break;
    }

    if(meta_data.station.coord.isValid()) {
        lvs += "Координаты: "+ meta_data.station.coord.toString();
    }
    if(0 < meta_data.distance_to_point ) {
        lvs +="\n(расстояние до пункта прогноза "+ QString::number(meta_data.distance_to_point)+" км)";
    }
    return lvs;
}

QVariant TFItem::getAddItemData(){

    if (currentIndex() > listData( ).size() - 1 || currentIndex() < 0) return QVariant();

    if (listData( ).count() != 0) {
        return listData( ).at(currentIndex());
    }

    return QVariant();
}

QVariant TFItem::getItemData() {
    switch(itemRole_){
    case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
        return itemData_;
        break;
    case meteo::forecast::ForecastMethodItemRole::kSimpleData:
    case meteo::forecast::ForecastMethodItemRole::kSrfData:
    case meteo::forecast::ForecastMethodItemRole::kAeroData:
    case meteo::forecast::ForecastMethodItemRole::kScriptableData:
    case meteo::forecast::ForecastMethodItemRole::kCalcData:
    case meteo::forecast::ForecastMethodItemRole::kCoordData:
    case meteo::forecast::ForecastMethodItemRole::kFieldData:
    case meteo::forecast::ForecastMethodItemRole::kResultData:
    case meteo::forecast::ForecastMethodItemRole::kSrcData:
    case meteo::forecast::ForecastMethodItemRole::kNoData:
        return itemData_;
    default:
        return QVariant();
    }
    return QVariant();
}

QVariant TFItem::getItemDataM() {
    switch(itemRole_){
    case meteo::forecast::ForecastMethodItemRole::kAdvFieldData: {
        meteo::GeoPoint gp = itemData_.value<meteo::GeoPoint>();
        if(gp.isValid()) return gp.toString();
        else return itemData_;
        break;}
    case meteo::forecast::ForecastMethodItemRole::kSimpleData:
    case meteo::forecast::ForecastMethodItemRole::kSrfData:
    case meteo::forecast::ForecastMethodItemRole::kAeroData:
    case meteo::forecast::ForecastMethodItemRole::kScriptableData:
    case meteo::forecast::ForecastMethodItemRole::kCalcData:
    case meteo::forecast::ForecastMethodItemRole::kCoordData:
    case meteo::forecast::ForecastMethodItemRole::kFieldData:
    case meteo::forecast::ForecastMethodItemRole::kResultData:
    case meteo::forecast::ForecastMethodItemRole::kNoData:
    case meteo::forecast::ForecastMethodItemRole::kSrcData:
        return itemData_;
    default:
        return QVariant();
    }
    return QVariant();
}

void TFItem::setRole(const meteo::forecast::ForecastMethodItemRole& arole){
    itemRole_ = arole;
}

void TFItem::setTypeLevel(const QString& alvl){
    type_level_ = alvl;
    meta_data_.type_level = getValuePoName(alvl).toInt();
}

void TFItem::setLevel(const QString& alvl){
  setDependPoName(alvl);
  level_ = alvl;
}

void TFItem::setSrok(const QString &asrok){
  setDependPoName(asrok);
  srok_ = asrok;
}

QVariant TFItem::data(int column){
    switch(column){
    case 0:
        return itemText_;
    case 1:
        switch(itemRole_){
        case meteo::forecast::ForecastMethodItemRole::kFieldData:
            return getItemData();
        case meteo::forecast::ForecastMethodItemRole::kSimpleData:
        case meteo::forecast::ForecastMethodItemRole::kSrfData:
        case meteo::forecast::ForecastMethodItemRole::kAeroData:
        case meteo::forecast::ForecastMethodItemRole::kScriptableData:
        case meteo::forecast::ForecastMethodItemRole::kCalcData:
        case meteo::forecast::ForecastMethodItemRole::kCoordData:
        case meteo::forecast::ForecastMethodItemRole::kComboData:
        case meteo::forecast::ForecastMethodItemRole::kScriptableComboData:
        case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
        case meteo::forecast::ForecastMethodItemRole::kResultData:
        case meteo::forecast::ForecastMethodItemRole::kSrcData:
        default:
            return getItemDataM();
        case meteo::forecast::ForecastMethodItemRole::kNoData:
            return getItemData();

        }
    case 2:
        switch(itemRole_){
            case meteo::forecast::ForecastMethodItemRole::kFieldData:
            case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
            case meteo::forecast::ForecastMethodItemRole::kSrcData:
                return getAddItemData();
            default:
                return QVariant();
        }

    }
return QVariant();
}

TFModel *TFItem::model(){
    return model_;
}

TFItem* TFItem::parent(){
    if( nullptr == model_) return nullptr;
    return model_->getItem(parentItem_);
}

TFItem* TFItem::constParent() const{
    if( nullptr == model_) return nullptr;
    return model_->getItem(parentItem_);

}


int TFItem::row() const
{
    if (model_ && model_->getItem(parentItem_)) {
        return model_->getItem(parentItem_)->childItems_.indexOf(const_cast<TFItem*>(this));
    }

    return 0;
}

TFItem *TFItem::getPoDescriptor(const QString& aitemDescriptor_)
{
    if(aitemDescriptor_ == itemDescriptor_ ) return this;
    foreach(TFItem *ait,childItems_){
        if(ait){
            TFItem *itvs = ait->getPoDescriptor(aitemDescriptor_);
            if(nullptr != itvs) return itvs;
        }
    }
    return nullptr;
}

void TFItem::resetAll()
{
    reset();
    foreach(TFItem *ait,childItems_){
        if(ait) ait->resetAll();
    }
}

void TFItem::needResetAll(bool r)
{
    foreach(TFItem *ait,childItems_){
        if(ait){
            ait->needReset(r);
            ait->needResetAll(r);
        }
    }
}

void TFItem::resetChilds()
{
    if(!need_reset_) return;

    foreach(TFItem *ait,childItems_){
        if(ait)  {
            ait->reset();
            ait->setCurCoord(cur_coord_);
            ait->resetChilds();
        }
    }
    foreach(TFItem *ait,depItems_){
        if(ait)  {
            ait->reset();
            ait->resetChilds();
        }
    }
}

QStringList TFItem::listData( )
{
    QStringList vs;
    for (int i = 0; i < meta_list_.count(); ++i ) {
      int ind = meta_list_.keys().at(i);
      vs.insert(ind , getComboText(meta_list_.value(ind)));
    }
    return vs;

}

int TFItem::currentIndex( )
{
    return cur_value_index_;
}
/*
void TFItem::setAddData(const QVariant& ){
  //adata_ = adata;
}
*/
void TFItem::setInAdvection(bool sna){
    is_in_advection_ = sna;
}

bool TFItem::isInAdvection() const{
    return is_in_advection_;
}


/**
 * @brief Вызываем перерасчет по первому скрипту
 * 
 * @param item 
 */
bool TFItem::resetFirstChildItemForScript(){
    if ( !itemData_.toString().isEmpty() ) {
        resetChilds();
        return true;
    }
    foreach(TFItem *ait,childItems_){
        if(ait)  {
            if ( ait->resetFirstChildItemForScript() ){
                return true;
            }
        }
    }
    foreach(TFItem *ait,depItems_){
        if(ait)  {
            if ( ait->resetFirstChildItemForScript() ){
                return true;
            }
        }
    }
    return false;
}

