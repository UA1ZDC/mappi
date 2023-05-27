#include "selectlevel.h"

#include <cross-commons/debug/tlog.h>

#include "ui_selectlevel.h"

QMap<QAbstractButton*, QPair< int, int > > initAeroLevels( Ui::SelectLevel* ui )
{
  QMap< QAbstractButton*, QPair< int, int > > map;
  map.insert( ui->aeroAll, qMakePair( 100, -1 ) );
  map.insert( ui->aeroLE5, qMakePair( 100, 0 ) );
  map.insert( ui->aero5, qMakePair( 100, 5 ) );
  map.insert( ui->aero7, qMakePair( 100, 7 ) );
  map.insert( ui->aero10, qMakePair( 100,10 ) );
  map.insert( ui->aero20, qMakePair( 100,20 ) );
  map.insert( ui->aero30, qMakePair( 100,30 ) );
  map.insert( ui->aero50, qMakePair( 100,50 ) );
  map.insert( ui->aero70, qMakePair( 100,70 ) );
  map.insert( ui->aero100, qMakePair( 100,100 ) );
  map.insert( ui->aero150, qMakePair( 100,150 ) );
  map.insert( ui->aero200, qMakePair( 100,200 ) );
  map.insert( ui->aero250, qMakePair( 100,250 ) );
  map.insert( ui->aero300, qMakePair( 100,300 ) );
  map.insert( ui->aero400, qMakePair( 100,400 ) );
  map.insert( ui->aero500, qMakePair( 100,500 ) );
  map.insert( ui->aero700, qMakePair( 100,700 ) );
  map.insert( ui->aero850, qMakePair( 100,850 ) );
  map.insert( ui->aero925, qMakePair( 100,925 ) );
  map.insert( ui->aero1000, qMakePair( 100,1000 ) );
  map.insert( ui->aerotropo, qMakePair(7,0) );
  map.insert( ui->aeromaxwind, qMakePair(6,0) );
  return map;
}

QMap<QAbstractButton*, QPair< int, int > > initOceanLevels( Ui::SelectLevel* ui )
{
  QMap< QAbstractButton*, QPair< int, int > > map;
  map.insert( ui->ocean5, qMakePair( 160, 5 ) );
  map.insert( ui->ocean10, qMakePair( 160,10 ) );
  map.insert( ui->ocean15, qMakePair( 160,15 ) );
  map.insert( ui->ocean20, qMakePair( 160,20 ) );
  map.insert( ui->ocean25, qMakePair( 160,25 ) );
  map.insert( ui->ocean30, qMakePair( 160,30 ) );
  map.insert( ui->ocean40, qMakePair( 160,40 ) );
  map.insert( ui->ocean50, qMakePair( 160,50 ) );
  map.insert( ui->ocean100, qMakePair( 160, 100 ) );
  map.insert( ui->ocean150, qMakePair( 160, 150 ) );
  map.insert( ui->ocean200, qMakePair( 160, 200 ) );
  map.insert( ui->ocean250, qMakePair( 160, 250 ) );
  map.insert( ui->ocean300, qMakePair( 160, 300 ) );
  map.insert( ui->ocean400, qMakePair( 160, 400 ) );
  map.insert( ui->oceanbottom, qMakePair( 9, -1 ) );
  return map;
}

SelectLevel::SelectLevel( QWidget* p )
  : QWidget(p),
  ui_(new Ui::SelectLevel),
  mode_(kAero)
{
  ui_->setupUi(this);
  setViewMode(mode_);
  QObject::connect( ui_->groupaero, SIGNAL( buttonClicked( QAbstractButton* ) ),
      this, SLOT( slotAeroClicked( QAbstractButton* ) ) );
  QObject::connect( ui_->groupsea, SIGNAL( buttonClicked( QAbstractButton* ) ),
      this, SLOT( slotOceanClicked( QAbstractButton* ) ) );
  aerobuttons_ = initAeroLevels(ui_);
  oceanbuttons_ = initOceanLevels(ui_);
}

SelectLevel::~SelectLevel()
{
  delete ui_; ui_ = 0;
}

void SelectLevel::setViewMode( ViewMode m )
{
  mode_ = m;
  switch ( mode_ ) {
    case kAero:
      ui_->oceanbox->hide();
      ui_->custombox->hide();
      ui_->aerobox->show();
      break;
    case kOcean:
      ui_->aerobox->hide();
      ui_->custombox->hide();
      ui_->oceanbox->show();
      break;
    case kCustom:
      ui_->aerobox->hide();
      ui_->oceanbox->hide();
      ui_->custombox->show();
      break;
    case kNoMode:
      ui_->aerobox->hide();
      ui_->oceanbox->hide();
      ui_->custombox->hide();
      break;
  }
  setLevel( -1, -1, mode_ );
  QWidget::adjustSize();
}

void SelectLevel::setLevel( int lvl )
{
  level_ = lvl;
}

void SelectLevel::setTypeLevel( int tplvl )
{
  type_level_ = tplvl;
}

void SelectLevel::setLevel( int lvl, int tplvl, ViewMode m )
{
  if ( m != mode_ ) {
    setViewMode(m);
  }
  QMap< QAbstractButton*, QPair< int, int > > map;
  int l = 0;
  int t = 1;
  QAbstractButton* btn = 0;
  switch ( mode_ ) {
    case kAero:
      map = aerobuttons_;
      break;
    case kOcean:
      map = oceanbuttons_;
      break;
    case kCustom:
      map = custombuttons_;
      break;
    case kNoMode:
    default:
      break;
  }
  if ( true == map.isEmpty() ) {
    emit levelChanged( l, t );
    return;
  }
  if ( -1 == tplvl ) {
    l = map.begin().value().second;
    t = map.begin().value().first;
    btn = map.begin().key();
  }
  else {
    QMapIterator<QAbstractButton*, QPair< int, int > > it(map);
    while ( true == it.hasNext() ) {
      it.next();
      if ( it.value().second == lvl && it.value().first == tplvl ) {
        l = it.value().second;
        t = it.value().first;
        btn = it.key();
        break;
      }
    }
  }
  setLevel(l);
  setTypeLevel(t);
  if ( 0 == btn ) {
    error_log << QObject::tr("Не удалось установить уровень %1 с типом %2")
      .arg(lvl)
      .arg(tplvl);
  }
  else {
    if ( false == btn->isChecked() ) {
      btn->setChecked(true);
    }
  }
  emit levelChanged( level_, type_level_ );
}

void SelectLevel::slotAeroClicked( QAbstractButton* btn )
{
  if ( false == aerobuttons_.contains(btn) ) {
    error_log.msgBox() << QObject::tr("Неизвестный уровень '%1'").arg( btn->text() );
    return;
  }
  const QPair<int,int> lvl = aerobuttons_[btn];
  setLevel( lvl.second, lvl.first, kAero );
}

void SelectLevel::slotOceanClicked( QAbstractButton* btn )
{
  if ( false == oceanbuttons_.contains(btn) ) {
    error_log.msgBox() << QObject::tr("Неизвестный уровень '%1'").arg( btn->text() );
    return;
  }
  const QPair<int,int> lvl = oceanbuttons_[btn];
  setLevel( lvl.second, lvl.first, kOcean );
}
