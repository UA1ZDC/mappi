#include <QtGui>

#include "gribdatawidget.h"
#include "ui_gribdata.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>
#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/field.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/customtreewidgetitem.h>
//#include <meteo/commons/ui/map/widgets/settingsplugin/settingswidget.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options )
  {
    GribDataWidget* dlg = new GribDataWidget( parent, options );
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "gribdatawidget", createDialog );
  }

static bool res = registerDialog();
}
enum {
  Button          = 0,
  Center          = 1,
  Model           = 2,
  Hour            = 3,
  Level           = 4,
  CountPoint      = 5,
  TypeLevel       = 6,
  Id              = 7,
  Date            = 8,
  MeteoDescriptor = 9,
};

GribDataWidget::GribDataWidget(QWidget *parent, const QString& options) :
  MapWidget(parent),
  ui_(new Ui::GribData),
  channel_(0),
  sprinf_(0),
  levels_(true),
  view_(0)
{
  view_ = qobject_cast<MapView*>(parent);

  ui_->setupUi(this);
  menu_ = new QMenu(this);
  QAction* del = new QAction(QIcon(":/meteo/icons/tools/minus.png"),"Удалить слой", 0);
  connect( del, SIGNAL(triggered()), SLOT(slotRemoveLayer()));
  del->setIconVisibleInMenu(true);
  menu_->addAction(del);

  QAction* add = new QAction(QIcon(":/meteo/icons/tools/plus.png"),"Добавить слой", 0);
  connect( add, SIGNAL(triggered()), SLOT(slotAddLayer()));
  add->setIconVisibleInMenu(true);
  menu_->addAction(add);

  ui_->addBtn->setDisabled(true);
  ui_->delBtn->setDisabled(true);

  connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddLayer()));
  connect(ui_->delBtn, SIGNAL(clicked()), SLOT(slotRemoveLayer()));

  QList<QRadioButton*> radio;
  foreach( QObject* o, ui_->groupBox->children() ){
    QRadioButton* rb = static_cast<QRadioButton*>(o);
    if( 0 != rb && rb->metaObject()->className() == QString("QRadioButton") ){
      radio.append(rb);
      connect( rb, SIGNAL(clicked()), SLOT(slotRadioButtonClicked()) );
    }
  }
  ui_->radioButton->setChecked(true);
  currentLevel_ = ui_->radioButton->text();
  ui_->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect( ui_->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)));
  connect( ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(slotItemSelectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
  channel_ = rpc::ServiceLocation::instance()->serviceClient(
                            meteo::global::kServiceNames[meteo::global:: kFieldService], 500 );
  if ( 0 == channel_ ) {
    error_log << QObject::tr("Сервис не обнаружен");
  }else{
    debug_log << "ok";
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  sprinf_ = rpc::ServiceLocation::instance()->serviceClient( meteo::global::kServiceNames[meteo::global::kSprinfService], 500);
  QApplication::restoreOverrideCursor();
  if( 0 == sprinf_ ){
    error_log << QObject::tr("sprinf сервис не обнаружен");
  }else{
    debug_log << "sprinf ok";
  }
  QStringList lst;

  ui_->dateEdit->setDate(QDate::currentDate());
  QTime time;
  time.setHMS(QDateTime::currentDateTimeUtc().time().hour(), 0, 0);
  ui_->timeEdit->setTime(time);

  lst << ""<< "Центр" << "Модель" << "Срок" << "Уровень" << "Кол-во"<< "type_level"
      << "Id" << "DateTime" << "MeteoDescriptor";
  ui_->treeWidget->setStyleSheet(
        "QToolButton{"
        "background-color: white;"
        "border: 1px solid rgb(255,255,255);"
        "image: url(:/meteo/icons/tools/plus.png);"
        "}"
        "QToolButton:checked {"
        "image: url(:/meteo/icons/tools/minus.png);"
        "}");
  ui_->treeWidget->setHeaderLabels(lst);
  ui_->treeWidget->setRootIsDecorated(false);
  ui_->treeWidget->header()->setMinimumSectionSize(30);
  ui_->treeWidget->setColumnHidden(Date, true);
//  ui_->treeWidget->setColumnHidden(MeteoDescriptor, true);
  ui_->treeWidget->setColumnHidden(Id, true);

  connect(ui_->treeWidget->header(), SIGNAL(sectionClicked(int)), this, SLOT(slotSorting(int)));
  connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));
  connect(ui_->hourCombo, SIGNAL(currentIndexChanged(QString)), SLOT(slotHourChanged(QString)));
  connect(ui_->dateEdit, SIGNAL(dateTimeChanged(QDateTime)), SLOT(updateFields()));
  connect(ui_->timeEdit, SIGNAL(dateTimeChanged(QDateTime)), SLOT(updateFields()));
  connect(ui_->updateBtn, SIGNAL(clicked()), SLOT(updateFields()));
  setOptions(options);
  if( true == hasDocument() ){
    if( 0 != view_->mapscene()->document()->eventHandler() ){
      view_->mapscene()->document()->eventHandler()->installEventFilter(this);
    }
  }
}

GribDataWidget::~GribDataWidget()
{
  delete ui_; ui_ = 0;
}

bool GribDataWidget::hasLevels()
{
  return levels_;
}

void GribDataWidget::addLayer(QTreeWidgetItem* item)
{
  if( 0 == view_  || false == view_->hasMapscene() ){
    return;
  }
  if( layers_.contains(item) ){
    return;
  }
  ui_->addBtn->setDisabled(true);
  ui_->delBtn->setDisabled(false);
  rpc::TController ctrl(channel_);
  int id = item->text(Id).toInt();
  //int descr = item->text(MeteoDescriptor).toInt();
  meteo::field::SimpleDataRequest request;
  request.set_id(id);
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataReply* reply = ctrl.remoteCall( &meteo::field::FieldService::GetFieldDataPoID, request, 50000, true );
  if ( 0 == reply ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
    QApplication::restoreOverrideCursor();
    return;
  }
  if ( true == reply->has_result() && false == reply->result() ) {
    error_log << QObject::tr("Запрос поля не выполнен. Описание ошибки: %1")
                 .arg( QString::fromStdString( reply->error() ) );
    delete reply; reply = 0;
    QApplication::restoreOverrideCursor();
    return;
  }
  obanal::TField* field = new obanal::TField;
  QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
 // QDataStream stream(arr);
 // stream >> (*field);

  if(false ==  field->fromBuffer(&arr)) {
    delete field;
    QApplication::restoreOverrideCursor();
    return;
  }

  QString title = QString("%1 за %2 %3 мбар. анализ за срок %4 ч. центр: %5")
                  .arg("Grib")
                  .arg(item->text(Date))
                  .arg(item->text(Level))
                  .arg(item->text(Hour))
                  .arg(item->text(Center));
  Layer* l = 0;
  if( field->kolData() > 0 ){
    if(0 == l){
      l = new Layer( view_->mapscene()->document(), title );
      view_->mapscene()->document()->setActiveLayer(l->uuid());
      l->setField(field);
      layers_.insert(item, l);
    }
  }else{
    QMessageBox::warning(0,tr("Внимание"),tr("Нет данных для отображения"), tr("Закрыть"));
    QApplication::restoreOverrideCursor();
    return;
  }
  for ( int i = 0, isz = field->kolFi(); i < isz; ++i ) {
    if( 0 != i % 10 ){
      continue;
    }
    for ( int j = 0, jsz = field->kolLa(); j < jsz; ++j ) {
      if( 0 != j % 10 ){
        continue;
      }
      meteo::GeoPoint gp( field->netFi(i), field->netLa(j) );
      GeoText* gt = new GeoText(l);
      gt->setValue( field->pointValue(gp) );
      GeoVector gv;
      gv.append(gp);
      gt->setSkelet(gv);
    }
  }
  delete reply; reply = 0;
  //  if(0 != field){
  //    delete field;field = 0;
  //  }
  QApplication::restoreOverrideCursor();
}

void GribDataWidget::removeLayer(QTreeWidgetItem* item)
{
  if( 0 == view_  || false == view_->hasMapscene() ){
    return;
  }
  QString uuid;
  if( layers_.contains(item) ){
    uuid = layers_[item]->uuid();
  }
  if( true == uuid.isEmpty() ){
    return;
  }
  view_->mapscene()->document()->rmLayer(uuid);
  ui_->addBtn->setDisabled(false);
  ui_->delBtn->setDisabled(true);
}

  void GribDataWidget::hideNoLevelItems()
  { // need refactor
    QTreeWidgetItemIterator it( ui_->treeWidget, QTreeWidgetItemIterator::NotHidden );
    while( *it ){
      if( 0 != (*it)->text(Level).toInt() ){
        (*it)->setHidden(true);
      }
      ++it;
    }
  }

bool GribDataWidget::hasDocument()
{
  if( 0 == view_ ){
    return false;
  }
  if ( false == view_->hasMapscene() ) {
    return false;
  }
  if( 0 == view_->mapscene()->document() ){
    return false;
  }
  return true;
}

void GribDataWidget::setOptions(const QString& options)
{
  if( "gribsurface" == options){
    ui_->groupBox->setVisible(false);
    setTitle(QObject::tr("GRIB (приземные)"));
    levels_ = false;
  }else if("gribaerology" == options){
    ui_->groupBox->setVisible(true);
    setTitle(QObject::tr("GRIB (аэрология)"));
    levels_ = true;
  }
  updateFields();
}

field::DataDescResponse* GribDataWidget::sendRequest()
{
  rpc::TController ctrl(channel_);
  meteo::field::DataRequest request;
  //request.add_meteo_descr(descr);
  QDate date = ui_->dateEdit->date();
  QTime time = ui_->timeEdit->time();
  request.set_date_start(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                         .arg(time.toString("hh:00:00")).toStdString());
  request.set_date_end(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                       .arg(time.toString("hh:59:59")).toStdString());
  if( "Все" != currentLevel_ && true == hasLevels() ){
    request.add_level(currentLevel_.toInt());
  }
//  if( true == hasLevels() ){
//    request.set_type_level(100);
//  }
   if( false == hasLevels() ){
     request.add_level(0);
   }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataDescResponse* reply = ctrl.remoteCall( &meteo::field::FieldService::GetAvailableData, request, 50000, true );
  qApp->restoreOverrideCursor();
  if ( 0 == reply ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
  }
  return reply;
}

void GribDataWidget::hideItems()
{
  QTreeWidgetItemIterator it( ui_->treeWidget );
  while( *it ){
    (*it)->setHidden(true);
    ++it;
  }
}

void GribDataWidget::updateItems()
{
  if( 0 == ui_){
    return;
  }
  if( 0 == view_  || false == view_->hasMapscene() ){
    return;
  }
  QList<Layer*> layers = view_->mapscene()->document()->layers();
  QTreeWidgetItem* item = 0;
  Layer* l = 0;
  foreach( l, layers_ ){
    item = layers_.key(l);
    if( 0 == item ){
      return;
    }
    if( false == layers.contains(l) ){
      for( int i = 0; i <= ui_->treeWidget->columnCount(); i++ ){
        if( 0 != item ){
//          if( true == itemsWidget_.contains(item) ){
//            ItemWidget* w = itemsWidget_.value(item);
//            if( 0 != w ){
//              w->blockSignals(true);
//              w->setChecked(false);
//              w->blockSignals(false);
//            }
//         }
          if( item->isSelected() ){
            ui_->delBtn->setDisabled(true);
            ui_->addBtn->setDisabled(false);
          }
        }
      }
      layers_.remove(item);
    }else{
      for( int i = 0; i <= ui_->treeWidget->columnCount(); i++ ){
        if( 0 != item ){
//          if( true == itemsWidget_.contains(item) ){
//            ItemWidget* w = itemsWidget_.value(item);
//            if( 0 != w ){
//              w->blockSignals(true);
//              w->setChecked(true);
//              w->blockSignals(false);
//            }
//          }
          if( item->isSelected() ){
            ui_->delBtn->setDisabled(false);
            ui_->addBtn->setDisabled(true);
          }
        }
      }
    }
  }
}

bool GribDataWidget::eventFilter(QObject* watched, QEvent* ev)
{
  if( 0 == view_  || false == view_->hasMapscene() ){
    return MapWidget::eventFilter(watched, ev);
  }
  if ( watched != view_->mapscene()->document()->eventHandler()) {
    return MapWidget::eventFilter(watched, ev);
  }
  else if ( meteo::map::LayerEvent::LayerChanged == ev->type() ) {
    updateItems();
  }
  return MapWidget::eventFilter(watched, ev);
}

void GribDataWidget::updateFields()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataDescResponse* reply = sendRequest();
  if( 0 == reply ){
    QApplication::restoreOverrideCursor();
    return;
  }
  rpc::TController ctrl(sprinf_);
  meteo::sprinf::MeteoCenterRequest request;
  for( int i = 0; i < reply->descr_size(); i++){
    meteo::field::DataDesc d = reply->descr(i);
    QString date_str = QString::fromStdString (d.date());
    QDateTime dt = QDateTime::fromString(date_str, Qt::ISODate);
    if( 0 != ui_->treeWidget->findItems(QString::number(d.id()), Qt::MatchContains, Id ).count()){
      continue;
    }
    CustomTreeWidgetItem* item = new CustomTreeWidgetItem(ui_->treeWidget);

    item->setText(Date, dt.toString("dd.MM.yy hh:mm"));
    item->setText(CountPoint, QString::number(d.count_point()));
    item->setText(MeteoDescriptor, QString::number(d.meteodescr()));
    item->setText(Center, QString::number(d.center()));
    item->setText(Level, QString::number(d.level()));
    item->setText(Hour, QString::number(d.hour()/3600));
    item->setText(Model, QString::number(d.model()));
    item->setText(Id, QString::number(d.id()));
    item->setText(TypeLevel, QString::number(d.level_type()));
    //ItemWidget* w = new ItemWidget(this);
    //ui_->treeWidget->setItemWidget( item, Button, w );
    ui_->treeWidget->setItemExpanded(item, true);
    //itemsWidget_.insert(item, w);
    //connect(w, SIGNAL(toggled(bool)), SLOT(slotSwitchLayer(bool)));
    request.add_center_id(d.center());
    centers_.insert(d.center(), item);
  }
  meteo::sprinf::MeteoCenters * response = ctrl.remoteCall(&meteo::sprinf::SprinfService::GetMeteoCenters, request, 10000);
  if (response != 0) {
    int sz = response->center_size();
    for( int i = 0; i < sz; i++ ){
      QString name = QString::fromStdString(response->center(i).name());
      QList<QTreeWidgetItem*> items = centers_.values(response->center(i).id());
      foreach( QTreeWidgetItem* item, items ){
        if( 0 != item ){
          item->setText(Center, name);
        }
      }
    }
    delete response;
  }
  for (int i = 0, sz = ui_->treeWidget->columnCount(); i < sz; ++i)
  {
    if( i != Center ){
      ui_->treeWidget->resizeColumnToContents(i);
    }
  }
  QDate date = ui_->dateEdit->date();
  QTime time = ui_->timeEdit->time();

  QString dt = QString("%1 %2").arg(date.toString("dd.MM.yy")).arg(time.toString("hh:mm"));
  QTreeWidgetItem* item = search(dt, hasLevels());
  if( false == hasLevels() ){
    hideNoLevelItems();
  }
  ui_->treeWidget->setCurrentItem(item);
  if( "Все" != ui_->hourCombo->currentText() ){
    ui_->treeWidget->setColumnWidth(Hour, 0);
  }else{
    ui_->treeWidget->resizeColumnToContents(Hour);
  }
  if( "Все" != currentLevel_ ){
    ui_->treeWidget->setColumnWidth(Level, 0);
  }else{
    ui_->treeWidget->resizeColumnToContents(Level);
  }
  QApplication::restoreOverrideCursor();
}

void GribDataWidget::slotRadioButtonClicked()
{
  QRadioButton* rb = static_cast<QRadioButton*>(sender());
  currentLevel_ = rb->text();
  updateFields();
}

void GribDataWidget::slotSorting(int col)
{
  Qt::SortOrder order = ui_->treeWidget->header()->sortIndicatorOrder();
  ui_->treeWidget->sortByColumn(col, order);
}

void GribDataWidget::slotSwitchLayer(bool /*on*/)
{
  /*
  ItemWidget* w = static_cast<ItemWidget*>(sender());
  if( 0 == w ){
    return;
  }
  QTreeWidgetItem* item = itemsWidget_.key(w);
  if( 0 == item ){
    return;
  }
  ui_->treeWidget->setCurrentItem(item);
  if( true == on ){
    addLayer(item);
  }else{
    removeLayer(item);
  }*/
}

void GribDataWidget::slotHourChanged(const QString& /*text*/)
{
  updateFields();
}

QTreeWidgetItem* GribDataWidget::search(const QString& text, bool hasLevel)
{
  QTreeWidgetItem* first = 0;
  QString searchText = text;

  bool needFilter = !searchText.isEmpty();

  if( !needFilter ){
    return 0;
  }
  QTreeWidgetItemIterator it2( ui_->treeWidget );
  while( *it2 ){
    (*it2)->setHidden( true );
    ++it2;
  }
  QTreeWidgetItemIterator it( ui_->treeWidget );
  while( *it ){
    (*it)->setHidden( needFilter );
    ++it;
  }
  QList<QTreeWidgetItem*> foundItems;
  foundItems  = ui_->treeWidget->findItems( searchText, Qt::MatchContains, Date );
  foundItems += ui_->treeWidget->findItems( searchText, Qt::MatchContains, Date );

  it = QTreeWidgetItemIterator( ui_->treeWidget, QTreeWidgetItemIterator::Hidden );

  QString hour = ui_->hourCombo->currentText();
  if( "Фактический" == hour ){
    hour = "0";
  }
  // need refactor
  if ( "Все" == currentLevel_ || false == hasLevel ){
    while( *it ){
      if( foundItems.contains(*it)){
        if( "Все" == hour ){
          (*it)->setHidden( false );
          if( first == 0 ){
            first = (*it);
          }
        foundItems.removeOne( *it );
        }else{
          if( hour == (*it)->text(Hour)){
            (*it)->setHidden( false );
            if( first == 0 ){
              first = (*it);
            }
          foundItems.removeOne( *it );
          }
        }
      }
      ++it;
    }
     // need refactor
  }else{
    QString lvl = currentLevel_;
    while( *it ){
      if( foundItems.contains(*it) &&
        (*it)->text(Level) == lvl){
        if( "Все" == hour ){
          (*it)->setHidden( false );
          if( first == 0 ){
            first = (*it);
          }
        foundItems.removeOne( *it );
        }else{
          if( hour == (*it)->text(Hour)){
            (*it)->setHidden( false );
            if( first == 0 ){
              first = (*it);
            }
          foundItems.removeOne( *it );
          }
        }
      }
      ++it;
    }
  }
  return first;
}

void GribDataWidget::slotAddLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( 0 == item ){
    return;
  }
  addLayer(item);
}

void GribDataWidget::slotContextMenu(QPoint point)
{
  if( 0 == view_  || false == view_->hasMapscene() ){
    return;
  }
  QAction* del = menu_->actions()[0];
  QAction* add = menu_->actions()[1];
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( 0 == item || 0 == del ){
    return;
  }
  if( layers_.contains(item) ){
    del->setDisabled(false);
    add->setDisabled(true);
  }else{
    del->setDisabled(true);
    add->setDisabled(false);
  }
  menu_->exec(ui_->treeWidget->mapToGlobal(point));
}

void GribDataWidget::slotRemoveLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( 0 == item ){
    return;
  }
  removeLayer(item);
}

void GribDataWidget::slotItemSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
  QTreeWidgetItem* item = current;

  if( 0 == item ){
    return;
  }

  if( layers_.contains(item) ){
    ui_->addBtn->setDisabled(true);
    ui_->delBtn->setDisabled(false);
  }else{
    ui_->addBtn->setDisabled(false);
    ui_->delBtn->setDisabled(true);
  }
}

}
}
