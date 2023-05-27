#include "addsymbol.h"

#include <qevent.h>
#include <qvalidator.h>
#include <qmessagebox.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/puanson.h>

#include "ui_addsymbol.h"

#include "numformat.h"

namespace meteo {
namespace puanson {

AddSymbol::AddSymbol( QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_( new Ui::AddSymbol )
{
  ui_->setupUi(this);
//  setWindowFlags( windowFlags() | Qt::FramelessWindowHint );
  ui_->fontwidget->hideAlign();
  adjustSize();
  ui_->lemin->setValidator( new QDoubleValidator( -100000, 100000, 2, ui_->lemin ) );
  ui_->lemax->setValidator( new QDoubleValidator( -100000, 100000, 2, ui_->lemax ) );
  ui_->numformatbtn->setText( QChar(0x2116) );
  QFont fnt;
  fnt.setPointSize(16);
  ui_->numformatbtn->setFont(fnt);

  connectSlots();
}

AddSymbol::~AddSymbol()
{
  delete ui_; ui_ = 0;
}

void AddSymbol::setUiFromProto()
{
  disconnectSlots();
  QFont fnt = font2qfont(symbol_.font() );
  ui_->fontwidget->setFont(fnt, 0, symbol_.color() );
  ui_->letext->setFont(fnt);
  QPalette pal = ui_->letext->palette();
  pal.setColor( QPalette::Text, QColor::fromRgba( symbol_.color() ) );
  ui_->letext->setPalette(pal);
  QByteArray arr = QByteArray::fromHex( QByteArray( symbol_.patternba().data(), symbol_.patternba().size() ) );
  ui_->letext->setText( arr );
  ui_->lemin->setText( QString::number( symbol_.minval(), 'f', 2 ) );
  ui_->lemax->setText( QString::number( symbol_.maxval(), 'f', 2 ) );
  ui_->lelabel->setText( QString::fromStdString( symbol_.label() ) );
  if ( true == symbol_.has_codeval() ) {
    ui_->lecode->setText( QString::fromStdString( symbol_.codeval() ) );
  }


  if ( true == symbol_.has_number() ) {
    ui_->numformatclearbtn->setEnabled(true);
    ui_->lecode->setDisabled(true);
    ui_->letext->setDisabled(true);
    ui_->lelabel->setDisabled(true);
    number_.CopyFrom( symbol_.number() );
    has_number_ = true;
  }
  else {
    ui_->numformatclearbtn->setDisabled(true);
    ui_->lecode->setEnabled(true);
    ui_->letext->setEnabled(true);
    ui_->lelabel->setEnabled(true);
    number_.Clear();
    has_number_ = false;
  }

  connectSlots();
}

void AddSymbol::setProtoFromUi()
{
  symbol_.set_patternba( QString(ui_->letext->text().toUtf8().toHex()).toStdString() );
  symbol_.mutable_font()->CopyFrom( qfont2font( ui_->fontwidget->font() ) );
  symbol_.set_color( ui_->fontwidget->color() );
  symbol_.set_minval( ui_->lemin->text().toDouble() );
  symbol_.set_maxval( ui_->lemax->text().toDouble() );
  symbol_.set_label( ui_->lelabel->text().toStdString() );
  if ( 0 == ui_->lecode->text().simplified().size() ) {
    symbol_.clear_codeval();
  }
  else {
    symbol_.set_codeval( ui_->lecode->text().simplified().toStdString() );
  }
  if ( false == has_number_ ) {
    symbol_.clear_number();
  }
  else {
    symbol_.mutable_number()->CopyFrom(number_);
  }
}

void AddSymbol::accept()
{
  bool valid = true;
  double confl_min;
  double confl_max;
  emit rangeReady( &valid, &confl_min, &confl_max );
  if ( false == valid ) {
    int res = QMessageBox::warning( this,
        QObject::tr("Конфликт диапазонов"),
        QObject::tr("Диапазон в конфликте с дипазаоном min = %1 max = %2")
        .arg(confl_min)
        .arg(confl_max),
        QObject::tr("Продолжить редактирование"),
        QObject::tr("Закрыть редактор")
        );
    if ( 1 == res ) {
      QDialog::reject();
      return;
    }
    return;
  }
  QDialog::accept();
}

void AddSymbol::setMinMax( double mi, double ma )
{
  if ( mi > ma ) {
    warning_log << QObject::tr("Попытка установить неверный диапазон");
    mi = ma;
  }
  symbol_.set_minval(mi);
  symbol_.set_maxval(ma);
  setUiFromProto();
}

void AddSymbol::setMin( double m )
{
  if ( m == symbol_.minval() ) {
    return;
  }
  bool maxerr = false;
  if ( m > symbol_.maxval() ) {
    maxerr = true;
  }
  symbol_.set_minval(m);
  setUiFromProto();
  if ( true == maxerr ) {
    setMax(m);
  }
}

void AddSymbol::setMax( double m )
{
  if ( m == symbol_.maxval() ) {
    return;
  }
  bool minerr = false;
  if ( m < symbol_.minval() ) {
    minerr = true;
  }
  symbol_.set_maxval(m);
  setUiFromProto();
  if ( true == minerr ) {
    setMin(m);
  }
}

void AddSymbol::keyReleaseEvent( QKeyEvent* e )
{
  QDialog::keyReleaseEvent(e);
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    AddSymbol::accept();
  }
}

void AddSymbol::connectSlots()
{
  QObject::connect( ui_->fontwidget, SIGNAL( fontChanged( const QFont& ) ), this, SLOT( slotFontChanged( const QFont& ) ) );
  QObject::connect( ui_->fontwidget, SIGNAL( symbolChanged( const QChar& ) ), this, SLOT( slotSymbolChanged( const QChar& ) ) );
  QObject::connect( ui_->fontwidget, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotColorChanged( QRgb ) ) );
  QObject::connect( ui_->lemin, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotMinTextChanged( const QString& ) ) );
  QObject::connect( ui_->lemax, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotMaxTextChanged( const QString& ) ) );
  QObject::connect( ui_->letext, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( const QString& ) ) );
  QObject::connect( ui_->lelabel, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotLabelChanged( const QString& ) ) );
  QObject::connect( ui_->lemin, SIGNAL( textEdited( const QString& ) ), this, SLOT( slotLeminTextEdited( const QString& ) ) );
  QObject::connect( ui_->lemax, SIGNAL( textEdited( const QString& ) ), this, SLOT( slotLemaxTextEdited( const QString& ) ) );

  QObject::connect( ui_->numformatbtn, &QAbstractButton::clicked, this, &AddSymbol::slotSetNumbeRule );
  QObject::connect( ui_->numformatclearbtn, &QAbstractButton::clicked, this, &AddSymbol::slotClearNumbeRule );
}

void AddSymbol::disconnectSlots()
{
  QObject::disconnect( ui_->fontwidget, SIGNAL( fontChanged( const QFont& ) ), this, SLOT( slotFontChanged( const QFont& ) ) );
  QObject::disconnect( ui_->fontwidget, SIGNAL( symbolChanged( const QChar& ) ), this, SLOT( slotSymbolChanged( const QChar& ) ) );
  QObject::disconnect( ui_->fontwidget, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotColorChanged( QRgb ) ) );
  QObject::disconnect( ui_->lemin, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotMinTextChanged( const QString& ) ) );
  QObject::disconnect( ui_->lemax, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotMaxTextChanged( const QString& ) ) );
  QObject::disconnect( ui_->letext, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( const QString& ) ) );
  QObject::disconnect( ui_->lelabel, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotLabelChanged( const QString& ) ) );
  QObject::disconnect( ui_->lemin, SIGNAL( textEdited( const QString& ) ), this, SLOT( slotLeminTextEdited( const QString& ) ) );
  QObject::disconnect( ui_->lemax, SIGNAL( textEdited( const QString& ) ), this, SLOT( slotLemaxTextEdited( const QString& ) ) );

  QObject::disconnect( ui_->numformatbtn, &QAbstractButton::clicked, this, &AddSymbol::slotSetNumbeRule );
  QObject::disconnect( ui_->numformatclearbtn, &QAbstractButton::clicked, this, &AddSymbol::slotClearNumbeRule );
}

void AddSymbol::slotFontChanged( const QFont& f )
{
  Q_UNUSED(f);
  setProtoFromUi();
  setUiFromProto();
}

void AddSymbol::slotMinTextChanged( const QString& str )
{
  double m = str.toDouble();
  if ( m != min_ ) {
    setMin(m);
  }
  ui_->lemin->setCursorPosition(mincurpos_);
}

void AddSymbol::slotMaxTextChanged( const QString& str )
{
  double m = str.toDouble();
  if ( m != max_ ) {
    setMax(m);
  }
  ui_->lemax->setCursorPosition(maxcurpos_);
}

void AddSymbol::slotTextChanged( const QString& str )
{
  Q_UNUSED(str);
  setProtoFromUi();
}

void AddSymbol::slotLabelChanged( const QString& str )
{
  Q_UNUSED(str);
  setProtoFromUi();
}

void AddSymbol::slotSymbolChanged( const QChar& smb )
{
  QString str = ui_->letext->text() + smb;
  disconnectSlots();
  ui_->letext->setText(str);
  connectSlots();
  setProtoFromUi();
}

void AddSymbol::slotColorChanged( QRgb rgb )
{
  Q_UNUSED(rgb);
  setProtoFromUi();
  setUiFromProto();
}

void AddSymbol::slotLeminTextEdited( const QString& str )
{
  Q_UNUSED(str);
  mincurpos_ = ui_->lemin->cursorPosition();
}

void AddSymbol::slotLemaxTextEdited( const QString& str )
{
  Q_UNUSED(str);
  maxcurpos_ = ui_->lemax->cursorPosition();
}

void AddSymbol::slotSetNumbeRule()
{
  auto dlg = new NumFormat(this);
  dlg->setNumbeRule( &number_, kTopCenter, 0 );
  if ( QDialog::Accepted == dlg->exec() ) {
    number_.CopyFrom( dlg->numbeRule() );
    has_number_ = true;
    setProtoFromUi();
    setUiFromProto();
  }
  delete dlg;
}

void AddSymbol::slotClearNumbeRule()
{
  number_.Clear();
  has_number_ = false;
  setProtoFromUi();
  setUiFromProto();
}

}
}
