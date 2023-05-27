#include "radarsettings.h"

#include <qsettings.h>
#include <qapplication.h>
#include <qmenu.h>
#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cross-commons/app/paths.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <meteo/commons/punchrules/punchrules.h>

#include <meteo/commons/global/global.h>

#include "ui_radarsettings.h"
#include "ui_addradardescr.h"

namespace meteo {
namespace map {

static const QString punchico(":/meteo/icons/drawtools/text.png");
static const QString gradico(":/meteo/icons/tools/fill.xpm");

const int kColumnDescr  = 0;
const int kColumnName   = 1;
const int kColumnOpis   = 2;

RadarSettings::RadarSettings( app::MainWindow* parent)
  : QWidget(parent),
  ui_(new Ui::RadarSettings),
  params_( global::kRadarParamPath() )
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  Q_UNUSED(wf);
  ui_->setupUi(this);

  ui_->radartypes->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
  ui_->znakilbl->setTextFormat( Qt::RichText );

  loadPunches();
  showGradient();
  loadSettings();

  ui_->btnclrmin->setStyleSheet( "background: black" );
  ui_->btnclrmax->setStyleSheet( "background: black" );
  ui_->btnadd->setVisible(false);
  ui_->btnrm->setVisible(false);
  ui_->cmbpunch->setDisabled(true);

  ui_->minspin->setMinimumHeight(30);
  ui_->maxspin->setMinimumHeight(30);
  ui_->stepspin->setMinimumHeight(30);
  ui_->emptyspin->setMinimumHeight(30);

  turnSignals();

  if ( 0 != ui_->radartypes->topLevelItemCount() ) {
    ui_->radartypes->topLevelItem(0)->setSelected(true);
  }
}

RadarSettings::~RadarSettings()
{
  delete ui_;
  ui_ = 0;
}
    
QTreeWidget* RadarSettings::radartypes() const
{
  return ui_->radartypes;
}

void RadarSettings::closeEvent( QCloseEvent* e )
{
  proto::RadarColors clrs = currentParams();
  if ( false == global::isEqual( clrs, params_.protoParams() ) ) {
    int res = QMessageBox::question(
        this,
        QObject::tr("Настройки изменены"),
        QObject::tr("Сохранить изменения?"),
        QObject::tr("Сохранить и закрыть"),
        QObject::tr("Закрыть без сохранения"),
        QObject::tr("Отмена") );
    if ( 0 == res ) {
      slotAccept();
      e->accept();
    }
    else if ( 1 == res ) {
      e->accept();
    }
    else {
      e->ignore();
    }
    return;
  }
  e->accept();
}

void RadarSettings::slotAccept()
{
  saveSettings();
}

void RadarSettings::slotReject()
{
  QWidget::close();
}

void RadarSettings::slotAddRadarDescr()
{
  AddRadarDescr* dlg = new AddRadarDescr(this);
  int res = dlg->exec();
  if ( QDialog::Accepted != res ) {
    delete dlg;
    return;
  }
  int descr = dlg->descr();
  QString name = dlg->name();
  QString description = dlg->description();

  proto::RadarColor color;
  color.set_descr(descr);
  color.set_name( name.toStdString() );
  color.set_description( description.toStdString() );
  std::string str = color.SerializeAsString();
  QByteArray arr( str.data(), str.size() );

  QTreeWidgetItem* item = new QTreeWidgetItem( ui_->radartypes );
  item->setData( kColumnDescr, Qt::UserRole, arr );
  item->setText( kColumnDescr, QString::number(descr) );
  item->setText( kColumnName, name );
  item->setText( kColumnOpis, description );
  setupProtoFromGradientControls(item);
  item->setSelected(true);

  delete dlg;
}

void RadarSettings::slotRmRadarDescr()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  int res = QMessageBox::question(
      this,
      QObject::tr("Удаление настройки"),
      QObject::tr("Удалить настройку '%1'?").arg(item->text(kColumnName)),
      QObject::tr("Удалить"),
      QObject::tr("Отмена")
      );
  if ( 0 == res ) {
    delete item;
  }
  if ( 0 != ui_->radartypes->topLevelItemCount() ) {
    ui_->radartypes->topLevelItem(0)->setSelected(true);
  }
}

void RadarSettings::slotItemSelectionChanged()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    showGradient();
    return;
  }
  proto::RadarColor clr = fromItem(item);
  showGradient();
  ui_->emptyspin->setValue( clr.empty_value() );
}

void RadarSettings::slotItemClicked( QTreeWidgetItem* item, int clmn )
{
  return;
  Q_UNUSED(clmn);
  Q_UNUSED(item);
}

void RadarSettings::slotItemDoubleClicked( QTreeWidgetItem* item, int clmn )
{
  if ( kColumnName != clmn ) {
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
//    ui_->radartypes->openPersistentEditor(item, clmn);
  }
  else {
    item->setFlags( item->flags() | Qt::ItemIsEditable );
  }
}

void RadarSettings::slotItemChanged( QTreeWidgetItem* item, int clmn )
{
  Q_UNUSED(clmn);
  proto::RadarColor clr = fromItem(item);
  std::string str = item->text(kColumnName).toStdString();
  if ( str == clr.name() ) {
    return;
  }
  clr.set_name(str);
  setItemData( clr, item );
}

void RadarSettings::slotPunchCurrentIndexChanged( int indx )
{
  if ( -1 == indx ) {
    return;
  }
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  proto::RadarColor clr = fromItem(item);
  QString title = ui_->cmbpunch->currentText();
  QMapIterator< QString, puanson::proto::Puanson > it(punches_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( QString::fromStdString( it.value().name() ) == title ) {
      clr.set_punch_id( it.value().code() );
      setItemData( clr, item );
      break;
    }
  }
}

void RadarSettings::slotBtnMinColorClicked()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  QColorDialog* dlg = new QColorDialog( ui_->btnclrmin->palette().color( QPalette::Background ), this );
  dlg->setOption( QColorDialog::ShowAlphaChannel, true );
  int res = dlg->exec();
  QColor clr = dlg->selectedColor();
  delete dlg;
  dlg = 0;
  if ( QDialog::Accepted != res ) {
    return;
  }
  setColorMin(clr);
  setupProtoFromGradientControls();
}

void RadarSettings::slotBtnMaxColorClicked()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  QColorDialog* dlg = new QColorDialog( ui_->btnclrmax->palette().color( QPalette::Background ), this );
  dlg->setOption( QColorDialog::ShowAlphaChannel, true );
  int res = dlg->exec();
  QColor clr = dlg->selectedColor();
  delete dlg;
  dlg = 0;
  if ( QDialog::Accepted != res ) {
    return;
  }
  ui_->btnclrmax->setStyleSheet(QString("background: %1").arg( clr.name() ) );
  QPalette plt = ui_->btnclrmax->palette();
  plt.setColor( QPalette::Background, clr );
  ui_->btnclrmax->setPalette(plt);
  setColorMax(clr);
  setupProtoFromGradientControls();
}

void RadarSettings::slotBtnGradientClicked()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  proto::RadarColor clr = fromItem(item);
  GradientEditor* dlg = 0;
  if ( 0 != clr.gradient_size() ) {
    TColorGradList gradlist = RadarParams::gradParams(clr);
    dlg = new GradientEditor( gradlist, this );
    }
  else {
    dlg = new GradientEditor(
        ui_->minspin->value(),
        minColor(),
        ui_->maxspin->value(),
        maxColor(),
        ui_->stepspin->value(),
        this );
  }
  if ( QDialog::Accepted != dlg->exec() ) {
    delete dlg;
    return;
  }
  TColorGradList pal = dlg->gradient();
  proto::RadarColor newclr = RadarParams::gradlist2gradientcolor(pal);
  clr.mutable_gradient()->CopyFrom( newclr.gradient() );
  clr.set_step( dlg->step() );
  setItemData( clr, item );
  setupGradientFromProto();
  delete dlg;
}

void RadarSettings::slotMinChanged( int val )
{
  if ( val > ui_->maxspin->value() ) {
    ui_->maxspin->setValue(val);
  }
  setupProtoFromGradientControls();
}

void RadarSettings::slotMaxChanged( int val )
{
  if ( val < ui_->minspin->value() ) {
    ui_->minspin->setValue(val);
  }
  setupProtoFromGradientControls();
}

void RadarSettings::slotStepChanged( int val )
{
  if ( ui_->minspin->value() + val > ui_->maxspin->value() ) {
    ui_->maxspin->setValue(ui_->minspin->value() + val );
  }
  setupProtoFromGradientControls();
}

void RadarSettings::slotEmptyChanged( int val )
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return;
  }
  proto::RadarColor clr = fromItem(item);
  clr.set_empty_value(val);
  setItemData( clr, item );
}

void RadarSettings::muteSignals()
{
  QObject::disconnect( ui_->btnadd, SIGNAL( clicked() ), this, SLOT( slotAddRadarDescr() ) );
  QObject::disconnect( ui_->btnrm, SIGNAL( clicked() ), this, SLOT( slotRmRadarDescr() ) );
  QObject::disconnect( ui_->radartypes, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemClicked( QTreeWidgetItem*, int ) ) );
  QObject::disconnect( ui_->radartypes, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  QObject::disconnect( ui_->radartypes, SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelectionChanged() ) );
  QObject::disconnect( ui_->radartypes, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( slotItemChanged( QTreeWidgetItem*, int ) ) );

  QObject::disconnect( ui_->saveBtn, SIGNAL( clicked() ), this, SLOT( slotAccept() ) );
  QObject::disconnect( ui_->closeBtn, SIGNAL( clicked() ), this, SLOT( slotReject() ) );

  QObject::disconnect( ui_->cmbpunch, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotPunchCurrentIndexChanged( int ) ) );

  QObject::disconnect( ui_->btnclrmin, SIGNAL( clicked() ), this, SLOT( slotBtnMinColorClicked() ) );
  QObject::disconnect( ui_->btnclrmax, SIGNAL( clicked() ), this, SLOT( slotBtnMaxColorClicked() ) );
  QObject::disconnect( ui_->btngradient, SIGNAL( clicked() ), this, SLOT( slotBtnGradientClicked() ) );

  QObject::disconnect( ui_->minspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotMinChanged( int ) ) );
  QObject::disconnect( ui_->maxspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotMaxChanged( int ) ) );
  QObject::disconnect( ui_->stepspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotStepChanged( int ) ) );
  QObject::disconnect( ui_->emptyspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotEmptyChanged( int ) ) );
}

void RadarSettings::turnSignals()
{
  QObject::connect( ui_->btnadd, SIGNAL( clicked() ), this, SLOT( slotAddRadarDescr() ) );
  QObject::connect( ui_->btnrm, SIGNAL( clicked() ), this, SLOT( slotRmRadarDescr() ) );
  QObject::connect( ui_->radartypes, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemClicked( QTreeWidgetItem*, int ) ) );
  QObject::connect( ui_->radartypes, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  QObject::connect( ui_->radartypes, SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelectionChanged() ) );
  QObject::connect( ui_->radartypes, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( slotItemChanged( QTreeWidgetItem*, int ) ) );

  QObject::connect( ui_->saveBtn, SIGNAL( clicked() ), this, SLOT( slotAccept() ) );
  QObject::connect( ui_->closeBtn, SIGNAL( clicked() ), this, SLOT( slotReject() ) );

  QObject::connect( ui_->cmbpunch, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotPunchCurrentIndexChanged( int ) ) );

  QObject::connect( ui_->btnclrmin, SIGNAL( clicked() ), this, SLOT( slotBtnMinColorClicked() ) );
  QObject::connect( ui_->btnclrmax, SIGNAL( clicked() ), this, SLOT( slotBtnMaxColorClicked() ) );
  QObject::connect( ui_->btngradient, SIGNAL( clicked() ), this, SLOT( slotBtnGradientClicked() ) );

  QObject::connect( ui_->minspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotMinChanged( int ) ) );
  QObject::connect( ui_->maxspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotMaxChanged( int ) ) );
  QObject::connect( ui_->stepspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotStepChanged( int ) ) );
  QObject::connect( ui_->emptyspin, SIGNAL( valueChanged( int ) ), this, SLOT( slotEmptyChanged( int ) ) );
}

void RadarSettings::loadPunches()
{
  punches_.clear();
//  ui_->cmbpunch->clear();
  punches_ = WeatherLoader::instance()->punchlibraryspecial();
  QMapIterator< QString, puanson::proto::Puanson > it(punches_);
  while ( true == it.hasNext() ) {
    it.next();
    ui_->cmbpunch->addItem( QString::fromStdString( it.value().name() ) );
  }
}

void RadarSettings::showSymbol()
{
  setupGradientFromProto();
  QTreeWidgetItem* item = currentItem();
  if ( 0 != item ) {
    proto::RadarColor clr = fromItem(item);
    QString code = QString::fromStdString( clr.punch_id() );
    if ( true == punches_.contains(code) ) {
      QString title = QString::fromStdString( punches_[code].name() );
      int indx = ui_->cmbpunch->findText(title);
      if ( -1 != indx ) {
        ui_->cmbpunch->setCurrentIndex(indx);
      }
    }
  }
  setupLegendaFromProto();
}

void RadarSettings::showGradient()
{
  setupGradientFromProto();
  QTreeWidgetItem* item = currentItem();
  if ( 0 != item ) {
    proto::RadarColor clr = fromItem(item);
    if ( true == clr.has_punch_id() ) {
      QString code = QString::fromStdString( clr.punch_id() );
      if ( true == punches_.contains(code) ) {
        QString title = QString::fromStdString( punches_[code].name() );
        int indx = ui_->cmbpunch->findText(title);
        if ( -1 != indx ) {
          ui_->cmbpunch->setCurrentIndex(indx);
        }
      }
    }
    else {
      ui_->cmbpunch->setCurrentIndex(0);
    }
  }
  setupLegendaFromProto();
}

void RadarSettings::loadSettings()
{
  proto::RadarColors proto = params_.protoParams();
  for ( int i = 0, sz = proto.color_size(); i < sz; ++i ) {
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->radartypes );
    setupItem( item, proto.color(i) );
  }
}

void RadarSettings::saveSettings()
{
  params_.setParams( currentParams() );
  params_.saveParams();
}

QTreeWidgetItem* RadarSettings::currentItem()
{
  QTreeWidgetItem* item = 0;
  if ( 0 != ui_->radartypes->selectedItems().size() ) {
    item = ui_->radartypes->selectedItems()[0];
  }
  return item;
}

proto::RadarColors RadarSettings::currentParams()
{
  proto::RadarColors clrs;
  for ( int i = 0, sz = ui_->radartypes->topLevelItemCount(); i < sz; ++i ) {
    proto::RadarColor* clr = clrs.add_color();
    clr->CopyFrom( fromItem( ui_->radartypes->topLevelItem(i) ) );
  }
  return clrs;
}

proto::RadarColor RadarSettings::fromItem( QTreeWidgetItem* item)
{
  QByteArray arr(item->data( 0, Qt::UserRole ).toByteArray() );
  proto::RadarColor rclr;
  if ( 0 != arr.size() ) {
    rclr.ParseFromString( std::string( arr.data(), arr.size() ) );
  }
  else {
    rclr.set_descr( item->text(kColumnDescr).toInt() );
    rclr.set_name( item->text(kColumnName).toStdString() );
    rclr.set_description( item->text(kColumnOpis).toStdString() );
    setItemData( rclr, item );
  }
  return rclr;
}

void RadarSettings::setItemData( const proto::RadarColor& clr, QTreeWidgetItem* item )
{
  muteSignals();
  std::string str = clr.SerializeAsString();
  item->setData( kColumnDescr, Qt::UserRole, QByteArray( str.data(), str.size() ) );
  turnSignals();
}

void RadarSettings::setupItem( QTreeWidgetItem* item, const proto::RadarColor& clr ) 
{
  item->setText( kColumnDescr, QString::number( clr.descr() ) );
  item->setText( kColumnName, QString::fromStdString( clr.name() ) );
  item->setText( kColumnOpis, QString::fromStdString( clr.description() ) );
  setItemData( clr, item );
}

void RadarSettings::setupProtoFromGradientControls( QTreeWidgetItem* item )
{
  if ( 0 == item ) {
    item = currentItem();
  }
  if ( 0 == item ) {
    return;
  }
  proto::RadarColor clr = fromItem(item);
  QColor minclr = ui_->btnclrmin->palette().color( QPalette::Background );
  QColor maxclr = ui_->btnclrmax->palette().color( QPalette::Background );

  double val = ui_->minspin->value();
  double max = ui_->maxspin->value();
  int step = ui_->stepspin->value();
  TColorGrad grad( val, max, minclr, maxclr);
  TColorGradList list;

  if ( 0 != step ) {
    while ( val + step < max) {
      list.append( TColorGrad( val, val+step, grad.color(val), grad.color(val) ) );
      val += step;
    }
  }
  list.append( TColorGrad( val, max, grad.endcolor(), grad.endcolor() ) );
  proto::RadarColor newclr = RadarParams::gradlist2gradientcolor(list);
  clr.mutable_gradient()->CopyFrom(newclr.gradient());
  clr.set_step(step);
  setItemData( clr, item );
}

void RadarSettings::setupGradientFromProto()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    ui_->minspin->setValue(0);
    ui_->maxspin->setValue(1000);
    ui_->stepspin->setValue(100);
    setColorMin( Qt::black );
    setColorMax( Qt::black );
    return;
  }
  muteSignals();
  proto::RadarColor clr = fromItem(item);
  TColorGradList list = RadarParams::gradParams(clr);
  setColorMin( list.begcolor() );
  setColorMax( list.endcolor() );
  ui_->minspin->setValue( list.begval() );
  ui_->maxspin->setValue( list.endval() );
  ui_->stepspin->setValue( clr.step() );
  turnSignals();
}

void RadarSettings::setupLegendaFromProto()
{
  ui_->znakilbl->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    ui_->znakilbl->setWordWrap(true);
    ui_->znakilbl->setText( QObject::tr("Нет знаков для отображения") );
    ui_->znakilbl->adjustSize();
    return;
  }
  proto::RadarColor clr = fromItem(item);
  if ( 0 == ui_->cmbpunch->currentIndex() || false == clr.has_punch_id() ) {
    ui_->znakilbl->setWordWrap(true);
    ui_->znakilbl->setText( QObject::tr("Нет знаков для отображения") );
    ui_->znakilbl->adjustSize();
    return;
  }
  QString clrname = QString::fromStdString( clr.name() );
  QString punch_id = QString::fromStdString( clr.punch_id() );
  if ( false == punches_.contains(punch_id) ) {
    ui_->znakilbl->setWordWrap(true);
    ui_->znakilbl->setText( QObject::tr("Не найден шаблон наноски  с идентификатором '%1'").arg(punch_id) );
    ui_->znakilbl->adjustSize();
    return;
  }
  const puanson::proto::Puanson& punch = punches_[punch_id];
  puanson::proto::CellRule rule;
  bool ok = false;
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    if ( punch.rule(i).id().descr() == clr.descr() ) {
      ok = true;
      rule.CopyFrom( punch.rule(i) );
      break;
    }
  }
  if ( false == ok ) {
    ui_->znakilbl->setWordWrap(true);
    ui_->znakilbl->setText( QObject::tr("В шаблоне наноски на найдено правило для отображения '%1'").arg(clrname) );
    ui_->znakilbl->adjustSize();
    return;
  }
  if ( puanson::proto::kNumber == rule.type() ) {
    QString pattern = QObject::tr("Пример отображения: %1").arg( stringFromRuleValue( 12, rule ) );
    ui_->znakilbl->setText(pattern);
  }
  else if ( puanson::proto::kSymbol == rule.type() ) {
    QPixmap pix(ui_->znakilbl->size());
    pix.fill( QColor(0,0,0,0) );
    QPainter pntr(&pix);
    const puanson::proto::CellRule::SymbolRule& symbol = rule.symbol();
    QPoint topleft(0,0);
    int maxval_w = 0;
    int maxznk_w = 0;
    int maxval_h = 0;
    int maxznk_h = 0;
    for ( int i = 0, sz = symbol.symbol_size(); i < sz; ++i ) {
      const puanson::proto::CellRule::Symbol& smb = symbol.symbol(i);
      QString str = stringFromRuleValue( smb.minval(), symbol );
      QFont fnt = fontFromRuleValue( smb.minval(), symbol );
      QColor qclr =  colorFromRuleValue( smb.minval(), symbol );
      QString valstr;
      if ( false == smb.has_label() ) { 
        valstr = QObject::tr(" - %1").arg( smb.minval() );
        if ( false == MnMath::isEqual( smb.minval(), smb.maxval() ) ) {
            valstr += QObject::tr(":%1").arg( smb.maxval() );
        }
      }
      else {
        valstr = QObject::tr(" - %1").arg( QString::fromStdString( smb.label() ) );
      }

      QRect valrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, valstr );
      pntr.save();
      pntr.setFont(fnt);
      pntr.setPen(qclr);
      QRect znakrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
      pntr.restore();
      if ( maxval_w < valrect.width() ) {
        maxval_w = valrect.width();
      }
      if ( maxval_h < valrect.height() ) {
        maxval_h = valrect.height();
      }
      if ( maxznk_w < znakrect.width() ) {
        maxznk_w = znakrect.width();
      }
      if ( maxznk_h < znakrect.height() ) {
        maxznk_h = znakrect.height();
      }
    }
    int maxh = maxznk_h;
    if ( maxval_h > maxh ) {
      maxh = maxval_h;
    }
    QRect znakrect = QRect( 0, 0, maxznk_w, maxh );
    QRect valrect = QRect( 0, 0, maxval_w, maxh );
    for ( int i = 0, sz = symbol.symbol_size(); i < sz; ++i ) {
      const puanson::proto::CellRule::Symbol& smb = symbol.symbol(i);
      QString str = stringFromRuleValue( smb.minval(), symbol );
      QFont fnt = fontFromRuleValue( smb.minval(), symbol );
      QColor qclr =  colorFromRuleValue( smb.minval(), symbol );
      QString valstr;
      if ( false == smb.has_label() ) { 
        valstr = QObject::tr(" - %1").arg( smb.minval() );
        if ( false == MnMath::isEqual( smb.minval(), smb.maxval() ) ) {
            valstr += QObject::tr(":%1").arg( smb.maxval() );
        }
      }
      else {
        valstr = QObject::tr(" - %1").arg( QString::fromStdString( smb.label() ) );
      }
      int hgt = topleft.y() + maxh;
      if ( hgt > ui_->znakilbl->height() ) {
        topleft = QPoint( topleft.x() + maxval_w + maxznk_w, 0 );
      }
      pntr.save();
      pntr.setFont(fnt);
      pntr.setPen(qclr);
      znakrect.moveTo( topleft + QPoint( 0, 0 ) );
      pntr.drawText( znakrect, Qt::AlignHCenter | Qt::AlignBottom, str );
      pntr.restore();
      valrect.moveTo( topleft + QPoint( maxznk_w, 0 ) );
      pntr.drawText( valrect, Qt::AlignLeft | Qt::AlignBottom, valstr );
      pntr.save();
      pntr.setPen(Qt::gray);
      pntr.drawRect( QRect( znakrect.topLeft(), QSize( maxznk_w + maxval_w, maxh ) ) );
      pntr.restore();
      topleft += QPoint( 0, maxh );
    }
    ui_->znakilbl->setPixmap(pix);
  }
}

QColor RadarSettings::minColor()
{
  return ui_->btnclrmin->palette().color(QPalette::Background);
}

QColor RadarSettings::maxColor()
{
  return ui_->btnclrmax->palette().color(QPalette::Background);
}

void RadarSettings::setColorMin( const QColor& clr )
{
  ui_->btnclrmin->setStyleSheet(QString("background: %1").arg( clr.name() ) );
  QPalette plt = ui_->btnclrmin->palette();
  plt.setColor( QPalette::Background, clr );
  ui_->btnclrmin->setPalette(plt);
}

void RadarSettings::setColorMax( const QColor& clr )
{
  ui_->btnclrmax->setStyleSheet(QString("background: %1").arg( clr.name() ) );
  QPalette plt = ui_->btnclrmax->palette();
  plt.setColor( QPalette::Background, clr );
  ui_->btnclrmax->setPalette(plt);
}

AddRadarDescr::AddRadarDescr( RadarSettings* p )
  : QDialog(p),
  ui_( new Ui::AddRadarDescr ),
  params_(p)
{
  ui_->setupUi(this);
  QObject::connect( ui_->indexTree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  loadLastDescriptors();
  if ( 0 != ui_->indexTree->topLevelItemCount() ) {
    ui_->indexTree->topLevelItem(0)->setSelected(true);
  }
}

AddRadarDescr::~AddRadarDescr()
{
}

int AddRadarDescr::descr() const
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return -1;
  }
  return item->text(0).toInt();
}

QString AddRadarDescr::name() const
{
  if ( true == ui_->nameEdit->text().isEmpty() ) {
    warning_log << QObject::tr("Имя не указано. Будет использовано описание величины");
    return description();
  }
  return ui_->nameEdit->text();
}

QString AddRadarDescr::description() const
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return QObject::tr("Не выбран вид данных");
  }
  return item->text(1);
}

void AddRadarDescr::keyReleaseEvent( QKeyEvent* e )
{
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    accept();
    return;
  }
  QDialog::keyReleaseEvent(e);
}


void AddRadarDescr::loadLastDescriptors()
{
  QSettings setts( global::kLastRadarDescriptors, QSettings::IniFormat );
  if ( false == setts.contains("descriptors") ) {
    debug_log<<"loadDescriptorsFromSrcData НЕ ОПРЕДЕЛЕНА!";
    return;
  }
  QStringList descrlist = setts.value("descriptors").toStringList();
  loadDescriptors(descrlist);
}

void AddRadarDescr::saveLastdescriptors()
{
  QStringList list;
  QSettings setts( global::kLastRadarDescriptors, QSettings::IniFormat );
  for ( int i = 0, sz = ui_->indexTree->topLevelItemCount(); i < sz; ++i ) {
    list << ui_->indexTree->topLevelItem(i)->text(0);
  }
  setts.setValue("descriptors", list );
  setts.sync();
}

void AddRadarDescr::loadDescriptors( const QStringList& descrlist )
{
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  for ( int i = 0, sz = descrlist.size(); i < sz; ++i ) {
    if ( 0 != params_->radartypes()->findItems( descrlist[i], Qt::MatchExactly, kColumnDescr ).size() ) {
      continue;
    }
    int descr = descrlist[i].toInt();
    QString name("Неизвестный дескриптор");
    if ( false == md->isExist(descr) ) {
      error_log << QObject::tr("Дескриптор %1 не существует").arg(descr);
    }
    else {
      const meteodescr::Property& p = md->property(descr);
      name = p.description;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->indexTree );
    QSize qsz = item->sizeHint(0);
    qsz.setHeight(32);
    item->setSizeHint( 0, qsz );
    item->setText( 0, QString::number(descr) );
    item->setText( 1, name );
  }
}

void AddRadarDescr::accept()
{
  if ( 0 == ui_->indexTree->topLevelItemCount() ) {
    QDialog::accept();
    return;
  }
  if ( 0 == currentItem() ) {
    error_log.msgBox() << QObject::tr("Чтобы добавить вид данных, необходимо выбрать один элемент списка.");
    return;
  }
  QDialog::accept();
}
    
void AddRadarDescr::slotItemDoubleClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED(item);
  Q_UNUSED(column);
  accept();
}

QTreeWidgetItem* AddRadarDescr::currentItem() const
{
  QTreeWidgetItem* item = 0;
  if ( 0 != ui_->indexTree->selectedItems().size() ) {
    item = ui_->indexTree->selectedItems()[0];
  }
  return item;
}

}
}

