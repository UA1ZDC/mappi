#include "forecastwidget.h"
#include "ui_forecastwidget.h"

#include <cross-commons/debug/tlog.h>

#include <commons/obanal/tfield.h>
#include <commons/textproto/pbtools.h>


#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>


Q_DECLARE_METATYPE(TForecast*)

namespace meteo{
namespace map{
  static const QString kName = QObject::tr("forecastw_action");
  static const QString kTitle = QObject::tr("Прогнозирование ");
  static const QString kNameStationWgt = QObject::tr("forecastw");

ForecastWidget::ForecastWidget(QMainWindow *parent) :
  QMainWindow (parent),
  ui_(new Ui::ForecastWidget),
  station_widget_(nullptr),
  methods_list_(nullptr),
  result_wdgt_(nullptr),
  fw_(nullptr),
  fm_(nullptr),
  splitter_(nullptr),
  curSynSit_(0),
  curSeason_(0),
  curFrontSit_(0),
  p(parent)
{
  ui_->setupUi(this);
  ui_->startDate->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  for(int i=0; i<18; ++i){
    ui_->synsitCmb->addItem(obanal::TField::sitName(i));
  }
  ui_->frcstTypeTree->setColumnHidden(kAuto, true);

  method_namelist_order = nullptr;

  QDateTime dt = QDateTime::currentDateTimeUtc();
  // dt.setTime(QTime(dt.time().hour(), 0));
  dt.setTime(QTime( 0 , 0));
  ui_->startDate->setDate(dt.date());
  ui_->startDate->setCalendarPopup(true);
  // время по умолчанию устанавливаем кратное 12 часам
  // либо 00 либо 12
  ui_->startTime->setTime( dt.time() );
  // slotDtChanged();

  // таймер для зарежки сигналов для избежания дребезга
  mTimer = new QTimer(this);
  mTimer->setSingleShot(true);
  // вешаем сигнал таймера на изменение даты (чтобы при смене часа и даты - пересчитывалось не сразу)
  QObject::connect( mTimer,  SIGNAL(timeout()), this, SLOT(slotDtChanged()) );
  mTimer->start(1500);

  connect(ui_->frcstTypeTree, SIGNAL(itemDoubleClicked ( QTableWidgetItem * )),SLOT(slotStartForecast(QTableWidgetItem*)));

  // обрабатываем двойное нажатие на последнюю ячейку
  connect(ui_->frcstTypeTree, SIGNAL(cellDoubleClicked ( int, int )),SLOT(slotCellDoubleClicked(int, int)));

  connect(ui_->frcstTypeTree, SIGNAL(itemSelectionChanged ()),SLOT(setActiveMethod()));
  connect(ui_->startTime, SIGNAL(timeChanged(const QTime & )),SLOT(slotEmitFizzBuzzChaged()));
  connect(ui_->startDate, SIGNAL(dateChanged(const QDate & )),SLOT(slotEmitFizzBuzzChaged()));

  connect( ui_->resetBtn, SIGNAL(released()),SLOT(slotAnyChanged()));
  connect( ui_->synsitCmb, SIGNAL(activated(int )), SLOT(slotChangeCurSit(int )) );
  connect( ui_->frontSitCmb, SIGNAL(activated(int )), SLOT(slotChangeCurFrontSit(int )) );
  connect( ui_->seasonBox, SIGNAL(activated(int )), SLOT(slotChangeCurSeason(int )) );

  // кнопка отмены всех загрузок данных
  connect( ui_->cancelBtn, SIGNAL(released( )), SLOT(slotCancelAll( )) );

  ui_->frcstTypeTree->setIconSize(QSize(36,36));
  ui_->frcstTypeTree->setDragEnabled(false);
  ui_->frcstTypeTree->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->frcstTypeTree->setSortingEnabled(false);
  // ui_->frcstTypeTree->setDragDropMode(QAbstractItemView::InternalMove);

  ui_->frcstTypeTree->installEventFilter(this);
  ui_->frcstTypeTree->viewport()->installEventFilter(this);
  // расширяем на всю ширину (было QHeaderView::Interactive)
  ui_->frcstTypeTree->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

  addStWidget();
  ui_->frcstTypeTree->setSortingEnabled(false);

  // если тсанция введена - обновляем по ней данные
  slotDtChanged();
  slotAnyChanged();

}

ForecastWidget::~ForecastWidget(){
  if(nullptr != result_wdgt_){
    delete result_wdgt_;
    result_wdgt_=nullptr;
  }
  if ( nullptr != mTimer ){
    delete mTimer;
    mTimer=nullptr;
  }

  if ( nullptr != method_namelist_order ){
    delete method_namelist_order;
    method_namelist_order=nullptr;
  }

  if ( nullptr != fw_ ){
    delete fw_;
    fw_ = nullptr;
  }

  if (nullptr != fm_ ){
    delete fm_;
    fm_ = nullptr;
  }
}

bool ForecastWidget::addStWidget(){

  QList<sprinf::MeteostationType> stations_to_load;
  stations_to_load << meteo::sprinf::MeteostationType::kStationAero
                   << meteo::sprinf::MeteostationType::kStationSynop
                   << meteo::sprinf::MeteostationType::kStationAirport
                   << meteo::sprinf::MeteostationType::kStationAerodrome
                   << meteo::sprinf::MeteostationType::kStationHydro
                   << meteo::sprinf::MeteostationType::kStationOcean
                   << meteo::sprinf::MeteostationType::kStationRadarmap
                   << meteo::sprinf::MeteostationType::kStationGeophysics;

  station_widget_ = new StationWidget(ui_->stWgtToolbar);
  ui_->stWgtToolbar->addWidget(station_widget_);
  station_widget_->loadStation(stations_to_load);

  connect(station_widget_, SIGNAL(changed()), this, SLOT(slotAnyChanged()));
  station_widget_->show();
  station_widget_->setDelaySignal();
  return true;
}

void ForecastWidget::slotChangeCurFrontSit(int i){
  disableControls();
  curFrontSit_=i;
  if(methods_list_) methods_list_->setFrontType(i);
  rmForecastForm();
  fillTree();
  enableControls();

}

/**
 * слот отмены загрузки всех данных из базы
 */
void ForecastWidget::slotCancelAll(){
  // пока не работает но наверно надо сделать на случай
  // отсутствия соединения - ждать нереально
  emit cancelBtnClicked();
  return;
}


void ForecastWidget::slotChangeCurSit(int i){
  disableControls();
  curSynSit_=i;
  if(methods_list_) methods_list_->setSynSit(i);
  rmForecastForm();
 fillTree();
 enableControls();

}

void ForecastWidget::slotChangeCurSeason(int i){
  disableControls();
  curSeason_ = i;
  if(methods_list_) methods_list_->setSeason(i);
  rmForecastForm();
  fillTree();
  enableControls();
}


bool ForecastWidget::eventFilter(QObject* object, QEvent* event)
{
  if (object != ui_->frcstTypeTree->viewport())
    return  QWidget::eventFilter(object, event);

  switch (event->type()) {

    case QEvent::ContextMenu: {
      if(nullptr == fm_)  break;

      QMenu menu;
      QAction add(QString::fromUtf8("Просмотр результатов"), &menu);
     // QAction addFore(QString::fromUtf8("Просмотр расчетов"), &menu);
      QAction rm(QString::fromUtf8("Скрыть результаты"), &menu);
      QAction rmFore(QString::fromUtf8("Скрыть расчеты"), &menu);
      // QAction setAuto(QString::fromUtf8("Прогнозировать автоматически"), &menu);
      // QAction unsetAuto(QString::fromUtf8("Не прогнозировать автоматически"), &menu);
      QAction help(QString::fromUtf8("Справка о методе"), &menu);
      QList<QAction*> actions;
      actions.append(&add);
      // if(methods_list_->validateMethod(fm_->methodName())){
      //   actions.append(&unsetAuto);
      // } else {
      //   actions.append(&setAuto);
      // }


      if(nullptr != result_wdgt_){
        if(result_wdgt_->isVisible()){
          actions.append(&rm);
        }
      }
      if(nullptr != fm_ && fm_->getWidget()){
        if(fm_->getWidget()->isVisible()){
          actions.append(&rmFore);
        }
      }
      actions.append(&help);
      const QPoint& pos = static_cast<QContextMenuEvent*>(event)->pos();

      QAction* answer = menu.exec(actions, ui_->frcstTypeTree->mapToGlobal(pos));

      if ( answer == &help ) {
        QAction* action = WidgetHandler::instance()->mainwindow()->addActionToMenu({"prognoz_methody", QObject::tr("Прогноз (расчетные методы)")}, {{"helpmenu", QObject::tr("Помощь")}});
        if(nullptr != action) {
          auto oldata = action->data();
          action->setData(fm_->methodHelpFile());
          action->trigger();
          action->setData(oldata);
        }
      }

      if (answer == &add) {
        slotShowResult(fm_->methodName());
      }

      // if (answer == &unsetAuto) {
      //   if(methods_list_->setValidateMethod(fm_->methodName(),false))
      //   { setAutoItem(ui_->frcstTypeTree->currentRow(), false); }

      // }
      // if (answer == &setAuto) {
      //   if(methods_list_->setValidateMethod(fm_->methodName(),true))
      //   { setAutoItem(ui_->frcstTypeTree->currentRow(), true); }

      // }

      if (answer == &rm) {
        slotHideResult();
      }
      if (answer == &rmFore) {
        rmForecastForm();
      }
    }
    break;

    default:
      break;
  }
  return QWidget::eventFilter(object, event);
}

/**
 * слот обработки двойновго нажатия на третью ячейку
 * для расчета автомата
 * @param row [description]
 * @param col [description]
 */
void ForecastWidget::slotCellDoubleClicked(int row, int col){
  // если это колонка автомата - то переключаем его
  if ( methods_list_ && col==kAuto ){    

    if( methods_list_->validateMethod( method_namelist_order->at(row) ) ){
      if(methods_list_->setValidateMethod(method_namelist_order->at(row),false)){
        setAutoItem(row, false);
      }
    } else {
      if(methods_list_->setValidateMethod(method_namelist_order->at(row),true) ) {
        setAutoItem(row, true);
      }
    }

  }

}

void ForecastWidget::slotDtChanged(){
  QString dateTime = ui_->startTime->time().toString("hh:mm") +" "+ ui_->startDate->date().toString("dd.MM.yyyy");
  if( dateTime.isEmpty() ){
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  cur_dt_ = QDateTime::fromString(dateTime, "hh:mm dd.MM.yyyy");
  curSeason_ = TForecast::oprSeason(cur_dt_.date());
  ui_->seasonBox->setCurrentIndex(curSeason_);

  if(nullptr != fm_) {
    fm_->setDate(cur_dt_);
    fm_->resetModels();
  }
  QApplication::restoreOverrideCursor ();

}

void ForecastWidget::disableControls(){
  ui_->startTime->disconnect();
  ui_->startDate->disconnect();
  ui_->seasonBox->disconnect();
  ui_->synsitCmb->disconnect();
  ui_->resetBtn->disconnect();
  ui_->startTime->setDisabled(true);
  ui_->startDate->setDisabled(true);
  ui_->seasonBox->setDisabled(true);
  ui_->synsitCmb->setDisabled(true);
  ui_->resetBtn->setDisabled(true);
  ui_->frontSitCmb->setDisabled(true);
  ui_->frcstTypeTree->setDisabled(true);
  ui_->cancelBtn->show();
}

void ForecastWidget::enableControls(){
  connect(ui_->startTime, SIGNAL(timeChanged(const QTime & )),SLOT(slotEmitFizzBuzzChaged()));
  connect(ui_->startDate, SIGNAL(dateChanged(const QDate & )),SLOT(slotEmitFizzBuzzChaged()));
  connect(ui_->resetBtn, SIGNAL(released()),SLOT(slotAnyChanged()));
  connect( ui_->synsitCmb, SIGNAL(activated(int )), SLOT(slotChangeCurSit(int )) );
  connect( ui_->seasonBox, SIGNAL(activated(int )), SLOT(slotChangeCurSeason(int )) );
  ui_->startTime->setDisabled(false);
  ui_->startDate->setDisabled(false);
  ui_->seasonBox->setDisabled(false);
  ui_->synsitCmb->setDisabled(false);
  ui_->resetBtn->setDisabled(false);
  ui_->frontSitCmb->setDisabled(false);
  ui_->frcstTypeTree->setDisabled(false);
  ui_->cancelBtn->hide();
}

void ForecastWidget::slotAnyChanged()
{
  static int q = 0;
  q++;

  // если выбрана станция, то делаем доступным поля ввода
  if ( !station_widget_->stationIndex().isEmpty() ||
       (false == MnMath::isZero(station_widget_->coord().fi()) &&
        false == MnMath::isZero(station_widget_->coord().la())) ) {
    // блокируем интерфейс, пока не выберем станцию
    enableControls();
    q = 1;
  }else{
    // если ничего не ввели - то и считать и обновлять незачем
    //
    return;
  }

  if ( q > 1 ) { return; }

  while ( q > 0 ) {
    disableControls();

    cur_station_.coord =  station_widget_->coord();
    cur_station_.index =  station_widget_->stationIndex();
    cur_station_.station_type =  meteo::sprinf::MeteostationType(station_widget_->stationType());
    if(!station_widget_->ruName().isEmpty()){
      cur_station_.name =  station_widget_->ruName();
    } else {
      if(!station_widget_->enName().isEmpty()){
        cur_station_.name =  station_widget_->enName();
      }
    }
    rmForecastForm();
    loadSynSit();

    fillTree();
    enableControls();

    // уменьшаем счётчик выполнения
    --q;
    // сбрасываем счётчик до 1, если метод был вызван несколько раз подряд
    if ( q > 1 ) {
      q = 1;
    }
  }

  //p->setWindowTitle(kTitle +station_widget_->placeName());
  setWindowTitle(kTitle +station_widget_->placeName());
}

/**
 * слот по сбросу таймера при многократном изменении значений
 */
void ForecastWidget::slotEmitFizzBuzzChaged(){
  mTimer->start(1000);
  return;
}

/**
 * удаляем форму с прогнозами
 */
void ForecastWidget::rmForecastForm(){
  if( nullptr != fm_ && nullptr != fw_ ){
    fw_->hide();
    fm_=nullptr;
    fw_=nullptr;
  }
}

/**
 * старутем прогнозирование
 * вызывается по двойному клику на методе
 * @param item [description]
 */
void ForecastWidget::slotStartForecast(QTableWidgetItem *item)
{
  if( kMethodName != item->column()){
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));


  rmForecastForm();
  fm_ = item->data(Qt::UserRole).value<TForecast*>();
  if(nullptr == fm_){
    QApplication::restoreOverrideCursor ();
    return;
  }
  disableControls();

  qApp->processEvents();
  fm_->setDate(cur_dt_);
  fm_->setStation(cur_station_);
  fm_->setMethodTime();
  cur_dt_ = fm_->getDate();
  ui_->startTime->setTime(cur_dt_.time());
  ui_->startDate->setDate(cur_dt_.date());
  ui_->seasonBox->setCurrentIndex(TForecast::oprSeason(cur_dt_.date()));

  // запускаем расчет прогнозов
  fm_->runForecastOnly( true);

  fw_ = fm_->getWidget();

  if(nullptr != fw_){
    if(ui_->hsplit){
      if(true == ui_->hsplit->children().contains(fw_)){
        fw_->show();
      } else {
         ui_->hsplit->addWidget(fw_);
      }
    } else {
      if(layout()){
        layout()->addWidget(fw_);
      }
    }
  } else {
    QApplication::restoreOverrideCursor ();
    qApp->processEvents();
    enableControls();
    return;
  }
  QMainWindow *mw = qobject_cast<  QMainWindow* >(parent());
  if(mw){
    QString title = fm_->methodFullName();
    //mw->setWindowTitle( fm_->methodFullName());
    setWindowTitle(title);
    // устанавливаем обрезанный заголовок
    if ( title.size()>20 ){
      title.truncate(20);
      title.append("...");
    }
    fw_->setWindowTitle( title );
  }

  fw_->show();



  updateRow(item);
  QApplication::restoreOverrideCursor ();
  ui_->frcstTypeTree->setDisabled(false);
  qApp->processEvents();
  enableControls();

  setWindowTitle(kTitle +station_widget_->placeName());
  //p->setWindowTitle(kTitle +station_widget_->placeName());
}

QTableWidgetItem* ForecastWidget::setActiveMethod(){
  QTableWidgetItem* item = nullptr;
  QList <QTableWidgetItem* > items = ui_->frcstTypeTree->selectedItems ();
  for(int i =0; i< items.count();++i ){
    item = items.at(i);
    if(!item) continue;
    int row = item->row();
    item = ui_->frcstTypeTree->item(row,kMethodName);
    if(!item) continue;
    fm_ = item->data(Qt::UserRole).value<TForecast*>();
    if(nullptr == fm_) continue;
    break;
  }
  return item;
}

void ForecastWidget::slotShowResult(const QString&  mn){
  if(nullptr ==result_wdgt_){
    result_wdgt_ = new ForecastResultWidget();
    result_wdgt_->setStation(cur_station_);
    if(ui_->vsplit){
      ui_->vsplit->addWidget(result_wdgt_);
     } else {
      if(layout())
        layout()->addWidget(result_wdgt_);
    }
  }
  result_wdgt_->setMethodName(mn);
  result_wdgt_->setStation(cur_station_);
  result_wdgt_->show();
}


void ForecastWidget::slotHideResult(){
  if(nullptr !=result_wdgt_){ result_wdgt_->hide();}
}

void ForecastWidget::loadSynSit(){
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  meteo::rpc::Channel* ctrl_field = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    QApplication::restoreOverrideCursor ();
    return;
  }
  meteo::field::DataRequest request;
  request.set_date_start(cur_dt_.addSecs(-3*3600).toString(Qt::ISODate).toStdString());
  request.set_date_end(cur_dt_.toString(Qt::ISODate).toStdString());
  // request.add_center(34);
  request.add_level(1000);
  request.add_type_level(100);
  request.add_hour(0);
  request.add_meteo_descr(10009);
  request.set_need_field_descr(true);
  request.set_only_best(true);
  ::meteo::surf::Point* p =  request.add_coords();
  p->set_fi( cur_station_.coord.fi());
  p->set_la(cur_station_.coord.la());

  QElapsedTimer ttt; ttt.start();
  meteo::field::ValueDataReply * reply =
  ctrl_field->remoteCall( &meteo::field::FieldService::getSynSit, request,  30000);
  info_log << QObject::tr("Время обработки запроса getSynSit: %1 мсек").arg(ttt.elapsed());

  delete ctrl_field; ctrl_field = nullptr;
  if ( nullptr == reply ) {
    error_log.msgBox()<<meteo::msglog::kServiceInfoAnswerFailed.arg(tr("тип синоптической ситуации"), tr("данных"));
    QApplication::restoreOverrideCursor ();
    return;
  }
  int i =reply->data_size()-1;
  QString tp = QObject::tr("Ситуация автоматически не определена");
  for ( ; i >= 0; --i ){
    if( reply->data(i).value() > ui_->synsitCmb->count()) continue;
    const meteo::field::OnePointData& d = reply->data(i);
    if(d.fdesc().count_point() < 2000) continue;
    curSynSit_ = (int)d.value();
    tp = QObject::tr("Ситуация определена по полю давления за %1 \nцентр %2")
    .arg(pbtools::toQString(d.fdesc().date()))
    .arg(pbtools::toQString(d.fdesc().center_name()));

    break;
  }
  ui_->synsitCmb->setToolTip(tp);
 // if(i ==0){
    ui_->synsitCmb->setCurrentIndex(curSynSit_);
 // }
  delete reply; reply = nullptr;
  QApplication::restoreOverrideCursor ();
}


void ForecastWidget::setAccuracyItem(int row, int opr, int not_opr){
  float accuracy = 0;
  int total = opr+not_opr;

  if(total > 0) {
    accuracy = 100.*float(opr)/float(total);
  }
  QString acc = QObject::tr("Нет данных");
  QBrush br;
  br.setColor( MANUAL );
  br.setStyle(Qt::SolidPattern);
  if(0 != total || 0 != opr){
    acc = QString::number(accuracy,'f',0);
    acc += "\%("+QString::number(opr);
    acc += "/"+QString::number(total)+")";
    if(49. <= accuracy){
      br.setColor( GOOD );
    }
    if(accuracy >= 0. && accuracy < 50.){
      br.setColor( ALARM );
    }
  }
  newItem(row, kAccuracy, acc, br)->setToolTip(
    QObject::tr("Всего проверено:%1\nоправдалось:%2\nне оправдалось:%3 ")
    .arg(total)
    .arg(opr)
    .arg(not_opr));
}


QTableWidgetItem *ForecastWidget::newItem(int row, int column, const QString& acc, const QBrush& br){
  QTableWidgetItem *item = ui_->frcstTypeTree->item(row, column);

  if(nullptr == item) {
    item = new QTableWidgetItem(acc);
    ui_->frcstTypeTree->setItem(row, column, item);
  }
  if(nullptr != item){
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    item->setTextAlignment ( Qt::AlignCenter );
    item->setText(acc);
    item->setBackground(br);
    if ( br.color()==GOOD || br.color() == ALARM || br.color() == MANUAL ){
      item->setTextColor( br.color().darker(250) );
    }
    item->setToolTip( acc );
  }
  return item;
}

void ForecastWidget::setAutoItem(int row,bool isauto){
  QString acc;
  QBrush br;
  br.setStyle(Qt::SolidPattern);
  if(isauto){
    acc =  QObject::tr("Работает");
    br.setColor(GOOD);
  }else {
    acc = QObject::tr("Остановлено");
    br.setColor(ALARM);
  }
  newItem(row, kAuto, acc, br);
}

void ForecastWidget::updateRow(QTableWidgetItem *item){

//  item->setData(Qt::UserRole,QVariant::fromValue(fm));
  TForecast* fm = item->data(Qt::UserRole).value<TForecast* >();
  if(nullptr == fm) return;
  item->setIcon(QIcon(fm->iconName()));
  item->setTextAlignment ( Qt::AlignLeft );
  int row = item->row();
  rpc::Channel* ctrl_forecast = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if(nullptr == ctrl_forecast ) {
    QApplication::restoreOverrideCursor ();
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
    return;
  }

  if(nullptr != ctrl_forecast){
    meteo::forecast::AccuracyRequest req;
    req.set_method(fm->methodName().toStdString());
    req.mutable_punkt()->set_index(fm->getStationIndex().toStdString());
    req.mutable_punkt()->set_fi(fm->getStationData().coord.fiDeg());
    req.mutable_punkt()->set_la(fm->getStationData().coord.laDeg());
    req.mutable_punkt()->set_height(fm->getStationData().coord.alt());
    req.mutable_punkt()->set_name(fm->getStationData().name.toStdString());

    // получаем от сервиса точность и настройки
    meteo::forecast::AccuracyReply* reply = ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetAccuracy, req,5000);
    if ( nullptr == reply ) {
      error_log << msglog::kServerAnswerFailed;
      error_log.msgBox() << meteo::msglog::kServerAnswerFailed;
    } else {
      if (reply->result()){
        int total = reply->total();
        newItem( row, kTotal, QString::number(total), QBrush());
        int opr = reply->opr();
        int not_opr = reply->not_opr();
        delete reply; reply = nullptr;
        setAccuracyItem(row,opr,not_opr);
        setAutoItem(row,methods_list_->validateMethod(fm->methodName()));
      } else {
        error_log << "Не удается получить данные от сервиса";
      }
    }
    delete ctrl_forecast;ctrl_forecast=nullptr;
  }
}


/**
 * Заполняем список прогностическими методами из файлов
 *
 */
void ForecastWidget::fillTree(){
  station_widget_->setEnabled(false);

  if ( nullptr != methods_list_){
    delete methods_list_;
    methods_list_=nullptr;
    fm_ = nullptr;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  //qApp->processEvents();
  QString methods_dir = global::kForecastMethodsDir;


  methods_list_ = new TForecastList(cur_dt_,cur_station_);

  if(0 == methods_list_->loadMethodsNoRun(methods_dir)){
    QApplication::restoreOverrideCursor ();
    error_log.msgBox()<< msglog::kForecastNoMethods.arg(methods_dir);
    station_widget_->setEnabled(true);
    return;
  }

  methods_list_->setSeason(curSeason_);
  methods_list_->setSynSit(curSynSit_);
  methods_list_->setFrontType(curFrontSit_);



  QMap< int, QMap<QString, TForecast*>  > yavle_type;
  foreach( TForecast* f, methods_list_->methodsList() ){
    if( nullptr != f ){
      yavle_type[f->methodYavlType().toInt()].insert(f->methodFullName(), f );
    }
  }
  QList<TForecast*> sort_list;
  QMapIterator< int, QMap<QString, TForecast*>  > it(yavle_type);
  while( it.hasNext() ){
    foreach( TForecast* f, it.next().value()){
      sort_list.append(f);
    }
  }

  // список методов в том порядке, что они есть
  method_namelist_order = new QStringList();

  QListIterator<TForecast*> i(sort_list);
  int row = 0;

  while (i.hasNext()) {
    TForecast* fm = i.next();
    if(nullptr == fm) continue;
    if(ui_->frcstTypeTree->rowCount() < row+1){
      ui_->frcstTypeTree->insertRow(row);
    }
    QString ss = fm->methodFullName();
    QBrush br;
    br.setStyle(Qt::SolidPattern);
    br.setColor(Qt::white);
    if(!fm->checkSeason()){
      ss+=tr(" (не работает в это время года)");
      br.setColor(Qt::darkGray);
    }
    if(!fm->checkSynSit()){
      ss+=tr(" (не работает в этой синситуации)");
      br.setColor(Qt::darkGray);
    }

    QTableWidgetItem *item = newItem( row, kMethodName, ss, br);
    item->setData(Qt::UserRole,QVariant::fromValue(fm));
    item->setIcon(QIcon(fm->iconName()));
    item->setTextAlignment ( Qt::AlignLeft );

    // заполняем список методов
    method_namelist_order->append(fm->methodName());

    ++row;
  }

  //
  // выполняем запрос по расчету оправдываемости
  //
    auto ctrl_forecast = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kForecastData));
    if(nullptr == ctrl_forecast ) {
      QApplication::restoreOverrideCursor ();
      error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
      ui_->frcstTypeTree->setRowCount(row);
      for (int i=0;i<ui_->frcstTypeTree->columnCount();++i){
        ui_->frcstTypeTree->resizeColumnToContents(i);
      }
      QApplication::restoreOverrideCursor ();
      station_widget_->setEnabled(true);
      return;
    }

    //проверка настроек пунктов прогнозирования в БД
    bool isAuto = false;
    meteo::forecast::PunktRequest streq;
    streq.set_requestcode(meteo::forecast::kPunktGetRequest);
    meteo::forecast::PunktResponce* streply =
      ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetForecastPunkts, streq, 30000);
    if ( 0 != streply ) {
      foreach (::meteo::forecast::PunktValue punkt, streply->punkts()) {
	if (QString::fromStdString(punkt.stationid()) == cur_station_.index &&
	    meteo::GeoPoint( punkt.fi(), punkt.la(),punkt.height()) == cur_station_.coord) {
	  if (punkt.isactive()) {
	    isAuto = true;
	  }
	  break;
	}
      }
      delete streply; 
    }
    
    // meteo::forecast::AccuracyRequest areq;
    // areq.mutable_punkt()->set_index(cur_station_.index.toStdString());
    // areq.mutable_punkt()->set_type(cur_station_.station_type);
    // areq.mutable_punkt()->set_fi(cur_station_.coord.fiDeg());
    // areq.mutable_punkt()->set_la(cur_station_.coord.laDeg());

    //  meteo::field::SimpleDataReply* simplerep = ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::CalcForecastOprStation, areq, 5000);
    //  if ( nullptr == simplerep ) {
    //   error_log << "Не получилось выполнить расчет оправдываемости";
    //  } else {
    //    error_log << "Все ок, оправдываемость рассчитaна" << simplerep->comment();
    //    delete simplerep;
    //  }
    //
    // ManyAccuracyRequest
    //
    meteo::forecast::ManyAccuracyRequest req;
    req.mutable_punkt()->set_index(cur_station_.index.toStdString());
    req.mutable_punkt()->set_type(cur_station_.station_type);
    req.mutable_punkt()->set_fi(cur_station_.coord.fiDeg());
    req.mutable_punkt()->set_la(cur_station_.coord.laDeg());
    req.mutable_punkt()->set_name(cur_station_.name.toStdString());

    // заполняем методы
    for (int j = 0; j < method_namelist_order->size(); ++j)
    {
      req.add_methods( method_namelist_order->at(j).toStdString() );
    }

    {
    auto reply = std::unique_ptr<meteo::forecast::ManyAccuracyReply>(ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetManyAccuracy, req, 5000));
    if ( nullptr == reply ) {
      error_log << msglog::kServerAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
      // error_log.msgBox()<<meteo::msglog::kServerAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
    }
    else {
      // проходимся по ответу

      QListIterator<TForecast*> i(sort_list);
      int row = 0;

      while (i.hasNext()) {
        TForecast* fm = i.next();
        if(nullptr == fm) continue;

        int total     = 0;
        int opr       = 0;
        int not_opr   = 0;
        QString mname = fm->methodName();
        // теперь ищем в ответе все методы, которые есть в списке
        for (int k = 0; k < reply->accuracy_size(); ++k){
          auto acc = reply->accuracy(k);

          if ( fm->methodName()==QString::fromStdString(acc.method()) ){
            total   = acc.total();
            opr     = acc.opr();
            not_opr = acc.not_opr();
            break;
          }
        }

        newItem( row, kTotal, QString::number(total), QBrush());
        setAccuracyItem(row,opr,not_opr);
	//debug_log << "setAuto" << mname << isAuto << methods_list_->validateMethod( mname );
        setAutoItem(row, isAuto && methods_list_->validateMethod( mname ));
        row++;
      }
    }
    }

  // END MANYACCYRACYREQUEST


  ui_->frcstTypeTree->setRowCount(row);

  for (int i=0;i<ui_->frcstTypeTree->columnCount();++i){
    ui_->frcstTypeTree->resizeColumnToContents(i);
  }

  QApplication::restoreOverrideCursor ();
  station_widget_->setEnabled(true);
}


/**
 * находим номер прогностической строки по имени метода
 * капец
 * @return [description]
 */
int ForecastWidget::findRowByMethodName( QList<TForecast*> list, QString method ){
  QListIterator<TForecast*> i(list);
  int row = 0;

  while (i.hasNext()) {
    TForecast* fm = i.next();
    if(nullptr == fm) continue;
    if ( fm->methodName()==method ){
      return row;
    }
    row++;
  }

  return -1;
}


}
}
