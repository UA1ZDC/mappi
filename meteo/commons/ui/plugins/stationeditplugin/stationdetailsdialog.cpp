#include "stationdetailsdialog.h"
#include "ui_stationdetailsdialog.h"

#include <qmessagebox.h>
#include "stationeditwidget.h"
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

const QString kDegreeSymbol = "°";
const QString kDegreePostfix = " " + kDegreeSymbol;
const QString kMeterSymbol = QObject::tr("м");
const QString kMeterPostfix = " " + kMeterSymbol;
//const double kCoordDelta = 2.4e-6; //  одна угловая секунда 4.84813681109536e-06 радиана (geoPointEditor работает с радианами)
/*
закомментированное выше значение полноценно не работает (а именно, несмотря на то, что пользователь не трогал координаты, они всё же определяются
как измененные) из-за того (я предполагаю), что где-то в недрах coordData есть строчки
void CoordData::setDecDegree(double dd)
{
  dd += .0001;
.0001 в градусах - это 0.36 угловой минуты
отсюда погрешность измерения выставим в 4.17e-06, что составляет 0.8601242420503918 угловой минуты в радианах
(осознавая при этом, что возникнут ситуации, когда изменение координат на одну угловую секунду не будет детектировано как изменение)
*/
const double kCoordDelta = 4.17e-6;
const double kFloatDoubleDelta = 1e-5; // экспериментально определенная погрешность поступающих значений типа double
const int kMinButtonHeight = 32;
const std::string kStubStationId = "";

StationDetailsDialog::StationDetailsDialog(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::StationDetailsWidget)
{
  ui_->setupUi(this);

  ui_->geoPointEditor->setOneLine(false);
  ui_->geoPointEditor->setAltitudeVisible(true);
  ui_->geoPointEditor->setMapButtonVisible(true);
  ui_->geoPointEditor->setSearchVisible(false);

  //QRegExp rxHydro("^[0-9]{0,}[,\\.]?[0-9]{0,}$");
  QRegExp rxHydro("^[0-9]{0,}[,\\.]?[0-9]{0,}( м)?$");
  QValidator *validatorHydro = new QRegExpValidator(rxHydro, this);
  ui_->dangerLevelLineEdit->setValidator(validatorHydro);
  ui_->floodLevelLineEdit->setValidator(validatorHydro);
  ui_->poimaLevelLineEdit->setValidator(validatorHydro);

  //QRegExp rxAir("^[0-9]{0,}[,\\.]?[0-9]{0,}°?$");
  QRegExp rxAir("^[0-9]{0,}[,\\.]?[0-9]{0,}( °)?$");
  QValidator *validatorAir = new QRegExpValidator(rxAir, this);
  ui_->airstripDirectionLineEdit->setValidator(validatorAir);

  QRegExp rxIndex("^[0-9]{0,}$");
  QValidator *validatorIndex = new QRegExpValidator(rxIndex, this);
  ui_->indexLineEdit->setValidator(validatorIndex);

  QRegExp rxIcao("^\\S{0,4}");
  QValidator *validatorIcao = new QRegExpValidator(rxIcao, this);
  ui_->ccccLineEdit->setValidator(validatorIcao);

  connect(ui_->saveChangesPushButton, SIGNAL(clicked(bool)),
          this, SLOT(saveChanges()));
  connect(ui_->stationTypeComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(switchWidgetsAvailability()));
  connect(ui_->airstripDirectionLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkAirstripLineEdit()));
  connect(ui_->dangerLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkHydroLineEdit()));
  connect(ui_->floodLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkHydroLineEdit()));
  connect(ui_->poimaLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkHydroLineEdit()));
  connect(ui_->ccccLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkCcccLineEdit()));

  connect(ui_->indexLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->ccccLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->nameRuLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->nameShortLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->nameEnLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->countryComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(checkChanges()));
  connect(ui_->stationTypeComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(checkChanges()));
  connect(ui_->geoPointEditor, SIGNAL(coordChanged(meteo::GeoPoint)),
          this, SLOT(checkChanges()));
  connect(ui_->geoPointEditor, SIGNAL(changed()),
          this, SLOT(checkChanges()));

  connect(ui_->airstripDirectionLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->poimaLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->floodLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));
  connect(ui_->dangerLevelLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(checkChanges()));

  // init
  ui_->indexLineEdit->blockSignals(true);
  initWidgetBlockings();
}

StationDetailsDialog::~StationDetailsDialog()
{
  if ( nullptr != ui_ ) {
    delete ui_;
    ui_ = nullptr;
  }
}

void StationDetailsDialog::setData(sprinf::Station st, bool force)
{
  // force - при установке в true устанавливает новые данные, игнорируя имеющиеся в виджете изменения
  if ( true == ui_->saveChangesPushButton->isEnabled() &&
       true == isVisible() &&
       false == force ) {
    QMessageBox askBox;
    askBox.setWindowTitle(QObject::tr("Подтверждение сохранения изменений"));
    askBox.setText(QObject::tr("Характеристики метеостанции изменены. Сохранить изменения?"));
    askBox.setWindowFlags( askBox.windowFlags() & (~Qt::WindowTitleHint) );
    QPushButton *btn = askBox.addButton(QObject::tr("Не сохранять"), QMessageBox::RejectRole);
    btn->setMinimumHeight(kMinButtonHeight);
    btn = askBox.addButton(QObject::tr("Сохранить"), QMessageBox::AcceptRole);
    btn->setMinimumHeight(kMinButtonHeight);
    int result = askBox.exec();

    if ( QMessageBox::Accepted == result ) {
      saveChanges();
    }
  }
  blockChildrenSignals(true); // сигналы виджетов блокируются, чтобы во время установки данных не выполнялась проверка на наличие изменений

  if ( true == st.has_name() ) {
    if ( true == st.name().has_international() ) {
      ui_->nameEnLineEdit->setText(QString::fromStdString(st.name().international()));
    }
    else {
      ui_->nameEnLineEdit->setText(QString(""));
    }
    if ( true == st.name().has_rus() ) {
      ui_->nameRuLineEdit->setText(QString::fromStdString(st.name().rus()));
    }
    else {
      ui_->nameRuLineEdit->setText(QString(""));
    }
    if ( true == st.name().has_short_() ) {
      ui_->nameShortLineEdit->setText(QString::fromStdString(st.name().short_()));
    }
    else {
      ui_->nameShortLineEdit->setText(QString(""));
    }
  }
  else {
    ui_->nameEnLineEdit->setText(QString(""));
    ui_->nameRuLineEdit->setText(QString(""));
    ui_->nameShortLineEdit->setText(QString(""));
  }

  if ( true == st.has_position() ) {
    if ( true == st.position().has_lat_radian() ) {
      ui_->geoPointEditor->setLatRadian(st.position().lat_radian());
    }
    else {
      ui_->geoPointEditor->setLatRadian(0.0);
    }
    if ( true == st.position().has_lon_radian() ) {
      ui_->geoPointEditor->setLonRadian(st.position().lon_radian());
    }
    else {
      ui_->geoPointEditor->setLonRadian(0.0);
    }
    if ( true == st.position().has_height_meters() ) {
      ui_->geoPointEditor->setAltitudeMeter(st.position().height_meters());
    }
    else {
      ui_->geoPointEditor->setAltitudeMeter(0.0);
    }
  }
  else {
    ui_->geoPointEditor->setLatRadian(0.0);
    ui_->geoPointEditor->setLonRadian(0.0);
    ui_->geoPointEditor->setAltitudeMeter(0.0);
  }

  if ( true == st.has_type() ) {
    int t = st.type();

    for ( int i = 0; i < ui_->stationTypeComboBox->count(); i++) {
      if ( ui_->stationTypeComboBox->itemData(i).toInt() == t ) {
        ui_->stationTypeComboBox->setCurrentIndex(i);
        break;
      }
    }
  }
  else {
    ui_->stationTypeComboBox->setCurrentIndex(0);
  }

  if ( true == st.has_country() ) {
    int countryNumber = st.country().number();

    for ( int i = 0; i < ui_->countryComboBox->count(); i++) {
      if ( ui_->countryComboBox->itemData(i).toInt() == countryNumber ) {
        ui_->countryComboBox->setCurrentIndex(i);
        break;
      }
    }
  }
  else {
    ui_->countryComboBox->setCurrentIndex(0);
  }

  if ( true == st.has_index() ) {
    int idx = st.index();

    ui_->indexLineEdit->setText(QString::number(idx));
  }
  else {
    ui_->indexLineEdit->setText(QString(""));
  }

  if ( true == st.has_cccc() ) {
    QString cccc = QString::fromStdString(st.cccc());

    ui_->ccccLineEdit->setText(cccc);
  }
  else {
    ui_->ccccLineEdit->setText(QString(""));
  }

  if ( true == st.has_airstrip_direction() ) {
    double airstrip_direction = st.airstrip_direction();

    ui_->airstripDirectionLineEdit->setText(QString::number(airstrip_direction));
  }
  else {
    ui_->airstripDirectionLineEdit->setText(QString(""));
  }

  if ( true == st.has_hydro_levels() ) {
    if ( true == st.hydro_levels().has_floodplain() ) {
      double poima = st.hydro_levels().floodplain();
      ui_->poimaLevelLineEdit->setText(QString::number(poima));
    }
    else {
      ui_->poimaLevelLineEdit->setText(QString(""));
    }
    if ( true == st.hydro_levels().has_flooding() ) {
      double zatop = st.hydro_levels().flooding();
      ui_->floodLevelLineEdit->setText(QString::number(zatop));
    }
    else {
      ui_->floodLevelLineEdit->setText(QString(""));
    }
    if ( true == st.hydro_levels().has_danger_level() ) {
      double danger_level = st.hydro_levels().danger_level();
      ui_->dangerLevelLineEdit->setText(QString::number(danger_level));
    }
    else {
      ui_->dangerLevelLineEdit->setText(QString(""));
    }
  }
  else {
    ui_->poimaLevelLineEdit->setText(QString(""));
    ui_->floodLevelLineEdit->setText(QString(""));
    ui_->dangerLevelLineEdit->setText(QString(""));
  }

  station_ = st;
  switchWidgetsAvailability();
  checkAirstripLineEdit();
  checkHydroLineEdit();
  blockChildrenSignals(false);


  ui_->saveChangesPushButton->setEnabled(false);
}

void StationDetailsDialog::setTypesMap(QMap<QString, int> typesMap)
{
  stationTypesMap_ = typesMap;

  QList <QString> auxKeys = stationTypesMap_.keys();

  for (int i = 0; i < stationTypesMap_.size(); i++) {
    ui_->stationTypeComboBox->addItem(auxKeys.at(i),
                                      QVariant(stationTypesMap_.value(auxKeys.at(i))));
  }
  ui_->stationTypeComboBox->setCurrentIndex(0);
}

void StationDetailsDialog::setCountriesMap(QMap<int, QString> countriesMap)
{
  countriesMap_ = countriesMap;

  QMap<QString, int> inverseCountriesMap;
  QList<int> countryNumbers = countriesMap_.keys();

  for ( int i = 0; i < countryNumbers.size(); i++ ) {
    inverseCountriesMap.insert(countriesMap_.value(countryNumbers.at(i)),
                               countryNumbers.at(i));
  }

  QList<QString> countryNames = inverseCountriesMap.keys();
  for ( int i = 0; i < countryNames.size(); i++ ) {
    ui_->countryComboBox->addItem(countryNames.at(i),
                                  QVariant(inverseCountriesMap.value(countryNames.at(i))));
  }
  ui_->countryComboBox->setCurrentIndex(0);
  ui_->countryComboBox->count();
}


sprinf::Station StationDetailsDialog::getEditedStation()
{
  meteo::sprinf::Station editedSt;

  if ( true == station_.has_id() ) {
    editedSt.set_id(station_.id());
  }
  else {
    editedSt.set_id(kStubStationId);
  }

  // index
  if ( true == ui_->indexLineEdit->isEnabled() &&
       QString("") != ui_->indexLineEdit->text().trimmed()) {
    editedSt.set_index(ui_->indexLineEdit->text().trimmed().toInt());
  }

  // type
  if ( ui_->stationTypeComboBox->currentIndex() != -1 ) {
    editedSt.set_type(ui_->stationTypeComboBox->currentData().toInt());
  }

  // name
  meteo::sprinf::StationName stName;
  bool hasName = false;
  if ( QString("") != ui_->nameRuLineEdit->text().trimmed() ) {
    stName.set_rus(ui_->nameRuLineEdit->text().trimmed().toStdString());
    hasName = true;
  }
  if ( QString("") != ui_->nameShortLineEdit->text().trimmed() ) {
    stName.set_short_(ui_->nameShortLineEdit->text().trimmed().toStdString());
    hasName = true;
  }
  if ( QString("") != ui_->nameEnLineEdit->text().trimmed() ) {
    stName.set_international(ui_->nameEnLineEdit->text().trimmed().toStdString());
    hasName = true;
  }
  if ( true == hasName ) {
    editedSt.mutable_name()->CopyFrom(stName);
  }

  // coordinates inc. elevation
  meteo::GeoPointPb geoPt;
  double cmpLat = 0.0, cmpLon = 0.0, cmpAlt = 0.0;
  if ( true == station_.has_position() ) {
    if ( true == station_.position().has_lat_radian() ) {
      cmpLat = station_.position().lat_radian();
    }
    if ( true == station_.position().has_lon_radian() ) {
      cmpLon = station_.position().lon_radian();
    }
    if ( true == station_.position().has_height_meters() ) {
      cmpAlt = station_.position().height_meters();
    }
  }
  if ( fabs(cmpLat - ui_->geoPointEditor->latRadian()) > kCoordDelta ) {
    geoPt.set_lat_radian(ui_->geoPointEditor->latRadian());
  }
  else {
    geoPt.set_lat_radian(cmpLat);
  }
  if ( fabs(cmpLon - ui_->geoPointEditor->lonRadian())  > kCoordDelta ) {
    geoPt.set_lon_radian(ui_->geoPointEditor->lonRadian());
  }
  else {
    geoPt.set_lon_radian(cmpLon);
  }
  if ( false == MnMath::isEqual(cmpAlt, ui_->geoPointEditor->altitudeMeter()) ) {
    geoPt.set_height_meters(ui_->geoPointEditor->altitudeMeter());
  }
  else {
    geoPt.set_height_meters(cmpAlt);
  }
  editedSt.mutable_position()->CopyFrom(geoPt);

  // icao = cccc
  if ( true == ui_->ccccLineEdit->isEnabled() &&
       QString("") != ui_->ccccLineEdit->text().trimmed() ) {
    editedSt.set_cccc(ui_->ccccLineEdit->text().toUpper().toStdString());
  }

  if ( true == station_.has_data_type() ) {
    editedSt.set_data_type(station_.data_type());
  }

  // station пропускается, т.к. является зависимым полем

  // country
  if ( ui_->countryComboBox->currentIndex() != -1 ) {
    editedSt.mutable_country()->set_number(ui_->countryComboBox->currentData().toInt()); //FIXME сохранение!
  }

  // hydro levels
  meteo::sprinf::HydroLevels hydroLvls;
  bool hasHydroLvls = false;
  double cmpPoima = 0.0, cmpFlood = 0.0, cmpDanger = 0.0;
  if ( true == station_.has_hydro_levels() ) {
    if ( true == station_.hydro_levels().has_floodplain() ) {
      cmpPoima = station_.hydro_levels().floodplain();
    }
    if ( true == station_.hydro_levels().has_flooding() ) {
      cmpFlood = station_.hydro_levels().flooding();
    }
    if ( true == station_.hydro_levels().has_danger_level() ) {
      cmpDanger = station_.hydro_levels().danger_level();
    }
  }
  if ( true == ui_->poimaLevelLineEdit->isEnabled() &&
       QString("") != ui_->poimaLevelLineEdit->text().trimmed() ) {
    hasHydroLvls = true;
    double poimaLvl = getPoimaLevel();
    if ( false == MnMath::isEqual(cmpPoima, poimaLvl) ) {
      hydroLvls.set_floodplain(poimaLvl);
    }
    else {
      hydroLvls.set_floodplain(cmpPoima);
    }
  }
  if ( true == ui_->floodLevelLineEdit->isEnabled() &&
       QString("") != ui_->floodLevelLineEdit->text().trimmed() ) {
    hasHydroLvls = true;
    double floodLvl = getFloodLevel();
    if ( false == MnMath::isEqual(cmpFlood, floodLvl) ) {
      hydroLvls.set_flooding(floodLvl);
    }
    else {
      hydroLvls.set_flooding(cmpFlood);
    }
  }
  if ( true == ui_->dangerLevelLineEdit->isEnabled() &&
       QString("") != ui_->dangerLevelLineEdit->text().trimmed() ) {
    hasHydroLvls = true;
    double dangerLvl = getDangerLevel();
    if ( false == MnMath::isEqual(cmpDanger, dangerLvl) ) {
      hydroLvls.set_danger_level(dangerLvl);
    }
    else {
      hydroLvls.set_danger_level(cmpDanger);
    }
  }
  if ( true == hasHydroLvls ) {
    editedSt.mutable_hydro_levels()->CopyFrom(hydroLvls);
  }

  // airstrip direction,
  QString t = ui_->airstripDirectionLineEdit->text().remove(kDegreeSymbol).trimmed().replace(QString(","), QString("."));
  if ( QString("") != t &&
       true == ui_->airstripDirectionLineEdit->isEnabled()) {
    editedSt.set_airstrip_direction(getAirstripDirection());
  }

  return editedSt;
}

/*! Хардкод, определяющий, какие виджеты блокируются при определенных выбранных типах станций
 */
void StationDetailsDialog::initWidgetBlockings()
{

  allSwitchedWidgets_.append(ui_->ccccLineEdit);
  allSwitchedWidgets_.append(ui_->indexLineEdit);
  allSwitchedWidgets_.append(ui_->airstripDirectionLineEdit);
  allSwitchedWidgets_.append(ui_->floodLevelLineEdit);
  allSwitchedWidgets_.append(ui_->poimaLevelLineEdit);
  allSwitchedWidgets_.append(ui_->dangerLevelLineEdit);

  // Синоптическая фиксированная наземная станция
  QList<QLineEdit*> enabledForType0;
  enabledForType0.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(0, enabledForType0);

  // Синоптическая подвижная станция
  QList<QLineEdit*> enabledForType1;
  enabledForType1.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(1, enabledForType1);

  // аэродромная система
  QList<QLineEdit*> enabledForType2;
  enabledForType2.append(ui_->ccccLineEdit);
  enabledForType2.append(ui_->airstripDirectionLineEdit);

  enabledWidgetsByType_.insert(2, enabledForType2);

  // аэрологическая фиксированная наземная станция
  QList<QLineEdit*> enabledForType3;
  enabledForType3.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(3, enabledForType3);

  // Аэрологическая подвижная станция
  QList<QLineEdit*> enabledForType4;
  enabledForType4.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(4, enabledForType4);

  // гидрологический пост
  QList<QLineEdit*> enabledForType5;
  enabledForType5.append(ui_->indexLineEdit);
  enabledForType5.append(ui_->floodLevelLineEdit);
  enabledForType5.append(ui_->poimaLevelLineEdit);
  enabledForType5.append(ui_->dangerLevelLineEdit);

  enabledWidgetsByType_.insert(5, enabledForType5);

  // Океан (по глубинам)
  QList<QLineEdit*> enabledForType6;
  enabledForType6.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(6, enabledForType6);

  // Радиолокационная система, передающая информацию о тропических циклонах
  QList<QLineEdit*> enabledForType7;
  enabledForType7.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(7, enabledForType7);

  // Радиолокационная система, данные на карте
  QList<QLineEdit*> enabledForType8;
  enabledForType8.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(8, enabledForType8);

  // Геофизическая
  QList<QLineEdit*> enabledForType9;
  enabledForType9.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(9, enabledForType9);

  // Консультативные центры по вулканическому пеплу
  QList<QLineEdit*> enabledForType10;
  enabledForType10.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(10, enabledForType10);

  // Районы полетной информации
  QList<QLineEdit*> enabledForType11;
  enabledForType11.append(ui_->ccccLineEdit);

  enabledWidgetsByType_.insert(11, enabledForType11);

  // Данные с борта самолёта
  QList<QLineEdit*> enabledForType12;
  enabledForType12.append(ui_->indexLineEdit);

  enabledWidgetsByType_.insert(12, enabledForType12);

  // Справочник станций для обмена служебными сообщениями
  QList<QLineEdit*> enabledForType13;
  //enabledForType13.append(ui_->indexLineEdit);
  enabledForType13.append(ui_->ccccLineEdit);

  enabledWidgetsByType_.insert(13, enabledForType13);

  // аэродромная система (военный)
  QList<QLineEdit*> enabledForType14;
  enabledForType14.append(ui_->ccccLineEdit);
  enabledForType14.append(ui_->airstripDirectionLineEdit);

  enabledWidgetsByType_.insert(14, enabledForType14);
}

bool StationDetailsDialog::hasChanges()
{
  // index
  int cmpIndex = 0; // обеспечивает false при invalid значениях в поле ввода индекса
  if ( true == station_.has_index() ) {
    cmpIndex = station_.index();
  }
  if ( true == ui_->indexLineEdit->isEnabled() &&
       cmpIndex != ui_->indexLineEdit->text().toInt() ) {
    //debug_log << "index vary"; // TODO REMOVE
    return  true;
  }
  
  // icao = cccc
  QString cmpCCCC("");
  if ( true == station_.has_cccc() ) {
    cmpCCCC = QString::fromStdString(station_.cccc());
  }
  if ( true == ui_->ccccLineEdit->isEnabled() &&
       cmpCCCC != ui_->ccccLineEdit->text() ) {
    //debug_log << "cccc vary"; // TODO REMOVE
    return true;
  }
  
  // name
  QString cmpRus(""), cmpShort(""), cmpIntern("");
  if ( station_.has_name() == true ) {
    if ( true == station_.name().has_rus() ) {
      cmpRus = QString::fromStdString(station_.name().rus());
    }
    if ( true == station_.name().has_short_() ) {
      cmpShort = QString::fromStdString(station_.name().short_());
    }
    if ( true == station_.name().has_international() ) {
      cmpIntern = QString::fromStdString(station_.name().international());
    }
  }
  if ( cmpRus != ui_->nameRuLineEdit->text().trimmed() ||
       cmpShort != ui_->nameShortLineEdit->text().trimmed() ||
       cmpIntern != ui_->nameEnLineEdit->text().trimmed() ) {
    //debug_log << "names vary"; // TODO REMOVE
    return true;
  }

  // country
  int cmpCountry = -1;
  if ( true == station_.has_country() ) {
    cmpCountry = station_.country().number();
  }
  if ( cmpCountry != -1 ) {
    if ( ui_->countryComboBox->currentIndex() != -1 ) {
      int cbCountry = ui_->countryComboBox->currentData().toInt();
      if ( cmpCountry != cbCountry ) {
        //debug_log << "country vary"; // TODO REMOVE
        return true;
      }
    }
  }
  else { // cmpCountry == -1
    if ( ui_->countryComboBox->currentIndex() != -1 ) {
      //debug_log << "country vary 2"; // TODO REMOVE
      return true;      
    }
  }

  // type
  int cmpType = -1;
  if ( true == station_.has_type() ) {
    cmpType = station_.type();
  }
  if ( cmpType != -1 ) {
    if ( ui_->stationTypeComboBox->currentIndex() != -1 ) {
      int cbType = ui_->stationTypeComboBox->currentData().toInt();
      if ( cmpType != cbType ) {
        //debug_log << "type vary"; // TODO REMOVE
        return true;        
      }
    }
  }
  else { // cmpType == -1
    if ( ui_->stationTypeComboBox->currentIndex() != -1 ) {
      //debug_log << "type vary 2"; // TODO REMOVE
      //debug_log << "current_index " << ui_->stationTypeComboBox->currentIndex();
      return true;      
    }
  }

  // coordinates inc. elevation
  double cmpLat = 0.0, cmpLon = 0.0, cmpAlt = 0.0;
  if ( true == station_.has_position() ) {
    if ( true == station_.position().has_lat_radian() ) {
      cmpLat = station_.position().lat_radian();
    }
    if ( true == station_.position().has_lon_radian() ) {
      cmpLon = station_.position().lon_radian();
    }
    if ( true == station_.position().has_height_meters() ) {
      cmpAlt = station_.position().height_meters();
    }
  }
  // с координатами MnMath::isEqual не работает, ложно определяет как не равные
  if (  fabs(cmpLat - ui_->geoPointEditor->latRadian()) > kCoordDelta ||
        fabs(cmpLon - ui_->geoPointEditor->lonRadian())  > kCoordDelta ||
        fabs(cmpAlt - ui_->geoPointEditor->altitudeMeter()) > kFloatDoubleDelta ) {
    //debug_log << "coordinates vary"; // TODO REMOVE
    return true;    
  }

  // airstrip direction,
  double cmpAirstripDir = 0.0;
  if ( true == station_.has_airstrip_direction() ) {
    cmpAirstripDir = station_.airstrip_direction();
  }
  if ( true == ui_->airstripDirectionLineEdit->isEnabled() &&
       fabs(cmpAirstripDir - getAirstripDirection()) > kFloatDoubleDelta ) {
    //debug_log << "runway dir vary"; // TODO REMOVE
    return true;    
  }

  // hydro levels
  double cmpPoima = 0.0, cmpFlood = 0.0, cmpDanger = 0.0;
  if ( true == station_.has_hydro_levels() ) {
    if ( true == station_.hydro_levels().has_floodplain() ) {
      cmpPoima = station_.hydro_levels().floodplain();
    }
    if ( true == station_.hydro_levels().has_flooding() ) {
      cmpFlood = station_.hydro_levels().flooding();
    }
    if ( true == station_.hydro_levels().has_danger_level() ) {
      cmpDanger = station_.hydro_levels().danger_level();
    }
  }
  if ( ( true == ui_->poimaLevelLineEdit->isEnabled() &&
         fabs(cmpPoima - getPoimaLevel()) > kFloatDoubleDelta ) ||
       ( true == ui_->floodLevelLineEdit->isEnabled() &&
         fabs(cmpFlood - getFloodLevel()) > kFloatDoubleDelta ) ||
       ( true == ui_->dangerLevelLineEdit->isEnabled() &&
         fabs(cmpDanger - getDangerLevel()) > kFloatDoubleDelta ) ) {
    //debug_log << "hydro levels vary"; // TODO REMOVE
    return true;
  }

  return false;
}

void StationDetailsDialog::checkAirstripLineEdit()
{
  QString t = ui_->airstripDirectionLineEdit->text();
  if ( t == "" ) {
    return;
  }

  QString strippedT = t.remove(kDegreePostfix).trimmed();
  if ( strippedT == "" ) {
    ui_->airstripDirectionLineEdit->clear();
    return;
  }

  if ( false == t.endsWith(kDegreePostfix) ) {
    int cursorPos = ui_->airstripDirectionLineEdit->cursorPosition();
    ui_->airstripDirectionLineEdit->setText(strippedT + kDegreePostfix);
    ui_->airstripDirectionLineEdit->setCursorPosition(cursorPos);
  }
}

/*! Функция предназначена для проверки и форматирования значений в полях ввода
 * опасных уровней для гидропостов
 * Действует аналогично checkAirstripLineEdit(), но дополнительно проверяет,
 * от какого поля поступил сигнал.
 * Если это одно из трех полей ввода опасных уровней, то проверяется это поле.
 * Если отправитель равен 0, то форматируются все три поля (это позволяет вызывать функцию
 * из кода для проверки всех полей после обновления данных станции).
 */
void StationDetailsDialog::checkHydroLineEdit()
{
  QList <QLineEdit *> editsList;
  editsList.append(ui_->dangerLevelLineEdit);
  editsList.append(ui_->floodLevelLineEdit);
  editsList.append(ui_->poimaLevelLineEdit);

  QLineEdit *senderPtr = static_cast<QLineEdit*>(sender());
  if ( nullptr != senderPtr ) {
    if ( false == editsList.contains(senderPtr) ) { // недопустимая ситуация
      return;
    }
    editsList.clear();
    editsList.append(senderPtr); // форматировать будем только отправителя сигнала
  }

  foreach (QLineEdit *lineEdit, editsList) {
    QString t = lineEdit->text();
    if ( t == "" ) {
      continue;
    }

    QString strippedT = t.remove(kMeterPostfix).trimmed();
    if ( strippedT == "" ) {
      lineEdit->clear();
      continue;
    }

    if ( false == t.endsWith(kMeterPostfix) ) {
      int cursorPos = lineEdit->cursorPosition();
      lineEdit->setText(strippedT + kMeterPostfix);
      lineEdit->setCursorPosition(cursorPos);
    }
  }
}

void StationDetailsDialog::checkCcccLineEdit()
{
  QString t = ui_->ccccLineEdit->text();
  if ( t == "" ||
       false == ui_->ccccLineEdit->isEnabled()) {
    return;
  }

  int cursorPos = ui_->ccccLineEdit->cursorPosition();
  ui_->ccccLineEdit->setText(ui_->ccccLineEdit->text().toUpper());
  ui_->ccccLineEdit->setCursorPosition(cursorPos);
}

double StationDetailsDialog::getAirstripDirection()
{
  QString t = ui_->airstripDirectionLineEdit->text().remove(kDegreeSymbol).trimmed().replace(QString(","), QString("."));
  return t.toDouble();
}

double StationDetailsDialog::getDangerLevel()
{
  QString t = ui_->dangerLevelLineEdit->text().remove(kMeterSymbol).trimmed().replace(QString(","), QString("."));
  return t.toDouble();
}

double StationDetailsDialog::getFloodLevel()
{
  QString t = ui_->floodLevelLineEdit->text().remove(kMeterSymbol).trimmed().replace(QString(","), QString("."));
  return t.toDouble();
}

double StationDetailsDialog::getPoimaLevel()
{
  QString t = ui_->poimaLevelLineEdit->text().remove(kMeterSymbol).trimmed().replace(QString(","), QString("."));
  return t.toDouble();
}

void StationDetailsDialog::saveChanges()
{
  StationEditWidget* parent = qobject_cast<StationEditWidget*>(this->parent()->parent());
  if ( nullptr == parent ) {
    error_log << "Ошибка обращения к виджету окна";
    return;
  }
  sprinf::Station st = getEditedStation();
  parent->saveStationToDb(st);
}

void StationDetailsDialog::switchWidgetsAvailability()
{
  int stType = ui_->stationTypeComboBox->currentData().toInt();

  // если в map искомого ключа нет, разблокируем все виджеты
  QList<QLineEdit*> enabledWidgets = enabledWidgetsByType_.value(stType, allSwitchedWidgets_);

  foreach (QLineEdit* le, allSwitchedWidgets_) {
    if ( true == enabledWidgets.contains(le) ) {
      le->setEnabled(true);
    }
    else {
      le->setEnabled(false);
    }
  }
}

void StationDetailsDialog::checkChanges()
{
  if ( true == hasChanges() ) {
    ui_->saveChangesPushButton->setEnabled(true);
  }
  else {
    ui_->saveChangesPushButton->setEnabled(false);
  }
}

void StationDetailsDialog::blockChildrenSignals(bool block)
{
  ui_->indexLineEdit->blockSignals(block);
  ui_->ccccLineEdit->blockSignals(block);
  ui_->nameRuLineEdit->blockSignals(block);
  ui_->nameShortLineEdit->blockSignals(block);
  ui_->nameEnLineEdit->blockSignals(block);
  ui_->countryComboBox->blockSignals(block);
  ui_->stationTypeComboBox->blockSignals(block);
  ui_->geoPointEditor->blockSignals(block);
  ui_->airstripDirectionLineEdit->blockSignals(block);
  ui_->poimaLevelLineEdit->blockSignals(block);
  ui_->floodLevelLineEdit->blockSignals(block);
  ui_->dangerLevelLineEdit->blockSignals(block);
}

} // namespace meteo {
} // namespace map {
