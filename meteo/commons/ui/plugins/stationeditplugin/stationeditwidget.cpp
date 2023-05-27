#include "ui_stationeditwidget.h"
#include "stationeditwidget.h"
#include <meteo/commons/global/global.h>
#include <qsettings.h>
//#include <commons/geobasis/geopoint.h>
//#include <meteo/commons/ui/custom/stationwidget.h>
//#include <meteo/commons/ui/mainwindow/widgethandler.h>
//#include <meteo/commons/proto/forecast.pb.h>
//#include <meteo/commons/proto/sprinf.pb.h>

//#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
//#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo {
namespace map {
/*
enum {
  kStationName  = 1,
  kStationIndex = 0,
  kCoordinate   = 2,
  kHeight       = 3,
  kId           = 4,
  kActive       = 5,
  kTypeData     = 6
};
*/
const int kAllTypes = -2;
const int kRowHeight = 30;
const int kIndexColumn = 3;
const int kMinButtonHeight = 32;
const double kDefaultRequestTimeout = 20.0; // in seconds
const QString kSettingsFile = MnCommon::userSettingsPath() + QObject::tr("/stations_editor") + "/stations_editor.ini";


// обеспечивает правильную (с точки зрения человека) сортировку по столбцу с индексом
class StationsListWidgetItem : public QTreeWidgetItem
{
public:
  StationsListWidgetItem(QTreeWidget* parent, QStringList strings) : QTreeWidgetItem(parent, strings) {}

private:
  bool operator < (const QTreeWidgetItem &other) const
  {
    QTreeWidgetItem* header = treeWidget()->headerItem();
    int col = treeWidget()->sortColumn();
    if ( header != nullptr && col == kIndexColumn ) {
      return text(col).toInt() < other.text(col).toInt();
    }
    return text(col).toLower() < other.text(col).toLower();
  }
};

StationEditWidget::StationEditWidget(meteo::app::MainWindow* parent) :
  QWidget(parent),
  ui_(new Ui::StationEditWidget),
  mainWindow_(parent),
  stationTypes_(nullptr),
  stations_(nullptr),
  countries_(nullptr),
  detailsDialog_(nullptr),
  splitter_(nullptr),
  requestTimeout_(kDefaultRequestTimeout * 1000),
  closing_(false)
{

  ui_->setupUi(this);

  ui_->deleteStationPushButton->setIcon(QIcon(":/meteo/icons/minus.png"));
  ui_->newStationPushButton->setIcon(QIcon(":/meteo/icons/plus.png"));


  connect(ui_->clearSearchLinePushButton, SIGNAL(clicked(bool)),
          ui_->searchLineEdit, SLOT(clear()));
  connect(ui_->stationsTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
          this, SLOT(showStationDetails(QTreeWidgetItem*))); // //	currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous))
  connect(ui_->newStationPushButton, SIGNAL(clicked(bool)),
          this, SLOT(showNewStationDetails()));
  connect(ui_->deleteStationPushButton, SIGNAL(clicked(bool)),
          this, SLOT(deleteStation()));
  connect(ui_->stationsTreeWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateButtonsAvailability()));



  connect(ui_->searchLineEdit, SIGNAL(textChanged(QString)),
          this, SLOT(filterStations()));

  connect(ui_->stationTypeComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(filterStations()));
}

bool StationEditWidget::init(){

  // в первой версии StationDetailsDialog был отдельным диалоговым окном
  // потом было принято решение сделать одно окно из двух частей
  // отсюда название dialog и особенности логики работы
  detailsDialog_ = new StationDetailsDialog(this);
  splitter_ = new QSplitter(this);
  this->layout()->addWidget(splitter_);
  splitter_->addWidget(ui_->leftPaneWidget);
  splitter_->addWidget(detailsDialog_);
  splitter_->setChildrenCollapsible(false);

  detailsDialog_->hide();// начальное состояние части окна с информацией о станции - скрыта

  QStringList hdrLabels;
  hdrLabels.append(QObject::tr("Название (русское)"));
  hdrLabels.append(QObject::tr("Название (международное)"));
  hdrLabels.append(QObject::tr("Код ИКАО"));
  hdrLabels.append(QObject::tr("Индекс"));
  hdrLabels.append("");
  ui_->stationsTreeWidget->setHeaderLabels(hdrLabels);
  ui_->stationsTreeWidget->setHeaderHidden(false);

  ui_->stationsTreeWidget->headerItem()->setSizeHint(0,
                                                     QSize(ui_->stationsTreeWidget->headerItem()->sizeHint(0).width(),
                                                           kRowHeight));
  // ширина столбцов и виджетов в сплиттере
  auto settings = new QSettings( kSettingsFile, QSettings::IniFormat );
  QList<QVariant> colWidths = settings->value("ColumnWidth").toList();
  //debug_log << colWidths;
  for (int ix = 0; ix < colWidths.size(); ix++) {
    int w = colWidths.at(ix).toInt();
    //debug_log << w;
    ui_->stationsTreeWidget->setColumnWidth(ix, w);
  }
  QList<QVariant> splitterWidths = settings->value("SplitterWidth").toList();
  QList<int> splitterWidthsInt;
  for (int ix = 0; ix < splitterWidths.size(); ix++) {
    int w = splitterWidths.at(ix).toInt();
    splitterWidthsInt.append(w);
  }
  splitter_->setSizes(splitterWidthsInt);
  if ( true == settings->contains("RequestTimeout") ) {
    requestTimeout_ = settings->value("RequestTimeout").toDouble() * 1000;
  }
  delete settings;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  if ( true == loadStationTypes() ) {
    fillStationTypeCombo();
    detailsDialog_->setTypesMap(stationTypesMap_);
  }
  if ( true == loadStations() ) {
    fillStationsTree();
  }
  if ( true == loadCountries() ) {
    fillCountriesMap();
    detailsDialog_->setCountriesMap(countriesMap_);
  }
  QGuiApplication::restoreOverrideCursor();
  updateButtonsAvailability();

  return true;
}

bool StationEditWidget::loadStationTypes(bool forceReload)
{

  if (nullptr != stationTypes_) {
    if ( false == forceReload ) {
      return false;
    }
    else { // true == forceReload
      delete stationTypes_;
      stationTypes_ = nullptr;
    }
  }
  if ( true == closing_) {
    return false;
  }

  meteo::sprinf::TypesRequest typesRequest;
  int flowControl = QMessageBox::Retry;
  meteo::rpc::Channel* ch = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    if ( nullptr == ch ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kSprinf);
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список типов станций"),
                                        QObject::tr("Не удалось подключиться к службе справочной информации"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        return false;
      }
    }
    else {
      break;
    }
  }

  flowControl = QMessageBox::Retry;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    stationTypes_ = ch->remoteCall( &meteo::sprinf::SprinfService::GetStationTypes,
                                    typesRequest, requestTimeout_ );
    QString errorMsg = QString("");
    qApp->restoreOverrideCursor();
    if ( nullptr == stationTypes_ ) {
      errorMsg = QObject::tr("возращено значение nullptr");
    } else if ( true == stationTypes_->has_result() &&
                false == stationTypes_->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
    }
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список типов станций"),
                                        QObject::tr("Не удалось выполнить запрос к базе данных"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        delete ch;
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return false;
      }
    }
    else {
      break;
    }
  }

  delete ch;
  return true;
}

bool StationEditWidget::loadStations(bool forceReload)
{
  if ( nullptr != stations_) {
    if ( false == forceReload ) {
      return false;
    }
    else { // true == forceReload
      delete stations_;
      stations_ = nullptr;
    }
  }
  if ( true == closing_) {
    return false;
  }

  meteo::sprinf::MultiStatementRequest stationsRequest;
  int flowControl = QMessageBox::Retry;

  meteo::rpc::Channel* ch = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    if ( nullptr == ch ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kSprinf);
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список станций"),
                                        QObject::tr("Не удалось подключиться к службе справочной информации"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        return false;
      }
    }
    else {
      break;
    }
  }

  flowControl = QMessageBox::Retry;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    stations_ = ch->remoteCall( &meteo::sprinf::SprinfService::GetStations,
                                stationsRequest, requestTimeout_ );
    QString errorMsg = QString("");
    qApp->restoreOverrideCursor();
    if ( nullptr == stations_ ) {
      errorMsg = QObject::tr("возращено значение nullptr");
    } else if ( true == stations_->has_result() &&
                false == stations_->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
    }
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список станций"),
                                        QObject::tr("Не удалось выполнить запрос к базе данных"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        delete ch;
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return false;
      }
    }
    else {
      break;
    }
  }

  delete ch;
  return true;
}

bool StationEditWidget::loadCountries()
{
  if ( nullptr != countries_ ) {
    return false;
  }
  if ( true == closing_) {
    return false;
  }

  meteo::sprinf::CountryRequest countriesRequest;
  int flowControl = QMessageBox::Retry;

  meteo::rpc::Channel* ch = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    if ( nullptr == ch ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kSprinf);
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список стран"),
                                        QObject::tr("Не удалось подключиться к службе справочной информации"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        return false;
      }
    }
    else {
      break;
    }
  }

  flowControl = QMessageBox::Retry;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    countries_ = ch->remoteCall( &meteo::sprinf::SprinfService::GetCountry,
                                 countriesRequest, requestTimeout_ );
    QString errorMsg = QString("");
    qApp->restoreOverrideCursor();
    if ( nullptr == countries_ ) {
      errorMsg = QObject::tr("возращено значение nullptr");
    } else if ( true == countries_->has_result() &&
                false == countries_->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
    }
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось получить список стран"),
                                        QObject::tr("Не удалось выполнить запрос к базе данных"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        closing_ = true;
        close();
        delete ch;
        return false;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return false;
      }
    }
    else {
      break;
    }
  }

  delete ch;
  return true;
}

void StationEditWidget::fillStationTypeCombo()
{
  ui_->stationTypeComboBox->clear();
  ui_->stationTypeComboBox->blockSignals(true);

  ui_->stationTypeComboBox->addItem(QObject::tr("Все"), QVariant(kAllTypes));

  stationTypesMap_.clear();

  for (int i = 0; i < stationTypes_->station_size(); i++) {
    QString typeName = QString::fromStdString(stationTypes_->station(i).name());
    int type = stationTypes_->station(i).type();
    stationTypesMap_.insert(typeName, type);
  }
  QList <QString> auxKeys = stationTypesMap_.keys();

  for (int i = 0; i < stationTypesMap_.size(); i++) {
    ui_->stationTypeComboBox->addItem(auxKeys.at(i),
                                      QVariant(stationTypesMap_.value(auxKeys.at(i))));
  }
  ui_->stationTypeComboBox->blockSignals(false);
  ui_->stationTypeComboBox->setCurrentIndex(0);
}

void StationEditWidget::fillStationsTree()
{
  ui_->stationsTreeWidget->setSortingEnabled(false);

  for (int i = 0; i < stations_->station_size(); i++) {
    sprinf::Station st = stations_->station(i);
    QTreeWidgetItem *item = addStationToTree(&st);
    stationTreeItemsMap_.insert(item, i);
  }

  ui_->stationsTreeWidget->setSortingEnabled(true);
  ui_->stationsTreeWidget->sortItems(0, Qt::AscendingOrder);
}

QTreeWidgetItem* StationEditWidget::addStationToTree(const sprinf::Station *st)
{
  bool noNameFlag = true;
  QStringList itemStrings;
  if ( true == st->has_name() ) {
    if ( true == st->name().has_rus() ) {
      itemStrings.append(QString::fromStdString(st->name().rus()));
      noNameFlag = false;
    }
    else {
      itemStrings.append(QString(""));
    }
    if ( true == st->name().has_international() ) {
      itemStrings.append(QString::fromStdString(st->name().international()));
      noNameFlag = false;
    }
    else {
      itemStrings.append(QString(""));
    }
  }
  if ( true == noNameFlag ) {
    itemStrings.clear();
    itemStrings.append(QObject::tr("[ НЕТ НАЗВАНИЯ ]"));
    itemStrings.append(QString(""));
  }
  if ( true == st->has_cccc() ) {
    itemStrings.append(QString::fromStdString(st->cccc()));
  }
  else {
    itemStrings.append(QString(""));
  }
  if ( true == st->has_index() ) {
    itemStrings.append(QString::number(st->index()));
  }
  else {
    itemStrings.append(QString(""));
  }

  StationsListWidgetItem* item = new StationsListWidgetItem(ui_->stationsTreeWidget, itemStrings);
  item->setSizeHint(0,
                    QSize(item->sizeHint(0).width(),
                          kRowHeight));

  ui_->stationsTreeWidget->addTopLevelItem(item);

  return item;
}

void StationEditWidget::updateStationInTree(int stationIdx)
{
  QTreeWidgetItem *item = stationTreeItemsMap_.key(stationIdx);

  if ( stationIdx >= stations_->station_size() ) {
    return;
  }

  sprinf::Station st = stations_->station(stationIdx);

  bool noNameFlag = true;
  QStringList itemStrings;
  if ( true == st.has_name() ) {
    if ( true == st.name().has_rus() ) {
      itemStrings.append(QString::fromStdString(st.name().rus()));
      noNameFlag = false;
    }
    else {
      itemStrings.append(QString(""));
    }
    if ( true == st.name().has_international() ) {
      itemStrings.append(QString::fromStdString(st.name().international()));
      noNameFlag = false;
    }
    else {
      itemStrings.append(QString(""));
    }
  }
  if ( true == noNameFlag ) {
    itemStrings.clear();
    itemStrings.append(QObject::tr("[ НЕТ НАЗВАНИЯ ]"));
    itemStrings.append(QString(""));
  }
  if ( true == st.has_cccc() ) {
    itemStrings.append(QString::fromStdString(st.cccc()));
  }
  else {
    itemStrings.append(QString(""));
  }
  if ( true == st.has_index() ) {
    itemStrings.append(QString::number(st.index()));
  }
  else {
    itemStrings.append(QString(""));
  }

  for ( int i = 0; i < ui_->stationsTreeWidget->columnCount() - 1; i++ ) {
    item->setText(i, itemStrings.at(i));
  }
  ui_->stationsTreeWidget->scrollToItem(item);
}

void StationEditWidget::fillCountriesMap()
{
  countriesMap_.clear();

  for (int i = 0; i < countries_->country_size(); i++) {
    if ( countries_->country(i).has_number() &&
         countries_->country(i).has_name_ru() ) {
      int countryNumber = countries_->country(i).number();
      QString countryNameRu =
          QString::fromStdString(countries_->country(i).name_ru());
      countriesMap_.insert(countryNumber, countryNameRu);
    }
  }
}

int StationEditWidget::getStationIdx(std::string stationId)
{
  for ( int i = 0; i < stations_->station_size(); i++ ) {
    if ( stationId == stations_->station(i).id() ) {
      return i;
    }
  }
  return -1;
}

int StationEditWidget::errorMsgBoxForUser(QString text, QString informationText, QString title)
{
  QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
  QMessageBox * msgBox = new QMessageBox();
  msgBox->setText(text);
  msgBox->setWindowTitle(title);
  msgBox->setInformativeText(informationText);
  msgBox->setIcon(QMessageBox::Critical);
  QPushButton * abortBtn = msgBox->addButton(QMessageBox::Abort);
  abortBtn->setText(QObject::tr("Завершить"));
  abortBtn->setToolTip(QObject::tr("Завершить работу редактора станций"));
  abortBtn->setMinimumHeight(kMinButtonHeight);
  QPushButton * retryBtn = msgBox->addButton(QMessageBox::Retry);
  retryBtn->setText(QObject::tr("Повторить"));
  retryBtn->setToolTip(QObject::tr("Повторить запрос"));
  retryBtn->setMinimumHeight(kMinButtonHeight);
  QPushButton * ignoreBtn = msgBox->addButton(QMessageBox::Ignore);
  ignoreBtn->setText(QObject::tr("Пропустить"));
  ignoreBtn->setToolTip(QObject::tr("Игнорировать ошибку и продолжить работу"));
  ignoreBtn->setMinimumHeight(kMinButtonHeight);

  int btn = msgBox->exec();
  delete msgBox;
  QGuiApplication::restoreOverrideCursor();
  return btn;
}

/*!
 * \brief Проверяет, имеется ли уже станция с таким же типом и индексом.
 *
 */
bool StationEditWidget::stationExists(sprinf::Station st)
{
  for ( int ix = 0; ix < stations_->station_size(); ix++) {
    const sprinf::Station cmpSt = stations_->station(ix);
    if ( true == cmpSt.has_type() && // типы существуют и равны
         true == st.has_type() &&
         cmpSt.type() == st.type() && // и (
         ( ( true == cmpSt.has_cccc() &&    // индексы ИКАО существуют и равны
             true == st.has_cccc() &&
             cmpSt.cccc() ==st.cccc() ) || // или
           ( true == cmpSt.has_index() &&
             true == st.has_index() &&
             cmpSt.index() == st.index() ) ) && // синоптические индексы существуют и равны ) и
         true == cmpSt.has_id() &&
         true == st.has_id() &&
         cmpSt.id() != st.id() )  {    // id в БД существуют и НЕ равны (если равны - происходит обновление существующей станции без изменения индекса и типа)
      return true;
    }
  }
  return false;
}

void StationEditWidget::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);

  // сохранение настроек - ширина столбцов и виджетов в сплиттере
  auto settings = new QSettings( kSettingsFile, QSettings::IniFormat );
  QList<QVariant> colWidths;
  for (int ix = 0; ix < ui_->stationsTreeWidget->columnCount(); ix++) {
    int w = ui_->stationsTreeWidget->columnWidth(ix);
    colWidths.append(QVariant(w));
  }
  settings->setValue("ColumnWidth", QVariant(colWidths));

  QList<QVariant> splitterWidths;
  for (int ix = 0; ix < splitter_->sizes().size(); ix++) {
    int w = splitter_->sizes().at(ix);
    splitterWidths.append(QVariant(w));
  }
  settings->setValue("SplitterWidth", QVariant(splitterWidths));

  settings->setValue("RequestTimeout", double(requestTimeout_) / 1000.0 );

  //settings->sync(); // выполняется автоматически в деструкторе
  delete settings;

  // проверка изменений и запрос на сохранение при их наличии
  if ( false == closing_ && // true == closing_, когда закрытие вызвано ошибкой
       true == detailsDialog_->hasChanges() ) {
    QMessageBox askBox;
    askBox.setText(QObject::tr("Сохранить изменения?"));
    askBox.setWindowFlags( askBox.windowFlags() & (~Qt::WindowTitleHint) );
    QPushButton *btn = askBox.addButton(QObject::tr("Не сохранять"), QMessageBox::RejectRole);
    btn->setMinimumHeight(kMinButtonHeight);
    btn = askBox.addButton(QObject::tr("Сохранить"), QMessageBox::AcceptRole);
    btn->setMinimumHeight(kMinButtonHeight);
    int result = askBox.exec();

    if ( QMessageBox::Accepted == result ) {
      detailsDialog_->saveChanges();
    }
  }
  event->accept();
}

void StationEditWidget::showStationDetails(QTreeWidgetItem *item)
{
  meteo::sprinf::Station st = stations_->station(stationTreeItemsMap_.value(item));

  detailsDialog_->setData(st);

  if ( true == detailsDialog_->isHidden() ) {
    detailsDialog_->show();
  }
}

void StationEditWidget::showNewStationDetails()
{
  // пустая станция
  meteo::sprinf::Station st;
  st.set_type(0);//исключает добавление станции без типа

  detailsDialog_->setData(st);

  if ( true == detailsDialog_->isHidden() ) {
    detailsDialog_->show();
  }
}

void StationEditWidget::deleteStation()
{
  QTreeWidgetItem *item = ui_->stationsTreeWidget->currentItem();
  if ( nullptr == item ) {
    return;
  }
  int stIdx = stationTreeItemsMap_.value(item);

  sprinf::Station request = stations_->station(stIdx);

  QString questionText = QObject::tr("ВНИМАНИЕ! Выбранная станция \"");
  bool hasName = false;
  if ( true == request.has_name() ) {
    if ( true == request.name().has_rus() ) {
      questionText += QString::fromStdString(request.name().rus());
      hasName = true;
    }
    if (true == request.name().has_international() ) {
      if ( true == hasName ) {
        questionText += " / ";
      }
      questionText += QString::fromStdString(request.name().international());
      hasName = true;
    }
  }
  if ( false == hasName ) {
    questionText += QObject::tr("[ НЕТ НАЗВАНИЯ ], ");
  }
  else {
    questionText += QObject::tr(", ");
  }
  if ( true == request.has_station() ) {
    questionText += QObject::tr("индекс ") +
                    QString::fromStdString(request.station()) +
                    QObject::tr("\"");
  }
  questionText += QObject::tr(" будет удалена без возможности восстановления. Продолжить?");

  QMessageBox askBox;
  askBox.setWindowTitle(QObject::tr("Подтверждение удаления станции"));
  askBox.setText(questionText);
  askBox.setWindowFlags( askBox.windowFlags() & (~Qt::WindowTitleHint) );
  QPushButton *btn = askBox.addButton(QObject::tr("Не удалять"), QMessageBox::RejectRole);
  btn->setMinimumHeight(kMinButtonHeight);
  btn = askBox.addButton(QObject::tr("Удалить безвозвратно"), QMessageBox::AcceptRole);
  btn->setMinimumHeight(kMinButtonHeight);
  int result = askBox.exec();

  if ( QMessageBox::Accepted != result ) {
    return;
  }

  int flowControl = QMessageBox::Retry;

  meteo::rpc::Channel* ch = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    if ( nullptr == ch ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kSprinf);
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось удалить станцию"),
                                        QObject::tr("Не удалось подключиться к службе справочной информации"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        // несколько всё же костыль, предотвращающий повторный вызов запроса на сохранение при завершении плагина REMOVE?
//        meteo::sprinf::Station st;
//        st.set_type(0);//исключает добавление станции без типа
//        detailsDialog_->setData(st, true);
        closing_ = true;
        close();
        return;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        return;
      }
    }
    else {
      break;
    }
  }

  sprinf::ReportStationsAdded *response = nullptr;
  flowControl = QMessageBox::Retry;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    response = ch->remoteCall( &meteo::sprinf::SprinfService::DeleteStation,
                               request, requestTimeout_ );
    QString errorMsg = QString("");
    qApp->restoreOverrideCursor();
    if ( nullptr == response ) {
      errorMsg = QObject::tr("возращено значение nullptr");
    } else if ( true == response->has_result() &&
                false == response->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
    }
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось удалить станцию"),
                                        QObject::tr("Не удалось выполнить запрос к базе данных"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        // несколько всё же костыль, предотвращающий повторный вызов запроса на сохранение при завершении плагина REMOVE?
//        meteo::sprinf::Station st;
//        st.set_type(0);//исключает добавление станции без типа
//        detailsDialog_->setData(st, true);
        closing_ = true;
        close();
        delete ch;
        return;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return;
      }
    }
    else {
      break;
    }
  }

  // очищает правую часть со свойствами станции и предотвращет вызов запроса на сохранение, если станция изменялась перед удалением
  meteo::sprinf::Station st;
  st.set_type(0);//исключает добавление станции без типа
  detailsDialog_->setData(st, true);

  int itemIdx = ui_->stationsTreeWidget->indexOfTopLevelItem(item);
  item = ui_->stationsTreeWidget->takeTopLevelItem(itemIdx);
  if ( nullptr != item) {
    delete item;
  }
  stationTreeItemsMap_.remove(item);
  updateButtonsAvailability();
  delete ch;
  return;
}

void StationEditWidget::updateButtonsAvailability()
{
  if ( nullptr == ui_->stationsTreeWidget->currentItem() ) {
    //ui_->editStationPushButton->setEnabled(false);
    ui_->deleteStationPushButton->setEnabled(false);
  }
  else {
    //ui_->editStationPushButton->setEnabled(true);
    ui_->deleteStationPushButton->setEnabled(true);
  }
}

void StationEditWidget::filterStations()
{
  QString searchStr = ui_->searchLineEdit->text();
  QList<QTreeWidgetItem*> items = stationTreeItemsMap_.keys();

  // нужный тип станции
  int requiredType = kAllTypes;

  int cidx = ui_->stationTypeComboBox->currentIndex();
  if ( -1 != cidx ) {
    requiredType = ui_->stationTypeComboBox->itemData(cidx).toInt();
  }

  for ( int i = 0; i < items.size(); i++) {
    bool displayFlag = true;
    int idx = stationTreeItemsMap_.value(items.at(i));
    meteo::sprinf::Station station = stations_->station(idx);


    // проверка типа станции
    if ( kAllTypes != requiredType ) {
      if ( (true == station.has_type()) &&
           (requiredType != station.type()) ) {
        displayFlag = false;
      }
    }

    // поиск по кодам
    bool codeFitsFlag = false;
    if ( true == displayFlag ) { // если флаг уже в false, сокращаем время на поиски
      //bool codeFitsFlag = false;
      // ICAO (cccc)
      if ( true == station.has_cccc() ) {
        QString cccc = QString::fromStdString(station.cccc());
        if ( true == cccc.contains(searchStr, Qt::CaseInsensitive) ) {
          codeFitsFlag = true;
        }
      }

      // index, он же синоптический (! целочисленный)
      if ( true == station.has_index() ) {
        QString stIndex = QString::number(station.index());
        QString auxSearchStr = searchStr;
        while ( true == auxSearchStr.startsWith("0") ) { // отсекаем левые нули для поиска в числе
          auxSearchStr = auxSearchStr.right(auxSearchStr.size() - 1);
        }
        if ( (true == stIndex.contains(searchStr, Qt::CaseInsensitive)) ||
             (true == stIndex.startsWith(auxSearchStr)) ) {
          codeFitsFlag = true;
        }
      }
    }

    // поиск по именам
    bool nameFitsFlag = false;
    if ( true == displayFlag ) {
      if ( true == searchStr.isEmpty() ) {
        nameFitsFlag = true;
      }
      else {
        // cтанции, не имеющие имен вообще, отображаются только при условии,
        // что строка поиска - пустая
        if ( true == station.has_name() ) {
          if ( true == station.name().has_international() ) {
            QString nameEn = QString::fromStdString(station.name().international());
            if ( true == nameEn.contains(searchStr, Qt::CaseInsensitive) ) {
              nameFitsFlag = true;
            }
          }
          if ( true == station.name().has_rus() ) {
            QString nameRu = QString::fromStdString(station.name().rus());
            if ( true == nameRu.contains(searchStr, Qt::CaseInsensitive) ) {
              nameFitsFlag = true;
            }
          }
        }
      }
    }

    if ( true == displayFlag ) {
      displayFlag = codeFitsFlag || nameFitsFlag;
    }
    if ( true == displayFlag ) {
      items.at(i)->setHidden(false);
    }
    else {
      items.at(i)->setHidden(true);
    }
  }
}

void StationEditWidget::saveStationToDb(sprinf::Station st)
{
  if ( nullptr == detailsDialog_ ) {
    return;
  }

  // проверка на наличие существующей станции с указанными индексом и типом
  if ( true == stationExists(st) ) {
    QMessageBox warnBox;
    warnBox.setWindowTitle(QObject::tr("Станция существует"));
    warnBox.setText(QObject::tr("Станция с таким типом и индексом уже существует. Сохранение невозможно."));
    QPushButton *btn = warnBox.addButton(QMessageBox::Ok);
    btn->setMinimumHeight(kMinButtonHeight);
    warnBox.exec();
    return;
  }
  // проверка на наличие индекса (без него не сохранится в базе)
  if ( false == st.has_index() &&
       false == st.has_cccc() &&
       ( false == st.has_name() ||
         ( true == st.has_name() &&
           false == st.name().has_international() ) ) ) {
    QMessageBox warnBox;
    warnBox.setWindowTitle(QObject::tr("Не задано обязательное поле"));
    warnBox.setText(QObject::tr("Для метеостанции обязательно должен быть заполнено одно из трех полей: индекс, код ИКАО, "
                                   "международное название. Сохранение невозможно."));
    QPushButton *btn = warnBox.addButton(QMessageBox::Ok);
    btn->setMinimumHeight(kMinButtonHeight);
    warnBox.exec();
    return;
  }
  // предупреждение о том, что не задан индекс
  if ( false == st.has_index() &&
       false == st.has_cccc() ) {
    QMessageBox askBox;
    askBox.setWindowTitle(QObject::tr("Не задан индекс"));
    askBox.setText(QObject::tr("Для метеостанции не задан индекс или код ИКАО. В качестве идентификатора станции будет использовано "
                                   "международное название. Продолжить?"));

    askBox.setStandardButtons(QMessageBox::NoButton);
    QPushButton * noBtn = askBox.addButton(QObject::tr("Нет"), QMessageBox::RejectRole);
    noBtn->setMinimumHeight(kMinButtonHeight);
    QPushButton * yesBtn = askBox.addButton(QObject::tr("Да"), QMessageBox::AcceptRole);
    yesBtn->setMinimumHeight(kMinButtonHeight);
    askBox.exec();
    if ( yesBtn != askBox.clickedButton() ) {
      return;
    }
  }

  int flowControl = QMessageBox::Retry;

  meteo::rpc::Channel* ch = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
    if ( nullptr == ch ) {
      error_log << msglog::kNoConnect.arg(meteo::settings::proto::kSprinf);
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось сохранить станцию"),
                                        QObject::tr("Не удалось подключиться к службе справочной информации"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        // несколько всё же костыль, предотвращающий повторный вызов запроса на сохранение при завершении плагина REMOVE?
//        meteo::sprinf::Station st;
//        st.set_type(0);//исключает добавление станции без типа
//        detailsDialog_->setData(st, true);
        closing_ = true;
        close();
        return;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return;
      }
    }
    else {
      break;
    }
  }

  flowControl = QMessageBox::Retry;
  sprinf::ReportStationsAdded *response = nullptr;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    response = ch->remoteCall( &meteo::sprinf::SprinfService::UpdateStation,
                                st, requestTimeout_ );
    QString errorMsg = QString("");
    if ( nullptr == response ) {
      errorMsg = QObject::tr("возращено значение nullptr");
    } else if ( true == response->has_result() &&
                false == response->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
    }
    qApp->restoreOverrideCursor();
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Не удалось сохранить станцию"),
                                        QObject::tr("Не удалось выполнить запрос к базе данных"),
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        // несколько всё же костыль, предотвращающий повторный вызов запроса на сохранение при завершении плагина  REMOVE?
//        meteo::sprinf::Station st;
//        st.set_type(0);//исключает добавление станции без типа
//        detailsDialog_->setData(st, true);
        closing_ = true;
        close();
        delete ch;
        return;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return;
      }
    }
    else {
      break;
    }
  }

  // проверка записи и обновление параметров в detailsDialog
  sprinf::MultiStatementRequest stationTestReq;
  if ( true == st.has_id() ) {
    if ( "" != st.id() ) {
      stationTestReq.add_id(st.id());
    }
    else {
       // ! comment в случае успешной записи НОВОЙ станции содержит ее _id
      stationTestReq.add_id(response->comment());
    }
  }
  else {
     // ! comment в случае успешной записи НОВОЙ станции содержит ее _id
    stationTestReq.add_id(response->comment());
  }
  flowControl = QMessageBox::Retry;
  sprinf::Stations* stFromDb = nullptr;
  bool ok = true;
  while ( QMessageBox::Retry == flowControl ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    stFromDb = ch->remoteCall(&meteo::sprinf::SprinfService::GetStations,
                              stationTestReq, requestTimeout_);
    QString errorMsg = QString("");
    QString warningMsg = QString("Не удалось получить подтверждение от базы данных о сохранении станции");
    qApp->restoreOverrideCursor();
    if ( nullptr == stFromDb ) {
      errorMsg = QObject::tr("возращено значение nullptr");
      ok = false;
    } else if ( true == stFromDb->has_result() &&
                false == stFromDb->result() ) {
      errorMsg = QObject::tr("поле result в ответе = false");
      ok = false;
    }
    else if ( 0 == stFromDb->station_size() ) {
      errorMsg = QObject::tr("Из БД не получено ни одного документа для обновленной станции");
      warningMsg = errorMsg;
      ok = false;
    }
    else if ( 1 < stFromDb->station_size() ) {
      errorMsg = QObject::tr("Из БД получено более одного документа для обновленной станции");
      warningMsg = errorMsg;
      ok = false;
    }
    if ( QString("") != errorMsg ) {
      error_log << msglog::kDbRequestFailed << ": " << errorMsg;
      flowControl = errorMsgBoxForUser( QObject::tr("Ошибка при сохранении станции"),
                                        warningMsg,
                                        QObject::tr("Ошибка") );
      if ( QMessageBox::Abort == flowControl ) {
        // несколько всё же костыль, предотвращающий повторный вызов запроса на сохранение при завершении плагина REMOVE?
//        meteo::sprinf::Station st;
//        st.set_type(0);//исключает добавление станции без типа
//        detailsDialog_->setData(st, true);
        closing_ = true;
        close();
        delete ch;
        return;
      }
      else if ( QMessageBox::Ignore == flowControl ) {
        delete ch;
        return;
      }
    }
    else {
      break;
    }
  }

  int idx = getStationIdx(st.id());

  if ( true == ok ) {
    detailsDialog_->setData(stFromDb->station(0), true);
    if ( -1 != idx ) {
      // обновление станции
      stations_->mutable_station(idx)->CopyFrom(stFromDb->station(0));
      updateStationInTree(idx);
    }
    else {
      // добавление станции
      int newIdx = stations_->station_size();
      sprinf::Station *newSt = stations_->add_station();
      newSt->CopyFrom(stFromDb->station(0));
      QTreeWidgetItem *newItem = addStationToTree(newSt);
      ui_->stationsTreeWidget->scrollToItem(newItem);
      ui_->stationsTreeWidget->setCurrentItem(newItem);
      stationTreeItemsMap_.insert(newItem, newIdx);
    }
  }
  else {
    detailsDialog_->setData(st, true);
    if ( -1 != idx ) {
      stations_->mutable_station(idx)->CopyFrom(st);
    }
  }
  delete ch;
  filterStations();
}

StationEditWidget::~StationEditWidget()
{
  if ( nullptr != stations_ ) {
    delete stations_;
    stations_ = nullptr;
  }
  if ( nullptr != stationTypes_ ) {
    delete stationTypes_;
    stationTypes_ = nullptr;
  }
  if ( nullptr != countries_ ) {
    delete countries_;
    countries_ = nullptr;
  }
  if ( nullptr != ui_ ) {
    delete ui_;
    ui_ = nullptr;
  }
}

} // namespace map
} // namespace meteo

