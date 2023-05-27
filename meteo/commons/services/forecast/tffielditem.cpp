#include "tffielditem.h"
#include "tdataprovider.h"
#include "tfmodel.h"

#include <qdatetime.h>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/pbtools.h>

Q_DECLARE_METATYPE(meteo::GeoPoint)

TFFieldItem::TFFieldItem(const QString &aitemText, const QString &aitemName, TFModel *aparentI)
  : TFItem(aitemText, aitemName, aparentI)
{
  itemRole_       = meteo::forecast::ForecastMethodItemRole::kFieldData;
  isNeedValue     = false;
  srok_           = "0";
  time_           = "0";
  needed_val_     = fValue;
  cur_field       = -1;
}

void TFFieldItem::reset() {
  fieldlist_.clear();
  if (!dataProvider()) return;

  int asrok = getValuePoName(srok_).toFloat() * 3600;
  //debug_log<<time_<<getValuePoName(time_);
  // debug_log<<srok_<<getValuePoName(srok_);
  // debug_log<<srok_<<asrok;

  needAdvection(getValuePoName(time_));
  fieldProp.set_forecast_start(dataProvider()->getDate().addSecs(asrok).toString(Qt::ISODate).toStdString());
  //
  // не ставим зазор 3 часа, чтобы хоть какие-то данные получить, даже если не попали в срок
  //
  fieldProp.set_forecast_end(dataProvider()->getDate().addSecs(asrok).toString(Qt::ISODate).toStdString());
  fieldProp.clear_coords();
  meta_data_.level = getValuePoName(level_).toInt();
  meta_data_.type_level = getValuePoName(type_level_).toInt();
  fieldProp.clear_level();
  fieldProp.clear_type_level();
  fieldProp.add_level(meta_data_.level);
  fieldProp.add_type_level(meta_data_.type_level);
  meteo::surf::Point* p = fieldProp.add_coords();
  p->set_fi(curCoord().fi());
  p->set_la(curCoord().la());

  if (0 == dataProvider()->getFieldList(fieldProp, &fieldlist_)) {
    setNoData();
    return;
  }
  setData(0);
}


void TFFieldItem:: setData(const QVariant &value)
{
    /*if( value.toInt() == cur_field ){
        return;
    }*/
  cur_field = value.toInt();

  if (-1 < cur_field) {
    setMarker( GOOD_DATA);

    // error_log<<"is_need_advection_ : "<< is_need_advection_;

    if (is_need_advection_ && getAdvectPoint() ) {
      resetChilds();
    }

    // error_log<<"isNeedValue : "<< isNeedValue;

    if (isNeedValue && getValue()) {
      resetChilds();
    }
  } else {
    setNoData();
    resetChilds();
  }

}

bool TFFieldItem::getAdvectPoint() {
  bool result = false;
  if (currentIndex() > fieldlist_.size() - 1 || currentIndex() < 0) return result;
  const meteo::field::DataDesc &cur_fielddesc =  fieldlist_.at(currentIndex());
  ::meteo::GeoPoint station_coord = curCoord();
  if (parent()) {
    station_coord = parent()->curCoord();
  }
  int atime = getValuePoName(time_).toFloat() * 3600;

  if (dataProvider()->getAdvectPoint( cur_fielddesc.level(), cur_fielddesc.center(), atime, &station_coord)) {
    result = true;
    setRole(meteo::forecast::ForecastMethodItemRole::kAdvFieldData);
    itemData_ = QVariant::fromValue(station_coord);//.toString() ;
    setCurCoord(station_coord);

    setMarker(GOOD_DATA);

  } else {
    result = false;
    setRole(meteo::forecast::ForecastMethodItemRole::kFieldData);
    setNoData();
  }

  return result;
}

bool TFFieldItem::getValue() {

  if (!dataProvider()) {
    return false;
  }
  bool ok = false;
  if (currentIndex() > fieldlist_.size() - 1 || currentIndex() < 0) return false;

  const meteo::field::DataDesc &cur_fielddesc =  fieldlist_.at(currentIndex());
  if (cur_fielddesc.has_id()) {
    std::string findex = cur_fielddesc.id();
    fieldValue fval;
    if (dataProvider()->getFiedValuePoIndex(parent()->curCoord(), findex, &fval)) {
      switch (needed_val_) {
      case fGradX:
        itemData_ = QString::number(fval.dx / 63.71, 'f', 3);
        break;
      case fGradY:
        itemData_ = QString::number(fval.dy / 63.71, 'f', 3);
        break;
      case fRadKriv:
        itemData_ = QString::number(fval.rk, 'f', 2);
        break;
      case fLapl:
        itemData_ = QString::number((fval.dxx + fval.dyy) / 63.71 / 63.71 / 25, 'f', 8);
        break;
      default:
        QString itemDescriptor_ = TMeteoDescriptor::instance()->name( cur_fielddesc.meteodescr() );
        itemData_               = QString::number(fval.value, 'f', checkRoundDigits() );
        break;
      }
      setMarker(GOOD_DATA);
      ok    = true;
      fval_ = fval;
    } else {
      setNoData();
    }
  }
  return ok;
}

void TFFieldItem::needAdvection(const QVariant &atime) {
  if ( 0 != atime.toFloat() ) {
    is_need_advection_ = true;
  } else is_need_advection_ = false;
}

void TFFieldItem::needValue() {
  isNeedValue = true;
}

void TFFieldItem::setMarker(const QColor& mrk) {
  if ( mrk == BAD_DATA ) {
    good_data_ = false;
  } else {
    good_data_ = true;
  }

  // испускаем сигнал только если маркер поменялся
  if ( mrk != marker_ ) {
    marker_ = mrk;
    emit(markerChanged());
  }
}

void TFFieldItem::setFieldPropAdd(const QString & sparam, const QString & typeAdd, const QString &asrok, const QString &alevel, const QString &atypelevel, const QString &atime) {
// debug_log<<typeAdd;
  if (typeAdd.contains("gradx")) {
    needed_val_ = fGradX;
  }
  if (typeAdd.contains("grady")) {
    needed_val_ = fGradY;
  }
  if (typeAdd.contains("radKriv")) {
    needed_val_ = fRadKriv;
  }
  if (typeAdd.contains("lapl")) {
    needed_val_ = fLapl;
  }
  setFieldProp(sparam, asrok, alevel, atypelevel, atime);
}

void TFFieldItem::setFieldProp(const QString & sparam, const QString &ssrok, const QString &slevel, const QString &stypelevel, const QString &stime) {
  int aparam  = TMeteoDescriptor::instance()->descriptor(sparam);
  QString str = QString::number(aparam);
  str         = str.right(5);
  aparam      = str.toInt();
  srok_       = ssrok;
  time_       = stime;
  type_level_ = stypelevel;
  level_     = slevel;
  setDependPoName(stime);
  setDependPoName(ssrok);
  setDependPoName(slevel);
  setDependPoName(stypelevel);
  // int atime   = getValuePoName(stime).toFloat()*3600;
  int asrok      = getValuePoName(ssrok).toFloat() * 3600;
  int alevel     = getValuePoName(slevel).toFloat();
  int atypelevel = getValuePoName(stypelevel).toFloat();


// debug_log<<alevel<<slevel<<getValuePoName(slevel);
// debug_log<<ssrok<<getValuePoName(ssrok);
  needAdvection(getValuePoName(time_));

  fieldProp.clear_level();
  fieldProp.clear_type_level();
  fieldProp.clear_hour();
  fieldProp.clear_meteo_descr();
  fieldProp.add_level(alevel);
  fieldProp.add_type_level(atypelevel);
  //
  // убираем онлиласт потому что запросить данные за прошедший срок становится невозможно
  //
  // fieldProp.set_only_last(true);
  //fieldProp.set_only_best(true);
  // fieldProp.add_hour(asrok);
  fieldProp.add_meteo_descr(aparam);
  //


  if (dataProvider()) {
      fieldProp.set_forecast_start(dataProvider()->getDate().addSecs(asrok).toString(Qt::ISODate).toStdString());
      fieldProp.set_forecast_end(dataProvider()->getDate().addSecs(asrok).toString(Qt::ISODate).toStdString());
  }

  // error_log<<fieldProp.DebugString();
}

QString TFFieldItem::getItemInfo() {
  QString lvs;
  if (currentIndex() > fieldlist_.size() - 1 || currentIndex() < 0) return "";

  // for (int i =0; i <fieldlist_.count();++i ){
  //   error_log<<fieldlist_.at(i).date()<<fieldlist_.at(i).dt1()<<fieldlist_.at(i).dt2()<<fieldlist_.at(i).hour();
  // }


  const meteo::field::DataDesc &d =  fieldlist_.at(currentIndex());
  if (d.level() > 0 ) {
    lvs = QObject::tr("Данные на уровне ") + QString::number(d.level()) + QObject::tr("гПа\n");
  } else {
    lvs = QObject::tr("Приземные данные\n");

  }
  lvs += QObject::tr("центр ") + pbtools::toQString(d.center_name()) + "\n";
  lvs += QObject::tr("срок ") + pbtools::toQString(d.date()) + " ";
  if (d.hour() > 0) {
    lvs += QObject::tr("прогноз на ") + QString::number(d.hour() / 3600) + QObject::tr("ч\n");
  } else {
    lvs += QObject::tr("анализ \n");
  }
  if (curCoord().isValid()) {
    lvs += QObject::tr("Координаты: ") + curCoord().toString();
  }

  return lvs;
}

int TFFieldItem::currentIndex( )
{
  return cur_field;
}


void TFFieldItem::setCurrentIndex( int index)
{
  cur_field = index;
}

/**
 * Получаем данные для комбобокса в котором доступные прогнозы сидят
 * @return [description]
 */
QStringList TFFieldItem::listData( )
{
  QStringList vs;
  for (int i = 0; i < fieldlist_.count(); ++i ) {
    int ind = vs.count();

    //  if(fieldlist_.values().value(i).has_priority() && fieldlist_.values().value(i).priority() > -1){
//     ind = fieldlist_.values().value(i).priority()-1;
//   }
    vs.insert(ind , getComboText(fieldlist_.at(i)));
  }
  return vs;
}

QVariant TFFieldItem::getAddItemData() {
  if (currentIndex() > listData( ).size() - 1 || currentIndex() < 0) return QVariant();

  if (listData( ).count() != 0) {
    return listData( ).at(currentIndex());
  }
  return QVariant();
}

/**
 * Получаем текст для комбо-айтема (там, где прогнозы вывалены)
 * @param  d [description]
 * @return   [description]
 */
QString TFFieldItem::getComboText(const meteo::field::DataDesc &d) {

    QDateTime dt = QDateTime::fromString(pbtools::toQString(d.date()), "yyyy-MM-ddThh:mm:ss");
    int srok = 0;
    if (d.has_hour() && 0 < d.hour()) {
      srok = (d.hour() / 3600);
    }
    QString center = pbtools::toQString(d.center_name());
    FMetaData md;
    md.level = d.level();
    md.station.name = center;
    md.srok =srok;
    md.dt = dt;
    return TFItem::getComboText(md);
}

void TFFieldItem::appendChild(TFItem *item)
{
  item->setDataSourceType(near_station_src);
  item->setInAdvection(is_need_advection_);

  TFItem::appendChild(item);
}

