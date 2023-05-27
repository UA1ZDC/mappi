#include "numformat.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "ui_numformat.h"

#include "dlgspinbox.h"

namespace meteo {
namespace puanson {

NumFormat::NumFormat( QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_( new Ui::NumFormat ),
  type_( proto::kNumber )
{
  ui_->setupUi(this);
  ui_->fontwidget->hideSymbol();
  ui_->fontwidget->hideColor();
  connectSlots();
}

NumFormat::~NumFormat()
{
  delete ui_; ui_ = 0;
}

void NumFormat::setNumbeRule( proto::CellRule::NumbeRule* r, int a, double val )
{
  rule_.CopyFrom(*r);
  testval_ = val;
  const Font& f = rule_.font();
  QFont fnt = font2qfont(f);
  ui_->fontwidget->setFont( fnt, a );

  setUiFromProto();
}

void NumFormat::setSwitchButtonText( const QString& txt, const QFont& fnt )
{
  ui_->btnswitch2signview->setText(txt);
  ui_->btnswitch2signview->setFont(fnt);
}

int NumFormat::align() const
{
  return ui_->fontwidget->align();
}

void NumFormat::connectSlots()
{
  QObject::connect( ui_->btnswitch2signview, SIGNAL( clicked() ), this, SLOT( slotSwitch2Signview() ) );
  QObject::connect( ui_->cmbintcount, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbIntCount( int  ) ) );
  QObject::connect( ui_->cmbintmode, SIGNAL(  currentIndexChanged( int )  ), this, SLOT( slotCmbIntMode( int ) ) );
  QObject::connect( ui_->cmbfractcount, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbFractCount( int ) ) );
  QObject::connect( ui_->cmbsepmode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbSepMode( int ) ) );
  QObject::connect( ui_->cmbsignmode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbSignMode( int ) ) );
  QObject::connect( ui_->lbltest, SIGNAL( doubleclick() ), this, SLOT( slotLblTestDoubleClicked() ) );
  QObject::connect( ui_->btnok, SIGNAL( clicked() ), this, SLOT( slotBtnOk() ) );
  QObject::connect( ui_->btnno, SIGNAL( clicked() ), this, SLOT( slotBtnNo() ) );
  QObject::connect( ui_->fontwidget, SIGNAL( paramChanged( const QFont&, int, QRgb ) ), this, SLOT( slotFontParamChanged( const QFont&, int ) ) );
  QObject::connect( ui_->btnpluscolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotPlusColorChanged( QRgb ) ) );
  QObject::connect( ui_->btnminuscolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotMinusColorChanged( QRgb ) ) );
  QObject::connect( ui_->spinfactor, SIGNAL( valueChanged( double ) ), this, SLOT( slotSpinFactorChanged( double ) ) );
  QObject::connect( ui_->lepostfix, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPostfixChanged() ) );
  QObject::connect( ui_->checkzerohide, SIGNAL( toggled( bool ) ), this, SLOT( slotZeroHideToggled() ) );
}

void NumFormat::disconnectSlots()
{
  QObject::disconnect( ui_->btnswitch2signview, SIGNAL( clicked() ), this, SLOT( slotSwitch2Signview() ) );
  QObject::disconnect( ui_->cmbintcount, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbIntCount( int  ) ) );
  QObject::disconnect( ui_->cmbintmode, SIGNAL(  currentIndexChanged( int )  ), this, SLOT( slotCmbIntMode( int ) ) );
  QObject::disconnect( ui_->cmbfractcount, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbFractCount( int ) ) );
  QObject::disconnect( ui_->cmbsepmode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbSepMode( int ) ) );
  QObject::disconnect( ui_->cmbsignmode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbSignMode( int ) ) );
  QObject::disconnect( ui_->lbltest, SIGNAL( doubleclick() ), this, SLOT( slotLblTestDoubleClicked() ) );
  QObject::disconnect( ui_->btnok, SIGNAL( clicked() ), this, SLOT( slotBtnOk() ) );
  QObject::disconnect( ui_->btnno, SIGNAL( clicked() ), this, SLOT( slotBtnNo() ) );
  QObject::disconnect( ui_->fontwidget, SIGNAL( paramChanged( const QFont&, int, QRgb ) ), this, SLOT( slotFontParamChanged( const QFont&, int ) ) );
  QObject::disconnect( ui_->btnpluscolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotPlusColorChanged( QRgb ) ) );
  QObject::disconnect( ui_->btnminuscolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotMinusColorChanged( QRgb ) ) );
  QObject::disconnect( ui_->spinfactor, SIGNAL( valueChanged( double ) ), this, SLOT( slotSpinFactorChanged( double ) ) );
  QObject::disconnect( ui_->lepostfix, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPostfixChanged() ) );
  QObject::disconnect( ui_->checkzerohide, SIGNAL( toggled( bool ) ), this, SLOT( slotZeroHideToggled() ) );
}

void NumFormat::setUiFromProto()
{
  disconnectSlots();
  ui_->cmbintcount->setCurrentIndex( rule_.intcount() + 1 );
  if ( -1 == rule_.intcount() || proto::kNoCut == rule_.cutorder() ) {
    ui_->cmbintmode->setCurrentIndex(0);
    ui_->cmbintmode->hide();
  }
  else {
    ui_->cmbintmode->setCurrentIndex( rule_.cutorder() );
    ui_->cmbintmode->show();
  }
  ui_->cmbfractcount->setCurrentIndex( rule_.fracount() + 1 );
  ui_->cmbsepmode->setCurrentIndex( rule_.septype() );
  ui_->cmbsignmode->setCurrentIndex( rule_.signview() );

  ui_->btnpluscolor->setColor( rule_.pluscolor() );
  ui_->btnminuscolor->setColor( rule_.minuscolor() );

  QString str = map::stringFromRuleValue( testval_, rule_ );
  QFont fnt = map::fontFromRuleValue( testval_, rule_ );
  QColor clr = map::colorFromRuleValue( testval_, rule_ );
  QPalette pal = ui_->lbltest->palette();
  pal.setColor( QPalette::WindowText, clr );
  ui_->lbltest->setPalette(pal);
  ui_->lbltest->setFont(fnt);
  ui_->lbltest->setText(str);

  ui_->lepostfix->setFont(fnt);
  if ( true == rule_.zerohide() ) {
    ui_->checkzerohide->setCheckState( Qt::Checked );
  }
  else {
    ui_->checkzerohide->setCheckState( Qt::Unchecked);
  }

  ui_->spinfactor->setValue( rule_.factor() );
  if ( rule_.has_postfix() ) {
    ui_->lepostfix->setText( QString::fromStdString( rule_.postfix() ) );
  }

  connectSlots();
}

void NumFormat::setProtoFromUi()
{
  rule_.set_intcount( ui_->cmbintcount->currentIndex() - 1 );
  if ( -1 == rule_.intcount() ) {
    rule_.set_cutorder( proto::kNoCut );
  }
  else {
    rule_.set_cutorder( (proto::CutOrder)ui_->cmbintmode->currentIndex() );
  }
  rule_.set_fracount( ui_->cmbfractcount->currentIndex() - 1 );
  rule_.set_septype( (proto::SepType)ui_->cmbsepmode->currentIndex() );
  rule_.set_signview( (proto::SignView)ui_->cmbsignmode->currentIndex() );
  rule_.set_pluscolor( ui_->btnpluscolor->color() );
  rule_.set_minuscolor( ui_->btnminuscolor->color() );
  if ( Qt::Checked == ui_->checkzerohide->checkState() ) {
    rule_.set_zerohide(true);
  }
  else {
    rule_.set_zerohide(false);
  }
  const QFont& fnt = ui_->fontwidget->font();
  ui_->lbltest->setFont(fnt);
  Font proto = qfont2font(fnt);
  rule_.mutable_font()->CopyFrom(proto);
  rule_.set_factor( ui_->spinfactor->value() );
  if ( 0 == ui_->lepostfix->text().simplified().size() ) {
    rule_.clear_postfix();
  }
  else {
    rule_.set_postfix( ui_->lepostfix->text().toStdString() );
  }
}

void NumFormat::slotSwitch2Signview()
{
  type_ = proto::kSymbol;
  accept();
}

void NumFormat::slotCmbIntCount( int indx )
{
  Q_UNUSED(indx);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotCmbIntMode( int indx )
{
  Q_UNUSED(indx);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotCmbFractCount( int indx )
{
  Q_UNUSED(indx);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotCmbSepMode( int indx )
{
  Q_UNUSED(indx);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotCmbSignMode( int indx )
{
  Q_UNUSED(indx);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotPlusColorChanged( QRgb clr )
{
  Q_UNUSED(clr);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotMinusColorChanged( QRgb clr )
{
  Q_UNUSED(clr);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotSpinFactorChanged( double val )
{
  Q_UNUSED(val);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotLblTestDoubleClicked()
{
  DlgSpinbox* dlg = new DlgSpinbox( ui_->lbltest );
  dlg->setValue(testval_);
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    testval_ = dlg->value();
    setUiFromProto();
  }
}

void NumFormat::slotFontParamChanged( const QFont& font, int align )
{
  Q_UNUSED(font);
  Q_UNUSED(align);
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotPostfixChanged()
{
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotZeroHideToggled()
{
  setProtoFromUi();
  setUiFromProto();
}

void NumFormat::slotBtnOk()
{
  QDialog::accept();
}

void NumFormat::slotBtnNo()
{
  QDialog::reject();
}

}
}
