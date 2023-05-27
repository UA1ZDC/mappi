#include "tablowindow.h"
#include "ui_tablowindow.h"
#include "ui_savepreset.h"

#include <qfile.h>
#include <qfilesystemwatcher.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>


#include <meteo/commons/global/global.h>

namespace meteo {

namespace meteotablo {
QString confFile() { return QDir::homePath() + "/.meteo/meteotablo.conf"; }
QString presetsFile() { return QDir::homePath() + "/.meteo/meteotablo.presets.conf";}
}

TabloWindow::TabloWindow(QWidget *parent) :
  QMainWindow(parent),
  ui_(new Ui::TabloWindow)
{
  loading_ = false;

  ui_->setupUi(this);
  setWindowFlags(Qt::Dialog);

  watcher_ = new QFileSystemWatcher(this);
  watcher_->addPath(meteotablo::confFile());
  QObject::connect( watcher_, SIGNAL(fileChanged(QString)), SLOT(slotLoadSettings()) );
  QObject::connect( watcher_, SIGNAL(fileChanged(QString)), SLOT(slotUpdateData()) );

  QTimer* timer = new QTimer(this);
  timer->setInterval(90000);
  timer->start();
  QObject::connect( timer, SIGNAL(timeout()), SLOT(slotUpdateData()) );

  QObject::connect( ui_->tablo, SIGNAL(itemMoved()), SLOT(slotSavePreset()) );
  QObject::connect( ui_->tablo, SIGNAL(placeAdded()), SLOT(slotUpdateData()) );
  QObject::connect( ui_->tablo, SIGNAL(placeAdded()), SLOT(slotSavePreset()) );
  QObject::connect( ui_->tablo, SIGNAL(paramChanged()), SLOT(slotUpdateData()) );
  QObject::connect( ui_->tablo, SIGNAL(settingsChanged()), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->tablo, SIGNAL(settingsChanged()), SLOT(slotUpdateData()) );
  QObject::connect( ui_->tablo, SIGNAL(settingsChanged()), SLOT(slotSavePreset()) );
  QObject::connect( ui_->tablo, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotArchive(QTreeWidgetItem*, int)) );

  QObject::connect( ui_->btnselectdt, &CalendarButton::dtChanged, this, &TabloWindow::slotDtChanged );
  QObject::connect( ui_->btncleardt, &QAbstractButton::clicked, this, &TabloWindow::slotClearDtClicked );
  QObject::connect( ui_->savePresetBtn, SIGNAL(clicked(bool)), SLOT(slotAddPreset()));
  QObject::connect( ui_->deletePresetBtn, SIGNAL(clicked(bool)), SLOT(slotDeletePreset()));
  QObject::connect( ui_->presetCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotPresetChange()));
  QObject::connect( ui_->sortButton, SIGNAL(clicked(bool)), this, SLOT(slotOnSortButton()));

  ui_->savePresetBtn->setIcon(QIcon(":/meteo/icons/plus.png"));
  ui_->deletePresetBtn->setIcon(QIcon(":/meteo/icons/minus.png"));
  ui_->sortButton->setIcon(QIcon(":/meteo/icons/sort.png"));

  slotLoadSettings();
  loadPresets();
  ui_->tablo->setSortingEnabled(ui_->sortButton->isChecked());
}

TabloWindow::~TabloWindow()
{
  delete ui_;
}

void TabloWindow::setVisible(bool visible)
{
  QMainWindow::setVisible(visible);

  slotUpdateData();

  if ( !visible ) {
    save();
  }
}

void TabloWindow::slotUpdateData()
{
  if ( !isVisible() ) {
    return;
  }

  if ( loading_ ) {
    return;
  }
  BoolLocker lock(&loading_);


  ui_->tablo->adjustColumns(2);

  QDateTime beg = QDateTime::currentDateTimeUtc();
  beg.setTimeSpec( Qt::LocalTime );
  if ( true == dt_selected_.isValid() ) {
    beg = dt_selected_;
  }
  ui_->btnselectdt->setDatetime(beg);
  QDateTime end(beg);
  beg = beg.addSecs( -24*60*60 );

  surf::DataRequest req;
  req.set_date_start(pbtools::toString( beg.toString( Qt::ISODate ) ) );
  req.set_date_end(pbtools::toString( end.toString( Qt::ISODate ) ) );
  req.set_type_level(1);
  req.set_level_p(0);
  req.set_query_type(surf::kLastValue);
  req.add_type(surf::kSynopFix);
  req.add_type(surf::kAirportRegType);
  req.add_type(surf::kAerodrome);

  QStringList descrs = ui_->tablo->descriptors();
  for ( int i=0,isz=descrs.size(); i<isz; ++i ) {
    req.add_meteo_descrname(pbtools::toString(descrs.at(i)));
  }

  QList<int> stations = ui_->tablo->stations();
  for ( int i=0,isz=stations.size(); i<isz; ++i ) {
    req.add_station(QString::number(stations.at(i)).toStdString());
  }
  QStringList airports = ui_->tablo->airports();
  foreach ( const QString& cccc, airports ) {
    req.add_station(pbtools::toString(cccc));
  }
  if ( 0 == req.station_size() ) {
    setStatus( QObject::tr("Выберите станции") );
    return;
  }
  req.set_as_proto(true);

  rpc::Channel* channel = global::serviceChannel(settings::proto::kSrcData);
  if ( nullptr == channel ) {
    setStatus( QObject::tr("Ошибка: отсутствует подключение к сервису данных.") );
    return;
  }
  qApp->setOverrideCursor(Qt::WaitCursor);

  setStatus( QObject::tr("Запрос данных..."));
  surf::DataReply* resp = channel->remoteCall(&::meteo::surf::SurfaceService::GetMeteoDataByDay, req, 30000);
  channel->disconnect();
  delete channel;

  if ( nullptr == resp ) {
    error_log << QObject::tr("--> ответ не получен");
    setStatus(tr("Ответ от сервиса данных не получен."));
    qApp->restoreOverrideCursor();
    return;
  }

  ui_->tablo->setData(resp);
  ui_->tablo->checkAlerts();
  ui_->tablo->adjustColumns(2);
  delete resp;

  setDescription(ui_->tablo);
  QString cur_date = QObject::tr("%1").arg( end.toString(Qt::ISODate).replace("T", " "));
  QString st_date = QObject::tr("%1").arg( beg.toString(Qt::ISODate).replace("T", " "));
  setWindowTitle(QString("Метеорологические данные за период с %1 по %2").arg(st_date).arg(cur_date));
  qApp->restoreOverrideCursor();
  setStatus( cur_date );
}

void TabloWindow::slotSaveSettings()
{
  save();
}

void TabloWindow::slotLoadSettings()
{
  loadSettings(ui_->tablo);
}

void TabloWindow::slotDtChanged( const QDateTime& dt )
{
  dt_selected_ = dt;
  loadPresets();
  //slotUpdateData();
}

void TabloWindow::slotClearDtClicked()
{
  if ( false == dt_selected_.isValid() ) {
    return;
  }
  dt_selected_ = QDateTime();
  loadPresets();
//  slotUpdateData();
}

void TabloWindow::slotArchive(QTreeWidgetItem *item, int column)
{
  Q_UNUSED(column);

  rpc::Channel* ch = nullptr;
  surf::DataReply* resp = nullptr;
  auto errFunc = [this, ch, resp]()
  {
    setStatus( QObject::tr("Ошибка: не удалось получить данные по выбранной станции") );
    if ( nullptr != ch ) {
      delete ch;
    }
    if ( nullptr != resp ) {
      delete resp;
    }
    qApp->restoreOverrideCursor();
    return;
  };

  qApp->setOverrideCursor(Qt::WaitCursor);
  QString station = item->text(0);

  surf::DataRequest req;
  req.add_station(station.toStdString());
  QDateTime curDate;
  if ( true == dt_selected_.isValid() ) {
    curDate = dt_selected_;
  }
  else {
    curDate = QDateTime::currentDateTimeUtc();
  }
  req.set_date_end(curDate.toString(Qt::ISODate).toStdString());

  curDate = curDate.addDays(-1);
  req.set_date_start(curDate.toString(Qt::ISODate).toStdString());
  req.set_type_level(1);
  req.set_level_p(0);
  req.set_query_type(surf::kLastValue);
  req.add_type(surf::kSynopFix);
  req.add_type(surf::kAirportRegType);
  req.add_type(surf::kAerodrome);
  req.set_as_proto(true);

  QStringList descrs = ui_->tablo->descriptors();
  for ( int i=0,isz=descrs.size(); i<isz; ++i ) {
    req.add_meteo_descrname(pbtools::toString(descrs.at(i)));
  }

  QDateTime dte = QDateTime::fromString(QString::fromStdString(req.date_end()), Qt::ISODate);
  QDateTime dts = QDateTime::fromString(QString::fromStdString(req.date_start()), Qt::ISODate);

  ch = global::serviceChannel(settings::proto::kSrcData);
  if ( nullptr == ch ) {
    return errFunc();
  }

  resp = ch->remoteCall(&::meteo::surf::SurfaceService::GetMeteoDataOnStation, req, 30000);
  if ( nullptr == resp ) {
    return errFunc();
  }
  if( 0 == resp->meteodata_proto_size() ){
    return errFunc();
  }
  archive_ = new TabloWidget(this);
  archive_->setArchive(true);
  archive_->setWindowTitle(QString("Данные за период с %1 по %2 по станции %3").
                   arg(dts.toString(Qt::ISODate).replace("T", " ").replace("Z", " ") ).
                   arg(dte.toString(Qt::ISODate).replace("T", " ").replace("Z", " ") ).
                   arg(QString::fromStdString(resp->meteodata_proto(0).station_info().name_rus())));
  loadSettings(archive_);
  for ( int i = 0, sz = archive_->topLevelItemCount(); i < sz; ++i ) {
    delete archive_->topLevelItem(archive_->topLevelItemCount() - 1);
  }
  archive_->hideColumn(TabloWidget::kColumnId);
  archive_->hideColumn(TabloWidget::kColumnName);
  archive_->setData(resp);
  setDescription(archive_);
  archive_->checkAlerts();
  archive_->setWindowFlags( archive_->windowFlags() | Qt::WindowType::Dialog );
  archive_->setAttribute(Qt::WA_DeleteOnClose);
  archive_->resize(ui_->tablo->size());
  archive_->show();
  delete ch; ch = nullptr;
  delete resp; resp = nullptr;
  qApp->restoreOverrideCursor();
}

void TabloWindow::slotAddPreset()
{
  tablo::Settings preset;

  QDialog* dlg = new QDialog();
  Ui::SavePreset ui;
  ui.setupUi(dlg);
  for ( auto p : presets_.preset() ) {
    ui.comboBox->addItem(QString::fromStdString(p.name()));
  }

  QObject::connect(ui.lineEdit, &QLineEdit::textEdited, this, [ui,this](QString text){
    for ( auto p : presets_.preset() ) {
      if ( text == QString::fromStdString(p.name()) ) {
        ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
        ui.buttonBox->button(QDialogButtonBox::Ok)->setToolTip(
              QObject::tr("Предустановка метеотабло с таким именем уже существует."));
        break;
      }
      else {
        ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
        ui.buttonBox->button(QDialogButtonBox::Ok)->setToolTip(
              QObject::tr("Сохранить набор станций"));
      }
    }
  });

  int res = dlg->exec();
  QString name;
  if ( QDialog::Accepted == res ) {
    name = ui.lineEdit->text();

    auto newP = presets_.add_preset();

    if ( ui.comboBox->currentIndex() > 0 ) {
      newP->CopyFrom(presets_.preset(ui.comboBox->currentIndex() - 1));
    }
    newP->set_name(name.toStdString());

    if ( false == TProtoText::toFile(presets_, meteotablo::presetsFile()) ) {
      error_log << tr("Не удалось сохранить пользовательские предустановки (%1).").arg(meteotablo::presetsFile());
    }

    loadPresets();
  }
  delete dlg;
  ui_->presetCombo->setCurrentIndex(ui_->presetCombo->count() - 1);
}

void TabloWindow::slotDeletePreset()
{
  tablo::Settings preset;
  QByteArray ba = ui_->presetCombo->currentData().toByteArray();
  preset.ParseFromString(pbtools::toBytes(ba));

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(nullptr, "Внимание",
                                 QObject::tr("Удалить набор станций %1?")
                                 .arg(QString::fromStdString(preset.name())));
  if ( reply == QMessageBox::StandardButton::No ) {
    return;
  }

  tablo::Presets tmp;
  for( auto p : presets_.preset() ) {
    if ( p.name() == preset.name() ) {
      continue;
    }
    auto tp = tmp.add_preset();
    tp->CopyFrom(p);
  }
  presets_.Clear();
  presets_.CopyFrom(tmp);

  if ( false == TProtoText::toFile(presets_, meteotablo::presetsFile()) ) {
    error_log << tr("Не удалось сохранить пользовательские предустановки (%1).").arg(meteotablo::presetsFile());
  }

  loadPresets();
}

void TabloWindow::slotSavePreset()
{
  for ( int i =0, sz = presets_.preset_size(); i < sz; ++i ) {
    if ( presets_.preset(i).name() == ui_->presetCombo->currentText().toStdString() ) {
      auto opt = ui_->tablo->save();
      presets_.mutable_preset(i)->mutable_places()->CopyFrom(opt.places());
    }
  }

  if ( false == TProtoText::toFile(presets_, meteotablo::presetsFile()) ) {
    error_log << tr("Не удалось сохранить пользовательские предустановки (%1).").arg(meteotablo::presetsFile());
  }
  int index = ui_->presetCombo->currentIndex();
  loadPresets();
  ui_->presetCombo->setCurrentIndex(index);
}

void TabloWindow::slotPresetChange()
{
  tablo::Settings preset;
  QByteArray ba = ui_->presetCombo->currentData().toByteArray();
  preset.ParseFromString(pbtools::toBytes(ba));
  auto opt = ui_->tablo->save();
  opt.mutable_places()->CopyFrom(preset.places());
  opt.set_sorting(ui_->sortButton->isChecked());

  if ( false == TProtoText::toFile(opt, meteotablo::confFile())) {
    error_log << QObject::tr("Не удалось изменить набор станций");
  }
}

void TabloWindow::slotOnSortButton()
{
  ui_->tablo->setSortingEnabled(ui_->sortButton->isChecked());
  if ( false == ui_->sortButton->isChecked() ) {
    loadPresets();
  }
}

void TabloWindow::setStatus(const QString& text)
{
  ui_->statusLabel->setText(text);
}

void TabloWindow::save()
{
  tablo::Settings opt = ui_->tablo->save();
  opt.set_geometry(pbtools::toBytes(saveGeometry().toHex()));
  opt.set_sorting(ui_->sortButton->isChecked());

  if ( !TProtoText::toFile(opt, meteotablo::confFile()) ) {
    error_log << tr("Не удалось сохранить настройки (%1).").arg(meteotablo::confFile());
  }
}

void TabloWindow::loadPresets()
{
  if ( false == QFile::exists(meteotablo::presetsFile()) ) {
    auto preset = presets_.add_preset();
    preset->set_name(QObject::tr("Набор станций 1").toStdString());
    auto opt = ui_->tablo->save();
    preset->mutable_places()->CopyFrom(opt.places());
    if ( false == TProtoText::toFile(presets_, meteotablo::presetsFile()) ) {
      error_log << tr("Не удалось сохранить пользовательские предустановки (%1).").arg(meteotablo::presetsFile());
      return;
    }

    loadPresets();
    return;
  }

  if ( false == TProtoText::fromFile(meteotablo::presetsFile(), &presets_ ) ) {
    error_log << QObject::tr("Не удалось загрузить пользовательские предустановки");
    return;
  }

  if ( presets_.preset_size() < 2 ) {
    ui_->deletePresetBtn->setEnabled(false);
  }
  else {
    ui_->deletePresetBtn->setEnabled(true);
  }

  ui_->presetCombo->clear();

  for ( auto preset : presets_.preset() ) {
    ui_->presetCombo->addItem(QString::fromStdString(preset.name()), pbtools::fromBytes(preset.SerializeAsString()));
  }
}

void TabloWindow::loadSettings(TabloWidget *wgt)
{
  tablo::Settings opt;

  if (!QFile::exists(meteotablo::confFile())) {
    QFile::copy(MnCommon::etcPath()+"meteotablo.conf", meteotablo::confFile());
  }

  if ( false == QFile::exists(meteotablo::confFile())) {
    error_log << tr("Не найден конфигурационный файл метеотабло. Обратитесь к администратору.");
  }

  if ( !TProtoText::fromFile(meteotablo::confFile(), &opt) ) {
    error_log << tr("Не удалось загрузить настройки (%1).").arg(meteotablo::confFile());
  }

  wgt->blockSignals(true);
  wgt->restore(opt);
  if ( opt.has_geometry() ) {
    restoreGeometry(QByteArray::fromHex(pbtools::fromBytes(opt.geometry())));
  }

  ui_->sortButton->setChecked(opt.sorting());

  if ( opt.columns_size() == 0 ) {
    wgt->addMeteoparam("T");
    wgt->addMeteoparam("P0");
    wgt->addMeteoparam("P");
    wgt->addMeteoparam("PQNH");
    wgt->addMeteoparam("a");
//    wgt->addMeteoparam("N");
    wgt->addMeteoparam("Nh");
    wgt->addMeteoparam("h");
//    wgt->addMeteoparam("CL");
//    wgt->addMeteoparam("CM");
//    wgt->addMeteoparam("CH");
//    wgt->addMeteoparam("Cn");
    wgt->addMeteoparam("C");
    wgt->addMeteoparam("w");
    wgt->addMeteoparam("V");
    wgt->addMeteoparam("dd");
    wgt->addMeteoparam("ff");
    wgt->addMeteoparam("U");
  }
  wgt->blockSignals(false);
  opt_.CopyFrom(opt);
}

void TabloWindow::setDescription(TabloWidget* wgt)
{
  for ( int i = 0, sz = wgt->topLevelItemCount(); i < sz; ++i ) {
    auto item = wgt->topLevelItem(i);
    for ( int j = 0, columnCount = item->columnCount(); j < columnCount; ++j ) {
      if ( TabloWidget::kColumnTime == j ) {
        continue;
      }
      item->setData(j, Qt::ToolTipRole, QString());
    }
  }

  QList<ValueDescr> list = wgt->values();
  QStringList descrlist;
  for ( auto vd : list ) {
    descrlist.append( QString::number( TMeteoDescriptor::instance()->descriptor(vd.descr) ) );
//    debug_log << "vd.descr =" << vd.descr << "name =" << TMeteoDescriptor::instance()->descriptor(vd.descr) << "VALUE =" << vd.value;
  }
  global::loadBufrTables(descrlist);

  for ( auto vd : list ) {
    QString strdescr = QString::number( TMeteoDescriptor::instance()->descriptor(vd.descr) );
    QString tooltip = global::bufrValueDescription( strdescr, vd.value );
    if ( "" != vd.text && "Nh" == vd.descr ) {
      tooltip.clear();
      QStringList nhs = vd.text.split('/');
      QStringList tooltips;
      for ( auto nh : nhs ) {
        tooltips << global::bufrValueDescription( strdescr, nh.toDouble() );
      }
      tooltip = tooltips.join('/');
    }
    if ( 0 != tooltip.size() ) {
      if ( false == wgt->isArchive() ) {
        wgt->setDescription( vd.placeId, vd.descr, tooltip );
      }
      else {
        wgt->setDescription(vd.dt, vd.descr, tooltip);
      }
    }
  }
}

}
