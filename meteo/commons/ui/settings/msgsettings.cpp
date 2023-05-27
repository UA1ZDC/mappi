#include "msgsettings.h"
#include "ui_msgsettings.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>

#include <commons/textproto/tprototext.h>

#include <QtWidgets>

namespace {
  const QString settingsHumanName() { return QString::fromUtf8("Журналирование"); }
  //QString redledIconPath() { return ":/novost/icons/ledred.png"; }
  //QString greenledIconPath() { return ":/novost/icons/ledgreen.png"; }
  QString yesStatusIconPath() { return ":/meteo/icons/ok.png"; }
  QString noStatusIconPath() { return ":/meteo/icons/process_error.png"; }
}

namespace meteo {

MsgSettings::MsgSettings(QWidget* parent) :
  SettingsWidget(parent),
  ui_(new Ui::MsgSettings)
{
  setObjectName(::settingsHumanName());
  ui_->setupUi(this);
  ui_->tbl->setIconSize(QSize(24,24));
  ui_->tbl->resizeColumnsToContents();
  ui_->tbl->installEventFilter(this);
  ui_->tbl->viewport()->installEventFilter(this);
}

MsgSettings::~MsgSettings()
{
  delete ui_;
  ui_ = nullptr;
}

void MsgSettings::load()
{  
  if ( true == global::Settings::instance()->load() ) {
    auto msg = global::Settings::instance()->displayMessageConfig();
    setMsg(msg);
  }
  else {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка"), QString::fromUtf8("Не удалось получить параметры регистрации и светографической индикации событий"), QMessageBox::Ok);
  }
}

QPixmap MsgSettings::setCellIcon(bool status) const
{
  QPixmap result;
  if (status == true) {
   result.load(::yesStatusIconPath());
  }
  else {
   result.load(::noStatusIconPath());
  }
  return result;
}

void MsgSettings::setCellPixmap(const QPixmap& pixmap, int row, int column)
{
  QPixmap resPix = pixmap.scaled(17, 17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  QLabel* lb = new QLabel;
  lb->setPixmap(resPix);
  lb->setAlignment(Qt::AlignCenter);
  ui_->tbl->setCellWidget(row, column, lb);
}


bool MsgSettings::setMsg(const meteo::settings::MsgSettings &msg)
{
  if (false == msg.IsInitialized())
  {
    error_log << QObject::tr("Ошибка: сообщение не инициализировано");
    return false;
  }

  QTableWidgetItem* item = new QTableWidgetItem();
  ui_->tbl->setItem(kDebug, kReg, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kDebug, kShow, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kInfo, kReg, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kInfo, kShow, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kWarning, kReg, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kWarning, kShow, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kError, kReg, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kError, kShow, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kCritical, kReg, item);

  item = new QTableWidgetItem();
  ui_->tbl->setItem(kCritical, kShow, item);

  setRowIcons(msg.debug(), kDebug);
  setRowIcons(msg.info(), kInfo);
  setRowIcons(msg.warning(), kWarning);
  setRowIcons(msg.error(), kError);
  setRowIcons(msg.critical(), kCritical);
  return true;
}

void MsgSettings::setRowIcons(meteo::settings::RegLevel level, int row) //устанавливаем иконки для всей строки для уровня
{
  if (meteo::settings::kNone == level)
  {
    ui_->tbl->model()->setData(ui_->tbl->model()->index(row,kName),level,Qt::UserRole);
    setCellPixmap(setCellIcon(false), row, kReg);
    setCellPixmap(setCellIcon(false), row, kShow);
  }
  else if (meteo::settings::kReg == level)
  {
    ui_->tbl->model()->setData(ui_->tbl->model()->index(row,kName),level,Qt::UserRole);
    setCellPixmap(setCellIcon(true), row, kReg);
    setCellPixmap(setCellIcon(false), row, kShow);
  }
  else if (meteo::settings::kShow == level)
  {
    ui_->tbl->model()->setData(ui_->tbl->model()->index(row,kName),level,Qt::UserRole);
    setCellPixmap(setCellIcon(true), row, kReg);
    setCellPixmap(setCellIcon(true), row, kShow);
  }
}

bool MsgSettings::eventFilter(QObject* object, QEvent* event)
{
  if (object == ui_->tbl->viewport()) {
    switch (event->type()) {
      case QEvent::MouseButtonDblClick: {
          const QPoint& pos = static_cast<QMouseEvent*>(event)->pos();
          QTableWidgetItem* current = ui_->tbl->itemAt(pos);
          if (current != 0 && current->column() == kReg) {
            QModelIndex index = ui_->tbl->model()->index(current->row(),kName);
            if (!index.isValid()) { break; }
            meteo::settings::RegLevel level = (meteo::settings::RegLevel) ui_->tbl->model()->data(index, Qt::UserRole).toInt();
            int row = current->row();
            meteo::settings::RegLevel newlevel;
            if ((meteo::settings::kReg == level) || (meteo::settings::kShow == level))
            {
              newlevel = meteo::settings::kNone;
            }
            else if (meteo::settings::kNone == level)
            {
              newlevel = meteo::settings::kReg;
            }
            setRowIcons(newlevel,row);
            slotChangeState();
          }
          if (current != 0 && current->column() == kShow) {
            QModelIndex index = ui_->tbl->model()->index(current->row(),kName);
            if (!index.isValid()) { break; }
            meteo::settings::RegLevel level = (meteo::settings::RegLevel) ui_->tbl->model()->data(index,Qt::UserRole).toInt();
            int row = current->row();
            meteo::settings::RegLevel newlevel;
            if (meteo::settings::kShow == level)
            {
              newlevel = meteo::settings::kReg;
            }
            else if ((meteo::settings::kReg == level) || (meteo::settings::kNone == level))
            {
              newlevel = meteo::settings::kShow;
            }
            setRowIcons(newlevel,row);
            slotChangeState();
          }
       }
      break;
    default:
      break;
   }
 }
  return SettingsWidget::eventFilter(object, event);
}

meteo::settings::MsgSettings MsgSettings::msg()
{
  meteo::settings::MsgSettings msg;
  msg.set_debug(debug());
  msg.set_info(info());
  msg.set_warning(warning());
  msg.set_error(error());
  msg.set_critical(critical());
  return msg;
}

meteo::settings::RegLevel MsgSettings::level(meteo::MsgType type)
{
  QModelIndex index = ui_->tbl->model()->index(type,kName);
  meteo::settings::RegLevel level = (meteo::settings::RegLevel) ui_->tbl->model()->data(index,Qt::UserRole).toInt();
  return level;
}

meteo::settings::RegLevel MsgSettings::debug()
{
 return level(kDebug);
}

meteo::settings::RegLevel MsgSettings::info()
{
  return level(kInfo);
}


meteo::settings::RegLevel MsgSettings::warning()
{
  return level(kWarning);
}


meteo::settings::RegLevel MsgSettings::error()
{
  return level(kError);
}


meteo::settings::RegLevel MsgSettings::critical()
{
return level(kCritical);
}

void MsgSettings::save()
{
  auto configName =  global::Settings::instance()->displayMessageConfigName();
  auto config = this->msg();
  auto data = TProtoText::toText(config);

  auto path = QObject::tr("%1/%2")
      .arg(MnCommon::etcPath())
      .arg(configName);
  QFile ofile(path);
  if ( false == ofile.open(QIODevice::WriteOnly)){
    error_log.msgBox() << meteo::msglog::kFileWriteFailed
                          .arg(path)
                          .arg(ofile.errorString());
    return;
  }

  if ( ofile.write(data) != data.size() ){
    error_log << meteo::msglog::kFileWriteFailed
                 .arg(path)
                 .arg(ofile.errorString());
    return;
  }
  changed_ = false;
}

void MsgSettings::slotChangeState()
{
  changed_ = true;
  emit changed();
}



} // meteo
