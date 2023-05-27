#include "alphanumgen.h"
#include "ancgen.pb.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <meteo/commons/proto/locsettings.pb.h>
#include <commons/mathtools/mnmath.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/alphanum/wmomsg.h>
#include <meteo/commons/alphanum/metadata.h>
#include <meteo/commons/control/tmeteocontrol.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <qfiledialog.h>
#include <qtreewidget.h>

#include <math.h>

namespace meteo {
  namespace anc {
    QString func_RRR(float val);
    QString func_dd(float val);
    QString func_hhh(float val);
    QString func_P(float val);
    QString func_P1(float val);
    QString func_P1h(float val);
    //    QString func_h1h1(float val);
    QString func_T1T1(float val);
    QString func_D1D1(float val);
  }
}

using namespace meteo;

enum Column {
  kNameCol  = 0,
  kCodeNameCol = 1,
  kCodeCol  = 2,
  kValueCol = 3,
  kUnitCol  = 4,
};

AlphanumGen::AlphanumGen(const QString& file):
  _templ(nullptr),
  _ctrl(nullptr)
{
  _templ = new anc::AncTemplate;
  _ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
  //_ctrl = meteo::global::serviceController( meteo::settings::proto::kSprinf );

  readTemplate(file);
  registerFunc();
}

AlphanumGen::~AlphanumGen()
{
  if (nullptr != _templ) {
    delete _templ;
    _templ = 0;
  }

  if (nullptr != _ctrl) {
    delete _ctrl;
    _ctrl = 0;
  }
}

//! Чтение файла с шаблоном кода
bool AlphanumGen::readTemplate(const QString& confFile)
{
  QFile file(confFile);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << msglog::kFileOpenError.arg(confFile).arg(file.error());
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, _templ) ) {
    error_log << msglog::kFileReadFailed.arg(confFile);
    return false;
  }

  return true;
}

void AlphanumGen::registerFunc(const QStringList& dl, anc::FuncGetCode func)
{
  for (int i = 0; i < dl.count(); i++) {
    _funcs.insert(dl.at(i), func);
  }
}

void AlphanumGen::registerFunc()
{
  _funcs.insert("RRR", meteo::anc::func_RRR);
  _funcs.insert("dd", meteo::anc::func_dd);
  _funcs.insert("hhh", meteo::anc::func_hhh);
  _funcs.insert("P", meteo::anc::func_P);
  _funcs.insert("P1", meteo::anc::func_P1);
  _funcs.insert("P1h", meteo::anc::func_P1h);
  _funcs.insert("T1T1", meteo::anc::func_T1T1);
  _funcs.insert("D1D1", meteo::anc::func_D1D1);
}

void AlphanumGen::decode(const QByteArray& ba, QTreeWidget* view)
{
  view->clear();

  QStringList forms;
  forms << "FM12" << "FM32" << "FM35";
  QList<int> types;
  types << anc::kWmoFormat;

  meteo::anc::MetaData::instance()->load(forms, types);

  QMap<QString, QString> rt;
  meteo::anc::WmoMsg wm;
  wm.setCodec(QTextCodec::codecForName("Windows-1251"));
  QDateTime dt;
  wm.parse(rt, ba, dt);
  while(wm.hasNext()) {
    bool ok = wm.decodeNext();
    if (ok) {
      TMeteoData* data = wm.data();
      fillCoord(data);
      TMeteoControl::instance()->control(control::LIMIT_CTRL | control::CONSISTENCY_CTRL, data);
      //      data->printData();
      fillDecodeView(data, view);
    }
  }
}

void AlphanumGen::fillCoord(TMeteoData* data)
{
  if (data == 0) return;
  if (!data->hasParam(TMeteoDescriptor::instance()->descriptor("station_index"))) {
    return;
  }

  int index = data->getValue(TMeteoDescriptor::instance()->descriptor("station_index"), -1);
  if (index == -1) return;
  int dataType = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1);
  if (dataType == -1) return;

  if (_station.has_index() && index == _station.index()) {
    if (_station.has_position()) {
      const meteo::GeoPointPb& pnt = _station.position();
      if (pnt.has_height_meters()) {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())), pnt.height_meters());
      } else {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())));
      }
    }
    return;
  }

  //  bool ok;
  //  rpc::Address addr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf, &ok );
  // debug_log << "sprinf.host =" << addr.host();
  //if (addr.host().isEmpty()) return;

  //  rpc::TController*  _ctrl = new rpc::TController(rpc::ServiceLocation::instance()->serviceClient(addr));
  if (0 == _ctrl) return;

  meteo::sprinf::MultiStatementRequest request;
  request.add_index(index);
  request.add_data_type(dataType);

  meteo::sprinf::Stations* response = _ctrl->remoteCall(&meteo::sprinf::SprinfService::GetStations, request, 10000);
  if (response != 0 && response->result() == true) {
    if (response->station_size() != 0 && response->station(0).has_position()) {
      const meteo::GeoPointPb& pnt = response->station(0).position();
      if (pnt.has_height_meters()) {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())), pnt.height_meters());
      } else {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())));
      }
    }
    if (response->station_size() != 0) {
      _station.CopyFrom(response->station(0));
    }
  }
  if (0 != response) {
    delete response;
  }
}


void AlphanumGen::fillDecodeView(const TMeteoData* md, QTreeWidget* view, QTreeWidgetItem* parent /* = 0*/)
{
  QMap<QString, TMeteoParam> vals =  md->allByNames(false);

  QMapIterator<QString, TMeteoParam> it(vals);
  while (it.hasNext()) {
    it.next();

    meteodescr::Property prop;
    TMeteoDescriptor::instance()->property(it.key(), &prop);

    QTreeWidgetItem* item;
    if (parent != nullptr) {
      item = new QTreeWidgetItem(parent);
    } else {
      item = new QTreeWidgetItem(view);
    }
    item->setText(kNameCol, prop.description);
    item->setText(kCodeNameCol, it.key());
    item->setData(kCodeNameCol, Qt::DisplayRole, TMeteoDescriptor::instance()->descriptor(it.key()));
    item->setText(kCodeCol,  it.value().code());
    item->setText(kValueCol, QString::number(it.value().value()));
    if (it.value().quality() == control::DOUBTFUL) {
      item->setForeground(kCodeCol, QColor(240, 120, 0));
      item->setForeground(kValueCol,QColor(240, 120, 0));
    } else if (it.value().quality() == control::MISTAKEN) {
      item->setForeground(kCodeCol, Qt::red);
      item->setForeground(kValueCol, Qt::red);
    } else if (it.value().quality() == control::NO_OBSERVE) {
      item->setForeground(kCodeCol, Qt::darkGray);
      item->setForeground(kValueCol, Qt::darkGray);
    }
    if (prop.unitsRu != "NO") {
      item->setText(kUnitCol, prop.unitsRu);
    }
    item->setTextAlignment(kCodeCol, Qt::AlignRight);
    item->setTextAlignment(kValueCol, Qt::AlignRight);
  }

  if (nullptr != parent) {
    parent->sortChildren(kCodeNameCol, Qt::AscendingOrder);
  } else {
    view->sortItems(kCodeNameCol, Qt::AscendingOrder);
  }

  for (int idx = 0; idx < md->childs().size(); idx++) {
    QTreeWidgetItem* item;
    if (parent != nullptr) {
      item = new QTreeWidgetItem(parent);
    } else {
      item = new QTreeWidgetItem(view);
    }
    item->setExpanded(true);
    //    item->setText(kNameCol, QString::number(idx));
    fillDecodeView(md->childs().at(idx), view, item);
  }

}

//! Формирование кода
QString AlphanumGen::generate()
{
  //  trc;
  if (0 == _templ || !_templ->IsInitialized()) {

    return QString();
  }

  QString code;
  int length = 0;

  //  var(_templ->DebugString());

  for (int sidx = 0; sidx < _templ->section_size(); sidx++) {
    QString sectionCode;
    bool sectEmpty = true; //пустая секция (все группы пустые)

    sectionCode += groupsCode(_templ->section(sidx).mgroup(), &length, &sectEmpty);

    if (!sectEmpty || _templ->section(sidx).require()) {
      if (sectEmpty && _templ->section(sidx).has_defvalue()) {
        //	code += QString::fromStdString(_templ->section(sidx).defvalue()) + "\n";
      } else {
        if (_templ->section(sidx).has_mark()) {
          code += QString::fromStdString(_templ->section(sidx).mark()) + " ";
        }
        if (!sectionCode.isEmpty()) {
          code += sectionCode + "\n";
        }
      }
      length = 0;
    }

    for (int idx = 1; idx < _templ->section(sidx).mgroupset_size(); idx++) {
      bool sectSetEmpty = true;
      QString setCode = groupsCode(_templ->section(sidx).mgroupset(idx).mgroup(), &length, &sectSetEmpty);
      if (!sectSetEmpty) {
        if (sectEmpty && _templ->section(sidx).has_mark()) {
          code += QString::fromStdString(_templ->section(sidx).mark()) + " ";
        }
        code += setCode + "\n";
        length = 0;
        sectEmpty = false;
      }
    }
    if (sectEmpty && _templ->section(sidx).require() && _templ->section(sidx).has_defvalue()) {
      code += QString::fromStdString(_templ->section(sidx).defvalue()) + "\n";
    }

  }

  code = code.trimmed();
  code += "=\r\r\n";

  return code;
}

QString AlphanumGen::groupsCode(const ::google::protobuf::RepeatedPtrField< ::meteo::anc::Group>& mgrouplist,
                                int* length, bool* sectEmpty)
{
  QString sectionCode;

  for (int idx = 0; idx < mgrouplist.size(); idx++) {
    bool empty = true; //пустая группа (все параметры отсутствуют)
    const anc::Group& mgroup = mgrouplist.Get(idx);
    if (mgroup.plist_size() != 0) {
      for (int lidx = 0; lidx < mgroup.plist_size(); lidx++) {
        sectionCode += paramCode(mgroup.plist(lidx).param(),
                                 mgroup.require(), length, &empty);
        if (!empty) {
          *sectEmpty = false;
        }
      }
    } else {
      sectionCode += paramCode(mgroup.param(), mgroup.require(),
                               length, &empty);
      if (!empty) {
        *sectEmpty = false;
      }
    }
  }
  return sectionCode;
}

QString AlphanumGen::paramCode(const ::google::protobuf::RepeatedPtrField< ::meteo::anc::Param >& paramlist,
                               bool required, int* length, bool* empty)
{
  QString code;
  QString group;
  for (int pidx = 0; pidx < paramlist.size(); pidx++) {
    const anc::Param& param = paramlist.Get(pidx);
    if (param.has_text()) {
      QString pcode = QString::fromStdString(param.text());
      if (param.has_name() && !pcode.contains('/')) {
        *empty = false;
      }
      group += pcode;
    } else {
      group += QString().leftJustified(param.width(), '/');
    }
  }

  if (!*empty || required) {
    if (*length + group.length() > 80) {
      code += "\n";
      *length = 0;
    }
    code += group + " ";
    *length += group.length() + 1;
  }
  return code;
}

//! Поиск группы по имени
anc::Group* AlphanumGen::findGroup(const QString& name)
{
  for (int sidx = 0; sidx < _templ->section_size(); sidx++) {
    for (int idx = 0; idx < _templ->section(sidx).mgroup_size(); idx++) {
      if (_templ->section(sidx).mgroup(idx).has_name() &&
          QString::fromStdString(_templ->section(sidx).mgroup(idx).name()) == name) {
        return _templ->mutable_section(sidx)->mutable_mgroup(idx);
      }
    }
  }

  return 0;
}

anc::Section* AlphanumGen::findGroupSet(const QString& name)
{
  for (int sidx = 0; sidx < _templ->section_size(); sidx++) {
    for (int idx = 0; idx < _templ->section(sidx).mgroupset_size(); idx++) {
      if (_templ->section(sidx).mgroupset(idx).has_name() &&
          QString::fromStdString(_templ->section(sidx).mgroupset(idx).name()) == name) {
        return _templ->mutable_section(sidx);
      }
    }
  }
  return 0;
}

//! Очистка повторяющихся наборов группы
void AlphanumGen::clearGroupList(const QString& name)
{
  anc::Group* group = findGroup(name);
  if (0 == group) return;

  group->clear_plist();
}

//! Установка значения параметры из набора с добавлением нового набора
int AlphanumGen::addListValue(const QString& groupName, const QString& name, float value)
{
  anc::Group* group = findGroup(groupName);
  if (group == 0) return -1;

  group->add_plist();
  setListValue(groupName, group->plist_size()-1, name, value);
  return group->plist_size()-1;
}


void AlphanumGen::clearGroupSet(const QString& name)
{
  anc::Section* sect = findGroupSet(name);
  if (0 == sect) return;

  anc::GroupList templ;
  templ.CopyFrom(sect->mgroupset(0));

  sect->clear_mgroupset();
  anc::GroupList* gset = sect->add_mgroupset();
  gset->CopyFrom(templ);
}

int AlphanumGen::addGroupSetValue(const QString& setName, const QString& name, float value)
{
  anc::Section* sect = findGroupSet(setName);
  if (0 == sect || 0 == sect->mgroupset_size()) return -1;

  anc::GroupList* gset = sect->add_mgroupset();
  gset->CopyFrom(sect->mgroupset(0));
  setGroupSetValue(setName, sect->mgroupset_size()-1, name, value);

  return sect->mgroupset_size()-1;
}

void AlphanumGen::setGroupSetRequire(const QString& setName, int sidx, const QString& name, bool require)
{
  anc::Section* sect = findGroupSet(setName);
  if (0 == sect || sidx < 0 || sidx > sect->mgroupset_size()) return;

  anc::GroupList* gset = sect->mutable_mgroupset(sidx);
  if (gset == 0) return;
  for (int gidx = 0; gidx < gset->mgroup_size(); gidx++) {
    for (int pidx = 0; pidx < gset->mgroup(gidx).param_size(); pidx++) {
      if (QString::fromStdString(gset->mgroup(gidx).param(pidx).name()) == name) {
        gset->mutable_mgroup(gidx)->set_require(require);
        return;
      }
    }
  }

}


//! Поиск настроек метеопараметра
anc::Param* AlphanumGen::findParam(const QString& name)
{
  for (int sidx = 0; sidx < _templ->section_size(); sidx++) {
    for (int idx = 0; idx < _templ->section(sidx).mgroup_size(); idx++) {
      //      QString group;
      for (int pidx = 0; pidx < _templ->section(sidx).mgroup(idx).param_size(); pidx++) {
        if (QString::fromStdString(_templ->section(sidx).mgroup(idx).param(pidx).name()) == name) {
          return _templ->mutable_section(sidx)->mutable_mgroup(idx)->mutable_param(pidx);
        }
      }
    }
  }

  debug_log << QObject::tr("Параметр %1 не найден").arg(name);
  //  var(_templ->DebugString());

  return 0;
}

//! Сброс значения метеопараметра
void AlphanumGen::clearValue(const QString& name)
{
  anc::Param* param = findParam(name);
  if (0 != param) {
    param->clear_text();
  }
}

void AlphanumGen::setRequire(const QString& name, bool require)
{
  anc::Group* group = findGroup(name);
  if (0 != group) {
    group->set_require(require);
  }
}

//! Установка кодового значения метеопараметра
bool AlphanumGen::setCode(const QString& name, const QString& val)
{
  anc::Param* param = findParam(name);
  if (0 != param) {
    param->set_text(val.rightJustified(param->width(), param->fill()).toStdString());
  }

  return true;
}

//! Установка значения метеопараметра, с преобразованием в код при необходимости
bool AlphanumGen::setValue(const QString& name, float val)
{
  return setValue(findParam(name), val);
}

bool AlphanumGen::setValue(anc::Param* param, float val)
{
  if (0 != param) {
    QString code;

    if (param->has_table()) {
      code = tableConvert(param, val);
    } else if (param->has_func()) {
      code = functionConvert(param, val);
      //debug_log << name << code;
    } else {
      if (param->has_mul()) {
        val *= param->mul();
      }
      if (param->has_sign() && val < 0) {
        code = QString::fromStdString(param->sign());
        code += QString::number(fabs(val), 'f', 0).rightJustified(param->width() - 1, param->fill());
      } else {
        code = QString::number(val);
      }
    }

    param->set_text(code.rightJustified(param->width(), param->fill()).toStdString());
  }

  return true;
}

//! Установка значения метеопараметра, с преобразованием в код при необходимости
bool AlphanumGen::setValue(const QString& name, const QString& val)
{
  return setValue(findParam(name), val.toUpper());
}

bool AlphanumGen::setValue(anc::Param* param, const QString& value)
{
  bool ok = false;
  if (0 != param) {
    if (param->has_mul() || param->has_table() || param->has_func()) {
      ok = setValue(param, value.toFloat());
    } else {
      param->set_text(value.rightJustified(param->width(), param->fill()).toStdString());
      ok = true;
    }
  }

  return ok;
}

//! Табличное преобразование параметра
QString AlphanumGen::tableConvert(anc::Param* param, float value)
{
  if (0 == param) return QString();

  QString res;

  QStringList sList = QString::fromStdString(param->table()).split(";");
  for (int i=0; i< sList.count(); i++) {
    QString tcode;
    float tval = sList.at(i).section(',', -1, -1).toFloat();
    if (sList.at(i).contains(',')) {
      tcode = sList.at(i).section(',', -2, -2);
    } else {
      tcode = QString::number(i);
    }
    if (value >= tval) {
      res = tcode;
    } else {
      break;
    }
  }
  return res;
}

QString AlphanumGen::functionConvert(anc::Param* param, float val)
{
  if (0 == param) return QString();

  QString funcName = QString::fromStdString(param->func());

  if (!_funcs.contains(funcName)) {
    error_log << QObject::tr("Нет функции") << param->func(); //TODO
    return QString().rightJustified(param->width(), '/');
  }

  return _funcs.value(funcName)(val);
}

QString AlphanumGen::chooseFile(const QString& conf, const QString& name, QWidget* parent)
{
  QString caption = QString::fromUtf8("Cохранение в файл");
  if (defaultDir_.isEmpty()) {
    defaultDir_ = readDefaultDir(conf);
  }
  QString filter = QString::fromUtf8("Метеорологические сообщения (*.tlg);;"
                                     "Все (*)");

  QString selectedFilter;
  QString fileName = FileDialog::getSaveFileName(parent, caption,
                                                  defaultDir_ + "/" + name,
                                                  filter, &selectedFilter);

  if (!fileName.isEmpty()) {
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);
    saveDefaultDir(conf, defaultDir_);

    QRegExp rx("\\w\\s[(*]{2}(.\\w{3,4})");
    rx.indexIn(selectedFilter);

    selectedFilter = rx.cap(1);

    if (fileName.endsWith(selectedFilter) == false) {
      fileName += QString(selectedFilter);
    }

  }
  return fileName;
}

QString AlphanumGen::readDefaultDir(const QString& conf)
{
  QString dir = QDir::homePath();

  QFile file(conf);
  if ( !file.open(QIODevice::ReadOnly) ) {
    return dir;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  meteo::map::exportmap::ExportPath p;
  if ( !TProtoText::fillProto(text, &p) ) {
    file.remove(conf);
    return dir;
  }

  return QString::fromStdString(p.path());
}

void AlphanumGen::saveDefaultDir(const QString& conf, const QString& dir) const
{
  meteo::map::exportmap::ExportPath p;
  p.set_path(dir.toStdString());

  QString text = TProtoText::toText(p);
  if (text.isEmpty()) return;


  QFile file(conf);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
}

// зарегистированные функции, вызываются в соответствии с настройкой шаблона

//! Осадки
QString meteo::anc::func_RRR(float val)
{
  if (val > 989) { return "989"; }
  if (val >= 1 || val == 0)  { return QString::number(val, 'f', 0); }
  return QString::number(val*10 + 990, 'f', 0);

}

//! Ветер
QString meteo::anc::func_dd(float val)
{
  val = round(val / 10.0);

  if (val < 1) return "36";
  return QString::number(val, 'f', 0);
}

//! Геопотенциальная высота
QString meteo::anc::func_hhh(float val)
{
  return QString::number(static_cast<int>(val) % 1000, 'f', 0);
}

//! Давление в десятках гПа
QString meteo::anc::func_P(float val)
{
  return QString::number(val >= 1000 ? (val - 1000)*10 : val*10, 'f', 0);
}

//! Давление
QString meteo::anc::func_P1(float val)
{
  return QString::number(val >= 1000 ? val - 1000 : val, 'f', 0);
}

//! Давление
QString meteo::anc::func_P1h(float val)
{
  if (val == 1000) return "0";
  return QString::number(val >= 100 ? val / 10 : val, 'f', 0);
}

QString meteo::anc::func_T1T1(float val)
{
  int res = fabs(val)*10 + 0.05;

  if (val > 0 && 0 != (res % 2)) {
    res -= 1;
  }

  if (val < 0 && 0 == (res % 2)) {
    res += 1;
  }

  return QString::number(res);
}

QString meteo::anc::func_D1D1(float val)
{
  if (val > 5 && val < 6) {
    val = round(val);
  }
  if (val <= 5) return QString::number(val*10, 'f', 0);
  if (val <= 49) return QString::number(val + 50, 'f', 0);

  return "99";
}

//-----
// функции, вызываемые из кода

//! Видимость инструментално измеренная
QString meteo::anc::func_VVinstr(float val)
{
  if (val < 0.1) return 0;
  if (val > 5 && val < 6) {
    val = 5;
  }
  if (val <= 5) return QString::number(val*10, 'f', 0);
  if (val >=6  && val < 30) return QString::number(static_cast<int>(val) + 50);
  if (val > 70) return "89";
  return QString::number(static_cast<int>((val - 30) / 5 + 80));
}

//! Видимость визуально измеренная
QString meteo::anc::func_VVvis(float val)
{
  QList<float> table;
  table << 0.05 << 0.2 << 0.5 << 1 << 2 << 4 << 10 << 20 << 50;

  for (int idx = 0; idx < table.size(); idx++) {
    if (val < table.at(idx)) {
      return QString::number(90 + idx, 'f', 0);
    }
  }

  return "99";
}

QString meteo::anc::func_RR(float val)
{
  if (val >= 0.1 && val < 0.7) { return QString::number(90 + val*10); }
  if (val >= 0.7 && val < 1)   { return "1"; }
  if (val < 56) return QString::number(val, 'f', 0);
  if (val <= 400) return QString::number(val/10 + 50, 'f', 0);
  return "98";
}

QString meteo::anc::func_ss(float val)
{
  if (val >= 0.1 && val <= 0.6) { return QString::number(90 + val*10); }
  if (val > 0.6 && val < 1)   { return "1"; }
  if (val < 56) return QString::number(val, 'f', 0);
  if (val <= 400) return QString::number(val/10 + 50, 'f', 0);
  return "98";
}

//! ВНГО, измеренная инструментально
QString meteo::anc::func_hshsInstr(float val)
{

  if (val <= 1500) return QString::number(static_cast<int>(val / 30));
  if (val < 1800)  return "50";
  if (val <= 9000) return QString::number(static_cast<int>(val / 300) + 50);
  if (val < 10500) return "80";
  if (val > 21000) return "89";

  return QString::number(static_cast<int>(val / 1500) - 6 + 80, 'f', 0);
}

//! ВНГО, измеренная визуально
QString meteo::anc::func_hshsVis(float val)
{
  if (val < 50)    return "90";
  if (val < 100)   return "91";
  if (val < 200)   return "92";
  if (val < 300)   return "93";
  if (val < 600)   return "94";
  if (val < 1000)  return "95";
  if (val < 1500)  return "96";
  if (val < 2000)  return "97";
  if (val < 2500)  return "98";

  return "99";
}

void meteo::anc::func_ddfff(const QString& ddv, const QString& ffval, QString* ddcode, QString* ffcode)
{
  if (ffval.contains("/")) {
    *ffcode = "///";
  } else {
    *ffcode = ffval;
  }

  if (ddv == "~") {
    *ddcode = "99";
    return;
  }

  if (ddv.contains("/")) {
    *ddcode = "//";
    return;
  }

  if ((!ddv.isEmpty() && ddv.toInt() == 0) || (!ffval.isEmpty() && !ffval.contains('/') && ffval.toInt() == 0)) {
    *ddcode  = "0";
    *ffcode  = "0";
    return;
  }

  if (ddv.isEmpty()) {
    *ddcode = ddv;
    return;
  }

  QString ddval = ddv.rightJustified(3, '0');
  int ddl = ddval.right(1).toInt();

  if (ddl <= 2) {
    *ddcode = ddval.left(2);
  } else if (ddl <= 7) {
    *ddcode = ddval.left(2);
    if (!ffval.contains('/')) {
      int ff = ffval.toInt();
      *ffcode = QString::number(ff + 500);
    }
  } else {
    int dd = ddval.left(2).toInt();
    *ddcode = QString::number(dd + 1);
  }
}

