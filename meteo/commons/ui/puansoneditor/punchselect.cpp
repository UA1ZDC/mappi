#include "punchselect.h"

#include <qheaderview.h>
#include <qmessagebox.h>

#include "ui_punchselect.h"
#include "metaedit.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace puanson {

namespace {
 static const int kCodeClmn(1);
 static const int kTitleClmn(0);
 static const int kUserTab(0);
 static const int kSpecTab(1);
}

PunchSelect::PunchSelect( QWidget* p )
  : QDialog(p),
  ui_( new Ui::PunchSelect )
{
  ui_->setupUi(this);
  QObject::connect( ui_->punchtypetab, SIGNAL( currentChanged( int ) ), this, SLOT( slotCurrentTabChanged( int ) ) );
  QObject::connect( ui_->btnaddpunch, SIGNAL( clicked() ), this, SLOT( slotAddPunch() ) );
  QObject::connect( ui_->btnrmpunch, SIGNAL( clicked() ), this, SLOT( slotRmPunch() ) );
  ui_->btneditpunch->hide();
  ui_->btnrmpunch->hide();
  QObject::connect( ui_->btneditpunch, SIGNAL( clicked() ), this, SLOT( slotEditPunch() ) );
  QObject::connect( ui_->userpunchtree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( accept() ) );
  QObject::connect( ui_->specpunchtree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( accept() ) );
  loadPunchLibrary();
  if ( 0 != ui_->userpunchtree->topLevelItemCount() ) {
    ui_->userpunchtree->topLevelItem(0)->setSelected(true);
  }
}

PunchSelect::~PunchSelect()
{
  delete ui_; ui_ = 0;
}

void PunchSelect::setSelectedPunch( const meteo::puanson::proto::Puanson& punch )
{
  selectedpunch_ = punch;
  QTreeWidget* tree = 0;
  if ( meteo::puanson::proto::kSpecial == punch.datatype() ) {
    ui_->punchtypetab->setCurrentIndex(kSpecTab);
    tree = ui_->specpunchtree;
  }
  else {
    ui_->punchtypetab->setCurrentIndex(kUserTab);
    tree = ui_->userpunchtree;
  }
  if ( 0 == tree ) {
    error_log << QObject::tr("Текущий Шаблон пуансона не удалось определить, потому что выбрана вкладка с неизвестным индексом");
    return;
  }
  tree->clearSelection();
  QString code = QString::fromStdString( punch.code() );
  QList<QTreeWidgetItem*> list = tree->findItems( code, Qt::MatchExactly, kCodeClmn );
  if ( 0 != list.size() ) {
    list[0]->setSelected(true);
  }
}

void PunchSelect::loadPunchLibrary()
{
  meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
  if ( 0 == wl ) {
    error_log << QObject::tr("Не обнаружен загрузчик пуансонов");
    return;
  }
  wl->loadPunchLibrary();
  loadPunchesInTree( ui_->userpunchtree, wl->punchlibrary() );
  loadPunchesInTree( ui_->specpunchtree, wl->punchlibraryspecial() );
}

void PunchSelect::loadPunchesInTree( QTreeWidget* tree, const QMap<QString, meteo::puanson::proto::Puanson>& library )
{
  tree->clear();
  QMapIterator<QString, meteo::puanson::proto::Puanson> it(library);
  while ( true == it.hasNext() ) {
    it.next();
    const meteo::puanson::proto::Puanson& punch = it.value();
    if ( false == punch.libvisible() ) {
      continue;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(tree);
    QSize sz = item->sizeHint(kCodeClmn);
    sz.setHeight(32);
    item->setSizeHint( kCodeClmn, sz );
    item->setText( kCodeClmn, QString::fromStdString( punch.code() ) );
    item->setText( kTitleClmn, QString::fromStdString( punch.name() ) );
    std::string str;
    punch.SerializeToString(&str);
    QByteArray arr( str.data(), str.size() );
    item->setData( kTitleClmn, Qt::UserRole, QVariant(arr) );
  }
  tree->header()->setSectionResizeMode( kTitleClmn, QHeaderView::Interactive );
  tree->header()->setSectionResizeMode( kCodeClmn, QHeaderView::Stretch );
  tree->setColumnWidth( kTitleClmn, 300 );
}
    
meteo::puanson::proto::Puanson PunchSelect::currentPunch( bool* ok ) const
{
  meteo::puanson::proto::Puanson p;
  QTreeWidget* tree = 0;
  *ok = false;
  if ( kUserTab == ui_->punchtypetab->currentIndex() ) {
    tree = ui_->userpunchtree;
  }
  else {
    tree = ui_->specpunchtree;
  }
  if ( 0 == tree ) {
    error_log << QObject::tr("Текущий Шаблон пуансона не удалось определить, потому что выбрана вкладка с неизвестным индексом");
    return p;
  }
  QList<QTreeWidgetItem*> list = tree->selectedItems();
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Текущий Шаблон пуансона не удалось определить, потому что не выбран пуансон");
    return p;
  }
  QTreeWidgetItem* item = list[0];
  QByteArray arr = item->data( kTitleClmn, Qt::UserRole ).toByteArray();
  std::string str( arr.data(), arr.size() );
  *ok = p.ParseFromString(str);
  return p;
}

void PunchSelect::slotCurrentTabChanged( int indx )
{
  Q_UNUSED(indx);
}

void PunchSelect::slotAddPunch()
{
  Metaedit* dlg = new Metaedit(this);
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    meteo::puanson::proto::Puanson p;
    p.set_code( dlg->code().toStdString() );
    p.set_name( dlg->title().toStdString() );
    p.set_datatype( static_cast<meteo::puanson::proto::DataType>( dlg->type() ) );
    meteo::map::WeatherLoader::instance()->addPunchToLibrary(p);
    loadPunchLibrary();
    setSelectedPunch(p);
  }
  delete dlg;
}

void PunchSelect::slotRmPunch()
{
  bool ok = false;
  meteo::puanson::proto::Puanson p = currentPunch(&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Непредвиденная ошибка");
    return;
  }
  int res = QMessageBox::warning( this,
      QObject::tr("!!!УДАДЕНИЕ ШАБЛОНА!!!"),
      QObject::tr("Удалить шаблон %1? Это может нарушить функционирование системы!!!").arg( QString::fromStdString( p.name() ) ),
      QObject::tr("Да"),
      QObject::tr("Нет"));

  if ( 0 == res ) {
    meteo::map::WeatherLoader::instance()->rmPunchFromLibrary(p);
    loadPunchLibrary();
  }
}

void PunchSelect::slotEditPunch()
{
  bool ok = false;
  meteo::puanson::proto::Puanson p = currentPunch(&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Непредвиденная ошибка");
    return;
  }
  Metaedit* dlg = new Metaedit(this);
  dlg->setCurrentCodeAndTitleAndType( QString::fromStdString( p.code() ), QString::fromStdString( p.name() ), p.datatype() );
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    meteo::map::WeatherLoader::instance()->rmPunchFromLibrary(p);
    p.set_code( dlg->code().toStdString() );
    p.set_name( dlg->title().toStdString() );
    p.set_datatype( static_cast<meteo::puanson::proto::DataType>(dlg->type()) );
    std::string str;
    QByteArray arr( str.data(), str.size() );
    p.SerializeToString(&str);
    arr = QByteArray( str.data(), str.size() );
    meteo::map::WeatherLoader::instance()->addPunchToLibrary(p);
    loadPunchLibrary();
    meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
    if ( 0 == wl ) {
      error_log << QObject::tr("Не обнаружен загрузчик пуансонов");
      return;
    }
    if ( true == wl->punchlibrary().contains( dlg->code() ) ) {
      setSelectedPunch( wl->punchlibrary()[dlg->code()] ) ;
    }
    else if ( true == wl->punchlibraryspecial().contains( dlg->code() ) ) {
      setSelectedPunch( wl->punchlibraryspecial()[dlg->code()] ) ;
    }
  }
  delete dlg;
}

void PunchSelect::accept()
{
  bool ok = false;
  selectedpunch_ = currentPunch(&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Непредвиденная ошибка");
    return;
  }
  QDialog::accept();
}

void PunchSelect::reject()
{
  QDialog::reject();
}

}
}
