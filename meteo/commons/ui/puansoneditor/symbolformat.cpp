#include "symbolformat.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "ui_symbolformat.h"

#include "dlgspinbox.h"
#include "addsymbol.h"

namespace meteo {
namespace puanson {

enum {
  kMinValClmn   = 0,
  kMaxValClmn   = 1,
  kCodValClmn   = 2,
  kTexValClmn   = 3
};

SymbolFormat::SymbolFormat( QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_( new Ui::SymbolFormat ),
  type_( proto::kSymbol )
{
  ui_->setupUi(this);
  connectSlots();
}

SymbolFormat::~SymbolFormat()
{
  delete ui_; ui_ = 0;
}

void SymbolFormat::setSymbolRule( proto::CellRule::SymbolRule* r, int a, double val )
{
  Q_UNUSED(a);
  rule_.CopyFrom(*r);
  testval_ = val;

  setUiFromProto();
}

void SymbolFormat::setSwitchButtonText( const QString& txt, const QFont& fnt )
{
  ui_->btnswitch2signview->setText(txt);
  ui_->btnswitch2signview->setFont(fnt);
}

void SymbolFormat::connectSlots()
{
  QObject::connect( ui_->btnswitch2signview, SIGNAL( clicked() ), this, SLOT( slotSwitch2Signview() ) );
  QObject::connect( ui_->btnaddrange, SIGNAL( clicked() ), this, SLOT( slotBtnAddRange() ) );
  QObject::connect( ui_->btnrmrange, SIGNAL( clicked() ), this, SLOT( slotBtnRmRange() ) );
  QObject::connect( ui_->lbltest, SIGNAL( doubleclick() ), this, SLOT( slotLblTestDoubleClicked() ) );
  QObject::connect( ui_->btnok, SIGNAL( clicked() ), this, SLOT( slotBtnOk() ) );
  QObject::connect( ui_->btnno, SIGNAL( clicked() ), this, SLOT( slotBtnNo() ) );
  QObject::connect( ui_->treeranges, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  QObject::connect( ui_->codedigitbox, SIGNAL( stateChanged( int ) ), this, SLOT( slotCodeDigitStateChanged( int ) ) );
}

void SymbolFormat::disconnectSlots()
{
  QObject::disconnect( ui_->btnswitch2signview, SIGNAL( clicked() ), this, SLOT( slotSwitch2Signview() ) );
  QObject::disconnect( ui_->btnaddrange, SIGNAL( clicked() ), this, SLOT( slotBtnAddRange() ) );
  QObject::disconnect( ui_->btnrmrange, SIGNAL( clicked() ), this, SLOT( slotBtnRmRange() ) );
  QObject::disconnect( ui_->lbltest, SIGNAL( doubleclick() ), this, SLOT( slotLblTestDoubleClicked() ) );
  QObject::disconnect( ui_->btnok, SIGNAL( clicked() ), this, SLOT( slotBtnOk() ) );
  QObject::disconnect( ui_->btnno, SIGNAL( clicked() ), this, SLOT( slotBtnNo() ) );
  QObject::disconnect( ui_->treeranges, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  QObject::disconnect( ui_->codedigitbox, SIGNAL( stateChanged( int ) ), this, SLOT( slotCodeDigitStateChanged( int ) ) );
}

void SymbolFormat::setUiFromProto()
{
  disconnectSlots();
  QString str = meteo::map::stringFromRuleValue( testval_, rule_ );
  QFont fnt = meteo::map::fontFromRuleValue( testval_, rule_ );
  QColor clr = meteo::map::colorFromRuleValue( testval_, rule_ );
  QPalette pal = ui_->lbltest->palette();
  pal.setColor( QPalette::WindowText, clr );
  ui_->lbltest->setPalette(pal);
  ui_->lbltest->setFont(fnt);
  ui_->lbltest->setText(str);
  if ( true == rule_.codedigit() ) {
    ui_->codedigitbox->setChecked(true);
  }
  else {
    ui_->codedigitbox->setChecked(false);
  }

  ui_->treeranges->clear();
  for ( int i = 0, sz = rule_.symbol_size(); i < sz; ++i ) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeranges);
    setupTreeWidgetItem( item, rule_.symbol(i));
  }

  connectSlots();
}

void SymbolFormat::setProtoFromUi()
{
  if ( Qt::Checked == ui_->codedigitbox->checkState() ) {
    rule_.set_codedigit(true);
  }
  else {
    rule_.set_codedigit(false);
  }
}

void SymbolFormat::slotSwitch2Signview()
{
  type_ = proto::kNumber;
  accept();
}

void SymbolFormat::slotBtnAddRange()
{
  AddSymbol* dlg = new AddSymbol(this);
  QObject::connect( dlg, SIGNAL( rangeReady( bool*, double*, double* ) ), this, SLOT( slotRangeReady( bool*, double*, double* ) ) );
  double min = 0;
  proto::CellRule::Symbol newsmb;
  for ( int i = 0, sz = rule_.symbol_size(); i < sz; ++i ) {
    newsmb.CopyFrom(rule_.symbol(i));
    if ( rule_.symbol(i).maxval() > min ) {
      min = rule_.symbol(i).maxval()+1;
    }
  }
  newsmb.set_minval(min);
  newsmb.set_maxval(min+9);
  dlg->setSymbol(newsmb);
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    newsmb.CopyFrom(dlg->symbol());
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treeranges );
    setupTreeWidgetItem( item, newsmb );
    proto::CellRule::Symbol* smb = rule_.add_symbol();
    smb->CopyFrom(newsmb);
  }
  delete dlg;
}

void SymbolFormat::setupTreeWidgetItem( QTreeWidgetItem* item, const proto::CellRule::Symbol& symbol )
{
  item->setSizeHint( 2, QSize(32,32) );
  item->setText( kMinValClmn, QString::number( symbol.minval(), 'f', 2 ) );
  item->setText( kMaxValClmn, QString::number( symbol.maxval(), 'f', 2 ) );
  item->setText( kCodValClmn, QString::fromStdString( symbol.codeval() ) );
  item->setFont( kTexValClmn, font2qfont(symbol.font()) );

  QByteArray arr = QByteArray::fromHex( QByteArray( symbol.patternba().data(), symbol.patternba().size() ) );
  item->setText( kTexValClmn, arr );
  item->setForeground( kTexValClmn, QBrush( QColor::fromRgba( symbol.color() ) ) );
}

void SymbolFormat::slotBtnRmRange()
{
  QTreeWidgetItem* item = ui_->treeranges->currentItem();
  if ( 0 == item ) {
    return;
  }
  int indx = ui_->treeranges->indexOfTopLevelItem(item);
  for  ( int i = indx + 1, sz = rule_.symbol_size(); i < sz; ++i ) {
    rule_.mutable_symbol( i - 1 )->CopyFrom( rule_.symbol(i) );
  }
  rule_.mutable_symbol()->RemoveLast();
  delete item;
}

void SymbolFormat::slotItemDoubleClicked( QTreeWidgetItem* item, int clmn )
{
  Q_UNUSED(clmn);
  if ( 0 == item ) {
    return;
  }
  int indx = ui_->treeranges->indexOfTopLevelItem(item);
  if ( 0 > indx || rule_.symbol_size() <= indx ) {
    error_log << QObject::tr("Непонятная ошибка");
    return;
  }
  AddSymbol* dlg = new AddSymbol(this);
  QObject::connect( dlg, SIGNAL( rangeReady( bool*, double*, double* ) ), this, SLOT( slotRangeReady( bool*, double*, double* ) ) );
  dlg->setSymbol(rule_.symbol(indx));
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    int indx = ui_->treeranges->indexOfTopLevelItem(item);
    if ( -1 == indx ) {
      error_log << QObject::tr("Неизвестная ошибка");
    }
    else {
      int sz = rule_.symbol_size();
      if ( indx < sz ) {
        rule_.mutable_symbol(indx)->CopyFrom( dlg->symbol() );
      }
    }
    setupTreeWidgetItem( item, dlg->symbol() );
    setUiFromProto();
  }
  delete dlg;
}

void SymbolFormat::slotLblTestDoubleClicked()
{
  DlgSpinbox* dlg = new DlgSpinbox( ui_->lbltest );
  dlg->setValue(testval_);
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    testval_ = dlg->value();
    setUiFromProto();
  }
}

void SymbolFormat::slotFontParamChanged( const QFont& font, int align )
{
  Q_UNUSED(font);
  Q_UNUSED(align);
  setProtoFromUi();
}

void SymbolFormat::slotCodeDigitStateChanged( int state )
{
  Q_UNUSED(state);
  setProtoFromUi();
}


void SymbolFormat::slotBtnOk()
{
  QDialog::accept();
}

void SymbolFormat::slotBtnNo()
{
  QDialog::reject();
}

void SymbolFormat::slotRangeReady( bool* valid, double* confl_min, double* confl_max )
{
  AddSymbol* dlg = qobject_cast<AddSymbol*>( sender() );
  if ( 0 == dlg ) {
    return;
  }
  if ( 0 != valid ) {
    *valid = true;
    return;
  }
  return;
  double minval = dlg->min();
  double maxval = dlg->max();
  for ( int i = 0, sz = rule_.symbol_size(); i < sz; ++i ) {
    double min = rule_.symbol(i).minval();
    double max = rule_.symbol(i).maxval();
    if ( min <= minval && max >= minval ) {
      *valid = false;
    }
    else if ( min <= maxval && max >= maxval ) {
      *valid = false;
    }
    else if ( min < minval && max >= maxval ) {
      *valid = false;
    }
    else if ( min > minval && max < maxval ) {
      *valid = false;
    }
    if ( false == *valid ) {
      *confl_min = min;
      *confl_max = max;
      break;
    }
  }
}

}
}
