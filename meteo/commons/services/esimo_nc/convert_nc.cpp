#include "convert_nc.h"
#include "esimoparam.h"

#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <meteo/commons/primarydb/ancdb.h>

#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/tprototext.h>
#include <commons/obanal/func_obanal.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/landmask/landmask.h>

#include <qdir.h>
#include <qimage.h>

#define kConfFile MnCommon::etcPath() + "esimo.conf"

using namespace meteo;


//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1292", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //координаты станций наблюдения за загрязнением
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1737", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //координаты портов
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1759", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //позвыной,координаты станций рассылающих ШТОРМ
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2680", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //коды стран и георайонов из ОЯ

//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1577", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //словарь типов кодов ЕСИМО
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2320", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //рубрикатор, таблица esimo_elem
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2409", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //параметры, таблица esimo_elem
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2860", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //критические значения параметров, по коду
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1718", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //инфо, оценка нац безопасности в морской деят



//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1081", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //климат (без срока)     
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1291", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //климат (без срока)
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2170", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //климат, сетка по участку. давление (мин, макс, ср)
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2358", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //климат, скорость ветра, месяц, береговыые
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_2363", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //климат, высота волн, месяц, береговыые

//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1334", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //активность экстремальных циклонов за месяц, Балтийское море (год, месяц, кол-во, интенс, повторямость)
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1336", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //активность экстремальных циклонов за месяц, Атлантика
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1339", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //опасность в океанах по сезонам (мин, макс значения по коду параметра, сезону и квадрату координат)



//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1659", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsUnk)) //GRIB, P0496_00 (22073), волнение, высота


//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1300", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsMeteo)) //КП-68
//   << QPair<QString, EsimoType>("RU_RIHMI-WDC_1303", EsimoType(meteo::surf::kCheckDataType, ConvertNc::kEsMeteo))  //КП-68





//-------
//группа SpSpspsp в FM12 (без дескриптора)
// "P0977_00"
// "P0978_00"
// "P0980_00"
// "P0981_00"
// "P0982_00"


ConvertNc::ConvertNc()
{
  _codec = QTextCodec::codecForName("Windows-1251");
  _ep = new EsimoParam;
  
  meteo::ancdb()->loadEsimo();
}

ConvertNc::~ConvertNc()
{
  
}


bool ConvertNc::esimoid2datatype(const QString esimoId, meteo::surf::DataType *datatype, meteo::EsimoType *esimonc, NetType* nettype)
{
  //чтение конфига
  if (_conf.isEmpty()) {
    QFile file(kConfFile);
    if ( !file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(kConfFile);
      return false;
    }

    QString text = QString::fromUtf8(file.readAll());
    file.close();
    meteo::EsimoConf esconf;
    if ( !TProtoText::fillProto(text, &esconf) ) {
      error_log << QObject::tr("Ошибка в структуре файла конфигурации");
      return false;
    }

    for (const auto &esimo : esconf.esimo()) {
      _conf.insert(QString::fromStdString(esimo.id()), esimo);
    }
    
  }

  if (_conf.contains(esimoId)) {
    *datatype = _conf.value(esimoId).datatype();
    *esimonc = _conf.value(esimoId).estype();
    *nettype = NO_TYPE;
    if (_conf.value(esimoId).has_net_type()) {
      *nettype = NetType(_conf.value(esimoId).net_type());
    }
    return true;
  }

  return false;
}

descr_t ConvertNc::getDescriptor(const QString& esimoName)
{
  // if (name == "M4325") {
  if (esimoName == "M4401") {
    return 4;
  }
  return meteo::ancdb()->esimoDescr(esimoName);
}

//! Чтение размерности
/*!
  \param ncid - id файла
  \param paramId - id параметра
  \param size - общее число элементов (произведение всех размерностей)
  \param dims - размерности
*/
bool ConvertNc::readParamDimension(int ncid, int paramId, size_t* size, QList<size_t>* dims)
{
  if (nullptr == size || nullptr == dims) {
    return false;
  }
  
  int dimCnt;
  int status = nc_inq_varndims(ncid, paramId, &dimCnt);
  if (status != 0) {
    error_log << QObject::tr("Ошибка чтения размера");
    return false;
  }
  
  QVector<int> dimIds(dimCnt);
  status = nc_inq_vardimid(ncid, paramId, dimIds.data());
  if (status != 0) {
    error_log << QObject::tr("Ошибка чтения размера");
    return false;
  }

  *size = 1;
  for (int idDim = 0; idDim < dimCnt; idDim++) {
    size_t length;
    status = nc_inq_dimlen(ncid, dimIds[idDim], &length);
    if (status != 0) {
      error_log << QObject::tr("Ошибка чтения размера");
      return false;
    }
    dims->append(length);
    *size *= length;
  }
  
  return true;
}

//! Чтение данных
/*!
  \param ncid - id файла
  \param paramId - id параметра
  \param size - общее число элементов (произведение всех размерностей)
  \param dims - размерности
*/
bool ConvertNc::readParamData(int ncid, int paramId, int size, const QString& name, QVector<float>* data)
{
  if (nullptr == data) {
    return false;
  }
  
  nc_type type;
  int status = nc_inq_vartype(ncid, paramId, &type);
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения типа данных") << name << paramId;
    return false;
  }

  if (type == NC_NAT || type >= NC_STRING) {
    error_log << QObject::tr("Чтение типа данных %1 не реализовано").arg(type);
    return false;
  }

  data->resize(size);
  status = nc_get_var_float(ncid, paramId, data->data());
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения данных") << name;
    return false;
  }

  return true;
}


//! Чтение ascii данных (размерность esimo - код)
/*!
  \param ncid - id файла
  \param paramId - id параметра
  \param size - общее число элементов (произведение всех размерностей)
  \param dims - размерности
*/
bool ConvertNc::readParamCodeData(int ncid, int paramId, const QString& name, size_t rows, size_t length, QStringList* data)
{
  if (nullptr == data) {
    return false;
  }
  
  nc_type type;
  int status = nc_inq_vartype(ncid, paramId, &type);
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения типа данных") << name << paramId;
    return false;
  }

  if (type != NC_BYTE) {
    error_log << QObject::tr("Чтение типа данных %1 не реализовано").arg(type);
    return false;
  }

  
  QByteArray bytes(length*rows, '0');
  status = nc_get_var(ncid, paramId, bytes.data());
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения данных") << name;
    return false;
  }

  size_t start = 0;
  while (start + length <= (size_t)bytes.size()) {
    data->append(_codec->toUnicode(bytes.mid(start, length).replace(char(-127), ' ').simplified()));
    start += length;
  }

  return true;
}

//! Чтение ascii данных (размерность esimo - код) во float
/*!
  \param ncid - id файла
  \param paramId - id параметра
  \param size - общее число элементов (произведение всех размерностей)
  \param dims - размерности
*/
bool ConvertNc::readParamCodeData(int ncid, int paramId, const QString& name, size_t rows, size_t length, QVector<float>* data)
{
  if (nullptr == data) {
    return false;
  }
  
  nc_type type;
  int status = nc_inq_vartype(ncid, paramId, &type);
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения типа данных") << name << paramId;
    return false;
  }

  if (type != NC_BYTE) {
    error_log << QObject::tr("Чтение типа данных %1 не реализовано").arg(type);
    return false;
  }

  
  QByteArray bytes(length*rows, '0');
  status = nc_get_var(ncid, paramId, bytes.data());
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения данных") << name;
    return false;
  }

  bool ok = false;
  size_t start = 0;
  while (start + length <= (size_t)bytes.size()) {
    float one = bytes.mid(start, length).replace(char(-127), ' ').simplified().toFloat(&ok);
    if (!ok) {
      error_log << QObject::tr("Ошибка преобразования данных в числовой формат");
      break;
    }
    data->append(one);
    start += length;
  }

  return ok;
}

//---



//! Усвоение папки с ответом
//path - папка с файлами ответа (набор nc)
//datatype - тип данных (для метеодаты)
//esimonc - тип esimo данных (метео, сетка и пр.)
bool ConvertNc::assimilateDir(const QDir& diresimo, meteo::surf::DataType datatype, meteo::EsimoType esimonc, NetType net_type)
{
  bool ok = false;
  
  QStringList namefilter;
  namefilter << "*.nc";
  QStringList filelist = diresimo.entryList(namefilter, QDir::Files);
  for (auto file : qAsConst(filelist)) {
    debug_log << QObject::tr("Обработка файла %1").arg(file);
    switch (esimonc) {
      case meteo::kEsMeteo:
        //метеодата
        ok |= assimilateFile(diresimo.absolutePath() +  "/" + file, datatype);
      break;
      case meteo::kEsGrid:
        //поля
        ok |= assimilateGridFile(diresimo.absolutePath() +  "/" + file, net_type);
      break;
      default:
      break;
    }
    
    //var(ok);
  }

  return ok;
}

bool ConvertNc::assimilateFile(const QString& ncFile, int datatype)
{
  TMeteoData md;
  
  md.set(TMeteoDescriptor::instance()->descriptor("level_type"),
         QString::number(meteodescr::kSurfaceLevel),
         meteodescr::kSurfaceLevel, control::RIGHT);
  md.set(TMeteoDescriptor::instance()->descriptor("category"),
         QString::number(datatype),
         datatype, control::RIGHT);
  
  bool ok = convert(ncFile, &md);
  if (!ok) {
    return false;
  }

  int categ = datatype;
  
  for (int idx = 0; idx < md.childsCount(); idx++) {
    TMeteoData* child = md.child(idx);
    //child->printData();
    
    meteo::StationInfo info;
    ok = meteo::ancdb()->fillStationInfo(child, categ, &info);
    if (!ok) {
      error_log << QObject::tr("Ошибка заполнения информации о станции");
      continue;
    }
    
    QDateTime dt = TMeteoDescriptor::instance()->dateTime(*child);
    ok = meteo::ancdb()->saveReport(*child, categ, dt, info, "esimo");
    //debug_log << "saved type=" << categ << dt;
  }

  return ok;
}


bool ConvertNc::convert(const QString& ncFile, TMeteoData* md)
{
  if (nullptr == md) return false;
  
  int status = 0;
  int ncid = 0;

  if ((status = nc_open(ncFile.toStdString().data(), NC_NOWRITE, &ncid))) {
    error_log <<  QObject::tr("Ошибка открытия файла") << ncFile;
    return false;
  }
  
  int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  
  if (0 != (status = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in,
                            &unlimdimid_in))) {
    error_log << QObject::tr("Ошибка чтения атрибутов файла") << ncFile;;
    return false;
  }
  // debug_log << "dims" << ndims_in  << "vars" << nvars_in
  // 	    << "attr" << ngatts_in << "ulim" << unlimdimid_in;


  bool ok = fillTime(ncid, md);
  if (!ok) {
    return false;
  }
  
  for (int id = 0; id < nvars_in; id++) {
    QString sname;
    descr_t descr;
    size_t size = 0;
    QList<size_t> dims;
    nc_type type;
    
    ok = getParamAttrs(ncid, id, &sname, &descr, &size, &dims, &type);
    if (!ok) {
      continue;
    }

    if (type == NC_STRING || type == NC_BYTE) {
      fillStrMeteoData(ncid, id, sname, descr, dims, md);
    } else {
      fillNumMeteoData(ncid, id, sname, descr, size, md);
    }
    
  }

  /* Close the file, freeing all resources. */
  if ((status = nc_close(ncid))) {
    error_log << QObject::tr("Ошибка закрытия файла") << ncFile;
    return false;
  }

  return true;
}

//! получение атрибутов метеопараметра (название есимо, дескриптор, размер, тип)
bool ConvertNc::getParamAttrs(int ncid, int id, QString* sname, descr_t* descr, size_t* size, QList<size_t>* dims, nc_type* type)
{
  char name[NC_MAX_NAME];
  int status = nc_inq_varname(ncid, id, name);
  if (status != NC_NOERR) {
    return false;
  }

  *sname = QString(name);

  if (*sname == "M4400" ||
      //	sname == "M4401" ||
      *sname == "series") { //TODO
    return false;
  }

  *descr = getDescriptor(*sname);
  if (*descr == BAD_DESCRIPTOR_NUM) {
    // warning_log << QObject::tr("Неизвестный параметр") << *sname;
    //*descr = 4;
    return false;
  }

  //debug_log << *sname << *descr;

  bool ok = readParamDimension(ncid, id, size, dims);
  // var(size);
  if (!ok) {
    return false;
  }

  status = nc_inq_vartype(ncid, id, type);
  if (0 != status) {
    error_log << QObject::tr("Ошибка чтения типа данных") << *sname << id;
    return false;
  }

  if (*type == NC_NAT || *type > NC_STRING) {
    error_log << QObject::tr("Чтение типа данных %1 не реализовано").arg(*type);
    return false;
  }

  return true;
}

//заполнение строковых данных в TMeteoData из файла (чайлды уже созданы для всего массива) 
void ConvertNc::fillStrMeteoData(int ncid, int id, const QString& sname, descr_t descr, const QList<size_t>& dims, TMeteoData* md)
{
  if (nullptr == md || dims.size() != 2) {
    return;
  }
  
  QStringList str;
  bool ok = readParamCodeData(ncid, id, sname, dims.at(0), dims.at(1), &str);
  //var(str);

  if (md->childsCount() != str.size()) {
    error_log << QObject::tr("Размер данных %1(%2) = %3 не совпадает с размером временного ряда = %4")
                 .arg(sname).arg(descr)
                 .arg(str.size())
                 .arg(md->childsCount());
    return;
  }

  for (int idx = 0; idx < str.size(); idx++) {
    TMeteoData* ch = md->child(idx);
    if (nullptr == ch) {
      return;
    }

    float val = str.at(idx).toFloat(&ok);
    if (!ok) {
      val = BAD_METEO_ELEMENT_VAL;
    }
    
    ch->add(descr, str.at(idx), val, control::NO_CONTROL);
  }
}


//заполнение числовых данных в TMeteoData из файла (чайлды уже созданы для всего массива) 
void ConvertNc::fillNumMeteoData(int ncid, int id, const QString& sname, descr_t descr, size_t size, TMeteoData* md)
{
  if (nullptr == md) {
    return;
  }

  QVector<float> data;
  bool ok = readParamData(ncid, id, size, sname, &data);
  if (!ok) {
    return;
  }
  //var(data);
  

  if (md->childsCount() != data.size()) {
    error_log << QObject::tr("Размер данных %1(%2) = %3 не совпадает с размером временного ряда = %4")
                 .arg(sname).arg(descr)
                 .arg(data.size()).arg(md->childsCount());
    return;
  }
  
  for (int idx = 0; idx < data.size(); idx++) {
    TMeteoData* ch = md->child(idx);
    if (nullptr == ch) {
      return;
    }

    float val = data.at(idx);
    if (val > 9999 || val < -9999) { //999 в RU_Hydrometcentre_68
      //TODO потом убрать запись пустых. пока для отладки
      //val = BAD_METEO_ELEMENT_VAL;
      //ch->add(descr, "", val, control::MISTAKEN);
    } else {

      _ep->fillMeteo(sname, descr, val, ch);
      //ch->add(descr, "", val, qual);
    }
  }
}


bool ConvertNc::fillTime(int ncid, TMeteoData* data)
{
  int status = 0;
  
  std::string nameDt = "M4400"; //дата/время наблюдения
  int id;

  if (0 != (status = nc_inq_varid(ncid, nameDt.data(), &id))) {
    // var(status);
    nameDt = "M4401"; //дата/время начала наблюдения (используется в серии наблюдений)
    if (0 != (status = nc_inq_varid(ncid, nameDt.data(), &id))) {
      // var(status);
      bool ok = fillFragmentedTime(ncid, data);
      if (!ok) {
        error_log << QObject::tr("Ошибка: нет данных о сроке");
      }
      return ok;
    }
  }

  //чтение размеров переменных
  size_t size = 0;
  QList<size_t> dims;
  bool ok = readParamDimension(ncid, id, &size, &dims);
  if (!ok) {
    return false;
  }
  
  QStringList dt;
  ok = readParamCodeData(ncid, id, QString::fromStdString(nameDt), dims.at(0), dims.at(1), &dt);
  //var(dt.size());

  if (dt.size() == 0) {
    error_log << QObject::tr("Ошибка: нет данных о сроке");
    return false;
  }
  

  for (int idx = 0; idx < dt.size(); idx++) {
    TMeteoData& child = data->addChild();
    child.setDateTime(QDateTime::fromString(dt.at(idx), Qt::ISODate));
  }

  //var(data->childsCount());
  
  return true;
}

//! Чтение даты/времени, разбитых на дескрипторы
bool ConvertNc::fillFragmentedTime(int ncid, TMeteoData* data)
{
  int status = 0;
  QList<std::string> names;
  names << "M4404" << "M4405" << "M4406" << "M4407";
  QList<QVector<float>> fragDt;
  
  size_t checkSize = 0;
  for (auto name : names) {
    // var(name);
    int id;
    if (0 != (status = nc_inq_varid(ncid, name.data(), &id))) {
      return false;
    }
    
    //чтение размеров переменных
    size_t size = 0;
    QList<size_t> dims;
    bool ok = readParamDimension(ncid, id, &size, &dims);
    if (!ok) {
      return false;
    }

    if (checkSize == 0) {
      checkSize = size;
    } else if (checkSize != size) {
      error_log << QObject::tr("Несовпадение размеров при построении массива дат");
      return false;
    }
    
    QVector<float> data;
    ok = readParamData(ncid, id, size, QString::fromStdString(name), &data);
    if (!ok) {
      return false;
    }
    
    fragDt.append(data);
  }

  for (int idx = 0; idx < fragDt.at(0).size(); idx++) {
    TMeteoData& child = data->addChild();
    QDateTime dt(QDate(fragDt.at(0).at(idx), fragDt.at(1).at(idx), fragDt.at(2).at(idx)),
                 QTime(fragDt.at(3).at(idx), 0));
    child.setDateTime(dt);
  }

  //var(data->childsCount());
  
  return true;
}



//---

//усвоение полей



bool ConvertNc::readOneTime(int ncid, QString* dt, QString* dtstart)
{
  int status = 0;

  size_t size = 0;
  QList<size_t> dims;
  std::string nameDt = "M4400"; //дата/время наблюдения
  int id;

  if (0 != (status = nc_inq_varid(ncid, nameDt.data(), &id))) {
    error_log << QObject::tr("Ошибка: нет данных о сроке");
    return false;
  }

  bool ok = readParamDimension(ncid, id, &size, &dims);
  if (!ok) {
    return false;
  }
  
  QStringList dtlist; //TODO  может все ж можно одно считать
  ok = readParamCodeData(ncid, id, QString::fromStdString(nameDt), dims.at(0), dims.at(1), &dtlist);

  if (dtlist.size() == 0) {
    error_log << QObject::tr("Ошибка: нет данных о сроке");
    return false;
  }
  *dt = dtlist.at(1);


  nameDt = "M4401"; //дата/время начала наблюдения (используется в серии наблюдений)
  if (0 != (status = nc_inq_varid(ncid, nameDt.data(), &id))) {
    error_log << QObject::tr("Ошибка: нет данных о сроке");
    return false;
  }

  dtlist.clear();
  ok = readParamCodeData(ncid, id, QString::fromStdString(nameDt), dims.at(0), dims.at(1), &dtlist);
  //  var(dtlist.size());

  
  if (dtlist.size() == 0) {
    error_log << QObject::tr("Ошибка: нет данных о сроке");
    return false;
  }
  *dtstart = dtlist.at(1);
  
  return true;
}


bool ConvertNc::readGridCoords(int ncid, QVector<float>* lat, QVector<float>* lon)
{
  int status = 0;
  int id;
  size_t size = 0;
  QList<size_t> dims;

  //широта
  std::string name = "M4311";
  if (0 != (status = nc_inq_varid(ncid, name.data(), &id))) {
    error_log << QObject::tr("Ошибка: нет данных о координатах");
    return false;
  }
  
  if (! readParamDimension(ncid, id, &size, &dims)) {
    return false;
  }
  
  if (! readParamData(ncid, id, size, QString::fromStdString(name), lat)) {
    return false;
  }

  
  //долгота
  name = "M4312";
  if (0 != (status = nc_inq_varid(ncid, name.data(), &id))) {
    error_log << QObject::tr("Ошибка: нет данных о координатах");
    return false;
  }
  
  if (! readParamDimension(ncid, id, &size, &dims)) {
    return false;
  }
  
  if (! readParamData(ncid, id, size, QString::fromStdString(name), lon)) {
    return false;
  }
  
  return true;
}

//---

bool ConvertNc::assimilateGridFile(const QString& ncFile, NetType net_type)
{
  int status = 0;
  int ncid = 0;
  
  if ((status = nc_open(ncFile.toStdString().data(), NC_NOWRITE, &ncid))) {
    error_log <<  QObject::tr("Ошибка открытия файла") << ncFile;
    return false;
  }
  
  int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  
  if (0 != (status = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in,
                            &unlimdimid_in))) {
    error_log << QObject::tr("Ошибка чтения атрибутов файла") << ncFile;;
  }
  // debug_log << "dims" << ndims_in  << "vars" << nvars_in
  // 	    << "attr" << ngatts_in << "ulim" << unlimdimid_in;
  
  QString dt, dtstart;
  bool ok = readOneTime(ncid, &dt, &dtstart);
  if (!ok) {
    return false;
  }
  
  QVector<float> lat;
  QVector<float> lon;

  ok = readGridCoords(ncid, &lat, &lon);

  int hour = QDateTime::fromString(dtstart, Qt::ISODate).secsTo(QDateTime::fromString(dt, Qt::ISODate));

  
  meteo::field::DataDesc fdescr;
  fdescr.set_date(dtstart.toStdString());
  fdescr.set_center(250);
  fdescr.set_level(0);
  fdescr.set_hour(hour);
  fdescr.set_level_type(1);
  // fdescr.set_meteodescr();
  // fdescr.set_net_type();
  fdescr.set_model(0);
  fdescr.set_dt1(dt.toStdString());
  fdescr.set_dt2(dt.toStdString());
  fdescr.set_time_range(0);
  
  // fdescr.set_date(afdescr->date());
  // fdescr.set_center(afdescr->center());
  // fdescr.set_level(15000);
  // fdescr.set_hour(afdescr->hour());
  // fdescr.set_level_type(15000);
  // fdescr.set_meteodescr(10009);
  // fdescr.set_net_type(f.typeNet());
  // fdescr.set_model(afdescr->model());
  // fdescr.set_dt1(afdescr->dt1());
  // fdescr.set_dt2(afdescr->dt2());
  // fdescr.set_time_range(afdescr->time_range());

  ok = assimilateFields(ncid, nvars_in, net_type, lat, lon, &fdescr);
  

  if ((status = nc_close(ncid))) {
    error_log << QObject::tr("Ошибка закрытия файла") << ncFile;
    return false;
  }

  return ok;
}

bool ConvertNc::assimilateFields(int ncid, int nvars_in, NetType net_type,
																 const QVector<float>& lat, const QVector<float>& lon,
																 meteo::field::DataDesc* fdescr)
{
  int status = 0;
  
  for (int id = 0; id < nvars_in; id++) {
    char name[NC_MAX_NAME];
    status = nc_inq_varname(ncid, id, name);
    if (status != NC_NOERR) {
      continue;
    }

    QString sname(name);

    descr_t descr = getDescriptor(sname);
    if (descr == BAD_DESCRIPTOR_NUM ||
        TMeteoDescriptor::instance()->isIdentDescr(descr)) {
      continue;
    }

    //debug_log << sname << descr;

    //    if (descr != 12101 && descr != 13003) continue;//TODO временно
    
    size_t size = 0;
    QList<size_t> dims;
    bool ok = readParamDimension(ncid, id, &size, &dims);
    // var(size);
    if (!ok) {
      continue;
    }

    nc_type type;
    int status = nc_inq_vartype(ncid, id, &type);
    if (0 != status) {
      error_log << QObject::tr("Ошибка чтения типа данных") << name << id;
      return false;
    }

    if (type == NC_NAT || type > NC_STRING) {
      error_log << QObject::tr("Чтение типа данных %1 не реализовано").arg(type);
      continue;
    }

    QVector<float> data;
    //    QStringList str;
    
    if (type == NC_STRING || type == NC_BYTE) {
      ok = readParamCodeData(ncid, id, sname, dims.at(0), dims.at(1), &data);
    } else {
      ok = readParamData(ncid, id, size, sname, &data);
    }
    if (!ok) {
      continue;
    }

    if (lat.size() != data.size()) {
      error_log << QObject::tr("Размер данных %1(%2) = %3 не совпадает с размером координат = %4")
                   .arg(sname).arg(descr)
                   .arg(data.size()).arg(lat.size());
      continue;
    }

    float step = lon.at(1) - lon.at(0); //NOTE: считаем, что регулярная
    NetType fileNetType = obanal::netTypeFromStep(step*1000, &ok);

    //debug_log << "net_type" << net_type << "file net" << fileNetType << "near" << obanal::nearNetTypeFromStep(step*1000) << step;
    
    if (net_type == NO_TYPE) {
      net_type = obanal::nearNetTypeFromStep(step*1000);
    }



    fdescr->set_meteodescr(descr);
    fdescr->set_net_type(net_type);

    // debug_log << "lat" << lat;
    // debug_log << "lon" << lon;
    
    if (true == ok && net_type == fileNetType) {
      ok = saveField(fdescr, lat, lon, data); //чтоб не интерполировать, если в файле сетка совпадает с нашей стандартной
    } else {
      ok = saveInterpolField(fdescr, lat, lon, data);
    }
  }

  return true;
}

bool cmpLon(const float &a, const float &b)
{
  return MnMath::M180To180(a) < MnMath::M180To180(b);
}

bool ConvertNc::saveField(meteo::field::DataDesc* fdescr, const QVector<float>& lat, const QVector<float>& lon, const QVector<float>& data)
{
  //var(fdescr->Utf8DebugString());
  //debug_log << fdescr->date() << fdescr->meteodescr();
  
  ::obanal::TField field;
  field.setHour(fdescr->hour());
  field.setModel(fdescr->model());
  field.setCenter(fdescr->center());
  field.setDate(QDateTime::fromString(QString::fromStdString(fdescr->date()), Qt::ISODate));
  // field.setCenterName(QString::fromStdString(fdescr->center_name()));
  //field.setLevelTypeName( QString::fromStdString( fdescr->level_type_name() ) );
  field.setDescr(fdescr->meteodescr());
  field.setLevel(fdescr->level());
  field.setLevelType(fdescr->level_type());

  //float step = lon.at(1) - lon.at(0); //NOTE: считаем, что регулярная
  //debug_log << "step net" << step;
  
  //RegionParam corners(lat.first(), lon.first(), lat.last(), lon.last());
  // RegionParam corners(*std::min_element(lat.begin(), lat.end()),
  // 		      *std::min_element(lon.begin(), lon.end(), cmpLon),
  // 		      *std::max_element(lat.begin(), lat.end()), //+ step,
  // 		      *std::max_element(lon.begin(), lon.end(), cmpLon) //+ 0.1 //step
  // 		      );
  // debug_log << "corners" << corners.start << corners.end;
  
  // field.setNet(corners, step*M_PI/180, step*M_PI/180);

  QVector<RegionParam> corners;
  if (!addCorners(corners, lat, lon)) {
    return false;
  }
  field.setNet(corners, static_cast<NetType>(fdescr->net_type()));

  

  int cnt = 0, cnt1= 0;
  int cnt2 = 0;
  for (int idx = 0; idx < data.size(); idx++) {
    int numFi, numLa;
    bool ok = field.getNumPoFila(GeoPoint::fromDegree(lat[idx], lon[idx]), &numFi, &numLa);
    //if (mdata.data > 9999 || mdata.data < -999) {
    // if (fdescr->meteodescr() == 12101) {
    //   debug_log << data[idx] << numFi << numLa  << lat[idx] << lon[idx] << ok;
    // }
    //}
    if (ok) {
      if (data[idx] > 9999 || data[idx] < -9999) {
        field.setData(numFi, numLa, data[idx], FALSE_DATA);
        cnt2++;
      } else {
        field.setData(numFi, numLa, data[idx], TRUE_DATA);
        cnt1++;
      }
      cnt1++;
    } else {
      cnt2++;
    }
    cnt++;
  }

  // var(cnt);
  // var(cnt1);
  // var(cnt2);
  // var(field.getData(0));
  // debug_log << "kol" << field.kolFi() << field.kolLa();
  
  // if(land_sea_mask_need_.contains(fdescr->meteodescr())) {
  // {
  // for(int i = 0; i < field.kolFi(); ++i) {
  //   for(int j = 0; j < field.kolLa(); ++j) {
  // 	int is_land = meteo::map::LandMask::instance()->get(MnMath::rad2deg(field.netFi(i)), MnMath::rad2deg(field.netLa(j)));
  // 	if (is_land) {
  // 	  //field.setMasks(field.num(i, j), !(is_land == 1)); // TODO если океан - то другая маска!
  // 	  field.setMasks(field.num(i, j), false);
  // 	}
  //   }
  // }
  // }
  
  
  // int ret_val =  MnObanal::prepInterpolOrder(gdata, &field);
  // var(ret_val);

  //field.setNet(field.getRegPar(), netType);
  //field.setNetMask(field.getRegPar());
  // QStringList badData;
  // int ret_val = MnObanal::prepInterpolHaos(gdata, &field, -1, &badData);
  // var(badData.count());

  //field.setSrcPointCount(cnt1);
  field.setSrcPointCount(field.kolFi() * field.kolLa()); //TODO
  
  fdescr->set_count_point(field.getSrcPointCount());
  //  var(field.getSrcPointCount());
  
  
  // if(ERR_OBANAL == ret_val) {
  //   error_log << QObject::tr("ошибка интерполяции");
  //   return false;
  // }


  TObanalDb db;
  QString path = QDir::homePath() + "/obanal";
  db.setPath(path);
  bool ok = db.saveField(field, *fdescr);

  return ok;
}


bool ConvertNc::saveInterpolField(meteo::field::DataDesc* fdescr, const QVector<float>& lat, const QVector<float>& lon, const QVector<float>& data)
{
  //var(fdescr->Utf8DebugString());
  //debug_log << fdescr->date() << fdescr->meteodescr();
  
  ::obanal::TField field;
  field.setHour(fdescr->hour());
  field.setModel(fdescr->model());
  field.setCenter(fdescr->center());
  field.setDate(QDateTime::fromString(QString::fromStdString(fdescr->date()), Qt::ISODate));
  // field.setCenterName(QString::fromStdString(fdescr->center_name()));
  //field.setLevelTypeName( QString::fromStdString( fdescr->level_type_name() ) );
  field.setDescr(fdescr->meteodescr());
  field.setLevel(fdescr->level());
  field.setLevelType(fdescr->level_type());
  
  QVector<RegionParam> corners;
  if (!addCorners(corners, lat, lon)) {
    return false;
  }
  field.setNet(corners, static_cast<NetType>(fdescr->net_type()));
  
  GeoData gdata(data.size());

  bool okdata = false;
  for (int idx = 0; idx < data.size(); idx++) {
    if (data[idx] > 9999 || data[idx] < -9999) {
      gdata[idx] = meteo::MData(MnMath::deg2rad(lat[idx]), MnMath::deg2rad(lon[idx]), data[idx], FALSE_DATA);
    } else {
      gdata[idx] = meteo::MData(MnMath::deg2rad(lat[idx]), MnMath::deg2rad(lon[idx]), data[idx], TRUE_DATA);
      okdata = true;
    }
    // debug_log << idx << lat[idx] << lon[idx] << data[idx];

  }
  //var(okdata);
  if (false == okdata) {
    info_log << QObject::tr("Нет данных");
    return false;
  }

  //var(gdata.size());
  
  // if(land_sea_mask_need_.contains(fdescr->meteodescr())) {
  // {
  // for(int i = 0; i < field.kolFi(); ++i) {
  //   for(int j = 0; j < field.kolLa(); ++j) {
  // 	int is_land = meteo::map::LandMask::instance()->get(MnMath::rad2deg(field.netFi(i)), MnMath::rad2deg(field.netLa(j)));
  // 	if (is_land) {
  // 	  //field.setMasks(field.num(i, j), !(is_land == 1)); // TODO если океан - то другая маска!
  // 	  field.setMasks(field.num(i, j), false);
  // 	}
  //   }
  // }
  // }
  
  int ret_val =  MnObanal::prepInterpolOrder(&gdata, &field);
  //var(ret_val);

  //field.setNet(field.getRegPar(), netType);
  //field.setNetMask(field.getRegPar());
  // QStringList badData;
  // int ret_val = MnObanal::prepInterpolHaos(gdata, &field, -1, &badData);
  // var(badData.count());

  //field.setSrcPointCount(cnt1);
  field.setSrcPointCount(field.kolFi() * field.kolLa()); //TODO
  
  fdescr->set_count_point(field.getSrcPointCount());
  //var(field.getSrcPointCount());
  
  
  if(ERR_OBANAL == ret_val) {
    error_log << QObject::tr("Ошибка интерполяции");
    return false;
  }


  TObanalDb db;
  QString path = QDir::homePath() + "/obanal";
  db.setPath(path);
  bool ok = db.saveField(field, *fdescr);

  return ok;
}



//! добавляем координаты углов сетки
bool ConvertNc::addCorners(QVector<RegionParam>& corners, const QVector<float>& lat, const QVector<float>& lon)
{
  float latFirst = *std::min_element(lat.begin(), lat.end());
  float lonFirst = *std::min_element(lon.begin(), lon.end(), cmpLon);
  float latLast  = *std::max_element(lat.begin(), lat.end());
  float lonLast  = *std::max_element(lon.begin(), lon.end(), cmpLon);
  bool  lonAdd   = true;
  bool  latAdd   = true;

  if (MnMath::isEqual(latFirst, latLast) || MnMath::isEqual(lonFirst, lonLast)) {
    error_log << QObject::tr("Ошибка структуры сетки");
    return false;
  }
  
  RegionParam region;
  if (lonAdd) {
    region.start.setLaDeg(MnMath::M180To180(lonFirst));
    region.end.setLaDeg  (MnMath::M180To180(lonLast));
  }
  else {
    region.start.setLaDeg(MnMath::M180To180(lonLast));
    region.end.setLaDeg  (MnMath::M180To180(lonFirst));
  }

  if (latAdd) {
    region.start.setFiDeg(latFirst);
    region.end.setFiDeg(latLast);
  }
  else {
    region.start.setFiDeg(latLast);
    region.end.setFiDeg(latFirst);
  }

  //debug_log << region.start.toString() << region.end.toString();

  if (region.start.laDeg() > region.end.laDeg())
    // || (region.start.laDeg() < 0. && region.end.laDeg() > 0.))
  {
    RegionParam region1 = region;
    region.end.setLaDeg   (180.0);
    region1.start.setLaDeg(-180.0);
    corners.append(region);
    //debug_log << corners.last().start.toString() << corners.last().end.toString();
    corners.append(region1);
  }
  else {
    corners.append(region);
  }
  //debug_log << corners.size() << corners.last().start.toString() << corners.last().end.toString();

  return true;
}


// bool ConvertNc::convert(const QString& ncFile, ::obanal::TField* field)
// {
//   if (nullptr == field) return false;

//   int status = 0;
//   int ncid;

//   var("open");
//   if ((status = nc_open(ncFile.toStdString().data(), NC_NOWRITE, &ncid))) {
//     debug_log << "err";
//     return false;
//   }


//   var("get info");
//   /* We will learn about the data file and store results in these
//      program variables. */
//   int ndims_in, nvars_in, ngatts_in, unlimdimid_in;

//   /* There are a number of inquiry functions in netCDF which can be
//      used to learn about an unknown netCDF file. NC_INQ tells how
//      many netCDF variables, dimensions, and global attributes are in
//      the file; also the dimension id of the unlimited dimension, if
//      there is one. */
//   if ((status = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, 
// 		       &unlimdimid_in))) {
//     debug_log << "err";
//   }
//   debug_log << ndims_in << nvars_in << ngatts_in << unlimdimid_in;


//   int latid, lonid;
//   //чтение размерностей
//   size_t lat_length, lon_length;
//   status = nc_inq_dimid(ncid, "lat", &latid);
//   if (status != NC_NOERR) {
//     var(status);
//     return false;
//   }
//   status = nc_inq_dimlen(ncid, latid, &lat_length);
//   if (status != NC_NOERR) {
//    var(status);
//   }

//   status = nc_inq_dimid(ncid, "lon", &lonid);
//   if (status != NC_NOERR) {
//     var(status);
//     return false;
//   }
//   status = nc_inq_dimlen(ncid, lonid, &lon_length);
//   if (status != NC_NOERR) {
//    var(status);
//   }

//   debug_log << lonid << lon_length << latid << lat_length;

//   //чтение размеров переменных
//   if ((status = nc_inq_varid(ncid, "lat", &latid))) {
//     var(status);
//   }
//   if ((status = nc_inq_varid(ncid, "lon", &lonid))) {
//     var(status);
//   }

//   float* lats = new float[lon_length * lat_length];
//   float* lons = new float[lon_length * lat_length];
//   float* data = new float[lon_length * lat_length];


//   var(lats[0]);

//    /* Read the data. */
//   if ((status = nc_get_var_float(ncid, latid, &lats[0]))) {
//     debug_log << "err";
//     return false;
//   }

//   var("read ok");
//   var(lats[2000*2000]);



//    /* Check the data. */
//   uchar* imData = new uchar[lon_length*lat_length];
//   for (uint x = 0, idx = 0; x < lon_length; x++) {
//     for (uint y = 0; y < lat_length; y++) {
//       imData[idx] = uchar(lats[idx]);
//       idx++;
//     }
//   }
//   QImage imqt(imData, lon_length, lat_length, lon_length, QImage::Format_Grayscale8);
//   // QVector<QRgb> palette(256);
//   // for (uint i=0; i< 256; i++) {
//   //   palette[i] = QRgb((i<<16)+ (i<<8) + i);
//   // }

//   // imqt.setColorTable(palette);
//   imqt.save("pic_lat.bmp", "BMP");


//   var(lons[0]);

//    /* Read the data. */
//   //  if ((retval = nc_get_var_float(ncid, varid, &data_in[0][0]))) {
//   if ((status = nc_get_var_float(ncid, lonid, &lons[0]))) {
//     debug_log << "err";
//     return false;
//   }

//   var("read ok");
//   var(lons[2000 * 2000]);


//    /* Check the data. */
//   // uchar* imData = new uchar[lon_length*lat_length];
//   for (uint x = 0, idx = 0; x < lon_length; x++) {
//     for (uint y = 0; y < lat_length; y++) {
//       imData[idx] = uchar(lons[idx]);
//       idx++;
//     }
//   }
//   QImage imqt1(imData, lon_length, lat_length, lon_length, QImage::Format_Grayscale8);
//   // QVector<QRgb> palette(256);
//   // for (uint i=0; i< 256; i++) {
//   //   palette[i] = QRgb((i<<16)+ (i<<8) + i);
//   // }

//   // imqt.setColorTable(palette);
//   imqt1.save("pic_lon.bmp", "BMP");


//   int varid;
//   /* Get the varid of the data variable, based on its name. */
//   if ((status = nc_inq_varid(ncid, "cloudMask", &varid))) {
//     var(status);
//   }
//   var(varid);

//    var(data[0]);

//    /* Read the data. */
//   //  if ((retval = nc_get_var_float(ncid, varid, &data_in[0][0]))) {
//   if ((status = nc_get_var_float(ncid, varid, &data[0]))) {
//     debug_log << "err";
//     return false;
//   }

//   var("read ok");
//   var(data[2000*2000]);


//   /* Check the data. */
//   // uchar* imData = new uchar[lon_length*lat_length];
//   for (int x = lon_length - 1, idx = 0; x >= 0; x--) {
//     for (uint y = 0; y < lat_length; y++) {
//       imData[idx] = uchar(data[x*lat_length+y]);
//       idx++;
//     }
//   }
//   QImage imqt2(imData, lon_length, lat_length, lon_length, QImage::Format_Indexed8);
//   QVector<QRgb> palette(3);
//   palette[0] = QRgb(QColor("blue").rgb());
//   palette[1] = QRgb(QColor("green").rgb());
//   palette[2] = QRgb(QColor("white").rgb());

//   // for (uint i=0; i< 256; i++) {
//   //   palette[i] = QRgb((i<<16)+ (i<<8) + i);
//   // }

//   imqt2.setColorTable(palette);
//   imqt2.save("pic_mask.bmp", "BMP");



//    /* Close the file, freeing all resources. */
//   if ((status = nc_close(ncid))) {
//     debug_log << "err";
//     return false;
//   }

//   GeoData gdata(lon_length * lat_length);

//   for (int x = lon_length - 1, idx = 0; x >= 0; x--) {
//     for (uint y = 0; y < lat_length; y++) {
//       uint num = x*lat_length+y;
//       MData md;
//       md.point.setLatDeg(lats[num]);
//       md.point.setLonDeg(lons[num]);
//       md.mask = TRUE_DATA;
//       md.data = data[num];

//       gdata[idx] = md;
//       idx++;
//     }
//   }

//   // if ( "T2" == in.ncVar || "TT" == in.ncVar ) {
//   //   gdata.addDeltaData(-273.15);
//   // }
//   // else if ( "PSFC" == in.ncVar ) {
//   //   gdata.mnozData(.01);
//   // }
//   // f.setCenter(250);
//   // f.setModel(255);
//   // f.setDate(in.startTime_);
//   // f.setHour(hour * 3600);
//   // f.setDescr(in.descr);
//   // f.setLevel(in.level);
//   // f.setLevelType(in.levelType);
//   // //f.setNet(in.corners_, NetType::STEP_0500x0500);
//   // f.setNet(in.corners_, NetType::STEP_0100x0100);

//   //  int ret_val =  MnObanal::prepInterpolOrder(&gdata, &f);
//   // if ( ERR_OBANAL == ret_val ) {
//   //   error_log << QObject::tr("Ошибка интерполяции.");
//   //   return false;
//   // }
//   // if ( ERR_NODATA_OBANAL == ret_val ) {
//   //   info_log << QObject::tr("Нет данных для анализа.");
//   //   return false;
//   // }

//   // if ( TMeteoDescriptor::instance()->descriptor("N") == f.getDescr() ) {
//   //   f.smootchField(1);
//   // }


//   delete[] lats;
//   delete[] lons;
//   delete[] data;


//   // NcFile file(filePath.toStdString(), NcFile::ReadOnly);

//   // if ( !file.is_valid() ) {
//   //   error_log << QObject::tr("Некорректный формат файла.");
//   //   return false;
//   // }

//   // NcVar* dt = file->get_var("dt");

//   //  NcVar* dataNc = ncFile_->get_var(ncVar.toUtf8().constData());
//   // if ( 0 == dataNc ) {
//   //   error_log << QObject::tr("Переменная '%1' отсутствует в файле.").arg(ncVar);
//   //   return false;
//   // }


//   // file.close();

//   return true;
// }




// bool ConvertNc::convertAndSave(const QString& ncName)
// {
//   ::obanal::TField field;
//   bool ok = convert(ncName, &field);

//   if (!ok) return false;

//   //save
//   meteo::field::DataDesc fdescr;
//   // fdescr.set_date(afdescr->date());
//   // fdescr.set_center(afdescr->center());
//   // fdescr.set_level(15000);
//   // fdescr.set_hour(afdescr->hour());
//   // fdescr.set_level_type(15000);
//   // fdescr.set_meteodescr(10009); 
//   // fdescr.set_net_type(f.typeNet());
//   // fdescr.set_model(afdescr->model());
//   // fdescr.set_dt1(afdescr->dt1());
//   // fdescr.set_dt2(afdescr->dt2());
//   // fdescr.set_time_range(afdescr->time_range());


//   // field->setHour(afdescr->hour());
//   // field->setModel(afdescr->model());
//   // field->setCenter(afdescr->center());
//   // field->setDate(QDateTime::fromString(QString::fromStdString(afdescr->date()), Qt::ISODate));
//   // field->setCenterName(QString::fromStdString(afdescr->center_name()));
//   // field->setLevelTypeName( QString::fromStdString( fdescr.level_type_name() ) );
//   // f.setDescr(in.descr);
//   // f.setLevel(in.level);
//   // f.setLevelType(in.levelType);
//   // //f.setNet(in.corners_, NetType::STEP_0500x0500);
//   // f.setNet(in.corners_, NetType::STEP_0100x0100);


//   TObanalDb db;
//   QString path = QDir::homePath() + "/obanal";
//   db.setPath(path);
//   ok = db.saveField(field, fdescr);

//   return ok;	   
// }
