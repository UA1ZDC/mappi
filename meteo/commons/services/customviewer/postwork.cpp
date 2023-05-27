#include "customviewerservicehandler.h"
#include "postwork.h"

#include <meteo/commons/global/dateformat.h>
#include <qobject.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsondocument.h>

namespace meteo {

static QString textFromMenuForValue (const proto::ViewerConfig* conf, const QString& column, const QString& value){
  for (auto colConfig: conf->column() ){
    if ( 0 != column.compare(QString::fromStdString(colConfig.name())) ){
      continue;
    }
    for (auto menuItem: colConfig.menu() ){
      if ( 0 != value.compare(QString::fromStdString(menuItem.value())) ){
        continue;
      }
      return QString::fromStdString(menuItem.display_value());
    }
  }
  warning_log << QObject::tr("Значение %1 не найдено в меню для колонки %2")
                 .arg(value)
                 .arg(column);
  return value;
};

static QString iconFromMenuForValue (const proto::ViewerConfig* conf, const QString& column, const QString& value){
  for (auto colConfig: conf->column() ){
    if ( 0 != column.compare(QString::fromStdString(colConfig.name())) ){
      continue;
    }
    for (auto menuItem: colConfig.menu() ){
      if ( 0 != value.compare(QString::fromStdString(menuItem.value())) ){
        continue;
      }
      return QString::fromStdString(menuItem.icon());
    }
  }
  warning_log << QObject::tr("Значение %1 не найдено в меню для колонки %2")
                 .arg(value)
                 .arg(column);
  return value;
};

void defaultPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& targetColumn, proto::CustomViewerField* out)
{
  Q_UNUSED(conf);
  QString value = row.value(targetColumn, QString());  
  out->set_name(targetColumn.toStdString());
  out->set_value(value.toStdString());
  out->set_display_value(value.toStdString());
}

void defaultDatetimeWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& col, proto::CustomViewerField* out)
{
  auto type = CustomViewerServiceHandler::columnType(conf, col);
  if ( proto::ViewerType::kDateTime != type ){
    error_log << QObject::tr("Ошибка, этот обработчик предназначе для работы с колонками типа datetime!");
    defaultPostWorker(conf, row, col, out);
    return;
  }

  auto value = row[col];

  QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
  out->set_name(col.toStdString());
  out->set_value(value.toStdString());

  if ( true == dt.isValid() ){
    auto dtString = meteo::dateToHumanFull(dt);
    out->set_display_value(dtString.toStdString());
  }
  else {
    warning_log << QObject::tr("Неправильный формат даты в колонке %1, %2")
                   .arg(col)
                   .arg(value);
    out->set_display_value(value.toStdString());
  }
}

void msgviewerPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& column, proto::CustomViewerField* out)
{
  auto value = row[column];
  defaultPostWorker(conf, row, column, out);  


  if ( 0 == column.compare("type") ) {    
    auto icon = iconFromMenuForValue(conf, column, value);
    auto text = textFromMenuForValue(conf, column, value);
    out->set_icon(icon.toStdString());
    out->set_tooltip(text.toStdString());
    out->clear_display_value();
  }


  if ( 0 == column.compare("bin") ) {
    auto text = textFromMenuForValue(conf, column, value);
    auto icon = iconFromMenuForValue(conf, column, value);
    out->set_tooltip(text.toStdString());
    out->set_icon(icon.toStdString());
    out->clear_display_value();
  }


  if ( 0 == column.compare("decoded") ) {
    auto text = textFromMenuForValue(conf, column, value);
    auto icon = iconFromMenuForValue(conf, column, value);
    out->set_tooltip(text.toStdString());
    out->set_icon(icon.toStdString());
    out->clear_display_value();
  }

  if ( 0 == column.compare("external") ){    
    auto text = textFromMenuForValue(conf, column, value);
    auto icon = iconFromMenuForValue(conf, column, value);
    out->set_tooltip(text.toStdString());
    out->set_icon(icon.toStdString());
    out->clear_display_value();
  }

  if ( 0 == column.compare("format") ){
    auto text = textFromMenuForValue(conf, column, value);
    out->set_display_value(text.toStdString());
  }
}

void msgviewerRoutePostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& column, proto::CustomViewerField* out) {
  defaultPostWorker(conf, row, column, out);
  QString value = row[column];

  QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8());
  if ( true == doc.isNull() ){
    return;
  }
  auto obj = doc.object();
  QStringList routes;
  for (auto index: obj.keys()) {
    auto routeObject = obj[index].toObject();
    static auto kSrc = QObject::tr("src");
    static auto kDst = QObject::tr("dst");
    if ( true == routeObject.keys().contains(kSrc) &&
         true == routeObject.keys().contains(kDst) ){
      auto src = routeObject[kSrc].toString();
      auto dsts = routeObject[kDst].toArray();
      QStringList stringDests;
      for ( auto dst: dsts ){
        stringDests << dst.toString();
      }
      switch (stringDests.size()) {
      case 0:
        routes << QObject::tr("из %1").arg(src);
        break;
      case 1:
        routes << QObject::tr("из %1 в %2").arg(src).arg(stringDests.first());
        break;
      default:
        routes << QObject::tr("из %1 в [ %2 ]").arg(src).arg(stringDests.join(","));
      }

    }
  }

  out->set_display_value(routes.join(";").toStdString());
}

void journalPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& column, proto::CustomViewerField* out)
{
  auto value = row[column];
  if ( 0 == column.compare("priority") ){
    auto text = textFromMenuForValue(conf, column, value);
    auto icon = iconFromMenuForValue(conf, column, value);

    out->set_name(column.toStdString());
    out->set_tooltip(text.toStdString());
    out->set_icon(icon.toStdString());
    return;
  }
}


}
