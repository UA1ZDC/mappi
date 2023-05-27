#include "vkowidget.h"
#include "ui_vkowidget.h"
#include "vkostations.h"

#include <QMessageBox>
#include <QProcess>
#include <QHeaderView>
#include <qcompleter.h>
#include <qstandarditemmodel.h>
#include <qsortfilterproxymodel.h>


#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/ui/custom/selectstation.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/custom/userselectform.h>

const int kTimeout = 1000000;

enum VkoItem {
  kCommon = 0,
  kMonth  = 1,
  kVisibility = 2,
  kPhenomena = 3,
  kTemp = 4,
  kWind = 5,
  kAero = 6
};

enum CompliterColumn {
  kStationCol = 0,
  kRuNameCol = 1,
  kEnNameCol = 2,
  kStationTypeCol = 3
};

enum CompliterRole {
  kIndexRole = Qt::DisplayRole,
  kCompliterRole = Qt::UserRole,
  kStationRole,
  kStationTypeRole,
  kClimTypeRole
};

const QString kErrStyle = "\
      border-color:     red;      \
      border-width:     1px;      \
      border-style:     solid;    \
      border-radius:    3px;      \
      padding:          4px;      \
      background-color: #ffc0c0;";

using namespace meteo;

VkoWidget::VkoWidget(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::VkoWidget),
  stationsDlg_(new VkoStations)
{
  ui->setupUi(this);

  ui->selectUsersBtn->setIcon(QIcon(":/meteo/icons/users.png"));

  ui->dateStartEdit->setDate(QDate::currentDate());
  ui->dateEndEdit->setDate(QDate::currentDate());

  ui->station->setStyleSheet(kErrStyle);

  ui->month->setCurrentIndex(QDate::currentDate().month() - 1);

  connect(ui->stationBtn, SIGNAL(clicked(bool)), this, SLOT(chooseStation()));

  connect(ui->listWidget, SIGNAL(currentRowChanged(int)), SLOT(chooseVko(int)));
  connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(slotOnCreateButton()));
  connect(ui->createButton, SIGNAL(clicked(bool)), this, SLOT(slotOnCreateButton()));
  connect(ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
  connect(ui->selectUsersBtn, SIGNAL(clicked()), this, SLOT(slotShowSelectUsers()));

  formals_ = meteo::map::WeatherLoader::instance()->formals();
  if (! fillStations()) {
    QObject::connect(ui->station, SIGNAL(textChanged(QString)), this, SLOT(setCompleter()));
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Не удалось подключиться к сервису доступа к климатическим данным"));
  }

  ui->selectUsersBtn->setHidden(true);
  ui->sendUsersCheck->setHidden(true);
}

VkoWidget::~VkoWidget()
{
  delete ui;
  delete usersWidget_;
}

void VkoWidget::slotShowSelectUsers()
{
  if ( nullptr == usersWidget_) {
    usersWidget_ = new meteo::UserSelectForm();
  }
  usersWidget_->show();
}


void VkoWidget::enableGui(bool enable)
{
  qApp->processEvents();
  ui->createButton->setEnabled(enable);
  ui->listWidget->setEnabled(enable);
  ui->createButton->blockSignals(!enable);
  ui->listWidget->blockSignals(!enable);
}


void VkoWidget::slotOnCreateButton()
{
  if ( true == ui->listWidget->item(kCommon)->isSelected()) {
    createDocument(formals_[QObject::tr("Климатическая справка")]);
  }
  if ( true == ui->listWidget->item(kMonth)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая справка за месяц")], true);
  }
  if ( true == ui->listWidget->item(kPhenomena)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая характеристика (явления)")]);
  }
  if ( true == ui->listWidget->item(kTemp)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая характеристика (температура)")]);
  }
  if ( true == ui->listWidget->item(kAero)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая характеристика (аэрология)")]);
  }
  if ( true == ui->listWidget->item(kWind)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая характеристика (ветер)")]);
  }
  if ( true == ui->listWidget->item(kVisibility)->isSelected() ) {
    createDocument(formals_[QObject::tr("Климатическая характеристика (видимость)")]);
  }
}

void VkoWidget::chooseVko(int row)
{
  ui->month->setEnabled(row == kMonth);
}

void VkoWidget::openTemp(meteo::map::proto::ExportResponse *r)
{
  QTemporaryFile *tmpfile = new QTemporaryFile(this);
  tmpfile->setFileTemplate(QDir::tempPath()+QString("/mtptmp_XXXXXX.odt"));

  QByteArray bar = QByteArray::fromStdString( r->data() );

  tmpfile->open();
  tmpfile->write(bar);
  tmpfile->close();

  QProcess* process_office = new QProcess(this);
  offices_.insert(process_office, tmpfile);
  connect(process_office, SIGNAL(finished(int)), this, SLOT(slotOfficeClosed()));
  QString off = QString("libreoffice " + tmpfile->fileName());
  process_office->start(off);
  process_office->waitForStarted();
}

void VkoWidget::slotOfficeClosed()
{
  auto proc = static_cast<QProcess*>(sender());
  proc->deleteLater();
}

void VkoWidget::createDocument(meteo::map::proto::Map map, bool setmonth /*= false*/)
{
  enableGui(false);
  qApp->setOverrideCursor(Qt::WaitCursor);

  meteo::map::proto::StartArguments sa;
  sa.add_index(stationsDlg_->curIndex().toStdString());
  QString curName = stationsDlg_->curRuName();
  if (curName.isEmpty()) {
    curName = stationsDlg_->curEnName();
  }
  sa.set_name(curName.toStdString());
  sa.set_coord(stationsDlg_->curCoord().toStdString());

  sa.set_datetime(ui->dateStartEdit->date().toString(Qt::ISODate).toStdString());
  sa.set_datetime_end(ui->dateEndEdit->date().toString(Qt::ISODate).toStdString());
  sa.set_number(utc_);
  if (setmonth) {
    sa.set_month(ui->month->currentIndex() + 1);
  }
  //var(sa.Utf8DebugString());

  map.set_datetime(ui->dateStartEdit->dateTime().toString(Qt::ISODate).toStdString());
  //  map.add_station(ui->indexEdit->text().toStdString());
  meteo::sprinf::Station *st = map.add_station();
  st->set_station(stationsDlg_->curIndex().toStdString());
  st->mutable_name()->set_international(stationsDlg_->curEnName().toStdString());
  st->mutable_name()->set_rus(stationsDlg_->curRuName().toStdString());
  st->mutable_name()->set_short_(stationsDlg_->curRuName().toStdString());
  map.mutable_formal()->set_proto_args(sa.SerializeAsString());

  QStringList logins;
  if ( ui->sendUsersCheck->isChecked() ) {
    if ( nullptr == usersWidget_ ) {
       QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
       return;
    }
    else {
      logins = usersWidget_->getCurrentUsers();
      if ( logins.size() == 0 ) {
        QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
        return;
      }
    }
  }

  formaldoc_.slotCreateFormalDoc(map, logins);

  qApp->restoreOverrideCursor();
  enableGui(true);
}

//! заполнение параметров по выбранной станции из списка
void VkoWidget::chooseStation()
{
  if (nullptr == prop_) {
    fillStations();
    if (nullptr == prop_) {
      QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Не удалось подключиться к сервису доступа к климатическим данным"));
      return;
    }
  }

  int res = stationsDlg_->exec();

  if (QDialog::Accepted == res) {
    ui->station->blockSignals(true);
    ui->station->setStyleSheet("");
    ui->station->setText(stationText(stationsDlg_->curIndex(), stationsDlg_->curRuName(), stationsDlg_->curEnName()));
    ui->dateStartEdit->setDate(stationsDlg_->curDateStart());
    ui->dateEndEdit->setDate(stationsDlg_->curDateEnd());
    utc_ = stationsDlg_->curUTC();
    changeClimType(stationsDlg_->climType());
    ui->station->setCompleter(0);
    ui->station->blockSignals(false);
    QObject::connect(ui->station, SIGNAL(textChanged(QString)), this, SLOT(setCompleter()));
  } else {
    //TODO set old current
    stationsDlg_->resetCurrent();
  }
}

//! доступность климатических характеристик в зависимости от станции
void VkoWidget::changeClimType(meteo::sprinf::MeteostationType climType)
{
  Qt::ItemFlags flags = ui->listWidget->item(kVisibility)->flags();
  ui->listWidget->item(kCommon)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  ui->listWidget->item(kVisibility)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  ui->listWidget->item(kPhenomena)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  ui->listWidget->item(kTemp)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  ui->listWidget->item(kWind)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  ui->listWidget->item(kAero)->setFlags(flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  if (climType == sprinf::kStationAero) {
    ui->listWidget->item(kCommon)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    ui->listWidget->item(kVisibility)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    ui->listWidget->item(kPhenomena)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    ui->listWidget->item(kTemp)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    ui->listWidget->item(kWind)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
  } else if (climType == sprinf::kStationSynop) {
    ui->listWidget->item(kAero)->setFlags(flags & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
  }

  if (climType == sprinf::kStationSynop) {
    if (nullptr == ui->listWidget->currentItem() ||
        ui->listWidget->item(kAero) == ui->listWidget->currentItem()) {
      ui->listWidget->setCurrentItem(ui->listWidget->item(kCommon));
    }
  } else {
    ui->listWidget->setCurrentItem(ui->listWidget->item(kAero));
  }
}

//! получение списка станций, заполнение виджетов
bool VkoWidget::fillStations()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  ::meteo::rpc::Channel* channel = ::meteo::global::serviceChannel(meteo::settings::proto::kClimat);
  if (nullptr == channel) {
    qApp->restoreOverrideCursor();
    return false;
  }

  delete prop_;
  prop_ = channel->remoteCall(&::meteo::climat::Service::GetAvailableStations, climat::Dummy(), kTimeout);
  if (nullptr == prop_) {
    qApp->restoreOverrideCursor();
    return false;
  }

  QObject::disconnect(ui->station, SIGNAL(textChanged(QString)), this, SLOT(setCompleter()));

  sprinf::StationTypes types;
  sprinf::MultiStatementRequest req;
  global::loadStationTypes(&types, req);

  QStandardItemModel* model = new QStandardItemModel( prop_->stations_size(), 3, this);

  for (int idx = 0; idx < prop_->stations_size(); ++idx) {
    const climat::ClimatStation& sd =  prop_->stations(idx);
    QString station = QString::fromStdString(sd.station().station());
    QString ru = QString::fromStdString(sd.station().name().rus());
    QString en = QString::fromStdString(sd.station().name().international());

    QStandardItem* item1 = new QStandardItem;
    item1->setData(station, Qt::DisplayRole);
    item1->setData(station + " " + ru + " " + en, kCompliterRole);
    item1->setData(stationText(station, ru, en), kStationRole);

    item1->setData(sd.station().type(), kStationTypeRole);
    item1->setData(sd.cltype(), kClimTypeRole);

    QStandardItem* item2 = new QStandardItem;
    item2->setData(ru, Qt::DisplayRole);

    QStandardItem* item3 = new QStandardItem;
    item3->setData(en, Qt::DisplayRole);

    QString sttype = QString::number(sd.station().type());
    QStandardItem* item4 = new QStandardItem;
    for (auto st : types.station()) {
      if (st.type() == sd.station().type()) {
        sttype = QString::fromStdString(st.name());
        break;
      }
    }
    item4->setData(sttype, Qt::DisplayRole);

    model->appendRow(QList<QStandardItem*>() << item1 << item2 << item3 << item4);
  }


  QSortFilterProxyModel* filterModel = new QSortFilterProxyModel(this);
  filterModel->setSourceModel(model);
  filterModel->sort(0);

  delete completer_;
  completer_ = new QCompleter(this);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setModel(filterModel);
  completer_->setFilterMode(Qt::MatchContains);
  completer_->setCompletionRole(kCompliterRole);
  completer_->setCompletionColumn(kStationCol);
  VkoStationPopup* pop = new VkoStationPopup;
  completer_->setPopup(pop);

  ui->station->setCompleter(completer_);

  QObject::connect(ui->station, SIGNAL(textChanged(QString)), pop, SLOT(resizeWithText(QString)));
  QObject::connect(completer_, SIGNAL(activated(QModelIndex)), SLOT(choosePopup(QModelIndex)));

  stationsDlg_->fillStations(*prop_, types);

  qApp->restoreOverrideCursor();
  return true;
}

//при выборе станции из списка, заполнение полей и удаление автодополнения (иначе заполняет только всей своей строкой)
void VkoWidget::choosePopup(const QModelIndex& index)
{
  if (nullptr != ui->station->completer()) {
    QObject::disconnect(ui->station,  SIGNAL(textChanged(QString)), ui->station->completer()->popup(), SLOT(resizeWithText(QString)));
    QObject::disconnect(ui->station->completer(), SIGNAL(activated(QModelIndex)), this, SLOT(choosePopup(QModelIndex)));
  }

  ui->station->blockSignals(true);
  QModelIndex sibling = index.sibling(index.row(), kStationCol);
  if (sibling.isValid()) {
    ui->station->setText(sibling.data(kStationRole).toString());
    stationsDlg_->setCurrent(sibling.data(kIndexRole).toString(),
                             sibling.data(kStationTypeRole).toInt(),
                             sibling.data(kClimTypeRole).toInt());
    ui->dateStartEdit->setDate(stationsDlg_->curDateStart());
    ui->dateEndEdit->setDate(stationsDlg_->curDateEnd());
    utc_ = stationsDlg_->curUTC();
    changeClimType(stationsDlg_->climType());
  }

  ui->station->setStyleSheet("");
  ui->station->blockSignals(false);

  ui->station->setCompleter(0);
  QObject::connect(ui->station, SIGNAL(textChanged(QString)), this, SLOT(setCompleter()));
}

//установка автодополнения при редактировании текста
void VkoWidget::setCompleter()
{
  //если сервис был не доступен при запуске
  //debug_log << prop_ << completer_ << completer_->popup();
  if (nullptr == prop_ || completer_ == nullptr || nullptr == completer_->popup()) {
    fillStations();
    return;
  }

  QObject::disconnect(ui->station, SIGNAL(textChanged(QString)), this, SLOT(setCompleter()));

  ui->station->setCompleter(completer_);

  QObject::connect(ui->station, SIGNAL(textChanged(QString)), completer_->popup(), SLOT(resizeWithText(QString)));
  QObject::connect(completer_, SIGNAL(activated(QModelIndex)), SLOT(choosePopup(QModelIndex)));

  ui->station->setStyleSheet(kErrStyle);
  if (!ui->station->text().isEmpty()) {
    ui->station->setText(stationsDlg_->curIndex());
  }

  stationsDlg_->removeSelection();
  changeClimType(stationsDlg_->climType());
  ui->listWidget->setCurrentItem(nullptr);
}

//текст для отображения после выбора
QString VkoWidget::stationText(const QString& index, const QString& ru, const QString en)
{
  return index + " ("  + (ru.isEmpty() ? en : ru) + ")";
}



VkoStationPopup::VkoStationPopup()
{
  setRootIsDecorated(false);
  header()->setVisible(false);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  for (int i = 0, sz = columns_; i < sz; ++i) {
    resizeColumnToContents(i);
  }
}

void VkoStationPopup::resizeWithText(const QString& text)
{
  Q_UNUSED(text);
  // var(text);

  for (int i = 0, sz = columns_; i < sz; ++i) {
    resizeColumnToContents(i);
  }

  setFixedWidth(viewportSizeHint().width());
  // debug_log << width() << height() << viewportSizeHint();// << viewport().size().width();
}

