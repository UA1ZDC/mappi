#include "settingsform.h"
#include "ui_settings.h"
#include "dbsettings.h"
#include "appstartsettings.h"
#include "locsettings.h"
#include "msgsettings.h"
#include "climatsettings.h"
#include "stormsettings.h"
#include "pogodainputsettings.h"
#include "dblist.h"
#include "unistd.h"

#include "pogodainputsettingsperuser.h"

#include <meteo/commons/settings/settings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <QtCore>
#include <QtWidgets>
#include <qtreeview.h>
#include <qevent.h>

enum columns {
  Id,
  Icon,
  Name
};

namespace meteo {
  const QString settingsGroupName(GroupNumber num) {
    switch (num) {
    case (GroupNumber::GENERAL): {
      return QString::fromUtf8("Общие");
    }
    case (GroupNumber::DB_LIST): {
      return QString::fromUtf8("Базы данных");
    }
    case (GroupNumber::MSG_GROUP):{
      return QString::fromUtf8("Журналирование");
    }
    }
    return QString();
  }

TSettingsForm::TSettingsForm( QWidget* p ) :
  QWidget(p),
  ui_(new Ui::Settings)
{
  ui_->setupUi(this);


  registerPages();
  meteo::gSettings(new meteo::Settings);
  meteo::gSettings()->load();

  QObject::connect(ui_->saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));
  QObject::connect(ui_->closeButton, SIGNAL(clicked()), this, SLOT(close()));

  ui_->pageList->setHeaderLabel( "settings group" );
  ui_->pageList->setHeaderLabel( "id" );
  ui_->pageList->header()->hide();
  ui_->pageList->setRootIsDecorated(false);
  ui_->pageList->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->pageList->setColumnHidden(Id, true);
  ui_->pageList->setColumnHidden(Name, false);
  ui_->pageList->setColumnHidden(Icon, false);
  ui_->pageList->setColumnWidth(Icon, 32);

  QObject::connect( ui_->pageList, SIGNAL(itemSelectionChanged()), this, SLOT(slotSwitchPage()) );
  for (auto curGroup = tabs_.begin(), endGroup = tabs_.end(); curGroup != endGroup; ++curGroup) {
    QString groupName = settingsGroupName(curGroup.key());
    if (false == groupName.isEmpty()) {
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->pageList);
      item->setText(Name, groupName);
      item->setText(Id, QString::number(curGroup.key()));
      QPixmap pixmap(":/meteo/icons/apps/wrb.png");
      QIcon icon(pixmap);
      item->setIcon(Icon,icon);
      ui_->pageList->insertTopLevelItem(curGroup.key(), item);
      QWidget* w = qobject_cast<QWidget*>(curGroup.value());
      if (nullptr != w) {
        ui_->stackedWidget->insertWidget(curGroup.key(),curGroup.value());
      }
    }
  }

  ui_->pageList->topLevelItem(0)->setSelected(true);
  ui_->saveButton->setEnabled(false);
}

TSettingsForm::~TSettingsForm()
{
  delete ui_;
  ui_ = nullptr;
}

void TSettingsForm::init()
{
  for ( auto wgt: this->settWgts_ ){
    wgt->init();
  }
}

void TSettingsForm::load()
{  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  for ( auto wgt: this->settWgts_ ){
    wgt->load();
  }
  QApplication::restoreOverrideCursor();
}

void TSettingsForm::registerPages()
{
  addPage(GroupNumber::GENERAL, APP_START, new meteo::AppStartSettingsWidget(this));
  meteo::LocSettings* loc = new meteo::LocSettings(this);
  addPage(GroupNumber::GENERAL, LOC, loc);
  meteo::ClimatSettings* climat = new meteo::ClimatSettings(loc->loc(), this);

  addPage(GroupNumber::GENERAL, CLIMAT_STATIONS, climat);
  QObject::connect(loc, SIGNAL(locationChanged(const meteo::settings::Location&)),
                   climat, SLOT(ownStationChanged(const meteo::settings::Location&)));
  auto storm = new meteo::WgtStormSettings(loc->loc(), this);

  QObject::connect(loc, SIGNAL(locationChanged(const meteo::settings::Location&)),
                   storm, SLOT(ownStationChanged(const meteo::settings::Location&)));
  addPage(GroupNumber::GENERAL, STORM_RING_SETTINGS, storm);

  auto pogoda_input = new meteo::PogodaInputSettingsPerUser(loc->loc(), this);

  QObject::connect(loc, SIGNAL(locationChanged(const meteo::settings::Location&)),
                   pogoda_input, SLOT(ownStationChanged(const meteo::settings::Location&)));
  addPage(GroupNumber::GENERAL, POGODAINPUT_SETTINGS, pogoda_input);

  auto dbConfs = meteo::gSettings()->dbConfs();

  addPage(MSG_GROUP, MSG, new meteo::MsgSettings(this));

  addPage(DB_LIST, DB_LIST_PAGE, new meteo::DbListWidget(this));  
}

void TSettingsForm::addPage(GroupNumber gnum, PageNumber pnum, meteo::SettingsWidget* page)
{
  if (page == nullptr) {
      return;
  }
  if (false == tabs_.contains(gnum))
  {
    QTabWidget* w = new QTabWidget();
    tabs_.insert(gnum, w);
  }
  if (false == tabs_.contains(gnum))
  {
    return;
  }
  if ( (-1 == tabs_[gnum]->indexOf(page) ) && (nullptr == tabs_[gnum]->widget(pnum) ) )
  {
    tabs_[gnum]->insertTab(pnum, page, page->objectName());
    // connect(_ui->saveButton, SIGNAL(clicked()), page, SLOT(slotSave())); //это делается в this->slotSave()
    // connect(_ui->saveButton, SIGNAL(clicked()), this, SLOT(slotDeactivateBtn()));
    QObject::connect(page, SIGNAL(changed()), this, SLOT(slotActivateBtn()));
    //page->init();
  }

  this->settWgts_ << page;
}

void TSettingsForm::closeEvent(QCloseEvent* event )
{
  bool doClose = canClose();
  if (doClose == false)
  {
    event->ignore();
    return;
  }
//  QMdiSubWindow* parent = qobject_cast<QMdiSubWindow*>(this->parent());
//  if (parent != 0)
//  {
//   parent->close();
//   return;
//  }
  event->accept();
}

void TSettingsForm::slotSwitchPage()
{
  if( ui_->pageList->selectedItems().size() == 0 ){
     return;
   }
  QTreeWidgetItem* item = ui_->pageList->selectedItems().first();
  if ( nullptr == item ) {
    error_log.msgBox() << QObject::tr("Невозможно переключить страницу настроек. Страница выбрана, но по неизвестным причинам уже не существует.");
    return;
  }
  int groupId = item->text( Id ).toInt();
  ui_->stackedWidget->setCurrentIndex(groupId);
  ui_->stackedWidget->adjustSize();
}

void TSettingsForm::keyPressEvent(QKeyEvent* e)
{
  if( e->key() == Qt::Key_Escape ) {
    e->accept();
    close();
    return;
  }
  e->accept();
}

void TSettingsForm::slotSave()
{  
  if (widgetsIsChanged() == false) {
    return;
  }
  if (saveWidgets() == true) {
    slotDeactivateBtn();
  }
}

bool TSettingsForm::canClose()
{
  if (  true == widgetsIsChanged() ) {
    int answer = showQuestion(tr("Настройки изменены"),
                              tr("Сохранить изменения?"));
    if( 0  == answer ) {
      if (saveWidgets() == true) {
        slotDeactivateBtn();
      }
      else {
        return false;
      }
    }
    else if( 2  == answer ) {
      return false;
    }
  }
  return true;
}

bool TSettingsForm::widgetsIsChanged() const
{
  bool pagesChanged = false;
  for (auto begin = tabs_.begin(), it = begin, end = tabs_.end(); it != end; ++it) {
    QTabWidget* tab = it.value();
    if (nullptr != tab) {
      for (int i = 0, end = tab->count(); i < end; i++) {
        QWidget* w = tab->widget(i);
        meteo::SettingsWidget* wgt = qobject_cast<meteo::SettingsWidget*>(w);
        if (wgt != nullptr) {
          pagesChanged = (pagesChanged || wgt->isChanged());
        }
      }
    }
  }
  return pagesChanged;
}

bool TSettingsForm::saveWidgets()
{  
  for (auto it = tabs_.begin(), end = tabs_.end(); it != end; ++it) {
    QTabWidget* tab = it.value();
    if (nullptr != tab) {
      for (int i = 0, end = tab->count(); i < end; i++) {
        QWidget* w = tab->widget(i);
        meteo::SettingsWidget* wgt = qobject_cast<meteo::SettingsWidget*>(w);
        if (wgt != nullptr) {
          wgt->slotSave();
        }
      }
    }
  }
  return !widgetsIsChanged();
}

void TSettingsForm::slotActivateBtn()
{
  ui_->saveButton->setEnabled(true);
}

void TSettingsForm::slotDeactivateBtn()
{
  ui_->saveButton->setEnabled(false);
}

void TSettingsForm::showMsg(const QString& err)
{
    QMessageBox::warning(this,tr("Ошибка"),err);
    return;
}

int TSettingsForm::showQuestion(const QString& questionTitle, const QString& questionText)
{
  return QMessageBox::question( this,
                                questionTitle,
                                questionText,
                                tr("Сохранить и закрыть"),
                                tr("Закрыть без сохранения"),
                                tr("Отмена") );
}

void TSettingsForm::showEvent( QShowEvent* event )
{
  QWidget::showEvent(event);
  qApp->processEvents();
  QTimer::singleShot(50, this, &meteo::TSettingsForm::load);
}

}
