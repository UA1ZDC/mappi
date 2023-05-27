#include "av12widget.h"
#include "ui_av12widget.h"
#include "factwindstations.h"

#include <QMessageBox>
#include <QMenu>
#include <QPoint>
#include <QTemporaryFile>

#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/custom/userselectform.h>
#include <meteo/commons/settings/tusersettings.h>

#include <commons/textproto/tprototext.h>

namespace meteo {

namespace product {


Av12Widget::Av12Widget(QWidget* parent, WidgetType type):
  QWidget(parent),
  ui_(new Ui::Av12Widget),
  widgetType_(type),
  roleList_(new QStringList()),
  indexList_(new QStringList()),
  nameList_(new QStringList()),
  radiusMenu_(new QMenu()),
  radiusBox_(new QSpinBox()),
  radiusAct_(new QWidgetAction(nullptr))
{
  ui_->setupUi(this);
  if ( WidgetType::kAv12 == widgetType_ ) {
    map_ = meteo::map::WeatherLoader::instance()->formals()["Бюллетень погоды АВ-12"];
    setWindowTitle("АВ-12");
    setObjectName("av12");
    ui_->stackedWidget->setCurrentIndex(WidgetType::kAv12);
    ui_->dopSignButton->show();
    ui_->dopExecuterBox->show();
    ui_->dopExecutorName->show();
    ui_->dopExecutorRankLabel->show();
    ui_->dopExecutorRankComboBox->show();
  }
  else if ( WidgetType::kAv12d == widgetType_ ) {
    map_ = meteo::map::WeatherLoader::instance()->formals()["Бюллетень погоды АВ-12-Д"];
    setWindowTitle("АВ-12-Д");
    setObjectName("av12d");
    ui_->stackedWidget->setCurrentIndex(WidgetType::kAv12d);
    ui_->factWindBtn->hide();
    ui_->dopSignButton->hide();
    ui_->dopExecuterBox->hide();
    ui_->dopExecutorName->hide();
    ui_->dopExecutorRankLabel->hide();
    ui_->dopExecutorRankComboBox->hide();
  }
  ui_->dateStart->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->dateStart->setDate(QDate().currentDate());
  ui_->arrivalBut->setEnabled(false);
  radiusBox_->setMaximum(999);
  radiusBox_->setValue(150);
  radiusAct_->setDefaultWidget( radiusBox_ );
  radiusMenu_->addAction( radiusAct_ );
  QList<meteo::sprinf::MeteostationType> stationsToLoad;
  stationsToLoad << meteo::sprinf::MeteostationType::kStationAirport
                 << meteo::sprinf::MeteostationType::kStationAero
                 << meteo::sprinf::MeteostationType::kStationAeromob
                 << meteo::sprinf::MeteostationType::kStationAerodrome
                 << meteo::sprinf::MeteostationType::kStationSynop
                 << meteo::sprinf::MeteostationType::kStationSynmob;
  ui_->station->loadStation(stationsToLoad);
  ui_->createButton->setEnabled(false);
  ui_->createButton->setToolTip(QObject::tr("Добавьте как минимум одну станцию в список"));
  ui_->stationTable->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_->stationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui_->stationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui_->arrivalBut->setToolTip(QObject::tr("Добавить пункт посадки"));
  ui_->arrivalBut->setIcon(QIcon(":/meteo/icons/arrival.png"));
  ui_->arrivalBut->setIconSize(QSize(22,22));
  ui_->departureBut->setToolTip(QObject::tr("Добавить пункт вылета"));
  ui_->departureBut->setIcon(QIcon(":/meteo/icons/departure.png"));
  ui_->departureBut->setIconSize(QSize(22,22));
  ui_->onRouteBut->setToolTip(QObject::tr("Добавить пункт на маршруте"));
  ui_->onRouteBut->setIcon(QIcon(":/meteo/icons/onroute.png"));
  ui_->onRouteBut->setIconSize(QSize(22,22));
  ui_->reservBut->setToolTip(QObject::tr("Добавить запасной аэродром"));
  ui_->reservBut->setIcon(QIcon(":/meteo/icons/reserve.png"));
  ui_->reservBut->setIconSize(QSize(22,22));
  ui_->forecastArrivalBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->forecastRouteBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->ornitArrivalBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->ornitRouteBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->textForecastAv12dBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->forecastArrivalAv12dBtn->setIcon(QIcon(":/meteo/icons/misc/text-editor.png"));
  ui_->usersBtn->setIcon(QIcon(":/meteo/icons/users.png"));
  ui_->findStationBtn->setIcon(QIcon(":/meteo/icons/search.png"));
  ui_->factWindBtn->setIcon(QIcon(":/meteo/icons/map/vane.png"));
  ui_->dopSignButton->setIcon(QIcon(":/meteo/icons/edit.png"));
  ui_->findStationBtn->setPopupMode(QToolButton::MenuButtonPopup);
  ui_->findStationBtn->setMenu( radiusMenu_ );
  QMap<int,QString> ranks = meteo::TUserSettings::instance()->ranks();
  gSettings()->load();
  for( int id : ranks.keys() ) {
    ui_->executorRankComboBox->addItem(ranks.value(id), id);
    ui_->commanderRankComboBox->addItem(ranks.value(id), id);
    ui_->dopExecutorRankComboBox->addItem(ranks.value(id), id);
  }
  ui_->commanderRankComboBox->setCurrentIndex( gSettings()->location().rank_cmdr() -1);
  ui_->commanderName->setText( QString::fromStdString( gSettings()->location().fio_cmdr() ) );
  ui_->unitNumber->setText( QString::fromStdString( gSettings()->location().unit_number() ) ) ;
//  QObject::connect(ui_->findStationBtn, SIGNAL(triggered(QAction*)), SLOT(slotShowRadiusMenu(QAction*)));
  QObject::connect(ui_->typeBox, SIGNAL(currentIndexChanged(int)),this, SLOT(slotChangeType(int)));
  QObject::connect(ui_->forecastArrivalBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->forecastRouteBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->ornitArrivalBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->ornitRouteBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->textForecastAv12dBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->forecastArrivalAv12dBtn, SIGNAL(clicked(bool)), SLOT(slotShowForecastDlg()) );
  QObject::connect(ui_->usersBtn, SIGNAL(clicked(bool)), SLOT(slotShowSelectUsers()));
  QObject::connect(ui_->factWindBtn, SIGNAL(clicked()), SLOT(slotShowFactWindForm()));
  QObject::connect(radiusBox_, SIGNAL(valueChanged(int)), SLOT(slotChangeRadius()));
  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + this->objectName() + ".ini", QSettings::IniFormat);
  if (true == QFile(QDir::homePath() + "/.meteo/" + this->objectName() + ".ini").exists()) {
    slotLoadState();
  }
  QStringList lables;
  lables.append("Роль");
  lables.append("Индекс");
  lables.append("Название");
  lables.append("Время");
  lables.append("Эшелон");
  ui_->stationTable->setHorizontalHeaderLabels(lables);
  QObject::connect(ui_->createButton, SIGNAL(clicked(bool)), this, SLOT(slotCreateAv12()));
  QObject::connect(ui_->createButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveState()));
  QObject::connect(ui_->departureBut, SIGNAL(clicked(bool)),this, SLOT(slotAddDepartureStation()));
  QObject::connect(ui_->reservBut, SIGNAL(clicked(bool)), this, SLOT(slotAddReservStation()));
  QObject::connect(ui_->onRouteBut, SIGNAL(clicked(bool)), this, SLOT(slotAddOnRouteStation()));
  QObject::connect(ui_->arrivalBut, SIGNAL(clicked(bool)), this, SLOT(slotAddArrivalStation()));
  QObject::connect(ui_->stationTable, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowMenu(QPoint)));
  QObject::connect(ui_->departureTime, SIGNAL(timeChanged(QTime)), this, SLOT(slotTimeDepartureChanged(QTime)));
  QObject::connect(ui_->landingTime, SIGNAL(timeChanged(QTime)), this, SLOT(slotTimeLandingChanged(QTime)));
  QObject::connect(ui_->dateStart, SIGNAL(dateChanged(QDate)), SLOT(slotDateChanged()));
  QObject::connect(ui_->findStationBtn, SIGNAL(clicked(bool)), SLOT(slotChangeFindStation()));
  QObject::connect(ui_->dopSignButton, SIGNAL(toggled(bool)), SLOT(slotChangeEnableDopSign()) );
  installEventFilter(this);
  setVisibleUserBtn(isInterAvailable());
}

Av12Widget::~Av12Widget()
{
  delete ui_;
  delete roleList_;
  delete indexList_;
  delete nameList_;
  delete menu_;
  delete removeAct_;
  delete settings_;
  delete factWind_;
  if ( nullptr != forecastForm_ ) {
    delete forecastForm_;
  } 
}

void Av12Widget::keyReleaseEvent(QKeyEvent *event)
{
  if ( event->key() == Qt::Key_Escape ) {
    this->close();
  } else if (event->key() == Qt::Key_Delete && ui_->stationTable->hasFocus()) {
    slotRemoveStation();
  }
}

void Av12Widget::slotChangeType(int type)
{
  type_ = static_cast<BillType>(type);
  if ( BillType::kPerelet != type_ ) {
    ui_->arrivalBut->setEnabled(false);
  } else {
    ui_->arrivalBut->setEnabled(true);
  }
  if ( 0 == ui_->stationTable->rowCount() ) {
    ui_->createButton->setEnabled(false);
    ui_->createButton->setToolTip(QObject::tr("Добавьте как минимум одну станцию в список"));
  }
  if ( BillType::kVRP != type_ ) {
    ui_->executerBox->setCurrentIndex(0);
  }
  else {
    ui_->executerBox->setCurrentIndex(1);
  }
  loadStations();
}

void Av12Widget::slotRemoveStation()
{
  int row = ui_->stationTable->currentRow();
  if ( nullptr != factWind_ ) {
    factWind_->delStation( ui_->stationTable->item(row,kIndex)->text() );
  }
  ui_->stationTable->removeRow(row);
  if ( 0 == ui_->stationTable->rowCount() ) {
    ui_->createButton->setEnabled(false);
    ui_->createButton->setToolTip(QObject::tr("Добавьте как минимум одну станцию в список"));
  }
}

void Av12Widget::slotAddDepartureStation()
{
  meteo::sprinf::Station station = ui_->station->toStation();
  if ((0 == station.index())
   && (false == station.has_cccc())) {
    return;
  }
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if ("Пункт вылета" == ui_->stationTable->item(i, 0)->text()) {
      info_log.msgBox() << QObject::tr("Пункт вылета уже есть");
      return;
    }
  }
  QTableWidgetItem* roleItem  = new QTableWidgetItem("Пункт вылета");
  roleItem->setData(Qt::UserRole,meteo::surf::StationType::kDeparture);
  addStation(roleItem,ui_->departureTime->time().toString());
  if ( BillType::kPerelet != type_ ) {
    slotAddArrivalStation();
  }
  if ( nullptr != factWind_) {
    factWind_->addStation(station);
  }
}

void Av12Widget::slotAddOnRouteStation()
{
  meteo::sprinf::Station station = ui_->station->toStation();
  if ((0 == station.index())
   && (false == station.has_cccc())){
    return;
  }
  QTableWidgetItem* roleItem  = new QTableWidgetItem("На маршруте");
  roleItem->setData(Qt::UserRole, meteo::surf::StationType::kOnRoute);
  addStation(roleItem,ui_->departureTime->time().toString());
}

void Av12Widget::slotAddReservStation()
{
  meteo::sprinf::Station station = ui_->station->toStation();
  if ((0 == station.index())
   && (false == station.has_cccc())){
    return;
  }
  QTableWidgetItem* roleItem  = new QTableWidgetItem("Запасной аэродром");
  roleItem->setData(Qt::UserRole, meteo::surf::StationType::kReserve);
  addStation(roleItem,ui_->landingTime->time().toString());
}

void Av12Widget::slotAddArrivalStation()
{
  meteo::sprinf::Station station = ui_->station->toStation();
  if ((0 == station.index())
  && (false == station.has_cccc())){
    return;
  }
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if (ui_->stationTable->item(i, 0)->text() == "Пункт посадки") {
      info_log.msgBox() << QObject::tr("Пункт посадки уже есть");
      return;
    }
  }
  QTableWidgetItem* roleItem  = new QTableWidgetItem("Пункт посадки");
  roleItem->setData(Qt::UserRole, meteo::surf::StationType::kArrival);
  addStation(roleItem,ui_->landingTime->time().toString());
  if ( nullptr != factWind_ ) {
    factWind_->addStation(station);
  }
}

void Av12Widget::addStation(QTableWidgetItem* roleItem, QString time)
{
  meteo::sprinf::Station station = ui_->station->toStation();
  QString ind;
  if (0 != station.index()) {
    ind = QString::number(station.index());
  }
  else if(station.has_cccc()) {
    ind = QString::fromStdString(station.cccc());
  }
  QTableWidgetItem* indexItem = new QTableWidgetItem( ind );
  QTableWidgetItem* nameItem = new QTableWidgetItem (QString::fromUtf8(station.name().rus().data()) );
  QTableWidgetItem* echelonItem = new QTableWidgetItem( QString::number( ui_->echelonBox->value() ) );
  echelonItem->setData(Qt::UserRole, ui_->echelonBox->value());
  indexItem->setData(Qt::UserRole, ui_->station->stationType());
  indexItem->setData(kFi,ui_->station->coord().fi());
  indexItem->setData(kLa,ui_->station->coord().la());
  ui_->stationTable->setRowCount(ui_->stationTable->rowCount()+1);
  ui_->stationTable->setItem(ui_->stationTable->rowCount()-1,kRole,roleItem);
  ui_->stationTable->setItem(ui_->stationTable->rowCount()-1,kIndex,indexItem);
  ui_->stationTable->setItem(ui_->stationTable->rowCount()-1,kName,nameItem);
  ui_->stationTable->setItem(ui_->stationTable->rowCount()-1,kEchelon,echelonItem);
  QTimeEdit* timeEdit = new QTimeEdit(this);
  timeEdit->setTime(QTime::fromString(time,"hh:mm:ss"));
  ui_->stationTable->setCellWidget(ui_->stationTable->rowCount()-1, kTime, timeEdit);
  QSpinBox* spinBox = new QSpinBox(this);
  spinBox->setMaximum(15000);
  spinBox->setValue( ui_->echelonBox->value() );
  ui_->stationTable->setCellWidget(ui_->stationTable->rowCount()-1, kEchelon, spinBox );
  if (roleItem->text() == "Пункт посадки") {
    QObject::connect(timeEdit,SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeLandingChanged(QTime)));
  }
  if (roleItem->text() == "Пункт вылета") {
    QObject::connect(timeEdit,SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeDepartureChanged(QTime)));
  }
  ui_->createButton->setEnabled(true);
  ui_->createButton->setToolTip(QObject::tr("Запуск формирования бюллетеня"));
}

surf::Point* Av12Widget::getArrival()
{
  return getStationByRole("Пункт посадки");
}

surf::Point* Av12Widget::getDeparture()
{
  return getStationByRole("Пункт вылета");
}

surf::Point* Av12Widget::getStationByRole(QString role)
{
  for ( int i = 0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if ( ui_->stationTable->item(i, kRole)->text() == role ) {
      surf::Point* st = new surf::Point();
      st->set_index(ui_->stationTable->item(i,kIndex)->text().toStdString());
      st->set_name(ui_->stationTable->item(i,kName)->text().toStdString());
      st->set_av12type(static_cast<meteo::surf::StationType>(ui_->stationTable->item(i,kRole)->data(Qt::UserRole).toInt()));
      st->set_type(ui_->stationTable->item(i,kIndex)->data(Qt::UserRole).toInt());
      st->set_fi(ui_->stationTable->item(i,kIndex)->data(kFi).toDouble());
      st->set_la(ui_->stationTable->item(i,kIndex)->data(kLa).toDouble());
      QSpinBox* echelon;
      echelon = qobject_cast<QSpinBox*>(ui_->stationTable->cellWidget(i,kEchelon));
      st->set_echelon(echelon->value());
      QDateTime dateTime;
      dateTime.setDate(ui_->dateStart->date());
      dateTime.setTime(qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime))->time());
      st->set_date_time(dateTime.toString(Qt::ISODate).toStdString());
      return st;
    }
  }
  return nullptr;
}

void Av12Widget::loadStations()
{
  int rowCount = settings_->value(this->objectName()+".rowCount"+ ui_->typeBox->currentText()).toInt();
  int columnCount = settings_->value(this->objectName()+".columnCount"+ ui_->typeBox->currentText()).toInt();
  ui_->stationTable->clear();
  ui_->stationTable->setColumnCount(columnCount);
  ui_->stationTable->setRowCount(rowCount);
  if (rowCount > 0) {
    ui_->createButton->setEnabled(true);
  }
  for (int i = 0; i < rowCount; ++i) {
    for (int j = 0; j < columnCount; ++j) {
      if ( kTime == j ) {
        QString time = settings_->value(this->objectName() +".tabletext"+ ui_->typeBox->currentText()+QString::number(i)+QString::number(j)).toString();
        QTimeEdit* timeEdit = new QTimeEdit(this);
        timeEdit->setTime(QTime::fromString(time,"hh:mm:ss"));
        ui_->stationTable->setCellWidget(i, j, timeEdit);
        continue;
      }
      else if ( kEchelon == j ) {
        QString echelon = settings_->value(this->objectName() +".tabletext"+ ui_->typeBox->currentText()+QString::number(i)+QString::number(j)).toString();
        QSpinBox* echelonBox = new QSpinBox(this);
        echelonBox->setMaximum(15000);
        echelonBox->setValue(echelon.toInt());
        ui_->stationTable->setCellWidget(i, j, echelonBox);
        continue;
      }
      QString name = settings_->value(this->objectName() +".tabletext"+ ui_->typeBox->currentText()+QString::number(i)+QString::number(j)).toString();
      QTableWidgetItem* item = new QTableWidgetItem(name);
      if (settings_->contains(this->objectName() +".tabledata"+ ui_->typeBox->currentText()+QString::number(i)+QString::number(j))) {
        int data = settings_->value(this->objectName() +".tabledata"+ ui_->typeBox->currentText()+QString::number(i)+QString::number(j)).toInt();
        item->setData(Qt::UserRole,data);
      }
      if ( kIndex == j ) {
        QString string = this->objectName() +".tabledata"+ ui_->typeBox->currentIndex()+QString::number(i)+QString::number(j)+"Fi";
        if (settings_->contains(string)) {
          double data = settings_->value(string).toDouble();
          item->setData(kFi,data);
        }
        string = this->objectName() +".tabledata"+ ui_->typeBox->currentIndex()+QString::number(i)+QString::number(j)+"La";
        if (settings_->contains(string)) {
          double data = settings_->value(string).toDouble();
          item->setData(kLa,data);
        }
      }
      ui_->stationTable->setItem(i,j,item);
    }
    auto roleItem = ui_->stationTable->item(i,kRole);
    if (nullptr == roleItem) {
      continue;
    }
    auto timeEdit = qobject_cast<QTimeEdit*>( ui_->stationTable->cellWidget(i,kTime) );
    if (nullptr == timeEdit) {
      continue;
    }
    if (roleItem->text() == "Пункт посадки") {
      timeEdit->setTime(ui_->landingTime->time());
      QObject::connect(timeEdit,SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeLandingChanged(QTime)));
    }
    if (roleItem->text() == "Пункт вылета") {
      timeEdit->setTime(ui_->departureTime->time());
      QObject::connect(timeEdit,SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeDepartureChanged(QTime)));
    }
  }
  ui_->stationTable->setColumnCount(5);
  QStringList lables;
  lables.append("Роль");
  lables.append("Индекс");
  lables.append("Название");
  lables.append("Время");
  lables.append("Эшелон");
  ui_->stationTable->setHorizontalHeaderLabels(lables);
  if ( nullptr != factWind_ ) {
    bool visible = factWind_->isVisible();
    auto pos = factWind_->pos();
    delete factWind_;
    factWind_ = new FactWindStations();
    factWind_->setFindNear(ui_->findStationBtn->isChecked());
    factWind_->setFindRadius(radiusBox_->value());
    QDateTime dt;
    dt.setDate(ui_->dateStart->date());
    dt.setTime(ui_->departureTime->time());
    factWind_->setDt(dt);
    for ( int i = 0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
      surf::Point* st = new surf::Point();
      st->set_index(ui_->stationTable->item(i,kIndex)->text().toStdString());
      st->set_name(ui_->stationTable->item(i,kName)->text().toStdString());
      st->set_av12type(static_cast<meteo::surf::StationType>(ui_->stationTable->item(i,kRole)->data(Qt::UserRole).toInt()));
      st->set_type(ui_->stationTable->item(i,kIndex)->data(Qt::UserRole).toInt());
      st->set_fi(ui_->stationTable->item(i,kIndex)->data(kFi).toDouble());
      st->set_la(ui_->stationTable->item(i,kIndex)->data(kLa).toDouble());
      QSpinBox* echelon;
      echelon = qobject_cast<QSpinBox*>(ui_->stationTable->cellWidget(i,kEchelon));
      st->set_echelon(echelon->value());
      QDateTime dateTime;
      dateTime.setDate(ui_->dateStart->date());
      dateTime.setTime(qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime))->time());
      st->set_date_time(dateTime.toString(Qt::ISODate).toStdString());
      factWind_->setDt(QDateTime(ui_->dateStart->date(), ui_->departureTime->time()));
      factWind_->addStation(*st);
      delete st;
    }
    if (visible) {
      factWind_->setVisible(visible);
      factWind_->move(pos);
    }
  }
}

bool Av12Widget::isInterAvailable()
{
  settings::proto::Service service = global::Settings::instance()->service(settings::proto::ServiceCode::kInterMessagesService);
  return service.has_host() && service.has_port();
}

void Av12Widget::setVisibleUserBtn(bool visible)
{
  ui_->usersCheckBox->setVisible( visible );
  ui_->usersBtn->setVisible( visible );
}

void Av12Widget::slotTimeDepartureChanged(QTime time)
{
  ui_->departureTime->blockSignals(true);
  ui_->departureTime->setTime(time);
  ui_->departureTime->blockSignals(false);
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if (ui_->stationTable->item(i, 0)->text() == "Пункт вылета") {
      QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime));
      timeEdit->blockSignals(true);
      timeEdit->setTime(time);
      timeEdit->blockSignals(false);
    }
  }
  if ( nullptr != factWind_ ) {
    QDateTime dt;
    dt.setDate(ui_->dateStart->date());
    dt.setTime(ui_->departureTime->time());
    factWind_->setDt(dt);
  }
}

void Av12Widget::slotTimeLandingChanged(QTime time)
{
  ui_->landingTime->blockSignals(true);
  ui_->landingTime->setTime(time);
  ui_->landingTime->blockSignals(false);
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if (ui_->stationTable->item(i, 0)->text() == "Пункт посадки") {
      QTimeEdit *timeEdit = qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime));
      timeEdit->blockSignals(true);
      timeEdit->setTime(time);
      timeEdit->blockSignals(false);
    }
  }
}

void Av12Widget::slotDateChanged()
{
  if ( nullptr != factWind_ ) {
    QDateTime dt;
    dt.setDate(ui_->dateStart->date());
    dt.setTime(ui_->departureTime->time());
    factWind_->setDt(dt);
  }
}

void Av12Widget::slotShowForecastDlg()
{
  QPushButton* btn = qobject_cast<QPushButton*>( sender() );
  if ( btn == ui_->forecastArrivalBtn || btn == ui_->forecastRouteBtn
       || btn == ui_->forecastArrivalAv12dBtn || btn == ui_->textForecastAv12dBtn) {
    forecastForm_ = new ForecastFormWidget(this);
  }
  else {
    forecastForm_ = new ForecastFormWidget(this,meteo::global::kForecastWidgetFileNameOrnitolog);
  }
  forecastForm_->setWindowFlags( forecastForm_->windowFlags() | Qt::WindowType::Tool );
  forecastForm_->move(this->pos());
  if ( btn == ui_->forecastArrivalBtn ) {
    forecastForm_->setWindowTitle("Прогноз погоды в пункте посадки");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextFWA(QString) ) );
  }
  else if ( btn == ui_->forecastRouteBtn ) {
    forecastForm_->setWindowTitle("Прогноз погоды по маршруту");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextFWR(QString) ) );
  }
  else if ( btn == ui_->ornitArrivalBtn) {
    forecastForm_->setWindowTitle("Орнитологическая обстановка в пункте посадки");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextOA(QString) ) );
  }
  else if ( btn == ui_->ornitRouteBtn ) {
    forecastForm_->setWindowTitle("Орнитологическая обстановка по маршруту");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextOR(QString) ) );
  }
  else if ( btn == ui_->textForecastAv12dBtn ) {
    forecastForm_->setWindowTitle("Текстуальный прогноз погоды");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextTFWAv12d(QString) ) );
  }
  else if ( btn == ui_->forecastArrivalAv12dBtn ) {
    forecastForm_->setWindowTitle("Прогноз погоды в пункте посадки");
    QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT( slotSetTextFWAAv12d(QString) ) );
  }
  forecastForm_->show();
}

void Av12Widget::slotChangeEnableDopSign()
{
  if (ui_->dopSignButton->isChecked()) {
    ui_->dopExecuterBox->setEnabled(true);
    ui_->dopExecutorName->setEnabled(true);
    ui_->dopExecutorRankLabel->setEnabled(true);
    ui_->dopExecutorRankComboBox->setEnabled(true);
  }
  else {
    ui_->dopExecuterBox->setEnabled(false);
    ui_->dopExecutorName->setEnabled(false);
    ui_->dopExecutorRankLabel->setEnabled(false);
    ui_->dopExecutorRankComboBox->setEnabled(false);
  }
}

void Av12Widget::slotSetTextFWA(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->forecastWeatherArrival->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotSetTextFWR(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->forecastWeatherRoute->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotSetTextOA(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->ornitArrival->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotSetTextOR(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->ornitRoute->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotSetTextFWAAv12d(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->forecastArrivalAv12d->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotSetTextTFWAv12d(QString newValue)
{
  auto form = qobject_cast<ForecastFormWidget*>(sender());
  if ( nullptr == form ) {
    return;
  }
  form->close();
  ui_->textForecastAv12d->setText(newValue);
  delete form; form = nullptr;
}

void Av12Widget::slotShowSelectUsers()
{
  if ( nullptr == usersWidget_) {
    usersWidget_ = new meteo::UserSelectForm();
  }
  usersWidget_->show();
}

void Av12Widget::slotChangeFindStation()
{
  if ( nullptr != factWind_ ) {
    factWind_->setFindNear(ui_->findStationBtn->isChecked());
    factWind_->setFindRadius(radiusBox_->value());
    factWind_->checkAllItems();
  }
}

void Av12Widget::slotChangeRadius()
{
  if ( nullptr != factWind_ ) {
    factWind_->setFindRadius(radiusBox_->value());
  }
}

void Av12Widget::slotShowFactWindForm()
{
  if ( nullptr == factWind_ ) {
    factWind_ = new FactWindStations();
    factWind_->setFindNear(ui_->findStationBtn->isChecked());
    factWind_->setFindRadius(radiusBox_->value());
    QDateTime dt;
    dt.setDate(ui_->dateStart->date());
    dt.setTime(ui_->departureTime->time());
    factWind_->setDt(dt);
    auto station = getDeparture();
    if ( nullptr != station ) {
      factWind_->addStation(*station);
      delete station;
    }
    station = getArrival();
    if ( nullptr != station ) {
      factWind_->addStation(*station);
      delete station;
    }
  }
  factWind_->move( mapToGlobal( ui_->factWindBtn->pos() ) );
  factWind_->show();
}

void Av12Widget::hideEvent(QHideEvent *event)
{
  if (nullptr != factWind_) {
    factWind_->hide();
  }
  QWidget::hideEvent(event);
}

void Av12Widget::slotCreateAv12()
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  bool arrival = false;
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if (ui_->stationTable->item(i, 0)->text() == "Пункт посадки") {
      arrival = true;
    }
  }
  if ( false == arrival ) {
    warning_log.msgBox()<<"Отсутствует пункт посадки.";
  }
  if (ui_->departureTime->time() == ui_->landingTime->time() ) {
      qApp->restoreOverrideCursor();
      error_log.msgBox()<<"Время вылета и посадки совпадают";
    return;
  }
  meteo::map::proto::av12script av12proto;
  av12proto.set_findstation(ui_->findStationBtn->isChecked());
  av12proto.set_radiusfind(radiusBox_->value());
  av12proto.set_type(static_cast<meteo::map::proto::typeBulleten>(ui_->typeBox->currentIndex()));
  if ( WidgetType::kAv12 == widgetType_) {
    av12proto.set_weatherroute(ui_->forecastWeatherRoute->toPlainText().toStdString());
    av12proto.set_weatherarrival(ui_->forecastWeatherArrival->toPlainText().toStdString());
  }
  else if ( WidgetType::kAv12d == widgetType_ ) {
    av12proto.set_weatherroute(ui_->textForecastAv12d->toPlainText().toStdString());
    av12proto.set_weatherarrival(ui_->forecastArrivalAv12d->toPlainText().toStdString());
  }
  av12proto.set_birdsroute(ui_->ornitRoute->toPlainText().toStdString());
  av12proto.set_birdsarrival(ui_->ornitArrival->toPlainText().toStdString());
  av12proto.set_timestart(ui_->departureTime->time().toString(Qt::ISODate).toStdString());
  av12proto.set_timeend(ui_->landingTime->time().toString(Qt::ISODate).toStdString());
  av12proto.set_date(ui_->dateStart->date().toString(Qt::ISODate).toStdString());
  av12proto.set_echelon(ui_->echelonBox->value());
  if (ui_->timeBox->isChecked()) {
    av12proto.set_typetime(meteo::map::proto::typeTime::kNoUTC);
    av12proto.set_shifttime(QDateTime::currentDateTime().offsetFromUtc());
  } else {
    av12proto.set_typetime(meteo::map::proto::typeTime::kUTC);
  }
  meteo::settings::Location loc = gSettings()->location();
  QString rank_cmdr = ui_->commanderRankComboBox->currentText();
  QString synoptic = meteo::global::currentUserSurname();
  synoptic = meteo::global::currentUserRank() + " " + synoptic;
  QString fio_cmdr = ui_->commanderName->text();
  av12proto.set_commander(fio_cmdr.toStdString());
  av12proto.set_commanderrank(rank_cmdr.toStdString());
  av12proto.set_synoptic(synoptic.toStdString());
  QString unit_number = ui_->unitNumber->text();
  av12proto.set_number(unit_number.toStdString());
  appendStations(&av12proto);
  av12proto.set_recivername( ui_->executorName->text().toStdString() );
  av12proto.set_reciverrank( ui_->executorRankComboBox->currentText().toStdString() );
  av12proto.set_reciver( ui_->executerBox->currentText().toStdString() );
  av12proto.set_bullnumber(ui_->numberSpinBox->value());
  if ( nullptr != factWind_ ) {
    auto list = factWind_->getStationsList();
    for ( auto station : list ) {
      auto st = av12proto.add_factwindlist();
      st->set_name(station.name());
      st->set_index(station.index());
      st->set_type(station.type());
    }
  }
  else {
    factWind_ = new FactWindStations();
    for ( int i = 0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
      auto st = new surf::Point();
      st->set_index(ui_->stationTable->item(i,kIndex)->text().toStdString());
      st->set_name(ui_->stationTable->item(i,kName)->text().toStdString());
      st->set_av12type(static_cast<meteo::surf::StationType>(ui_->stationTable->item(i,kRole)->data(Qt::UserRole).toInt()));
      st->set_type(ui_->stationTable->item(i,kIndex)->data(Qt::UserRole).toInt());
      st->set_fi(ui_->stationTable->item(i,kIndex)->data(kFi).toDouble());
      st->set_la(ui_->stationTable->item(i,kIndex)->data(kLa).toDouble());
      QSpinBox* echelon;
      echelon = qobject_cast<QSpinBox*>(ui_->stationTable->cellWidget(i,kEchelon));
      st->set_echelon(echelon->value());
      QDateTime dateTime;
      dateTime.setDate(ui_->dateStart->date());
      dateTime.setTime(qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime))->time());
      st->set_date_time(dateTime.toString(Qt::ISODate).toStdString());
      factWind_->setDt(QDateTime(ui_->dateStart->date(), ui_->departureTime->time()));
      factWind_->addStation(*st);
      delete st;
    }
    auto list = factWind_->getStationsList();
    for ( auto station : list ) {
      auto st = av12proto.add_factwindlist();
      st->set_name(station.name());
      st->set_index(station.index());
      st->set_type(station.type());
    }
    delete factWind_;
    factWind_ = nullptr;
  }

  if (ui_->dopSignButton->isChecked()) {
    av12proto.set_dopsign(true);
    av12proto.set_dopreciver(ui_->dopExecuterBox->currentText().toStdString());
    av12proto.set_doprecivername(ui_->dopExecutorName->text().toStdString());
    av12proto.set_dopreciverrank(ui_->dopExecutorRankComboBox->currentText().toStdString());
  }

  std::string av12serial = av12proto.SerializeAsString();

  auto map = map_;
  map.set_datetime(QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
  map.mutable_formal()->set_date(QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
  map.mutable_formal()->set_proto_args(av12serial);
  map.mutable_document()->set_doctype(meteo::map::proto::kFormalDocument);
  map.set_format( meteo::map::proto::DocumentFormat::kOdt );
  QStringList logins;
  if ( ui_->usersCheckBox->isChecked() ) {
    if ( nullptr == usersWidget_ ) {
       QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
       qApp->restoreOverrideCursor();
       return;
    }
    else {
      logins = usersWidget_->getCurrentUsers();
      if ( logins.size() == 0 ) {
        QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
        qApp->restoreOverrideCursor();
        return;
      }
    }
  }
  formaldoc_.slotCreateFormalDoc(map, logins);
  qApp->restoreOverrideCursor();
}

void Av12Widget::appendStations(map::proto::av12script *av12)
{
  for(int i = 0, sz = ui_->stationTable->rowCount(); i < sz ; ++i) {
    surf::Point* st = av12->add_stations();
    st->set_index(ui_->stationTable->item(i,kIndex)->text().toStdString());
    st->set_name(ui_->stationTable->item(i,kName)->text().toStdString());
    st->set_av12type(static_cast<meteo::surf::StationType>(ui_->stationTable->item(i,kRole)->data(Qt::UserRole).toInt()));
    st->set_type(ui_->stationTable->item(i,kIndex)->data(Qt::UserRole).toInt());
    QSpinBox* echelon;
    echelon = qobject_cast<QSpinBox*>(ui_->stationTable->cellWidget(i,kEchelon));
    st->set_echelon(echelon->value());
    QDateTime dateTime;
    dateTime.setDate(ui_->dateStart->date());
    dateTime.setTime(qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,kTime))->time());
    st->set_date_time(dateTime.toString(Qt::ISODate).toStdString());
  }
}

void Av12Widget::slotShowMenu(QPoint point)
{
  if(ui_->stationTable->rowCount() > 0){
    if (nullptr == menu_) {
      menu_ = new QMenu(this);
      removeAct_ = new QAction("Удалить пункт",this);
      QObject::connect(removeAct_, SIGNAL(triggered()), this, SLOT(slotRemoveStation()));
    }
    menu_->addAction(removeAct_);
    menu_->popup(ui_->stationTable->viewport()->mapToGlobal(point));
  }
}

void Av12Widget::slotSaveState()
{
  settings_->setValue( objectName()+".weatherRoute", ui_->forecastWeatherRoute->toPlainText());
  settings_->setValue( objectName()+".birdsRoute", ui_->ornitRoute->toPlainText());
  settings_->setValue( objectName()+".weatherArrival", ui_->forecastWeatherArrival->toPlainText());
  settings_->setValue( objectName()+".birdsArrival", ui_->ornitArrival->toPlainText());
  settings_->setValue( objectName()+".textForecastAv12d", ui_->forecastArrivalAv12d->toPlainText());
  settings_->setValue( objectName()+".forecastArrivalAv12d", ui_->textForecastAv12d->toPlainText());
  settings_->setValue( objectName()+".type", ui_->typeBox->currentIndex());
  settings_->setValue( objectName()+".echelon", ui_->echelonBox->value());
  settings_->setValue( objectName()+".type_executer", ui_->executerBox->currentIndex());
  settings_->setValue( objectName()+".executer", ui_->executorName->text());
  settings_->setValue( objectName()+".rowCount"+ ui_->typeBox->currentText(),ui_->stationTable->rowCount());
  settings_->setValue( objectName()+".columnCount"+ ui_->typeBox->currentText(),ui_->stationTable->columnCount());
  for (int i=0,rowC=ui_->stationTable->rowCount();i<rowC;++i) {
    for(int j=0,columnC=ui_->stationTable->columnCount();j<columnC;++j) {
      if (kIndex == j) {
        settings_->setValue( objectName() +".tabledata" + ui_->typeBox->currentIndex() +QString::number(i)+QString::number(j)
                             + "Fi", ui_->stationTable->item(i,j)->data(kFi).toDouble());
        settings_->setValue( objectName() +".tabledata" + ui_->typeBox->currentIndex() +QString::number(i)+QString::number(j)
                             + "La", ui_->stationTable->item(i,j)->data(kLa).toDouble());
      }
      else if ( kTime == j ) {
        QString time = qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i,j))->time().toString();
        settings_->setValue( objectName() +".tabletext" + ui_->typeBox->currentText() + QString::number(i)+QString::number(j), time);
        continue;
      }
      else if ( kEchelon == j ) {
        int echelon = qobject_cast<QSpinBox*>(ui_->stationTable->cellWidget(i,j))->value();
        settings_->setValue( objectName() +".tabletext" + ui_->typeBox->currentText() + QString::number(i)+QString::number(j), QString::number(echelon));
        continue;
      }
      settings_->setValue( objectName() +".tabletext" + ui_->typeBox->currentText() +QString::number(i)+QString::number(j),
                          ui_->stationTable->item(i,j)->text());
      settings_->setValue( objectName() +".tabledata"+ ui_->typeBox->currentText() +QString::number(i)+QString::number(j),
                          ui_->stationTable->item(i,j)->data(Qt::UserRole));
    }
  }
}

void Av12Widget::slotLoadState()
{
  ui_->forecastWeatherRoute->setText( settings_->value(this->objectName()+".weatherRoute").toString());
  ui_->ornitRoute->setText( settings_->value(this->objectName()+".birdsRoute").toString());
  ui_->forecastWeatherArrival->setText( settings_->value(this->objectName()+".weatherArrival").toString());
  ui_->ornitArrival->setText( settings_->value(this->objectName()+".birdsArrival").toString());
  ui_->typeBox->setCurrentIndex(settings_->value(this->objectName()+".type").toInt());
  ui_->textForecastAv12d->setText( settings_->value( objectName() + ".textForecastAv12d" ).toString() );
  ui_->forecastArrivalAv12d->setText( settings_->value( objectName() + ".forecastArrivalAv12d").toString() );
  if(ui_->typeBox->currentIndex() == BillType::kPerelet) {
    ui_->arrivalBut->setEnabled(true);
  }
  ui_->echelonBox->setValue(settings_->value(this->objectName()+".echelon").toInt());
  ui_->executerBox->setCurrentIndex( settings_->value(this->objectName()+".type_executer").toInt());
  ui_->executorName->setText( settings_->value(this->objectName()+".executer").toString());
  loadStations();
  for (int i =0, sz = ui_->stationTable->rowCount(); i < sz; ++i ) {
    if (ui_->stationTable->item(i, 0)->text() == "Пункт посадки") {
      QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i, kTime));
      QObject::connect(timeEdit, SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeLandingChanged(QTime)));
      QTime time = timeEdit->time();
      ui_->landingTime->setTime( time );
    }
    if (ui_->stationTable->item(i, 0)->text() == "Пункт вылета") {
      QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(ui_->stationTable->cellWidget(i, kTime));
      QObject::connect(timeEdit,SIGNAL(timeChanged(QTime)),this, SLOT(slotTimeDepartureChanged(QTime)));
      QTime time = timeEdit->time();
      ui_->departureTime->setTime( time );
    }
  }
}


}
}
