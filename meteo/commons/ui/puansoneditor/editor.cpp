#include "editor.h"

#include <stdlib.h>

#include <qdialog.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qgraphicsscene.h>
#include <qgraphicsitem.h>
#include <qlist.h>
#include <qevent.h>
#include <qbytearray.h>
#include <qdatastream.h>
#include <qcursor.h>
#include <qpair.h>
#include <qpalette.h>
#include <qvalidator.h>
#include <qpainter.h>
#include <qmenu.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/custom/alignbutton.h>
#include <meteo/commons/ui/custom/textposdlg.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/custom/descrselect.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "dlgspinbox.h"

#include "ui_puanson.h"
#include "punchselect.h"

#include "numformat.h"
#include "symbolformat.h"

#include "puansonitem.h"
#include "gv.h"
#include "addsymbol.h"

namespace meteo {
namespace puanson {

const int kColumnVisible        (0);
const int kColumnType           (1);
const int kColumnDescr          (2);
const int kColumnTestval        (3);
const int kColumnAlign          (4);
const int kColumnDrawDD         (5);

const QString kLastParamsPath( QDir::homePath() + QString("/.meteo/puansoneditor") );

QChar symbolForType( proto::CellType type )
{
  switch ( type ) {
    case proto::kSymbol:
      return QChar(0x2614);
      break;
    case proto::kNumber:
      return QChar(0x2116);
      break;
    case proto::kSymNumber:
      return QChar(0x222b);
      break;
  }
  return QChar(0x2116);
}

QString tooltipForType( proto::CellType type )
{
  switch (type) {
    case proto::kNumber:
      return QObject::tr("Численное отображение");
      break;
    case proto::kSymbol:
      return QObject::tr("Символьное отображение");
      break;
    case proto::kSymNumber:
      return QObject::tr("Символьное и численное отображение");
      break;
  }
  return QObject::tr("Численное отображение");
}

QFont fontForType( proto::CellType type )
{
  Q_UNUSED(type);
  QFont fnt;
  fnt.setPointSize(16);
  return fnt;
}

Editor::Editor( QWidget* p, Qt::WindowFlags fl )
  : QWidget( p,fl ),
  ui_( new Ui::PuansonEdit ),
  puanson_(0),
  item_(0),
  recttype_(kNoRect),
  anchortype_(kNoAnchor),
  grid_( new GridItem )
{
  meteo::internal::WeatherFont* wf = meteo::WeatherFont::instance();
  if ( 0 == wf ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку шрифтов");
  }
  ui_->setupUi(this);
  QWidget* oldview = ui_->signview;

  ui_->gridLayout_2->removeWidget(oldview);
  Gv* gv = new Gv(ui_->signbox);

  ui_->signview = gv;
  ui_->signview->setMinimumSize(QSize(400, 400));
  ui_->gridLayout_2->addWidget(gv, 1, 0, 1, 4);
//  gv->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
//  gv->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  delete oldview;

  QGraphicsScene* scene = new QGraphicsScene(ui_->signview);
  scene->addItem(grid_);
  ui_->signview->setScene(scene);
  scene->setItemIndexMethod( QGraphicsScene::NoIndex );
//  ui_->signview->setResizeAnchor( QGraphicsView::AnchorViewCenter );
  ui_->signview->scale( 1.5/*ui_->signview->viewport()->width()*/, 1.5/*ui_->signview->viewport()->height()*/ );
  ui_->signview->setRenderHint( QPainter::HighQualityAntialiasing, true );
  ui_->signview->setRenderHint( QPainter::TextAntialiasing, true );
  ui_->descrtree->setIconSize( QSize(32,32) );

  ui_->btnsetuprect
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-withoutbound.png"), QObject::tr("Без рамок") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-innerbound.png"), QObject::tr("Рамка вокруг элементов пуансона") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-outbound.png"), QObject::tr("Рамка вокруг пуансона") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-bothbound.png"), QObject::tr("Рамки вокруг пуансона и его элементов") ) );

  ui_->btnsetupanchor
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-withoutcenterandanchor.png"), QObject::tr("Без якорей") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-anchor.png"), QObject::tr("Якоря у элементов пуансона") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-center.png"), QObject::tr("Якорь в центре пуансона") ) )
    ->addIco( qMakePair( QIcon(":/meteo/icons/puanson/thunderstorm-centerandanchor.png"), QObject::tr("Якоря в центре пуансона и у элементов") ) );

  disableEdit();

  ui_->fontwidget->setSpecFontEnabled(false);

  muteSignals();
  turnSignals();

  slotDrawWindClicked( ui_->haswind->isChecked() );
  slotDrawFlowClicked( ui_->hasflow->isChecked() );
  slotBtnGridClicked();

  if ( false == loadLastParams() ) {
    slotSelectPuanson();
  }
  updateScene();

  ui_->descrtree->setCurrentItem(0);
}

Editor::~Editor()
{
  delete ui_; ui_ = 0;
  delete puanson_; puanson_ = 0;
}

void Editor::muteSignals()
{
  QObject::disconnect( ui_->btnopenpunch, SIGNAL( clicked() ), this, SLOT( slotSelectPuanson() ) );
  QObject::disconnect( ui_->btnsavepunch, SIGNAL( clicked() ), this, SLOT( slotSavePuanson() ) );

  QObject::disconnect( ui_->haswind, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawWindClicked( bool ) ) );
  QObject::disconnect( ui_->drawdf, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawDFClicked( bool ) ) );

  QObject::disconnect( ui_->hasflow, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawFlowClicked( bool ) ) );

  QObject::disconnect( ui_->btnaddsign, SIGNAL( clicked() ), this, SLOT( slotAddSign() ) );
  QObject::disconnect( ui_->btnrmsign, SIGNAL( clicked() ), this, SLOT( slotRmSign() ) );

  QObject::disconnect( ui_->descrtree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemDoubleClicked( QTreeWidgetItem*, int ) ) );

  QObject::disconnect( ui_->descrtree, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemClicked( QTreeWidgetItem*, int ) ) );

  QObject::disconnect( ui_->descrtree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemChanged( QTreeWidgetItem*, int ) ) );

  QObject::disconnect( ui_->btnsetuprect, SIGNAL( indexChanged( int ) ), this, SLOT( slotBtnRectIndexChanged( int ) ) );
  QObject::disconnect( ui_->btnsetupanchor, SIGNAL( indexChanged( int ) ), this, SLOT( slotBtnAnchorIndexChanged( int ) ) );
  QObject::disconnect( ui_->btngrid, SIGNAL( clicked() ), this, SLOT( slotBtnGridClicked() ) );

  QObject::disconnect( ui_->centersignbox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCenterIndexChanged( int ) ) );
  QObject::disconnect( ui_->radiusspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotRadiusSpinValueChanged( int ) ) );

  QObject::disconnect( ui_->crisscrossbox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCrissCrossIndexChanged( int ) ) );
  QObject::disconnect( ui_->cmbtype, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotTypeChanged( int ) ) );

  QObject::disconnect( ui_->ddspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotDDCanged( int ) ) );
  QObject::disconnect( ui_->ffspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotFFCanged( int ) ) );
  QObject::disconnect( ui_->ffqualcmb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotFFQualCanged( int ) ) );
  QObject::disconnect( ui_->ddqualcmb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotDDQualCanged( int ) ) );
  QObject::disconnect( ui_->fontwidget, SIGNAL( fontChanged( const QFont& ) ), this, SLOT( slotWindFontChanged( const QFont& ) ) );

  QObject::disconnect( ui_->descrtree, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
      this, SLOT( slotSelectedDescriptorChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  QObject::disconnect( ui_->spinx, SIGNAL( valueChanged( int ) ), this, SLOT( slotXchanged( int ) ) );
  QObject::disconnect( ui_->spiny, SIGNAL( valueChanged( int ) ), this, SLOT( slotYChanged( int ) ) );

  QObject::disconnect( ui_->valuespin, SIGNAL( valueChanged( double ) ), this, SLOT( slotValueChanged( double ) ) );
  QObject::disconnect( ui_->alignbtn, SIGNAL( posChanged( int ) ), this, SLOT( slotAlignChanged( int ) ) );
  QObject::disconnect( ui_->cmbyplusdf, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotYPlusDfIndexChanged( int ) ) );
  QObject::disconnect( ui_->lblabsent, SIGNAL( doubleclick() ), this, SLOT( slotAbsentLabelClicked() ) );
  QObject::disconnect( ui_->btnabsentclear, SIGNAL( clicked() ), this, SLOT( slotAbsentClear() ) );
}

void Editor::turnSignals()
{

  QObject::connect( ui_->btnopenpunch, SIGNAL( clicked() ), this, SLOT( slotSelectPuanson() ) );
  QObject::connect( ui_->btnsavepunch, SIGNAL( clicked() ), this, SLOT( slotSavePuanson() ) );

  QObject::connect( ui_->haswind, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawWindClicked( bool ) ) );
  QObject::connect( ui_->drawdf, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawDFClicked( bool ) ) );

  QObject::connect( ui_->hasflow, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawFlowClicked( bool ) ) );

  QObject::connect( ui_->btnaddsign, SIGNAL( clicked() ), this, SLOT( slotAddSign() ) );
  QObject::connect( ui_->btnrmsign, SIGNAL( clicked() ), this, SLOT( slotRmSign() ) );

  QObject::connect( ui_->descrtree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemDoubleClicked( QTreeWidgetItem*, int ) ) );

  QObject::connect( ui_->descrtree, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemClicked( QTreeWidgetItem*, int ) ) );

  QObject::connect( ui_->descrtree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
      this, SLOT( slotCellItemChanged( QTreeWidgetItem*, int ) ) );

  QObject::connect( ui_->btnsetuprect, SIGNAL( indexChanged( int ) ), this, SLOT( slotBtnRectIndexChanged( int ) ) );
  QObject::connect( ui_->btnsetupanchor, SIGNAL( indexChanged( int ) ), this, SLOT( slotBtnAnchorIndexChanged( int ) ) );
  QObject::connect( ui_->btngrid, SIGNAL( clicked() ), this, SLOT( slotBtnGridClicked() ) );

  QObject::connect( ui_->centersignbox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCenterIndexChanged( int ) ) );
  QObject::connect( ui_->radiusspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotRadiusSpinValueChanged( int ) ) );

  QObject::connect( ui_->crisscrossbox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCrissCrossIndexChanged( int ) ) );
  QObject::connect( ui_->cmbtype, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotTypeChanged( int ) ) );

  QObject::connect( ui_->ddspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotDDCanged( int ) ) );
  QObject::connect( ui_->ffspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotFFCanged( int ) ) );

  QObject::connect( ui_->ffqualcmb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotFFQualCanged( int ) ) );
  QObject::connect( ui_->ddqualcmb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotDDQualCanged( int ) ) );

  QObject::connect( ui_->descrtree, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
      this, SLOT( slotSelectedDescriptorChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  QObject::connect( ui_->spinx, SIGNAL( valueChanged( int ) ), this, SLOT( slotXchanged( int ) ) );
  QObject::connect( ui_->spiny, SIGNAL( valueChanged( int ) ), this, SLOT( slotYChanged( int ) ) );

  QObject::connect( ui_->valuespin, SIGNAL( valueChanged( double ) ), this, SLOT( slotValueChanged( double ) ) );
  QObject::connect( ui_->alignbtn, SIGNAL( posChanged( int ) ), this, SLOT( slotAlignChanged( int ) ) );
  QObject::connect( ui_->fontwidget, SIGNAL( fontChanged( const QFont& ) ), this, SLOT( slotWindFontChanged( const QFont& ) ) );

  QObject::connect( ui_->cmbyplusdf, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotYPlusDfIndexChanged( int ) ) );
  QObject::connect( ui_->lblabsent, SIGNAL( doubleclick() ), this, SLOT( slotAbsentLabelClicked() ) );
  QObject::connect( ui_->btnabsentclear, SIGNAL( clicked() ), this, SLOT( slotAbsentClear() ) );
}

QTreeWidget* Editor::descrtree() const
{
  return ui_->descrtree;
}

void Editor::puansonChangedOnScene()
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  if ( ui_->spinx->value() != rule->pos().x() ) {
    ui_->spinx->setValue( rule->pos().x() );
  }
  if ( ui_->spiny->value() != rule->pos().y() ) {
    ui_->spiny->setValue( rule->pos().y() );
  }
}

void Editor::closeEvent( QCloseEvent* e )
{
  if ( 0 != puanson_ ) {
    QString code = QString::fromStdString( puanson_->code() );
    proto::Puanson saved;
    if ( true == meteo::map::WeatherLoader::instance()->punchlibrary().contains(code) ) {
      saved.CopyFrom( meteo::map::WeatherLoader::instance()->punchlibrary()[code] );
    }
    else if ( true == meteo::map::WeatherLoader::instance()->punchlibraryspecial().contains(code) ) {
      saved.CopyFrom( meteo::map::WeatherLoader::instance()->punchlibraryspecial()[code] );
    }
    std::string savedstr;
    saved.SerializePartialToString(&savedstr);
    std::string unsavedstr;
    puanson_->SerializePartialToString(&unsavedstr);
    if ( savedstr != unsavedstr ) {
//      TProtoText::toFile( *puanson_, "1.template" );
//      TProtoText::toFile( saved, "2.template" );
      int res = QMessageBox::question(
          this,
          QObject::tr("Сохранение"),
          QObject::tr("Сохранить шаблон наноски %1?").arg( QString::fromStdString( puanson_->name() ) ),
          QObject::tr("Сохранить и закрыть"),
          QObject::tr("Закрыть"),
          QObject::tr("Отмена")
          );
      if ( 0 == res ) {
        save();
      }
      if ( 2 == res ) {
        e->ignore();
        return;
      }
    }
  }
  saveLastParams();
  e->accept();
}

void Editor::showEvent( QShowEvent* e )
{
  QWidget::showEvent(e);
  if ( 0 != ui_->signview->scene() ) {
    ui_->signview->scene()->setSceneRect( QRect( ui_->signview->rect().center() - QPoint( 600, 600 ), QSize(1200,1200) ) );
    grid_->setBRect( ui_->signview->scene()->sceneRect() );
    if ( 0 != item_ ) {
      item_->setPos( ui_->signview->scene()->sceneRect().center() );
    }
  }
  ui_->signview->setAlignment( Qt::AlignCenter );
  ui_->signview->centerOn( ui_->signview->scene()->sceneRect().center() );
}

void Editor::setRectType( int rt )
{
  if ( rt == recttype_ ) {
    return;
  }
  recttype_ = rt;
  if ( 0 != item_ ) {
    item_->setRectType(rt);
  }
}

void Editor::setAnchorType( int at )
{
  if ( at == anchortype_ ) {
    return;
  }
  anchortype_ = at;
  if ( 0 != item_ ) {
    item_->setAnchorType(at);
  }
}

void Editor::loadFromProto( const proto::Puanson& proto )
{
  if ( 0 == ui_->signview->scene() ) {
    error_log << QObject::tr("Сцена не установлена");
    return;
  }
  muteSignals();
  if ( 0 != item_ ) {
    ui_->signview->scene()->removeItem(item_);
    delete item_;
    item_ = 0;
  }
  if ( 0 == puanson_ ) {
    puanson_ = new proto::Puanson;
  }
  puanson_->CopyFrom(proto);

  ui_->name->setText( QString::fromStdString( proto.name() ) );

  if ( 0 == item_ ) {
    item_ = new Item(this);
    ui_->signview->scene()->addItem(item_);
    item_->setPos( ui_->signview->scene()->sceneRect().center() );
    item_->setRectType(recttype_);
    item_->setAnchorType(anchortype_);
  }

  path_ = MnCommon::sharePath("meteo") + "/punch/" + QString::fromStdString( proto.code() ) + ".template";

  ui_->descrtree->clear();
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    const proto::Id& id = puanson_->rule(i).id();
    addDescrToTree(id);
    setupTreeItem(puanson_->mutable_rule(i));
  }
  switch ( puanson_->center() ) {
    case puanson::proto::kNoCenter:
      ui_->centersignbox->setCurrentIndex(0);
      break;
    case puanson::proto::kCircle:
      ui_->centersignbox->setCurrentIndex(1);
      break;
    case puanson::proto::kSquare:
      ui_->centersignbox->setCurrentIndex(2);
      break;
  }
  item_->setPunch(puanson_);
  item_->loadTestMeteoData();
  ui_->radiusspin->setValue(puanson_->radius());
  setWindValuesFromCells();

  enableEdit();
  turnSignals();
  if ( true == puanson_->wind() ) {
    ui_->haswind->setChecked( puanson_->wind() );
  }
  else if ( true == puanson_->flow() ) {
    ui_->hasflow->setChecked( puanson_->flow() );
  }
  else {
    ui_->haswind->setChecked(false);
    ui_->hasflow->setChecked(false);
  }
  ui_->fontwidget->setFont( font2qfont( puanson_->windfont() ) );
}

void Editor::saveToProto( proto::Puanson* proto )
{
  if ( 0 == proto ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Шаблон не открыть. Нечего сохранять");
    return;
  }
  if ( false == proto->IsInitialized() ) {
    warning_log << QObject::tr("Шаблон заполнен не полностью. Будет выполнена попытка сохранить незаполненный шаблон знака");
  }
  proto->set_code( puanson_->code() );
  proto->set_name( puanson_->name() );
  proto->set_path( puanson_->path() );
  int indx = ui_->centersignbox->currentIndex();
  switch ( indx ) {
    case 1:
      proto->set_center(puanson::proto::kCircle);
      break;
    case 2:
      proto->set_center(puanson::proto::kSquare);
      break;
    case 0:
    default:
      proto->set_center(puanson::proto::kNoCenter);
      break;
  }
  proto->set_radius( ui_->radiusspin->value() );
  proto->set_wind( false == ( Qt::Unchecked == ui_->haswind->checkState() ) );
  indx = ui_->crisscrossbox->currentIndex();
  switch ( indx ) {
    case 0:
      proto->set_crisscross(false);
      break;
    case 1:
    default:
      proto->set_crisscross(true);
      break;
  }
  indx = ui_->cmbtype->currentIndex();
  puanson_->mutable_windfont()->CopyFrom( qfont2font( ui_->fontwidget->font() ) );
}

void Editor::updateFromProto( const proto::Puanson& proto )
{
  if ( 0 == ui_->signview->scene() ) {
    error_log << QObject::tr("Сцена не установлена");
    return;
  }
  if ( 0 != item_ ) {
    ui_->signview->scene()->removeItem(item_);
    delete item_;
    item_ = 0;
  }
  if ( 0 == puanson_ ) {
    puanson_ = new proto::Puanson;
  }
  puanson_->CopyFrom(proto);

  ui_->name->setText( QString::fromStdString( proto.name() ) );

  if ( 0 == item_ ) {
    item_ = new Item(this);
    ui_->signview->scene()->addItem(item_);
    item_->setPos( ui_->signview->scene()->sceneRect().center() );
    item_->setRectType(recttype_);
    item_->setAnchorType(anchortype_);
  }

  item_->setPunch(puanson_);
  path_ = MnCommon::sharePath("meteo") + "/punch/" + QString::fromStdString( proto.code() ) + ".template";

  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    setupTreeItem(puanson_->mutable_rule(i));
  }
  switch ( puanson_->center() ) {
    case puanson::proto::kNoCenter:
      ui_->centersignbox->setCurrentIndex(0);
      break;
    case puanson::proto::kCircle:
      ui_->centersignbox->setCurrentIndex(1);
      break;
    case puanson::proto::kSquare:
      ui_->centersignbox->setCurrentIndex(2);
      break;
  }
  if ( false == puanson_->crisscross() ) {
    ui_->crisscrossbox->setCurrentIndex(0);
  }
  else {
    ui_->crisscrossbox->setCurrentIndex(1);
  }
  ui_->radiusspin->setValue(puanson_->radius());

  ui_->haswind->setChecked( puanson_->wind() );
  ui_->drawdf->setChecked( puanson_->draw_bad_dd() );
  setWindValuesFromCells();
  ui_->fontwidget->setFont( font2qfont( puanson_->windfont() ) );

  enableEdit();
}

void Editor::addDescrToTree( const proto::Id& id )
{
  QTreeWidgetItem* item = new QTreeWidgetItem(ui_->descrtree);
  item->setText( kColumnDescr, QString::fromStdString( id.name() ) );
  if ( true == meteo::map::Puanson::fakeDescriptor( QString::fromStdString( id.name() ) ) ) {
    item->setBackground( kColumnDescr, QBrush(Qt::red) );
  }
  item->setToolTip( kColumnDescr, QString::fromStdString( id.description() ) );
  item->setSizeHint( kColumnType, QSize(32,32) );
  item->setSizeHint( kColumnAlign, QSize(32,32) );
  ui_->descrtree->setColumnWidth( kColumnAlign, 32 );
  if ( item != ui_->descrtree->currentItem() ) {
    ui_->descrtree->setCurrentItem(item);
  }
}

proto::CellRule* Editor::addDescrToProto( const proto::Id& id )
{
  proto::CellRule* rule = puanson_->add_rule();
  rule->mutable_id()->CopyFrom(id);
  rule->mutable_pos()->set_x(-20);
  rule->mutable_pos()->set_y(-20);
  return rule;
}

void Editor::setupTreeItem( proto::CellRule* rule )
{
  if ( 0 == puanson_ ) {
    return;
  }
  int indx = -1;
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    if ( puanson_->mutable_rule(i) == rule ) {
      indx = i;
      break;
    }
  }
  if ( -1 == indx ) {
    return;
  }
  QTreeWidgetItem* item = ui_->descrtree->topLevelItem(indx);;

  if ( 0 == item ) {
    error_log << QObject::tr("Дескриптор %1 не добавлен в список. Невозможно настроить отображение не добавленного дескрпиптора")
      .arg( QString::fromStdString( rule->id().name() ) );
    return;
  }
  QString str = meteo::map::stringFromRuleValue( rule->testval(), *rule );
  QFont fnt = meteo::map::fontFromRuleValue( rule->testval(), *rule );
  QColor clr = meteo::map::colorFromRuleValue( rule->testval(), *rule );
  item->setText( kColumnTestval, str );
  item->setFont( kColumnTestval, fnt );
  item->setForeground( kColumnTestval, QBrush(clr) );
  item->setText( kColumnType, symbolForType( rule->type() ) );
  item->setToolTip( kColumnType, tooltipForType( rule->type() ) );
  item->setFont( kColumnType, fontForType( rule->type() ) );
  item->setTextAlignment( kColumnType, Qt::AlignHCenter | Qt::AlignVCenter );
  item->setTextAlignment( kColumnDescr, Qt::AlignHCenter | Qt::AlignVCenter );
  item->setTextAlignment( kColumnAlign, Qt::AlignHCenter | Qt::AlignVCenter );
  item->setTextAlignment( kColumnTestval, Qt::AlignHCenter | Qt::AlignVCenter );
  item->setIcon( kColumnAlign, AlignButton::icoForAlign( rule->align() ) );
  QIcon ico = ( true == rule->visible() ) ? QIcon(":/meteo/icons/tools/layer_visible.png") : QIcon(":/meteo/icons/tools/layer_hidden.png");
  item->setIcon( kColumnVisible, ico );
  Qt::CheckState chst = Qt::Unchecked;
  if ( true == rule->y_plus_bad_dd() ) {
    chst = Qt::Checked;
  }
  item->setCheckState( kColumnDrawDD, chst );
}

void Editor::save()
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Не выбран шаблон. Нечего сохранять.");
    return;
  }
  saveToProto(puanson_);
  QFile file( meteo::global::kPunchPath() + QString::fromStdString( puanson_->code() ) + ".template" );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Не удалось открыть файл для сохранения шаблона %1")
      .arg( QString::fromStdString( puanson_->code() ) );
    return;
  }
  QString str = TProtoText::toText( *puanson_ );
  file.write( str.toLocal8Bit() );
  file.flush();
  file.close();
  map::WeatherLoader::instance()->addPunchToLibrary(*puanson_);
}

void Editor::saveLastParams()
{
  if ( 0 == item_ ) {
    return;
  }
  QDir dir(kLastParamsPath);
  if ( false == dir.exists() ) {
    bool res = dir.mkpath(kLastParamsPath);
    if ( false == res ) {
      dir.setPath( QDir::currentPath() );
      error_log << QObject::tr("Не удалось создать директорию %1. Используется директория %2")
        .arg( kLastParamsPath )
        .arg( dir.absolutePath() );
    }
  }
  QFile file( dir.absoluteFilePath("punch.template") );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1").arg( file.fileName() );
    return;
  }
  file.write( path_.toUtf8() );
  file.flush();
  file.close();
  file.setFileName( dir.absoluteFilePath("punch.data") );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1").arg( file.fileName() );
    return;
  }
  QByteArray arr;
  QDataStream stream( &arr, QIODevice::WriteOnly );
  stream << item_->meteodata();
  file.write(arr);
  file.flush();
  file.close();
}

bool Editor::loadLastParams()
{
  QDir dir(kLastParamsPath);
  if ( false == dir.exists() && false == dir.mkpath(kLastParamsPath) ) {
    dir.setPath( QDir::currentPath() );
    error_log << QObject::tr("Не удалось создать директорию %1. Используется директория %2")
      .arg( kLastParamsPath )
      .arg( dir.absolutePath() );
  }
  QFile file( dir.absoluteFilePath("punch.template") );
  if ( false == file.exists() ) {
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1")
      .arg( file.fileName() );
    return false;
  }
  path_ = file.readAll();
  file.close();
  file.setFileName(path_);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл шаблона не найден. %1")
      .arg(path_);
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1")
      .arg( file.fileName() );
    return false;
  }
  QString str = file.readAll();
  file.close();
  proto::Puanson proto;
  if ( false == TProtoText::fillProto( str, &proto ) ) {
    error_log << QObject::tr("Не удалось загрузить параметры шаблона из текстового файла %1")
      .arg( file.fileName() );
    return false;
  }
  loadFromProto(proto);
  updateScene();
  return true;
}

void Editor::enableEdit()
{
  ui_->btnaddsign->setEnabled(true);
  ui_->btnrmsign->setEnabled(true);
}

void Editor::disableEdit()
{
  ui_->btnaddsign->setEnabled(true);
  ui_->btnrmsign->setEnabled(true);
}

void Editor::setupFormat( proto::CellRule* rule )
{
  int type = rule->type();
typeswitched:

  if ( proto::kSymNumber == type ) {
    QMenu* menu = new QMenu(this);
    menu->addAction( symbolForType(proto::kNumber) );
    menu->addAction( symbolForType(proto::kSymbol) );
    QAction* a = menu->exec( QCursor::pos() );
    if ( 0 != a ) {
      if ( a->text() == symbolForType(proto::kNumber) ) {
        type = proto::kNumber;
      }
      else {
        type = proto::kSymbol;
      }
    }
    delete menu;
  }

  switch ( type ) {
    case proto::kNumber: {
        NumFormat* dlg = new NumFormat(this);
        dlg->setNumbeRule(rule->mutable_number(), rule->align(), rule->testval() );
        QString typetext = symbolForType(rule->type());
        QFont typefont = fontForType(rule->type());
        dlg->setSwitchButtonText( typetext, typefont );
        int res = dlg->exec();
        if ( QDialog::Rejected == res ) {
          delete dlg;
          return;
        }
        rule->mutable_number()->CopyFrom( dlg->numbeRule() );
        rule->set_align( (proto::Align)dlg->align() );
        proto::CellType t = dlg->type();
        if ( proto::kSymNumber != rule->type() ) {
          rule->set_type(t);
        }
        rule->set_testval( dlg->testvalue() );
        delete dlg;
        if ( proto::kNumber != t ) {
          type = t;
          goto typeswitched;
        }
        return;
      }
      break;
    case proto::kSymbol: {
        SymbolFormat* dlg = new SymbolFormat(this);
        dlg->setSymbolRule(rule->mutable_symbol(), rule->align(), rule->testval() );
        QString typetext = symbolForType(rule->type());
        QFont typefont = fontForType(rule->type());
        dlg->setSwitchButtonText( typetext, typefont );
        int res = dlg->exec();
        if ( QDialog::Rejected == res ) {
          delete dlg;
          return;
        }
        rule->mutable_symbol()->CopyFrom( dlg->symbolRule() );
        proto::CellType t = dlg->type();
        if ( proto::kSymNumber != rule->type() ) {
          rule->set_type(t);
        }
        rule->set_testval( dlg->testvalue() );
        delete dlg;
        if ( proto::kSymbol != t ) {
          type = t;
          goto typeswitched;
        }
        return;
      }
      break;
    default:
      break;
  }
}

void Editor::setupAlign( proto::CellRule* rule )
{
  QPoint pos = ui_->descrtree->viewport()->mapFromGlobal(QCursor::pos());
  QModelIndex indx = ui_->descrtree->indexAt( pos );
  pos = ui_->descrtree->viewport()->mapToGlobal(ui_->descrtree->visualRect(indx).topLeft());
  TextPosdlg* dlg = new TextPosdlg( ui_->descrtree );
  dlg->move(pos);
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    rule->set_align( (meteo::puanson::proto::Align)dlg->currentPos() );
  }
  delete dlg;
}

void Editor::setupTestValue( proto::CellRule* rule )
{
  QPoint pos = ui_->descrtree->viewport()->mapFromGlobal(QCursor::pos());
  QModelIndex indx = ui_->descrtree->indexAt( pos );
  pos = ui_->descrtree->viewport()->mapToGlobal(ui_->descrtree->visualRect(indx).center());
  DlgSpinbox* dlg = new DlgSpinbox( ui_->descrtree );
  pos.setX( pos.x() - dlg->size().width()/2 );
  pos.setY( pos.y() - dlg->size().height()/2 );
  dlg->move(pos);
  dlg->setValue( rule->testval() );
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    rule->set_testval( dlg->value() );
  }
  delete dlg;
}

void Editor::setupValueView()
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  ui_->valuespin->setValue( rule->testval() );
  QString txt = map::stringFromRuleValue( rule->testval(), *rule );
  QColor clr =  map::colorFromRuleValue( rule->testval(), *rule );
  QFont fnt =  map::fontFromRuleValue( rule->testval(), *rule );
  ui_->viewle->setFont(fnt);
  ui_->viewle->setText(txt);
  QPalette plt = ui_->viewle->palette();
  plt.setColor( QPalette::Text, clr );
  ui_->viewle->setPalette(plt);
  ui_->alignbtn->setAlign( rule->align() );
  ui_->lbldescr->setText( QString::fromStdString( rule->id().name() ) );
  if ( true == rule->y_plus_bad_dd() ) {
    ui_->cmbyplusdf->setCurrentIndex(1);
  }
  else {
    ui_->cmbyplusdf->setCurrentIndex(0);
  }
  ui_->cmbtype->setCurrentIndex( rule->type() );
  if ( false == rule->has_absent_rule() ) {
    ui_->lblabsent->setText( QObject::tr("<НЕТ>") );
    QWidget* prnt = qobject_cast<QWidget*>( ui_->lblabsent->parent() );
    if ( 0 != prnt ) {
      ui_->lblabsent->setFont( prnt->font() );
    }
//    ui_->lblabsent->set
  }
  else {
    const proto::CellRule::Symbol& smb = rule->absent_rule();
    QString txt = QObject::tr( QByteArray::fromHex( QByteArray( smb.patternba().data(), smb.patternba().size() ) ) );
    QFont fnt = font2qfont( smb.font() );
    ui_->lblabsent->setText(txt);
    ui_->lblabsent->setFont(fnt);
  }
}

proto::CellRule Editor::defaultRule() const
{
  proto::CellRule rule;
  rule.set_align( proto::kNoPosition );
  rule.set_type( proto::kNumber );
  *(rule.mutable_number()) = proto::CellRule::NumbeRule();
  return rule;
}

proto::CellRule* Editor::currentRule()
{
  QTreeWidgetItem* item = ui_->descrtree->currentItem();
  return itemRule(item);
}

proto::CellRule* Editor::itemRule( QTreeWidgetItem* item )
{
  proto::CellRule* rule = 0;
  if ( 0 == item ) {
    return rule;
  }
  if ( 0 == puanson_ ) {
    return rule;
  }
  int indx = ui_->descrtree->indexOfTopLevelItem(item);
  if ( puanson_->rule_size() <= indx ) {
    return rule;
  }
  rule = puanson_->mutable_rule(indx);
  return rule;
}

void Editor::setWindValuesFromCells()
{
  QString ddname;
  QString ffname;
  if ( true == ui_->haswind->isChecked() ) {
    ddname = "dd";
    ffname = "ff";
  }
  else if ( true == ui_->hasflow->isChecked() ) {
    ddname = "dndn";
    ffname = "cncn";
  }
  else {
    return;
  }
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    if ( QString::fromStdString(puanson_->rule(i).id().name() ) == ddname ) {
      ui_->ddspin->setValue( puanson_->rule(i).testval() );
      int indx = ui_->ddqualcmb->findText( QString::number( puanson_->rule(i).testqual() ) );
      if ( -1 != indx ) {
        ui_->ddqualcmb->setCurrentIndex(indx);
      }
    }
    if ( QString::fromStdString(puanson_->rule(i).id().name() ) == ffname ) {
      ui_->ffspin->setValue( puanson_->rule(i).testval() );
      int indx = ui_->ffqualcmb->findText( QString::number( puanson_->rule(i).testqual() ) );
      if ( -1 != indx ) {
        ui_->ffqualcmb->setCurrentIndex(indx);
      }
    }
  }
  ui_->fontwidget->setFont( font2qfont( puanson_->windfont() ) );
}

void Editor::setCellsFromWindValues()
{
  QString ddname;
  QString ffname;
  if ( true == ui_->haswind->isChecked() ) {
    ddname = "dd";
    ffname = "ff";
  }
  else if ( true == ui_->hasflow->isChecked() ) {
    ddname = "dndn";
    ffname = "cncn";
  }
  else {
    return;
  }
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    if ( QString::fromStdString(puanson_->rule(i).id().name() ) == ddname ) {
      puanson_->mutable_rule(i)->set_testval(ui_->ddspin->value() );
      puanson_->mutable_rule(i)->set_testqual( ui_->ddqualcmb->currentText().toInt() );
    }
    if ( QString::fromStdString(puanson_->rule(i).id().name() ) == ffname ) {
      puanson_->mutable_rule(i)->set_testval(ui_->ffspin->value() );
      puanson_->mutable_rule(i)->set_testqual( ui_->ffqualcmb->currentText().toInt() );
    }
  }
  puanson_->mutable_windfont()->CopyFrom( qfont2font( ui_->fontwidget->font() ) );
}

void Editor::slotSelectPuanson()
{
  PunchSelect* dlg = new PunchSelect(this);
  if ( 0 != puanson_ ) {
    dlg->setSelectedPunch(*puanson_);
  }
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    loadFromProto( dlg->selectedPunch() );
  }
  delete dlg;
}

void Editor::slotSavePuanson()
{
  save();
}

void Editor::slotDrawWindClicked( bool checked )
{
  if ( 0 == puanson_ ) {
    return;
  }

  if ( true == checked && true == ui_->hasflow->isChecked() ) {
    QObject::disconnect( ui_->hasflow, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawFlowClicked( bool ) ) );
    ui_->hasflow->setChecked(false);
    slotDrawFlowClicked(false);
    QObject::connect( ui_->hasflow, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawFlowClicked( bool ) ) );
  }

  ui_->ddlabel->setEnabled(checked);
  ui_->ddlabel->setText("dd_ff");
  ui_->dfquallbl->setEnabled(checked);
  ui_->ddspin->setEnabled(checked);
  ui_->ffspin->setEnabled(checked);
  ui_->ddqualcmb->setEnabled(checked);
  ui_->ffqualcmb->setEnabled(checked);
  ui_->drawdf->setEnabled(checked);

  bool hasdd;
  bool hasff;
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    if ( QObject::tr("dd") == QString::fromStdString( puanson_->rule(i).id().name() ) ) {
      hasdd = true;
    }
    if ( QObject::tr("ff") == QString::fromStdString( puanson_->rule(i).id().name() ) ) {
      hasff = true;
    }
  }
  if ( false == hasdd && true == checked ) {
    proto::CellRule rule = defaultRule();
    proto::Id id;
//    id.set_index(0);
    id.set_name("dd");
    id.set_descr( TMeteoDescriptor::instance()->descriptor("dd") );
    *(rule.mutable_id()) = id;
    proto::CellRule* r = puanson_->add_rule();
    *r = rule;
  }
  if ( false == hasff && true == checked ) {
    proto::CellRule rule = defaultRule();
    proto::Id id;
//    id.set_index(0);
    id.set_name("ff");
    id.set_descr( TMeteoDescriptor::instance()->descriptor("ff") );
    *(rule.mutable_id()) = id;
    proto::CellRule* r = puanson_->add_rule();
    *r = rule;
  }
  if ( 0 != puanson_ ) {
    puanson_->set_wind(checked);
  }
  if ( ( false == hasff && true == checked ) || ( false == hasdd && true == checked ) ) {
    loadFromProto(*puanson_);
  }
  else {
    updateFromProto(*puanson_);
  }
  updateScene();
}

void Editor::slotDrawFlowClicked( bool checked )
{
  if ( 0 == puanson_ ) {
    return;
  }

  if ( true == checked && true  == ui_->haswind->isChecked() ) {
    QObject::disconnect( ui_->haswind, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawWindClicked( bool ) ) );
    ui_->haswind->setChecked(false);
    slotDrawWindClicked(false);
    QObject::connect( ui_->haswind, SIGNAL( clicked( bool ) ), this, SLOT( slotDrawWindClicked( bool ) ) );
  }

  ui_->ddlabel->setEnabled(checked);
  if ( true == checked ) {
    ui_->ddlabel->setText("dndn_cncn");
  }
  ui_->dfquallbl->setEnabled(checked);
  ui_->ddspin->setEnabled(checked);
  ui_->ffspin->setEnabled(checked);
  ui_->ddqualcmb->setEnabled(checked);
  ui_->ffqualcmb->setEnabled(checked);
  ui_->drawdf->setEnabled(false);

  bool hasdn;
  bool hascn;
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    if ( QObject::tr("dndn") == QString::fromStdString( puanson_->rule(i).id().name() ) ) {
      hasdn = true;
    }
    if ( QObject::tr("cncn") == QString::fromStdString( puanson_->rule(i).id().name() ) ) {
      hascn = true;
    }
  }
  if ( false == hasdn && true == checked ) {
    proto::CellRule rule = defaultRule();
    proto::Id id;
//    id.set_index(0);
    id.set_name("dndn");
    id.set_descr( TMeteoDescriptor::instance()->descriptor("dndn") );
    *(rule.mutable_id()) = id;
    proto::CellRule* r = puanson_->add_rule();
    *r = rule;
  }
  if ( false == hascn && true == checked ) {
    proto::CellRule rule = defaultRule();
    proto::Id id;
//    id.set_index(0);
    id.set_name("cncn");
    id.set_descr( TMeteoDescriptor::instance()->descriptor("cncn") );
    *(rule.mutable_id()) = id;
    proto::CellRule* r = puanson_->add_rule();
    *r = rule;
  }
  if ( 0 != puanson_ ) {
    puanson_->set_flow(checked);
  }
  if ( ( false == hascn && true == checked ) || ( false == hascn && true == checked ) ) {
    loadFromProto(*puanson_);
  }
  else {
    updateFromProto(*puanson_);
  }
  updateScene();
}

void Editor::slotDrawDFClicked( bool checked )
{
  if ( 0 != puanson_ ) {
    puanson_->set_draw_bad_dd(checked);
  }
  updateFromProto(*puanson_);
  updateScene();
}

void Editor::slotAddSign()
{
  if ( 0 == puanson_ ) {
    return;
  }
  DescrSelect* dlg = new DescrSelect(this);
  QStringList selected;
  for ( int i = 0, sz = puanson_->rule_size(); i < sz; ++i ) {
    selected.append( QString::fromStdString( puanson_->rule(i).id().name() ) );
  }
  dlg->setSelected(selected);
  dlg->loadDescriptors();
  int res = dlg->exec();
  if (  QDialog::Rejected == res ) {
    delete dlg;
    return;
  }
  proto::Id id;
  id.CopyFrom(dlg->selectedDescr());
  delete dlg;
  addDescrToTree(id);
  proto::CellRule* rule = addDescrToProto(id);
  setupTreeItem(rule);

  item_->setPunch(puanson_);
  updateScene();
}

void Editor::slotRmSign()
{
  if ( 0 == puanson_ ) {
    return;
  }
  QTreeWidgetItem* item = ui_->descrtree->currentItem();
  if ( 0 == item ) {
    return;
  }
  int indx = ui_->descrtree->indexOfTopLevelItem(item);
  for  ( int i = indx + 1, sz = puanson_->rule_size(); i < sz; ++i ) {
    puanson_->mutable_rule( i - 1 )->CopyFrom( puanson_->rule(i) );
  }
  puanson_->mutable_rule()->RemoveLast();
  meteo::puanson::proto::Puanson p;
  p.CopyFrom(*puanson_);
  loadFromProto(p);
  updateScene();
}

void Editor::updateScene()
{
  if ( 0 == puanson_ ) {
    return;
  }
  if ( 0 != item_ ) {
    item_->setPunch(puanson_);
    item_->loadTestMeteoData();
  }
}

void Editor::slotCellItemDoubleClicked( QTreeWidgetItem* item, int clmn )
{
  if ( 0 == item ) {
    warning_log << QObject::tr("Итем не выбран");
    return;
  }
  if ( 0 > clmn ) {
    warning_log << QObject::tr("Номер колонки не выбран");
    return;
  }
  QString name = item->text( kColumnDescr );
  proto::CellRule* rule = itemRule(item);
  if ( 0 == rule ) {
    error_log << QObject::tr("В шаблоне не найден дескриптор %1")
      .arg(name);
    return;
  }
  switch ( clmn ) {
    case kColumnType:
      setupFormat(rule);
      break;
    case kColumnDescr:
      setupFormat(rule);
      break;
    case kColumnAlign:
      setupAlign(rule);
      break;
    case kColumnTestval:
      setupTestValue(rule);
      break;
    default:
      break;
  }
  setupTreeItem(rule);
  setWindValuesFromCells();
  setupValueView();
  updateScene();
}

void Editor::slotCellItemClicked( QTreeWidgetItem* item, int clmn )
{
  if ( 0 == item ) {
    warning_log << QObject::tr("Итем не выбран");
    return;
  }
  if ( kColumnVisible != clmn ) {
    return;
  }
  proto::CellRule* rule = itemRule(item);
  if ( 0 == rule ) {
    error_log << QObject::tr("В шаблоне не найден дескриптор %1")
      .arg( item->text( kColumnDescr ) );
    return;
  }
  if ( true == rule->visible() ) {
    rule->set_visible(false);
  }
  else {
    rule->set_visible(true);
  }
  setupTreeItem(rule);
  updateScene();
}

void Editor::slotCellItemChanged( QTreeWidgetItem* item, int clmn )
{
  if ( 0 == item ) {
    warning_log << QObject::tr("Итем не выбран");
    return;
  }
  if ( kColumnDrawDD != clmn ) {
    return;
  }
  proto::CellRule* rule = itemRule(item);
  if ( 0 == rule ) {
    error_log << QObject::tr("В шаблоне не найден дескриптор %1")
      .arg( item->text( kColumnDescr ) );
    return;
  }
  if ( Qt::Checked == item->checkState(kColumnDrawDD) ) {
    rule->set_y_plus_bad_dd(true);
  }
  else {
    rule->set_y_plus_bad_dd(false);
  }
  setupTreeItem(rule);
  updateScene();
  setupValueView();
}

void Editor::slotBtnRectIndexChanged( int indx )
{
  setRectType(indx);
}

void Editor::slotBtnAnchorIndexChanged( int indx )
{
  setAnchorType(indx);
}

void Editor::slotBtnGridClicked()
{
  if ( ui_->btngrid->isChecked() ) {
    grid_->show();
  }
  else {
    grid_->hide();
  }
}

void Editor::slotCenterIndexChanged( int index )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(index);
  saveToProto(puanson_);
  updateScene();
}

void Editor::slotCrissCrossIndexChanged( int index )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(index);
  saveToProto(puanson_);
  updateScene();
}

void Editor::slotTypeChanged( int index )
{
  if ( 0 == puanson_ ) {
    return;
  }
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  rule->set_type( static_cast<proto::CellType>(index) );
  saveToProto(puanson_);
  updateScene();
}

void Editor::slotRadiusSpinValueChanged( int radius )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(radius);
  saveToProto(puanson_);
  updateScene();
}

void Editor::slotFFCanged( int val )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(val);
  setCellsFromWindValues();
  saveToProto(puanson_);
  updateFromProto(*puanson_);
  updateScene();
}

void Editor::slotDDCanged( int val )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(val);
  setCellsFromWindValues();
  saveToProto(puanson_);
  updateFromProto(*puanson_);
  updateScene();
}

void Editor::slotDDQualCanged( int val )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(val);
  setCellsFromWindValues();
  saveToProto(puanson_);
  updateFromProto(*puanson_);
  updateScene();
}

void Editor::slotFFQualCanged( int val )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(val);
  setCellsFromWindValues();
  saveToProto(puanson_);
  updateFromProto(*puanson_);
  updateScene();
}

void Editor::slotWindFontChanged( const QFont& fnt )
{
  if ( 0 == puanson_ ) {
    return;
  }
  Q_UNUSED(fnt);
  setCellsFromWindValues();
  saveToProto(puanson_);
  updateFromProto(*puanson_);
  updateScene();
}
    
void Editor::slotXchanged( int x )
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  if ( x != rule->pos().x() ) {
    rule->mutable_pos()->set_x(x);
    if ( 0 != item_ ) {
      item_->updatePunch();
    }
  }
}

void Editor::slotYChanged( int y )
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  if ( y != rule->pos().y() ) {
    rule->mutable_pos()->set_y(y);
    if ( 0 != item_ ) {
      item_->updatePunch();
    }
  }
}
    
void Editor::slotValueChanged( double val )
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  rule->set_testval(val);
  setupTreeItem(rule);
  setWindValuesFromCells();
  setupValueView();
  if ( 0 != item_ ) {
    item_->updatePunch();
  }
}

void Editor::slotAlignChanged( int align )
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  rule->set_align( (proto::Align)align );
  setupTreeItem(rule);
//  setWindValuesFromCells();
  setupValueView();
  if ( 0 != item_ ) {
    item_->updatePunch();
  }
}

void Editor::slotYPlusDfIndexChanged( int indx )
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  if ( 1 == indx ) {
   rule->set_y_plus_bad_dd(true);
  }
  else {
   rule->set_y_plus_bad_dd(false);
  }
  setupTreeItem(rule);
//  setWindValuesFromCells();
  setupValueView();
  if ( 0 != item_ ) {
    item_->updatePunch();
  }
}

void Editor::slotSelectedDescriptorChanged( QTreeWidgetItem* item, QTreeWidgetItem* previous )
{
  Q_UNUSED(previous);
  proto::CellRule* rule = itemRule(item);
  if ( 0 == rule ) {
    ui_->currentrulebox->setDisabled(true);
    return;
  }
  QString descrname = QString::fromStdString( rule->id().name() );
  if ( descrname != item->text( kColumnDescr ) ) {
    error_log.msgBox() << QObject::tr("Непонятная ошибка. Имя правила %1 не совпадает с текстом выбранного эелемента %2")
      .arg( descrname )
      .arg( item->text(kColumnDescr) );
  }
  if ( proto::kNoPosition == rule->align() ) {
    ui_->spinx->setDisabled(true);
    ui_->spiny->setDisabled(true);
  }
  else {
    ui_->spinx->setDisabled(false);
    ui_->spiny->setDisabled(false);
  }
  if ( true == rule->has_pos() ) {
    ui_->spinx->setValue( rule->pos().x() );
    ui_->spiny->setValue( rule->pos().y() );
  }
  setupValueView();
  ui_->currentrulebox->setDisabled(false);
}

void Editor::slotAbsentLabelClicked()
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  AddSymbol* dlg = new AddSymbol(this);
  proto::CellRule::Symbol smb;
  if ( true == rule->has_absent_rule() ) {
    smb.CopyFrom( rule->absent_rule() );
    dlg->setSymbol(smb);
  }
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    rule->mutable_absent_rule()->CopyFrom( dlg->symbol() );
    setupValueView();
  }
  delete dlg;
}

void Editor::slotAbsentClear()
{
  proto::CellRule* rule = currentRule();
  if ( 0 == rule ) {
    return;
  }
  if ( true == rule->has_absent_rule() ) {
    rule->clear_absent_rule();
    setupValueView();
  }
}

}
}
