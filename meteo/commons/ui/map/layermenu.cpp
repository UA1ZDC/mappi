#include "layermenu.h"

#include <qaction.h>
#include <qmenu.h>
#include <qicon.h>
#include <qwidgetaction.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qlabel.h>

#include "layer.h"

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/ui/custom/alphaedit.h>
#include <meteo/commons/ui/custom/doublespinaction.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/custom/layerename.h>
#include <meteo/commons/global/global.h>


#include "layeriso.h"
#include "layerpunch.h"
#include "layerairport.h"
#include "layerborder.h"
#include "document.h"
#include "weather.h"

namespace meteo {
namespace map {


const QString kNoOrnament = QString("kNoOrnament");
const QString kSolidLine = QObject::tr("Сплошная");
const QString kDashLine  = QObject::tr("Штриховой пунктир");
const QString kDotLine = QObject::tr("Точечный пунктир");
const QString kDashDotLine = QObject::tr("Штрихпунктир");
const QString kDashDotDotLine = QObject::tr("Штрихпунктир с двумя точками");
const QString kDblSolidLine = QObject::tr("Двойная сплошная");
const QString kDblDashLine  = QObject::tr("Двойная пунктирная");

const QStringList kLineTypes = QStringList()
  << kSolidLine
  << kDashLine
  << kDotLine
  << kDashDotLine
  << kDashDotDotLine;

static const QMap< int, QString > kAlphaFixed = TMap< int, QString >()
  << qMakePair(  100, QObject::tr("100%"))
  << qMakePair(  75,  QObject::tr("75%") )
  << qMakePair(  50,  QObject::tr("50%") )
  << qMakePair(  25,  QObject::tr("25%") )
  << qMakePair(  10,  QObject::tr("10%") )
  << qMakePair(  0,   QObject::tr("0%")  );
  
static const  QList<int> kLevels = QList<int>()
  << 5 << 7 << 10 << 20 << 30 << 50 << 70 << 100 << 150
  << 200 << 250 << 300 << 400 << 500 << 700 << 850 << 925 << 1000;

LayerMenu::LayerMenu( Layer* l, QObject* p)
  : QObject(p),
  layer_(l),
  alphaedit_(0),
  actionactive_(0),
  actionrm_(0),
  actionup_(0),
  actiondown_(0),
  actionshow_(0),
  actionshowall_(0),
  actionhide_(0),
  actionhideall_(0),
  actionopacity_(0),
  alphamenu_(0)
{
  actionrm_ = new QAction( QIcon(":/meteo/icons/tools/minus.png"), QObject::tr("Удалить"), this );
  QObject::connect( actionrm_, SIGNAL( triggered() ), this, SLOT( slotRmLayer() ) );

  actionshow_ = new QAction( QIcon(":/meteo/icons/tools/layer_visible.png"), QObject::tr("Показать"), this );
  QObject::connect( actionshow_, SIGNAL( triggered() ), this, SLOT( slotShowLayer() ) );

  actionhide_ = new QAction( QIcon(":/meteo/icons/tools/set_layer_hidden.png"), QObject::tr("Скрыть"), this );
  QObject::connect( actionhide_, SIGNAL( triggered() ), this, SLOT( slotHideLayer() ) );

  actionshowall_ = new QAction( QIcon(":/meteo/icons/tools/layer_visible.png"), QObject::tr("Показать все слои с данными"), this );
  QObject::connect( actionshowall_, SIGNAL( triggered() ), this, SLOT( slotShowAll() ) );

  actionhideall_ = new QAction( QIcon(":/meteo/icons/tools/set_layer_hidden.png"), QObject::tr("Скрыть все слои с данными"), this );
  QObject::connect( actionhideall_, SIGNAL( triggered() ), this, SLOT( slotHideAll() ) );

  actionactive_ = new QAction( QIcon(":/meteo/icons/tools/star.png"), QObject::tr("Сделать активным"), this );
  QObject::connect( actionactive_, SIGNAL( triggered() ), this, SLOT( slotSetActive() ) );

  actionup_ = new QAction( QIcon(":/meteo/icons/tools/layer_up.png"), QObject::tr("Поднять"), this );
  QObject::connect( actionup_, SIGNAL( triggered() ), this, SLOT( slotUp() ) );

  actiondown_ = new QAction( QIcon(":/meteo/icons/tools/layer_down.png"), QObject::tr("Опустить"), this );
  QObject::connect( actiondown_, SIGNAL( triggered() ), this, SLOT( slotDown() ) );

  QMenu* alphamenu = new QMenu( QObject::tr("Прозрачность") );
  QMapIterator< int, QString > ait(kAlphaFixed);
  while ( true == ait.hasNext() ) {
    ait.next();
    alphamenu->addAction( ait.value(), this, SLOT( slotSetAlphaFixed() ) );
  }

  QWidgetAction* alphact = new QWidgetAction(alphamenu);
  alphaedit_ = new AlphaEdit(alphamenu, alphact);
  alphact->setDefaultWidget(alphaedit_);
  QObject::connect( alphact, SIGNAL( triggered() ), this, SLOT( slotSetAlphaCustom() ) );
  alphamenu->addAction(alphact);
  actionopacity_ = alphamenu->menuAction();
  actionrename_ = new QAction( QObject::tr("Переименовать"), this );
  QObject::connect( actionrename_, SIGNAL( triggered() ), this, SLOT( slotRename() ) );
}

LayerMenu::~LayerMenu()
{
  delete alphamenu_; alphamenu_ = 0;
}

void LayerMenu::addActions( QMenu* menu )
{
  if ( 0 == menu) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( 0 == layer_ ) {
    return;
  }
  if ( true == layer_->visible() ) {
    actionshow_->setDisabled(true);
    actionhide_->setEnabled(true);
  }
  else {
    actionshow_->setEnabled(true);
    actionhide_->setDisabled(true);
  }

  bool visfl = false;
  bool hidfl = false;

  bool actall = false;
  QString postfix;
  int type = layer_->type();
  if ( 0 != layer_->document() ) {
    if ( ( kLayerIso == type ) ) {
      actall = true;
      postfix = QObject::tr("все изолинии");
    }
    else if ( ( kLayerPunch == type ) ) {
      actall = true;
      postfix = QObject::tr("всю наноску");
    }
    else if ( ( kLayerMrl == type ) ) {
      actall = true;
      postfix = QObject::tr("все радиолокационные данные");
    }
    Document* d = layer_->document();
    if ( false == actall ) {
      actionshowall_->setVisible(false);
      actionhideall_->setVisible(false);
    }
    else {
      actionshowall_->setVisible(true);
      actionshowall_->setText( QObject::tr("Показать %1").arg(postfix) );
      actionhideall_->setVisible(true);
      actionhideall_->setText( QObject::tr("Скрыть %1").arg(postfix) );
      for ( int i = 0, sz = d->layers().size(); i < sz; ++i ) {
        Layer* l = d->layers()[i];
        if ( ( l->type() == type ) ) {
          if ( true == l->visible() ) {
            hidfl = true;
          }
          else {
            visfl = true;
          }
        }
      }
      actionshowall_->setEnabled(visfl);
      actionhideall_->setEnabled(hidfl);
    }
    if ( layer_ == d->activeLayer() ) {
      actionactive_->setDisabled(true);
    }
    else {
      actionactive_->setEnabled(true);
    }

    const QMap< Layer*, int >& order = d->layerOrder();
    if ( false == order.contains(layer_) ) {
      actionup_->setEnabled(true);
      actiondown_->setEnabled(true);
    }
    else if ( 2 > order.size() ) {
      actionup_->setEnabled(false);
      actiondown_->setEnabled(false);
    }
    else {
      int num = order[layer_];
      if ( num+1 < d->layers().size() ) {
        actionup_->setEnabled(true);
      }
      else {
        actionup_->setDisabled(true);
      }
      if ( 0 < num ) {
        actiondown_->setEnabled(true);
      }
      else {
        actiondown_->setDisabled(true);
      }
    }
    bool flrm = false;
    for ( int i = 0, sz = foremove_.size(); i < sz; ++i ) {
      Layer* l = foremove_[i];
      if ( false == l->isBase() ) {
        flrm = true;
      }
    }
    actionrm_->setEnabled(flrm);
  }

  alphaedit_->setPercent(layer_->alphaPercent());

  menu->addAction(actionactive_);
  menu->addAction(actionrm_);
  menu->addAction(actionup_);
  menu->addAction(actiondown_);
  menu->addAction(actionshow_);
  menu->addAction(actionhide_);
  menu->addAction(actionshowall_);
  menu->addAction(actionhideall_);
  menu->addAction(actionopacity_);

  if ( false == layer_->isBase() ) {
    menu->addAction(actionrename_);
  }
}

void LayerMenu::setLayersForRemove( const QList<Layer*>& list )
{
  foremove_ = list;
}

void LayerMenu::slotShowLayer()
{
  if ( 0 == layer_ ) {
    return;
  }
  layer_->setVisisble(true);
}

void LayerMenu::slotHideLayer()
{
  if ( 0 == layer_ ) {
    return;
  }
  layer_->setVisisble(false);
}

void LayerMenu::slotShowAll()
{
  if ( 0 == layer_ ) {
    return;
  }
  int type = layer_->type();
  if ( 0 != layer_->document() ) {
    Document* d = layer_->document();
    for ( int i = 0, sz = d->layers().size(); i < sz; ++i ) {
      Layer* l = d->layers()[i];
      if ( ( l->type() == type ) ) {
        l->setVisisble(true);
      }
    }
  }
}

void LayerMenu::slotHideAll()
{
  if ( 0 == layer_ ) {
    return;
  }
  int type = layer_->type();
  if ( 0 != layer_->document() ) {
    Document* d = layer_->document();
    for ( int i = 0, sz = d->layers().size(); i < sz; ++i ) {
      Layer* l = d->layers()[i];
      if ( ( l->type() == type ) ) {
        l->setVisisble(false);
      }
    }
  }
}

void LayerMenu::slotSetActive()
{
  if ( 0 == layer_ ) {
    return;
  }
  layer_->setActive();
}

void LayerMenu::slotRmLayer()
{
  if ( 0 == foremove_.size() ) {
    return;
  }
  QString names;
  for ( int i = 0, sz = foremove_.size(); i < sz; ++i ) {
    Layer* l = foremove_[i];
    if ( true == l->isBase() ) {
      continue;
    }
    names += ( "\t- " + l->name() + '\n' );
  }
  int res = QMessageBox::question(
              0,
              QObject::tr("Удаление"),
              QObject::tr("Удалить следующие слои? \n%1").arg(names),
              QObject::tr("Да"),
              QObject::tr("Нет")
              );
  if( 0 != res ){
    foremove_.clear();
    return;
  }
  for ( int i = 0, sz = foremove_.size(); i < sz; ++i ) {
    Layer* l = foremove_[i];
    if ( false == l->isBase() ) {
      delete l;
    }
  }
  foremove_.clear();
}

void LayerMenu::slotUp()
{
  if ( 0 == layer_ ) {
    return;
  }
  if ( 0 != layer_->document() ) {
    layer_->document()->moveLayer( layer_->uuid(), 1 );
  }
}

void LayerMenu::slotDown()
{
  if ( 0 == layer_ ) {
    return;
  }
  if ( 0 != layer_->document() ) {
    layer_->document()->moveLayer( layer_->uuid(), -1 );
  }
}

void LayerMenu::slotSetAlphaCustom()
{
  if ( 0 == layer_ ) {
    return;
  }
  if ( 0 == alphaedit_ ) {
    return;
  }
  layer_->setAlphaPercent( alphaedit_->percent() );
  layer_->repaint();
}

void LayerMenu::slotSetAlphaFixed()
{
  QAction* a = qobject_cast<QAction*>( QObject::sender() );
  if ( 0 == a ) {
    return;
  }
  QString atext = a->text();
  atext.replace( "&", "" );
  QMapIterator< int, QString > ait(kAlphaFixed);
  while ( true == ait.hasNext() ) {
    ait.next();
    if ( ait.value() == atext ) {
      alphaedit_->setPercent( ait.key() );
      slotSetAlphaCustom();
      return;
    }
  }
}

int LayerMenu::getNextLevel( int lvl ) const
{
  if( false == kLevels.contains(lvl) ){
    return -1;
  }
  int idx = kLevels.indexOf(lvl);
  if( idx+1 >= kLevels.size() ){
    return -1;
  }
  return kLevels[idx+1];
}

int LayerMenu::getPrevLevel( int lvl ) const
{
  if( false == kLevels.contains(lvl) ){
    return -1;
  }
  int idx = kLevels.indexOf(lvl);
  if( 0 >= idx ){
    return -1;
  }
  return kLevels[idx-1];
}

void LayerMenu::showLayer(const proto::WeatherLayer& info ) const
{
  if ( 0 == layer_ ) {
    return;
  }
  Layer* l = findLayer(info);
  if ( 0 == l ){
    l = createLayer(info);
  }
  if ( 0 == l ) {
    QMessageBox::warning( 0, QObject::tr("Внимание"), QObject::tr("Нет данных для создания слоя"), QObject::tr("Закрыть"));
    return;
  }
  l->setVisisble(true);
  layer_->setVisisble(false);
}

Layer* LayerMenu::findLayer( const proto::WeatherLayer& info ) const
{
  if ( 0 == layer_ || 0 == layer_->document() ) {
    return 0;
  }
  Document* d = layer_->document();
  QList<Layer*> list = d->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if( true == l->isEqual(info) ) {
      return l;
    }
  }
  return 0;
}

Layer* LayerMenu::createLayer( const proto::WeatherLayer& info ) const
{
  if ( 0 == layer_ || 0 == layer_->document() ) {
    return 0;
  }
  int spline_koef = 20;
  if ( 0 != layer_->field() ) {
    spline_koef = layer_->field()->getSmootch();
  }
  Weather weather;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Layer* l = weather.buildLayer( layer_->document(), info, spline_koef );
  QApplication::restoreOverrideCursor();
  return l;
}

void LayerMenu::slotLevelUp()
{
  if ( 0 == layer_ ) {
    return;
  }
  proto::WeatherLayer info = layer_->info();
  int lvl = getNextLevel( info.level() );
  if ( 0 > lvl ) {
    return;
  }
  info.set_level(lvl);
  showLayer(info);
}

void LayerMenu::slotLevelDown()
{
  if ( 0 == layer_ ) {
    return;
  }
  proto::WeatherLayer info = layer_->info();
  int lvl = getPrevLevel( info.level() );
  if ( 0 > lvl ) {
    return;
  }
  info.set_level(lvl);
  showLayer(info);
}

void LayerMenu::slotSwitchHour()
{
  if ( 0 == layer_ ) {
    return;
  }
  proto::WeatherLayer info = layer_->info();
  QAction* a = qobject_cast<QAction*>(sender());
  if ( 0 == a ) {
    return;
  }
  int h = a->data().toInt();
  QDateTime dt = PsqlQuery::datetimeFromString( info.datetime() );
  dt = dt.addSecs(3600*h);
  std::string date_time = dt.toString(Qt::ISODate).toStdString();
  info.set_datetime( dt.toString(Qt::ISODate).toStdString() );
  showLayer(info);
}

void LayerMenu::slotRename()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerRename* dlg = new LayerRename( layer_->name() );
  int res = dlg->exec();
  QString newname = dlg->name();
  delete dlg;
  if ( QDialog::Accepted == res ) {
    layer_->setName(newname);
  }
}

static const QList< float > kStepFixed = QList< float >()
  <<  0.25 << 0.5 << 1 << 2 << 3  << 4 << 5 << 8 << 10 << 16;

IsoMenu::IsoMenu( Layer* l, QObject* p )
  : LayerMenu( l, p ),
  stepedit_(0),
  minedit_(0),
  maxedit_(0),
  actionmin_(0),
  actionmax_(0),
  actionminval_(0),
  actionmaxval_(0),
  actionwidth_(0),
  actionstyle_(0),
  actionstep_(0),
  actionfill_(0),
  actionlevelup_(0),
  actionleveldown_(0),
  actionhour_(0),
  menuwidth_(0),
  menustyle_(0),
  menustep_(0),
  menuminval_(0),
  menumaxval_(0),
  menuhour_(0)
{
  QPixmap pix(20,20);
  pix.fill(Qt::black);
  actionmin_ = new QAction( QIcon(pix), QObject::tr("Мин. цвет"), this );
  QObject::connect( actionmin_, SIGNAL( triggered() ), this, SLOT( slotColorMinChanged() ) );
  actionmax_ = new QAction( QIcon(pix), QObject::tr("Макс. цвет"), this );
  QObject::connect( actionmax_, SIGNAL( triggered() ), this, SLOT( slotColorMaxChanged() ) );

  menuwidth_ = new QMenu( QObject::tr("Толщина линии") );
  actionwidth_ = menuwidth_->menuAction();
  for ( int i = 1; i < 7; ++i ) {
    QLabel* l = new QLabel;    
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    l->setFrameShape(QFrame::StyledPanel);
    l->setFrameShadow(QFrame::Sunken);

    QPixmap pix = createPixmap(Qt::SolidLine, i, false, QString::number(i));
    QWidgetAction* wa = new QWidgetAction(menuwidth_);
    l->setPixmap(pix);
    wa->setDefaultWidget(l);
    menuwidth_->addAction(wa);
    QObject::connect( wa, SIGNAL( triggered() ), this, SLOT( slotWidthChanged() ) );
    wa->setData(i);
  }

  menustyle_ = new QMenu( QObject::tr("Стиль линии") );
  actionstyle_ = menustyle_->menuAction();

  for ( int i = Qt::SolidLine; i < Qt::CustomDashLine; ++i ) {
    QLabel* l = new QLabel;
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    l->setFrameShape(QFrame::StyledPanel);
    l->setFrameShadow(QFrame::Sunken);

    QPixmap pixmap = createPixmap(Qt::PenStyle(i), 2, false);
    QWidgetAction* wa = new QWidgetAction(menustyle_);
    l->setPixmap(pixmap);
    wa->setDefaultWidget(l);
    menustyle_->addAction(wa);
    QObject::connect( wa, SIGNAL( triggered() ), this, SLOT( slotStyleChanged() ) );
    wa->setData(i);
  }

  //
  menuminval_ = new QMenu( QObject::tr("Мин") );
  actionminval_ = menuminval_->menuAction();
  
  QWidgetAction* mincust = new QWidgetAction(menuminval_);
  QObject::connect( mincust, SIGNAL( triggered() ), this, SLOT( slotMinValueChanged() ) );
  minedit_ = new DoubleSpinAction( menuminval_, mincust );
  minedit_->setMinimum(-1000000);
  minedit_->setMaximum(1000000);
  minedit_->setSingleStep(5);
  minedit_->setDecimals(2);
  mincust->setDefaultWidget(minedit_);
  menuminval_->addAction(mincust);

  menumaxval_ = new QMenu( QObject::tr("Макс") );
  actionmaxval_ = menumaxval_->menuAction();
  
  QWidgetAction* maxcust = new QWidgetAction(menumaxval_);
  QObject::connect( maxcust, SIGNAL( triggered() ), this, SLOT( slotMaxValueChanged() ) );
  maxedit_ = new DoubleSpinAction( menumaxval_, maxcust );
  maxedit_->setMinimum(-1000000);
  maxedit_->setMaximum(1000000);
  maxedit_->setSingleStep(5);
  maxedit_->setDecimals(2);
  maxcust->setDefaultWidget(maxedit_);
  menumaxval_->addAction(maxcust);
  //

  menustep_ = new QMenu( QObject::tr("Шаг") );
  actionstep_ = menustep_->menuAction();

  for ( int i = 0; i < kStepFixed.size(); ++i ) {
    QAction* a = menustep_->addAction(QString::number(kStepFixed.at(i)));
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotStepChanged() ) );
  }
  QWidgetAction* stepcust = new QWidgetAction(menustep_);
  QObject::connect( stepcust, SIGNAL( triggered() ), this, SLOT( slotStepCustomChanged() ) );
  stepedit_ = new DoubleSpinAction( menustep_, stepcust );
  stepedit_->setMinimum(0.1);
  stepedit_->setMaximum(500000);
  stepedit_->setDecimals(2);
  stepcust->setDefaultWidget(stepedit_);
  menustep_->addAction(stepcust);

  actionfill_= new QAction( QIcon(":/meteo/icons/tools/fill.xpm"), QObject::tr("Заливка"), this );
  QObject::connect( actionfill_, SIGNAL( triggered() ), this, SLOT( slotColorFillDialog() ) );

  actionlevelup_ = new QAction( QIcon(":/meteo/icons/tools/layer_up.png"), QObject::tr("Показать на уровень выше"), this );
  QObject::connect( actionlevelup_, SIGNAL( triggered() ), this, SLOT( slotLevelUp() ) );
  actionleveldown_ = new QAction(QIcon(":/meteo/icons/tools/layer_down.png"), QObject::tr("Показать на уровень ниже"), this );
  QObject::connect( actionleveldown_, SIGNAL( triggered() ), this, SLOT( slotLevelDown() ) );

  menuhour_ = new QMenu( QObject::tr("Показать за другой срок") );
  actionhour_ = menuhour_->menuAction();
  actionhour_->setIcon( QIcon(":/novost/icons/clock.png") );
  QList<int> hours;
  hours << -24 << -18 << -12 << -6 << -1 << 1 << 6 << 12 << 18 << 24;
  for ( int i = 0, sz = hours.size(); i < sz; ++i ) {
    QString text = QObject::tr("%1 ч.");
    if( 0 < hours[i] ){
      text.insert(0, "+");
    }
    QAction* a = menuhour_->addAction( text.arg( hours[i] ) );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotSwitchHour() ) );
    a->setData( hours[i] );
  }
}

IsoMenu::~IsoMenu()
{
  delete menuwidth_; menuwidth_ = 0;
  delete menustyle_; menustyle_ = 0;
  delete menustep_; menustep_ = 0;
  delete menuhour_; menuhour_ = 0;
}

void IsoMenu::addActions( QMenu* menu )
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 != iso ) {
    QPixmap pix(20,20);
    pix.fill( iso->minColor() );
    actionmin_->setIcon( QIcon(pix) );
    pix.fill( iso->maxColor() );
    actionmax_->setIcon( QIcon(pix) );
    stepedit_->setValue( iso->stepIso() );
    minedit_->setValue( iso->minValue() );
    maxedit_->setValue( iso->maxValue() );
    changeWidthPixmap(menuwidth_, iso->pen().width());
    changeStylePixmap(menustyle_, iso->pen().style());
  }

  menu->addAction(actionlevelup_);
  menu->addAction(actionleveldown_);
  menu->addAction(actionhour_);
  menu->addAction(actionwidth_);
  menu->addAction(actionstyle_);
  menu->addAction(actionminval_);
  menu->addAction(actionmaxval_);
  menu->addAction(actionstep_);
  menu->addAction(actionmin_);
  menu->addAction(actionmax_);
  menu->addAction(actionfill_);
  menu->addSeparator();
  QMenu* m = menu->addMenu("Еще");
  LayerMenu::addActions(m);
}

void IsoMenu::slotColorMinChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  QColorDialog* dlg = new QColorDialog;
  dlg->setCurrentColor(iso->minColor());
  if ( QDialog::Accepted == dlg->exec() ) {
    QPixmap pixMin(20,20);
    pixMin.fill(dlg->currentColor());
    actionmin_->setIcon(QIcon(pixMin));
    actionmax_->setIcon(QIcon(pixMin));
    iso->setColorMin(dlg->currentColor());
    iso->setColorMax(dlg->currentColor());
    iso->repaint();
  }
  delete dlg;
}

void IsoMenu::slotColorMaxChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  QColorDialog* dlg = new QColorDialog();
  dlg->setCurrentColor(iso->maxColor());
  int res = dlg->exec();
  QColor clr = dlg->currentColor();
  delete dlg;
  if(  QDialog::Accepted != res ){
    return;
  }
  QPixmap pixMax(20,20);
  pixMax.fill(clr);
  actionmax_->setIcon(QIcon(pixMax));
  iso->setColorMax(clr);
  iso->repaint();
}

void IsoMenu::slotWidthChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  QWidgetAction* act = qobject_cast<QWidgetAction*>(sender());
  if( 0 == act ){
    return;
  }

  QPen pen = iso->pen();
  pen.setWidth(act->data().toInt());
  iso->setPen(pen);
  iso->repaint();

  changeWidthPixmap(menuwidth_, pen.width());
}

void IsoMenu::slotStyleChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  QWidgetAction* act = qobject_cast<QWidgetAction*>(sender());
  if( 0 == act ){
    return;
  }

  QPen pen = iso->pen();
  pen.setStyle((Qt::PenStyle)act->data().toInt());
  iso->setPen(pen);
  iso->repaint();

  changeStylePixmap(menustyle_, pen.style());
}

void IsoMenu::slotColorFillDialog()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }

  TColorGradList gradlist = iso->gradient();

  GradientEditor* dlg = new GradientEditor(gradlist);

  dlg->show();
  if (dlg->exec() == QDialog::Accepted) {
    gradlist = dlg->gradient();
    iso->setColor(gradlist);
    if ( false == iso->hasGradient() ) {
      iso->addGradient();
    }
    iso->repaint();
  }

  delete dlg;
}

void IsoMenu::slotStepChanged()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if( 0 == act ){
    return;
  }
  if ( 0 == stepedit_ ) {
    return;
  }

  QString actext = act->text();
  actext.replace("&","");
  stepedit_->setValue(actext.toDouble());
  slotStepCustomChanged();
}

void IsoMenu::slotStepCustomChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  if ( 0 == stepedit_ ) {
    return;
  }
  if ( false == MnMath::isEqual(static_cast<float>(stepedit_->value()), iso->stepIso())) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    iso->removeIsolines();
    iso->setStepIso(stepedit_->value());
    iso->addIsoLines();
    iso->addExtremums();
    iso->repaint();
    qApp->restoreOverrideCursor();
  }
}

void IsoMenu::slotMinValueChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  if ( 0 == minedit_ ) {
    return;
  }
  if ( false == MnMath::isEqual(static_cast<float>(minedit_->value()), iso->minValue())) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    iso->removeIsolines();
    iso->setMin(minedit_->value());
    iso->addIsoLines();
    iso->addExtremums();
    iso->repaint();
    qApp->restoreOverrideCursor();
  }
}

void IsoMenu::slotMaxValueChanged()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerIso* iso = maplayer_cast<LayerIso*>(layer_);
  if ( 0 == iso ) {
    return;
  }
  if ( 0 == maxedit_ ) {
    return;
  }
  //debug_log << maxedit_->value() << iso->maxValue();
  if ( false == MnMath::isEqual(static_cast<float>(maxedit_->value()), iso->maxValue())) {
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    iso->removeIsolines();
    iso->setMax(maxedit_->value());
    iso->addIsoLines();
    iso->addExtremums();
    iso->repaint();
    qApp->restoreOverrideCursor();
  }
}

QPixmap IsoMenu::createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text /* = QString()*/)
{
  QFont font;
  QFontMetrics fm(font);
  int textWidth = fm.width("6");
  if (text.isNull()) textWidth = 0;

  QPixmap pixmap(80 + textWidth, 20);
  QColor color = Qt::white;
  if (isSelected) {
    //    color = QColor(175, 215, 240);
    color = QColor(200, 233, 252);
  }
  pixmap.fill(color);
  
  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  QPen pen(brush, width, style);

  QPainter pntr(&pixmap);
  pntr.setBrush(brush);
  pntr.setPen(pen);
  pntr.drawLine(10, 10, 70, 10);
  if (!text.isNull()) {
    pntr.drawText(QRect(75, 5, textWidth, 10), Qt::AlignCenter, text);
  }

  return pixmap;
}

void IsoMenu::changeStylePixmap(QMenu* parent, int val)
{
  if( 0 == parent) {
    return;
  }

  QList<QAction*> allact = parent->actions();
  for (int idx = 0; idx < allact.size(); idx++) {
    QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
    if( 0 != wa ){
      QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
      if (0 != w) {
	if (wa->data().toInt() == val) {
	  w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, true));
	} else {
	  w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, false));
	}
      }
    }
  }
}

void IsoMenu::changeWidthPixmap(QMenu* parent, int val)
{ 
  if( 0 == parent) {
    return;
  }
  
  QList<QAction*> allact = parent->actions();
  for (int idx = 0; idx < allact.size(); idx++) {
    QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
    if( 0 != wa ){
      QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
      if (0 != w) {
       	if (wa->data().toInt() == val) {
	  w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), true, wa->data().toString()));
	} else {
	  w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), false, wa->data().toString()));
	}
      }
    }
  }
}

PunchMenu::PunchMenu( Layer* l, QObject* p )
  : LayerMenu( l, p ),
  actionlevelup_(0),
  actionleveldown_(0),
  actioncolor_(0),
  actionsize_(0),
  actionhour_(0),
  menucolor_(0),
  menusize_(0),
  menuhour_(0)
{

  actionlevelup_ = new QAction( QIcon(":/meteo/icons/tools/layer_up.png"), QObject::tr("Показать на уровень выше"), this );
  QObject::connect( actionlevelup_, SIGNAL( triggered() ), this, SLOT( slotLevelUp() ) );
  actionleveldown_ = new QAction(QIcon(":/meteo/icons/tools/layer_down.png"), QObject::tr("Показать на уровень ниже"), this );
  QObject::connect( actionleveldown_, SIGNAL( triggered() ), this, SLOT( slotLevelDown() ) );

  menucolor_ = new QMenu( QObject::tr("Цвет наноски") );
  actioncolor_ = menucolor_->menuAction();
  QAction* a = menucolor_->addAction( QObject::tr("Выбрать") );
  QPixmap pix(20,20);
  pix.fill( Qt::black );
  a->setIcon(QIcon(pix));
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchColor() ) );
  a = menucolor_->addAction( QObject::tr("Исходный") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotClearPunchColor() ) );

  menusize_ = new QMenu( QObject::tr("Размер наноски") );
  actionsize_ = menusize_->menuAction();
  a = menusize_->addAction( QObject::tr("50%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("80%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("90%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("100%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("110%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("120%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("150%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("200%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );
  a = menusize_->addAction( QObject::tr("300%") );
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotChoosePunchScale() ) );


  menuhour_ = new QMenu( QObject::tr("Показать за другой срок") );
  actionhour_ = menuhour_->menuAction();
  actionhour_->setIcon( QIcon(":/novost/icons/clock.png") );
  QList<int> hours;
  hours << -1 << -6 << -12 << -18 << -24 << 1 << 6 << 12 << 18 << 24;
  for ( int i = 0, sz = hours.size(); i < sz; ++i ) {
    QString text = QObject::tr("%1 ч.");
    if( 0 < hours[i] ){
      text.insert(0, "+");
    }
    QAction* a = menuhour_->addAction( text.arg( hours[i] ) );
    connect( a, SIGNAL( triggered() ), this, SLOT( slotSwitchHour() ) );
    a->setData( hours[i] );
  }
}

PunchMenu::~PunchMenu()
{
  delete menuhour_; menuhour_ = 0;
  delete menusize_; menusize_ = 0;
  delete menucolor_; menucolor_ = 0;
}

void PunchMenu::addActions( QMenu* menu )
{
  if ( 0 == layer_ ) {
    return;
  }
  menu->addAction(actionlevelup_);
  menu->addAction(actionleveldown_);
  menu->addAction(actionhour_);
  menu->addAction(actioncolor_);
  menu->addAction(actionsize_);
  menu->addSeparator();
  QMenu* m = menu->addMenu("Еще");
  LayerMenu::addActions(m);
}

void PunchMenu::slotChoosePunchColor()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerPunch* punch = maplayer_cast<LayerPunch*>(layer_);
  if ( 0 == punch ) {
    return;
  }
  QAction* a = qobject_cast<QAction*>( sender() );
  QColorDialog* dlg = new QColorDialog;
  if ( QDialog::Accepted == dlg->exec() ) {
    QPixmap pixMin(20,20);
    pixMin.fill(dlg->currentColor());
    a->setIcon(QIcon(pixMin));
    puanson::proto::Puanson p = punch->punch();
    p.set_color( dlg->currentColor().rgba() );
    punch->setPunch(p);
  }
  delete dlg;
}

void PunchMenu::slotClearPunchColor()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerPunch* punch = maplayer_cast<LayerPunch*>(layer_);
  if ( 0 == punch ) {
    return;
  }
  puanson::proto::Puanson p = punch->punch();
  p.clear_color();
  punch->setPunch(p);
}

void PunchMenu::slotChoosePunchScale()
{
  if ( 0 == layer_ ) {
    return;
  }
  LayerPunch* punch = maplayer_cast<LayerPunch*>(layer_);
  if ( 0 == punch ) {
    return;
  }
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( 0 == a ) {
    return;
  }
  QString txt = a->text().replace("%","");
  txt.replace( "&", "" );
  float scale = txt.toInt()/100.0;
  puanson::proto::Puanson p = punch->punch();
  if ( true == MnMath::isEqual( 100.0f, scale ) ) {
    p.clear_scale();
  }
  else {
    p.set_scale(scale);
  }
  punch->setPunch(p);
}

AirportMenu::AirportMenu( Layer* l, QObject* parent ) : PunchMenu ( l, parent ),
  changeLabel_(new QAction( QObject::tr("Отображать названия"), nullptr ))
{
  LayerAirport* punch = maplayer_cast<LayerAirport*>(layer_);
  if ( punch != nullptr ) {
    if ( punch->showCCCC() ) {
      changeLabel_->setText(QObject::tr("Отображать названия"));
    }
    else {
      changeLabel_->setText(QObject::tr("Отображать сссс"));
    }
  }
  QObject::connect( changeLabel_, SIGNAL( triggered() ), this, SLOT( slotChangeLabel() ) );
}

AirportMenu::~AirportMenu()
{
  delete changeLabel_;
  delete menu_;
  changeLabel_ = nullptr;
  menu_ = nullptr;
}

void AirportMenu::slotChangeLabel()
{
  if ( nullptr == layer_ ) {
    return;
  }
  LayerAirport* punch = maplayer_cast<LayerAirport*>(layer_);
  if ( nullptr == punch ) {
    return;
  }
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( nullptr == a ) {
    return;
  }
  punch->changeLabel();
  if ( punch->showCCCC() ) {
    a->setText(QObject::tr("Отображать названия"));
  }
  else {
    a->setText(QObject::tr("Отображать сссс"));
  }
}

void AirportMenu::addActions(QMenu *menu)
{
  if ( nullptr == layer_ ) {
    return;
  }
  menu->addAction(changeLabel_);
  PunchMenu::addActions(menu);
}

BorderMenu::BorderMenu( Layer* l, QObject* parent  ) : LayerMenu ( l, parent ),
  changeColor_(new QAction( QObject::tr("Изменить цвет"), nullptr )),
  width_(new QSpinBox()),
  style_(new QComboBox())
{
  QMenu* widthMenu = new QMenu(QObject::tr("Изменить толщину"));
  QWidgetAction* widthAct = new QWidgetAction(widthMenu);
  widthAct->setDefaultWidget(width_);
  width_->setRange(1, 20);
  widthMenu->addAction(widthAct);
  changeWidth_ = widthMenu->menuAction();
  QMenu* styleMenu = new QMenu(QObject::tr("Изменить стиль"));
  QWidgetAction* styleAct = new QWidgetAction(styleMenu);
  styleAct->setDefaultWidget(style_);
  styleMenu->addAction(styleAct);
  changeStyle_ = styleMenu->menuAction();
  fillLineCombo();
  loadProperty();
  QObject::connect( changeColor_, SIGNAL( triggered() ), SLOT( slotChangeColor() ) );
  QObject::connect( width_, SIGNAL(valueChanged(int)), SLOT(slotChangeWidth(int)));
  QObject::connect( style_, SIGNAL(currentIndexChanged(int)), SLOT(slotChangeStyle()) );
}

BorderMenu::~BorderMenu()
{
  delete changeColor_;
  delete changeWidth_;
  delete changeStyle_;
  delete menu_;
  delete width_;
  delete style_;
  changeColor_ = nullptr;
  changeWidth_ = nullptr;
  changeStyle_ = nullptr;
  menu_ = nullptr;
  width_ = nullptr;
  style_ = nullptr;
}

void BorderMenu::addActions( QMenu* menu ) {
  if ( nullptr == layer_ ) {
    return;
  }
  menu->addAction(changeColor_);
  menu->addAction(changeWidth_);
  menu->addAction(changeStyle_);
  LayerMenu::addActions(menu);
}

void BorderMenu::slotChangeColor()
{
  if ( nullptr == layer_ ) {
    return;
  }
  LayerBorder* border = maplayer_cast<LayerBorder*>(layer_);
  if ( nullptr == border ) {
    return;
  }
    QColorDialog* dlg = new QColorDialog;
    dlg->setCurrentColor(Qt::black);
    if ( QDialog::Accepted == dlg->exec() ) {
      border->changeColor(dlg->currentColor());
    }
    delete dlg;
}

void BorderMenu::slotChangeWidth(int value)
{
  if ( nullptr == layer_ ) {
    return;
  }
  LayerBorder* border = maplayer_cast<LayerBorder*>(layer_);
  if ( nullptr == border ) {
    return;
  }
  border->changeWidth(value);
}

void BorderMenu::slotChangeStyle()
{
  if ( nullptr == layer_ ) {
    return;
  }
  LayerBorder* border = maplayer_cast<LayerBorder*>(layer_);
  if ( nullptr == border ) {
    return;
  }
  border->changeStyle(lineStyle());
}

void BorderMenu::fillLineCombo()
{
  if ( nullptr == style_ ) {
    return;
  }
  Qt::PenStyle penStyle;
  for (auto type : kLineTypes) {
    penStyle = Qt::NoPen;
    if ( kSolidLine == type ) {
      penStyle = Qt::SolidLine;
    }
    else if ( kDashLine == type ) {
      penStyle = Qt::DashLine;
    }
    else if ( kDotLine== type ) {
      penStyle = Qt::DotLine;
    }
    else if ( kDashDotLine == type ) {
      penStyle = Qt::DashDotLine;
    }
    else if ( kDashDotDotLine == type ) {
      penStyle = Qt::DashDotDotLine;
    }
    else {
      error_log << QObject::tr("Неопределённый тип линии");
      style_->addItem(type);
    }
    if ( Qt::NoPen == penStyle ) {
      continue;
    }
    QPixmap icon(40, 10);
    icon.fill(Qt::white);
    QPainter painter;
    painter.begin(&icon);
    QPen pen(Qt::black);
    pen.setStyle(penStyle);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0,5,40,5);
    painter.end();
    style_->setIconSize(QSize(40,10));
    style_->addItem(QIcon(icon)," ");
    int index = style_->count() - 1;
    style_->setItemData( index, type, Qt::ToolTipRole  );
  }
}

Qt::PenStyle BorderMenu::lineStyle() const
{
  int curIdx = style_->currentIndex();
  if ( kLineTypes.indexOf(kSolidLine) == curIdx ) {
    return Qt::SolidLine;
  }
  else if ( kLineTypes.indexOf(kDashLine) == curIdx ) {
    return Qt::DashLine;
  }
  else if ( kLineTypes.indexOf(kDblSolidLine) == curIdx ) {
    return Qt::SolidLine;
  }
  else if ( kLineTypes.indexOf(kDblDashLine) == curIdx ) {
    return Qt::DashLine;
  }
  else if ( kLineTypes.indexOf(kDotLine) == curIdx ) {
    return Qt::DotLine;
  }
  else if ( kLineTypes.indexOf(kDashDotLine) == curIdx ) {
    return Qt::DashDotLine;
  }
  else if ( kLineTypes.indexOf(kDashDotDotLine) == curIdx ) {
    return Qt::DashDotDotLine;
  }

  error_log << tr("Не удалось определить тип линии");
  return Qt::NoPen;
}

void BorderMenu::loadProperty()
{
  proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
  doc.border_property();
  width_->setValue(doc.border_property().pen().width());
  QPen pen = pen2qpen(doc.border_property().pen());
  QBrush brush = brush2qbrush(doc.border_property().brush());
  int idx = kLineTypes.indexOf(kSolidLine);
  if ( pen.style() == Qt::SolidLine ) {
    idx = ( true == doc.border_property().double_line() ) ? kLineTypes.indexOf(kDblSolidLine) : kLineTypes.indexOf(kSolidLine);
  }
  else if ( pen.style() == Qt::DashLine ) {
    idx = ( true == doc.border_property().double_line() ) ? kLineTypes.indexOf(kDblDashLine) : kLineTypes.indexOf(kDashLine);
  }
  else if ( pen.style() == Qt::DotLine ) {
    idx = kLineTypes.indexOf(kDotLine);
  }
  else if ( pen.style() == Qt::DashDotLine ) {
    idx = kLineTypes.indexOf(kDashDotLine);
  }
  else if ( pen.style() == Qt::DashDotDotLine ) {
    idx = kLineTypes.indexOf(kDashDotDotLine);
  }
  style_->setCurrentIndex(idx);
}

}
}
