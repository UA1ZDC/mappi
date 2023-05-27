#include "tablowidget.h"

#include <qmenu.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/ui/custom/descrselect.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <meteo/commons/punchrules/punchrules.h>

#include <meteo/commons/ui/coloralert/alertcheck.h>
#include <meteo/commons/ui/coloralert/coloralertdlg.h>
#include <meteo/commons/ui/coloralert/conditionitem.h>

#include <meteo/commons/proto/surface_service.pb.h>

#include "stationdlg.h"

Q_DECLARE_METATYPE( TMeteoData )

namespace meteo {

//!
class TabloItem : public QTreeWidgetItem
{
public:
  TabloItem(QTreeWidget* parent) : QTreeWidgetItem(parent) {
    setFlags( flags() ^ Qt::ItemIsSelectable );
  }

private:
  bool operator<(const QTreeWidgetItem &other) const
  {
    QTreeWidgetItem* head = treeWidget()->headerItem();
    int col = treeWidget()->sortColumn();
    if ( nullptr != head && TabloWidget::kColumnMeteoParam == head->data(col,TabloWidget::kColumnTypeRole).toInt() ) {
      return data(col, TabloWidget::kRoleMeteoParamValue).toDouble() < other.data(col, TabloWidget::kRoleMeteoParamValue).toDouble();
    }
    return text(col).toLower() < other.text(col).toLower();
  }
};


//!
class GridDelegate : public QStyledItemDelegate
{
public:
  explicit GridDelegate(QObject * parent = nullptr) : QStyledItemDelegate(parent) { }

  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
  {
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    painter->setPen(QColor(170,170,170));
    painter->drawRect(option.rect);
    painter->restore();

  }
};


TabloWidget::TabloWidget(QWidget* parent)
  : QTreeWidget(parent)
{
  setItemDelegate(new GridDelegate(this));

  setRootIsDecorated(false);
  setAllColumnsShowFocus(true);
  setSortingEnabled(true);

  resetHeader();

  meteo::WeatherFont::instance()->loadFonts();

  meteo::puanson::proto::Puanson tmpl = map::WeatherLoader::instance()->punchlibraryspecial().value("meteotablo");
  for ( int i=0,isz=tmpl.rule_size(); i<isz; ++i ) {
    QString param = pbtools::toQString(tmpl.rule(i).id().name());
    templParams_[param] = tmpl.rule(i);
  }
  punchStub_.setPunch(tmpl);
  setSortingEnabled(false);
}

void TabloWidget::setData(surf::DataReply *resp)
{
  if ( true == isArchive_ ) {
    sortMeteodataByDt(resp);
  }
  bool ok = false;
  puanson::proto::Puanson punch = map::WeatherLoader::instance()->punchparams( "is", &ok );
  if ( false == ok ) {
    warning_log << QObject::tr("Шаблон наноски '%1' не найден").arg("is");
  }
  for ( auto data : resp->meteodata_proto() ) {

    QTreeWidgetItem* item = nullptr;

    QString stindex;
    if ( false == data.station_info().has_cccc() ) {
      error_log << QObject::tr("Неизвестная станция");
      continue;
    }
    else {
      stindex = QString::fromStdString(data.station_info().cccc()).remove("'");
    }
    if ( true == isArchive_ ) {
      item = new QTreeWidgetItem(this);
    }
    else {
      item = findItem(findColumn(kColumnId, kColumnTypeRole), stindex);
    }

    if ( nullptr == item ) {
      warning_log << QObject::tr("Не найдена станция '%1'")
        .arg(stindex);
      continue;
    }

    QDateTime dt = QDateTime::fromString(QString::fromStdString(data.dt()), Qt::ISODate);
    QTime time = dt.time();
    QDate date = dt.date();

    int column = findColumn(kColumnTime, kColumnTypeRole);

    item->setText(column, time.toString("hh:mm"));
    item->setToolTip(column, QObject::tr("срок:")+date.toString("dd.MM.yyyy ") + time.toString("hh:mm"));
    item->setData(column, kRoleDateTime, dt);

    bool shtihl = false;
    bool hasGust = false;
    QString gustValue;
    for(auto param : data.param()) {
      if ( "ff" == param.descrname() && 0 == param.value() ) {
        shtihl = true;
      }
      if ( "fx" == param.descrname() ) {
        hasGust = true;
        auto code = QString::fromStdString(param.code());
        gustValue = map::stringFromRuleValue(param.value(), templParams_["fx"], code);
      }
    }
    QList<float> Nhs;
    QStringList N{"N", "Nh"};
    QStringList C{"C", "Cn", "CH", "CM", "CL"};
    QStringList vngo{"h"};
    QStringList ws{"w", "w_w_", "w_tr"};
    QStringList vs{"V", "VV"};

    auto cloudMap = map::stringForCloud(&data, punch);
    for ( auto descr : descriptors() ) {
      int column;
      column = findColumn(descr, kMeteoDescrNameRole);
      if ( -1 == column ) {
        continue;
      }

      meteo::surf::MeteoParamProto param;
      bool found = false;
      for ( auto p : data.param() ) {
        if ( descr == QString::fromStdString(p.descrname())) {
          param = p;
          found = true;
          break;
        }
        else if ( true == ws.contains(descr) && true == ws.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          found = true;
          break;
        }
        else if ( true == vs.contains(descr) && true == vs.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          found = true;
          break;
        }
        else if ( true == N.contains(descr) && true == N.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          found = true;
          break;
        }
        else if ( true == C.contains(descr) && true == C.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          found = true;
          break;
        }
        else if ( true == vngo.contains(descr) && true == vngo.contains(QString::fromStdString(p.descrname()))) {
          descr = QString::fromStdString(p.descrname());
          param = p;
          found = true;
          break;
        }
      }
      item->setData(column, kRoleRealDescriptor, descr);
      QList<int> invalidValues = QList<int>() << 9999;
      if ( false == found || true == invalidValues.contains(param.quality())) {
        item->setText(column, QString());
        item->setData(column, kRoleMeteoData, QVariant());
        item->setData(column, kRoleMeteoParamValue, QVariant());
        continue;
      }


      if ( true == templParams_.contains(descr)) {
        if (N.contains(QString::fromStdString(param.descrname())) ) {
          item->setText( column, cloudMap["N"] );
        }
        else if (C.contains(QString::fromStdString(param.descrname())) ) {
          item->setText( column, cloudMap["C"] );
        }
        else if (vngo.contains(QString::fromStdString(param.descrname())) ) {
          item->setText( column, cloudMap["h"] );
        }
        else if ( "ff" == param.descrname() && true == shtihl ) {
          item->setText( column, QObject::tr("штиль") );
        }
        else if ( "ff" == param.descrname() && !shtihl && hasGust ) {
          auto code = QString::fromStdString(param.code());
          QString windValue = map::stringFromRuleValue(param.value(), templParams_[descr], code);
          item->setText(column, windValue + "/" + gustValue);
        }
        else if ( "dd" == param.descrname() && true == shtihl ) {
          item->setText( column, QObject::tr("") );
        }
        else {
          auto code = QString::fromStdString(param.code());
          item->setText(column, map::stringFromRuleValue(param.value(), templParams_[descr], code));
          item->setFont(column, map::fontFromRuleValue(param.value(), templParams_[descr]));
        }
        std::string str;
        param.SerializeToString(&str);
        QByteArray arr(str.data(), str.size());
        item->setData(column, kRoleMeteoData, arr);
        if ( "w_w_" == descr ) {
          item->setData(column, kRoleMeteoParamValue, QString::fromStdString(param.code()));
        }
        else if (C.contains(QString::fromStdString(param.descrname()))) {
          item->setData(column, kRoleMeteoParamValue, cloudMap["rawC"]);
        }
        else if (N.contains(QString::fromStdString(param.descrname()))) {
          item->setData(column, kRoleMeteoParamValue, cloudMap["rawN"]);
        }
        else if (vngo.contains(QString::fromStdString(param.descrname()))) {
          item->setData(column, kRoleMeteoParamValue, cloudMap["rawH"]);
        }
        else {
          item->setData(column, kRoleMeteoParamValue, param.value());
        }
      }
    }
  }
}

QList<int> TabloWidget::dataTypes() const
{
  int column = findColumn(kColumnId,kColumnTypeRole);
  if ( -1 == column ) {
    return QList<int>();
  }

  QSet<int> set;
  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = topLevelItem(i);
    set << item->data(column,kDataTypeRole).toInt();
  }

  return set.toList();
}

QStringList TabloWidget::descriptors() const
{
  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return QStringList();
  }

  QStringList list;
  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    if ( kColumnMeteoParam != head->data(i,kColumnTypeRole).toInt() ) {
      continue;
    }
    if ( isColumnHidden(i) ) {
      continue;
    }

    list << head->data(i,kMeteoDescrNameRole).toString();
  }

  if ( list.contains("N") || list.contains("Nh") ) {
    list << "N" << "Nh";
    list.removeDuplicates();
  }
  if ( list.contains("C") || list.contains("Cn") || list.contains("CH") || list.contains("CM") || list.contains("CL") ) {
    list << "C" << "Cn" << "CH" << "CM" << "CL";
    list.removeDuplicates();
  }
  if ( list.contains("h") || list.contains("hgr") ) {
    list << "h" << "hgr";
    list.removeDuplicates();
  }
  if ( list.contains("w") || list.contains("w_w_") || list.contains("w_tr") ) {
    list << "w" << "w_w_" << "w_tr";
    list.removeDuplicates();
  }
  if ( list.contains("V") || list.contains("VV") ) {
    list << "V" << "VV";
    list.removeDuplicates();
  }

  return list;
}

QList<qint64> TabloWidget::descriptorIds() const
{
  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return QList<qint64>();
  }

  QList<qint64> list;
  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    if ( kColumnMeteoParam != head->data(i,kColumnTypeRole).toInt() ) {
      continue;
    }

    list << TMeteoDescriptor::instance()->descriptor(head->data(i,kMeteoDescrNameRole).toString());
  }

  return list;
}

QList<int> TabloWidget::stations(int dataType) const
{
  int column = findColumn(kColumnId,kColumnTypeRole);
  if ( -1 == column ) {
    return QList<int>();
  }

  QRegExp ccccRx(tr("[А-Яа-яA-Za-z0-9]{4}"));

  QList<int> list;
  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = topLevelItem(i);

    if ( ccccRx.exactMatch(item->text(column)) ) {
      continue;
    }
    if ( -1 != dataType && dataType != item->data(column,kDataTypeRole).toInt() ) {
      continue;
    }

    list << item->text(column).toInt();
  }

  return list;
}

QStringList TabloWidget::airports(int dataType) const
{
  int column = findColumn(kColumnId,kColumnTypeRole);
  if ( -1 == column ) {
    return QStringList();
  }

  QRegExp ccccRx(tr("[А-Яа-яA-Za-z0-9]{4}"));

  QStringList list;
  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = topLevelItem(i);

    if ( !ccccRx.exactMatch(item->text(column)) ) {
      continue;
    }
    if ( -1 != dataType && dataType != item->data(column,kDataTypeRole).toInt() ) {
      continue;
    }
    list << item->text(column);
  }

  return list;
}

void TabloWidget::adjustColumns(int offset)
{
  for ( int i=offset,isz=columnCount(); i<isz; ++i ) {
    if ( !isColumnHidden(i) ) {
      resizeColumnToContents(i);
    }
  }
}

tablo::Settings TabloWidget::save() const
{
  tablo::Settings opt;

  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    tablo::Place* p = opt.add_places();
    p->set_id(pbtools::toString(topLevelItem(i)->text(findColumn(kColumnId,kColumnTypeRole))));
    p->set_name(pbtools::toString(topLevelItem(i)->text(findColumn(kColumnName,kColumnTypeRole))));
    p->set_data_type(topLevelItem(i)->data(findColumn(kColumnId,kColumnTypeRole),kDataTypeRole).toInt());
  }

  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return opt;
  }

  opt.set_header_state(pbtools::toBytes(header()->saveState().toHex()));

  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    if ( kColumnMeteoParam != head->data(i,kColumnTypeRole).toInt() ) {
      continue;
    }
    if ( isColumnHidden(i) ) {
      continue;
    }

    tablo::Column* c = opt.add_columns();
    c->set_meteo_descr(pbtools::toString(head->data(i,kMeteoDescrNameRole).toString()));

    QByteArray ba = head->data(i,kRoleAlert).toByteArray();
    if ( !ba.isEmpty() ) {
      tablo::ColorAlert alert;
      alert.ParseFromString(pbtools::toBytes(ba));
      if ( alert.condition_size() != 0 ) {
        *opt.add_alert() = alert;
      }
    }

    auto param = head->data(i, kMeteoDescrNameRole).toString();
    if ( "w" == param ) {
      auto w_tr = head->data(i, kRoleAlert2);
      if ( true == w_tr.isValid() ) {
        QByteArray ba_w_tr = head->data(i,kRoleAlert2).toByteArray();
        if ( !ba_w_tr.isEmpty() ) {
          tablo::ColorAlert alert;
          alert.ParseFromString(pbtools::toBytes(ba_w_tr));
          if ( alert.condition_size() != 0 ) {
            *opt.add_alert() = alert;
          }
        }
      }
      auto w_w_ = head->data(i, kRoleAlert3);
      if ( true == w_w_.isValid() ) {
        QByteArray ba_w_w_ = head->data(i,kRoleAlert3).toByteArray();
        if ( !ba_w_w_.isEmpty() ) {
          tablo::ColorAlert alert;
          alert.ParseFromString(pbtools::toBytes(ba_w_w_));
          if ( alert.condition_size() != 0 ) {
            *opt.add_alert() = alert;
          }
        }
      }
    }
    else if ( "V" == param ) {
      auto VV = head->data(i, kRoleAlert2);
      if ( true == VV.isValid() ) {
        QByteArray ba_VV = head->data(i,kRoleAlert2).toByteArray();
        if ( !ba_VV.isEmpty() ) {
          tablo::ColorAlert alert;
          alert.ParseFromString(pbtools::toBytes(ba_VV));
          if ( alert.condition_size() != 0 ) {
            *opt.add_alert() = alert;
          }
        }
      }
    }
    else if ( "h" == param ) {
      auto hgr = head->data(i, kRoleAlert2);
      if ( true == hgr.isValid() ) {
        QByteArray ba_hgr = head->data(i,kRoleAlert2).toByteArray();
        if ( !ba_hgr.isEmpty() ) {
          tablo::ColorAlert alert;
          alert.ParseFromString(pbtools::toBytes(ba_hgr));
          if ( alert.condition_size() != 0 ) {
            *opt.add_alert() = alert;
          }
        }
      }
    }
  }

  return opt;
}

void TabloWidget::restore(const tablo::Settings& opt)
{
  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return;
  }

  clear();
  resetHeader();

  for ( int i=0,isz=opt.columns_size(); i<isz; ++i ) {
    addMeteoparam(pbtools::toQString(opt.columns(i).meteo_descr()));
  }

  if ( opt.has_header_state() ) {
    header()->restoreState(QByteArray::fromHex(pbtools::fromBytes(opt.header_state())));
  }

  for ( int i=0,isz=opt.places_size(); i<isz; ++i ) {
    const tablo::Place& p = opt.places(i);
    QTreeWidgetItem* item = new TabloItem(this);
    item->setText(findColumn(kColumnId,kColumnTypeRole), pbtools::toQString(p.id()));
    item->setData(findColumn(kColumnId,kColumnTypeRole), kDataTypeRole, p.data_type());
    item->setText(findColumn(kColumnName,kColumnTypeRole), pbtools::toQString(p.name()));
  }

  for ( int i=0,isz=opt.alert_size(); i<isz; ++i ) {
    int col = findColumn(pbtools::toQString(opt.alert(i).descrname()), kMeteoDescrNameRole);
    if ( -1 != col ) {
      head->setData(col, kRoleAlert, pbtools::fromBytes(opt.alert(i).SerializeAsString()));

      QString descr = pbtools::toQString(opt.alert(i).descrname());
      if ( "w" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "w_tr" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
          }
          else if ("w_w_" == alert.descrname()) {
            head->setData(col, kRoleAlert3, pbtools::fromBytes(alert.SerializeAsString()));
          }
        }
      }
      else if ( "V" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "VV" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
            break;
          }
        }
      }
      else if ( "h" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "hgr" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
            break;
          }
        }
      }
    }
  }
}

QTreeWidgetItem* TabloWidget::findItem(int column, const QDateTime &dt) const
{
  for ( int i = 0, sz = topLevelItemCount(); i < sz; ++i ) {
    if ( topLevelItem(i)->data(column, kRoleDateTime).toDateTime() == dt ) {
      return topLevelItem(i);
    }
  }

  return nullptr;
}

QTreeWidgetItem* TabloWidget::findItem(int column, const QString& text) const
{
  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    if ( topLevelItem(i)->text(column) == text ) {
      return topLevelItem(i);
    }
  }

  return nullptr;
}

int TabloWidget::findColumn(const QVariant& data, int role) const
{
  QTreeWidgetItem* head = headerItem();
  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    if ( data == head->data(i,role) ) {
      return i;
    }
  }

  return -1;
}

void TabloWidget::addMeteoparam(const QString& param)
{
  if ( param.isEmpty() ) { return; }
  setColumnCount(columnCount() + 1);
  setupHeader(columnCount() - 1, param);
  emit paramChanged();
}

void TabloWidget::setupHeader(int column, const QString& param)
{
  QString name = param;
  QString tooltip = tr("Описание отсутствует.");

  meteodescr::Property prop;
  bool ok = TMeteoDescriptor::instance()->property(param, &prop);

  if ( ok ) {
    tooltip = prop.description;
  }
  if ( "ff" == param) {
    tooltip += "/Порывы";
  }
  QTreeWidgetItem* head = headerItem();
  if ( nullptr != head ) {
    head->setText(column, name);
    head->setData(column, Qt::ToolTipRole, tooltip);
    head->setData(column, kMeteoDescrNameRole, param);
    head->setData(column, kColumnTypeRole, kColumnMeteoParam);

    QVariant v = head->data(column, Qt::FontRole);
    QFont fnt = font();
    if ( v.isValid() && v.canConvert<QFont>() ) {
      fnt = qvariant_cast<QFont>(v);
    }
    QFontMetrics metr(fnt);

    QSize size = metr.boundingRect(name).size();
    size.rwidth() = qRound(size.width() * 1.75);
    if ( column == header()->sortIndicatorSection() ) {
      size.rwidth() += 30;
    }

    head->setData(column, Qt::SizeHintRole, size);
  }
}

QList<ValueDescr> TabloWidget::values() const
{
  QList<ValueDescr> list;

  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) { return list; }

  int columnId = findColumn(kColumnId,kColumnTypeRole);

  for ( int r=0,rows=topLevelItemCount(); r<rows; ++r ) {
    QTreeWidgetItem* item = topLevelItem(r);
    ValueDescr descr;
    descr.placeId = item->text(columnId);
    for ( int c=3,cols=columnCount(); c<cols; ++c ) {
      if ( !item->text(c).isEmpty() ) {
        descr.descr = item->data(c, kRoleRealDescriptor).toString();
        QByteArray arr = item->data(c, kRoleMeteoData).toByteArray();
        std::string str(arr.data(), arr.size());
        meteo::surf::MeteoParamProto param;
        param.ParseFromString(str);
        descr.value = param.value();
        descr.dt = item->data(kColumnTime, kRoleDateTime).toDateTime();
        if( "Nh" == descr.descr ) {
          descr.text = item->text(c);
        }
        list << descr;
      }
    }
  }

  return list;
}

void TabloWidget::setDescription(const QDateTime &dt, const QString &descrStr, const QString &text)
{
  int columnId = findColumn(kColumnTime, kColumnTypeRole);
  QTreeWidgetItem* item = findItem(columnId, dt);
  if ( nullptr == item ) { return; }

  int c = findColumn(descrStr, kMeteoDescrNameRole);
  if ( -1 == c ) { return; }

  item->setData(c, Qt::ToolTipRole, text);
}

void TabloWidget::setDescription(const QString& placeId, const QString& descrStr, const QString& text)
{
  int columnId = findColumn(kColumnId,kColumnTypeRole);
  QTreeWidgetItem* item = findItem(columnId, placeId);
  if ( nullptr == item ) { return; }

  int c = findColumn(descrStr,kMeteoDescrNameRole);
  if ( -1 == c ) { return; }

  item->setData(c, Qt::ToolTipRole, text);
}

bool TabloWidget::loadAlerts()
{
  clearAlerts();

  QString fileName = QDir::homePath() + "/.meteo/meteotablo.conf";

  tablo::Settings opt;
  if ( QFile::exists(fileName) && !TProtoText::fromFile(fileName, &opt) ) {
    error_log << tr("Не удалось загрузить настройки (%1).").arg(fileName);
    return false;
  }

  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return false;
  }

  for ( int i=0,isz=opt.alert_size(); i<isz; ++i ) {
    int col = findColumn(pbtools::toQString(opt.alert(i).descrname()), kMeteoDescrNameRole);
    if ( -1 != col ) {
      head->setData(col, kRoleAlert, pbtools::fromBytes(opt.alert(i).SerializeAsString()));

      QString descr = pbtools::toQString(opt.alert(i).descrname());
      if ( "w" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "w_tr" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
          }
          else if ("w_w_" == alert.descrname()) {
            head->setData(col, kRoleAlert3, pbtools::fromBytes(alert.SerializeAsString()));
          }
        }
      }
      else if ( "V" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "VV" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
            break;
          }
        }
      }
      else if ( "h" == descr ) {
        for ( auto alert : opt.alert()) {
          if ( "hgr" == alert.descrname() ) {
            head->setData(col, kRoleAlert2, pbtools::fromBytes(alert.SerializeAsString()));
            break;
          }
        }
      }
    }
  }

  return true;
}

void TabloWidget::checkAlerts()
{
  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) { return; }

  AlertCheck checker;
  QHash<QString,int> descrs;


  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    if ( isColumnHidden(i) || kColumnMeteoParam != head->data(i,kColumnTypeRole).toInt() ) {
      continue;
    }

    QList<Role> alertRoles;
    alertRoles << kRoleAlert << kRoleAlert2 << kRoleAlert3;

    for ( auto alertRole : alertRoles ) {
      QVariant v = head->data(i, alertRole);
      if ( v.isValid() ) {
        tablo::ColorAlert alert;
        alert.ParseFromString(pbtools::toBytes(v.toByteArray()));
        checker.addAlert(alert);
        descrs.insert(head->data(i,kMeteoDescrNameRole).toString(), i);
      }
    }
  }


  for ( int row=0,rows=topLevelItemCount(); row<rows; ++row ) {
    QTreeWidgetItem* item = topLevelItem(row);
    if ( nullptr == item ) { continue; }

    bool red = false;
    bool yellow = false;

    for ( int col=0,isz=columnCount(); col<isz; ++col ) {
      QString descr = item->data(col,kRoleRealDescriptor).toString();
      QByteArray arr = item->data(col, kRoleMeteoData).toByteArray();
      std::string str(arr.data(), arr.size());
      meteo::surf::MeteoParamProto param;
      param.ParseFromString(str);

      if ( false == red ) {
        QString key = descrs.key(col);
        QVariant var = item->data(descrs[key], kRoleMeteoParamValue);
        tablo::Color color = tablo::kNoColor;
        if ( "Nh" == key  || "h" == key || "C" == key) {
          auto values = var.toString().split("/");
          if ( true == var.isValid() && false == var.isNull() ) {
            for ( auto value : values ) {
              tablo::Color colorbuf = checker.check(descr, value.toFloat());
              if ( tablo::kRed == colorbuf ) {
                color = colorbuf;
                break;
              }
              if (color == tablo::kNoColor ) {
                color = colorbuf;
              }
              else if ( color == tablo::kGreen && colorbuf != tablo::kNoColor) {
                color = colorbuf;
              }
            }
          }
        }
        else if ( true == var.isValid() && false == var.isNull() ) {
          color = checker.check(descr, var);
        }
        switch ( color ) {
          case tablo::kRed:
            red = true;
            for ( int i = 0, sz = columnCount(); i < sz; ++i ) {
              item->setBackground(i, QBrush(QColor(255,0,0)));
              item->setForeground(i, QBrush(QColor(255,255,255)));
            }
            item->setBackground(descrs[key], QBrush(QColor(225,0,0)));
            break;
          case tablo::kYellow:
            yellow = true;
            for ( int i = 0, sz = columnCount(); i < sz; ++i ) {
              item->setBackground(i, QBrush(QColor(255,255,0)));
              item->setForeground(i, QBrush(Qt::black));
            }
            item->setBackground(descrs[key], QBrush(QColor(255,200,0)));
            break;
          case tablo::kGreen:
            if ( false == yellow ) {
              for ( int i = 0, sz = columnCount(); i < sz; ++i ) {
                item->setBackground(i, QBrush(QColor(190,230,200)));
                item->setForeground(i, QBrush(Qt::black));
              }
            }
            break;
          default:
            if ( false == yellow ) {
              for ( int i = 0, sz = columnCount(); i < sz; ++i ) {
                item->setBackground(i, QBrush(Qt::NoBrush));
                item->setForeground(i, QBrush(Qt::black));
              }
            }
        }
      }
      if ( isColumnHidden(col) || kColumnMeteoParam != head->data(col,kColumnTypeRole).toInt() ) {
        continue;
      }
    }
  }
}

void TabloWidget::clearAlerts()
{
  QTreeWidgetItem* head = headerItem();
  if ( nullptr == head ) {
    return;
  }

  for ( int i=0,isz=columnCount(); i<isz; ++i ) {
    head->setData(i, kRoleAlert, QVariant());
  }
}

void TabloWidget::slotAddPlace()
{
  StationDlg* dlg = qobject_cast<StationDlg*>(sender());
  if ( nullptr == dlg ) { return; }

  meteo::sprinf::Station st = dlg->getStation();

  QString idx = st.has_cccc() ? pbtools::toQString(st.cccc()) : QString::number(st.index());
  QString name = st.name().has_rus() ? pbtools::toQString(st.name().rus()) : pbtools::toQString(st.name().international());

  for ( int i=0,isz=topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = topLevelItem(i);
    if ( item->text(kColumnId) == idx && item->data(kColumnId,kDataTypeRole).toInt() == st.data_type() ) {
      dlg->setStatus(tr("Станция %2 уже добавлена в список").arg(idx), 7);
      return;
    }
  }

  QTreeWidgetItem* item = new TabloItem(this);
  item->setText(findColumn(kColumnId,kColumnTypeRole), idx);
  item->setData(findColumn(kColumnId,kColumnTypeRole), kDataTypeRole, st.data_type());
  item->setText(findColumn(kColumnName,kColumnTypeRole), name);

  adjustColumns();

  emit placeAdded();
  emit settingsChanged();
}

void TabloWidget::contextMenuEvent(QContextMenuEvent* event)
{
  if ( QContextMenuEvent::Mouse != event->reason() ) {
    return;
  }
  QTreeWidgetItem* item = itemAt(event->pos());
  QMenu menu;
  QAction* copyValAct = nullptr;
  QAction* copyLineAct = nullptr;
  if ( nullptr != item ) {
    copyValAct = menu.addAction(QIcon(":/meteo/icons/tools/copy.png"), tr("Копировать"));
    copyLineAct = menu.addAction(tr("Копировать строку"));
    menu.addSeparator();
  }
  QAction* addPlaceAct = nullptr;
  QAction* delPlaceAct = nullptr;
  QAction* addParamAct = nullptr;
  QAction* delParamAct = nullptr;
  QAction* setParamAct = nullptr;
  QAction* setAlertAct = nullptr;
  int column = columnAt(event->x());
  QTreeWidgetItem* head = headerItem();
  if( false == isArchive_){
    addPlaceAct = menu.addAction(tr("Добавить пункт..."));
    if ( nullptr != item ) {
      QString name = tr("Удалить пункт '%1 - %2'").arg(item->text(kColumnId), item->text(kColumnName));
      delPlaceAct = menu.addAction(QIcon(":/meteo/icons/delete-16.png"), name);
      menu.addSeparator();
    }
    addParamAct = menu.addAction(tr("Добавить метеопараметр..."));
    QString paramName;
    if ( nullptr != head ) {
      paramName = head->data(column, kMeteoDescrNameRole).toString();
    }
    if ( nullptr != head && kColumnMeteoParam == head->data(column,kColumnTypeRole).toInt() ) {
      QString name = tr("Удалить метеопараметр '%1'").arg(head->text(column));
      delParamAct = menu.addAction(QIcon(":/meteo/icons/delete-16.png"), name);
      menu.addSeparator();
      setParamAct = menu.addAction(tr("Изменить метеопараметр..."));
    }
    setAlertAct = menu.addAction(tr("Параметры цветовой индикации..."));
  }
  QAction* act = menu.exec(event->globalPos());
  if ( nullptr == act ) { return; }
  if ( act == addPlaceAct ) {
    StationDlg* dlg = new StationDlg(this);
    setCursor(Qt::WaitCursor);
    dlg->init();
    unsetCursor();
    QObject::connect( dlg, SIGNAL(addClicked()), SLOT(slotAddPlace()) );
    dlg->exec();
    dlg->deleteLater();
  }
  else if ( act == delPlaceAct ) {
    delete item;
    emit settingsChanged();
  }
  else if ( act == addParamAct ) {
    addMeteoparam(getMeteoparam());
  }
  else if ( act == delParamAct ) {
    QTreeWidgetItem* head = headerItem();
    if ( nullptr != head ) {
      head->setData(column, kMeteoDescrNameRole, -9999);
    }
    setColumnHidden(column, true);
  }
  else if ( act == setParamAct ) {
    QString param = getMeteoparam();
    if ( !param.isEmpty() ) {
      setupHeader(column, param);
      emit paramChanged();
    }
  }
  else if ( act == copyValAct ) {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(item->text(column));
  }
  else if ( act == copyLineAct ) {
    QClipboard* clipboard = QApplication::clipboard();
    QStringList list;
    for ( int i=0,isz=columnCount(); i<isz; ++i ) {
      list << item->text(i);
      QString tooltip = item->data(i, Qt::ToolTipRole).toString();
      if ( !tooltip.isEmpty() ) {
        list << "(" + tooltip + ")";
      }
    }
    clipboard->setText(list.join("\t"));
  }
  else if ( act == setAlertAct ) {
    ColorAlertDlg dlg(this);
    QByteArray ba = head->data(column, kRoleAlert).toByteArray();
    tablo::ColorAlert alert;
    if ( !ba.isEmpty() ) {
      alert.ParseFromString(pbtools::toBytes(ba));
    }
    else {
      alert.set_descrname(pbtools::toString(head->data(column,kMeteoDescrNameRole).toString()));
    }
    setCursor(Qt::WaitCursor);
    dlg.loadSettings();
    dlg.setCurrent(pbtools::toQString(alert.descrname()));
    unsetCursor();

    if ( QDialog::Accepted == dlg.exec() ) {
      loadAlerts();
      checkAlerts();
      emit settingsChanged();
    }
  }
  adjustColumns(2);
}

void TabloWidget::mousePressEvent(QMouseEvent *event)
{
  if ( false == isSortingEnabled() && false == isArchive_) {
    movingItem_ = this->itemAt(event->pos());
  }
  QTreeWidget::mousePressEvent(event);
}

void TabloWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if ( nullptr != movingItem_ && true == moved_ ) {
    emit itemMoved();
  }
  movingItem_ = nullptr;
  moved_ = false;
  QTreeWidget::mouseReleaseEvent(event);
}

void TabloWidget::mouseMoveEvent(QMouseEvent *event)
{
  if ( nullptr == movingItem_ || movingItem_ == itemAt(event->pos()) ) {
    return;
  }

  moved_ = true;
  takeTopLevelItem(indexOfTopLevelItem(movingItem_));
  auto newIndex = indexOfTopLevelItem(itemAt(event->pos()));
  if ( -1 == newIndex ) {
    newIndex = topLevelItemCount();
  }
  insertTopLevelItem(newIndex, movingItem_);
  QTreeWidget::mouseMoveEvent(event);
}

void TabloWidget::resetHeader()
{
  setColumnCount(0);
  setColumnCount(3);

  QTreeWidgetItem* head = headerItem();
  head->setText(0, tr("Код"));
  head->setData(0, kColumnTypeRole, kColumnId);
  head->setText(1, tr("Название"));
  head->setData(1, kColumnTypeRole, kColumnName);
  head->setText(2, tr("Срок"));
  head->setData(2, kColumnTypeRole, kColumnTime);
  head->setData(2, Qt::ToolTipRole, tr("Время наблюдения (UTC)"));

  header()->setSectionsMovable(true);
  header()->setStretchLastSection(true);
}

QString TabloWidget::getMeteoparam()
{
  DescrSelect dlg(this);
  auto list = descriptors();
  list.append("N");
  list.append("hgr");
  list.append("CH");
  list.append("CM");
  list.append("CL");
  list.append("Cn");
  dlg.loadDescriptors(list);

  if ( QDialog::Accepted == dlg.exec() && dlg.selectedDescr().has_descr() ) {
    return pbtools::toQString(dlg.selectedDescr().name());
  }

  return QString();
}

void TabloWidget::setRowForeground(int row, const QBrush& brush)
{
  QTreeWidgetItem* i = topLevelItem(row);
  if ( nullptr == i ) {
    return;
  }

  for ( int c=0,csz=columnCount(); c<csz; ++c ) {
    i->setForeground(c, brush);
  }
}

void TabloWidget::setRowBackground(int row, const QBrush& brush)
{
  QTreeWidgetItem* i = topLevelItem(row);
  if ( nullptr == i ) {
    return;
  }

  for ( int c=0,csz=columnCount(); c<csz; ++c ) {
    i->setBackground(c, brush);
  }
}

void TabloWidget::sortMeteodataByDt(surf::DataReply *resp)
{
  auto sortDataBydt = [](const surf::MeteoDataProto& a, const surf::MeteoDataProto& b)
  {
    QDateTime dta = QDateTime::fromString(QString::fromStdString(a.dt()), Qt::ISODate);
    QDateTime dtb = QDateTime::fromString(QString::fromStdString(b.dt()), Qt::ISODate);
    return dta > dtb; //больше, чтобы отсортировать от настоящего к прошлому
  };
  auto datas = resp->mutable_meteodata_proto();
  std::sort(datas->begin(), datas->end(), sortDataBydt);
}

}



