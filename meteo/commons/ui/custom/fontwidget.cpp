#include "fontwidget.h"

#include <qfontdatabase.h>
#include <qpainter.h>
#include <qmenu.h>

#include "ui_fontwidget.h"

#include "fontcombobox.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/fonts/weatherfont.h>

FontWidget::FontWidget( QWidget* p, Qt::WindowFlags fl )
  : QWidget( p, fl ),
  ui_( new Ui::FontWidget ),
  align_(meteo::kTopRight),
 specvisible_(true),
 symbolvisible_(true),
 colorvisible_(true),
 alignvisible_(true),
 boldvisible_(true),
 italicvisible_(true),
 underlinevisible_(true),
 strikevisible_(true)
{
  ui_->setupUi(this);

  ui_->cmbfamily->setItemDelegate( new FontItemDelegate(this) );

  connectSlots();

  reloadSpecialFonts();
}

FontWidget::~FontWidget()
{
  delete ui_; ui_ = 0;
}

void FontWidget::hideAlign()
{
  ui_->btnalign->hide();
}

void FontWidget::showAlign()
{
  ui_->btnalign->show();
}

void FontWidget::hideSymbol()
{
  ui_->btnsymbol->hide();
}

void FontWidget::showSymbol()
{
  ui_->btnsymbol->show();
}

void FontWidget::hideColor()
{
  ui_->btncolor->hide();
}

void FontWidget::showColor()
{
  ui_->btncolor->show();
}
    
void FontWidget::setSpecFontEnabled( bool fl )
{
    ui_->btnspecial->setChecked(fl);
}

QMap<meteo::Position, bool> FontWidget::allowedAligns()
{
    return ui_->btnalign->allowedAligns();
}

void FontWidget::setSpecialVisible( bool fl )
{
  ui_->btnspecial->setVisible(fl);
  specvisible_ = fl;
  if ( false == fl ) {
    setSpecFontEnabled(fl);
  }
}

void FontWidget::setSymbolVisible( bool fl )
{
  ui_->btnsymbol->setVisible(fl);
  symbolvisible_ = fl;
}

void FontWidget::setColorVisible( bool fl )
{
  ui_->btncolor->setVisible(fl);
  colorvisible_ = fl;
}

void FontWidget::setAlignVisible( bool fl )
{
  ui_->btnalign->setVisible(fl);
  alignvisible_ = fl;
}

void FontWidget::setBoldVisible( bool fl )
{
  ui_->btnbold->setVisible(fl);
  boldvisible_ = fl;
}

void FontWidget::setItalicVisible( bool fl )
{
  ui_->btnitalic->setVisible(fl);
  italicvisible_ = fl;
}

void FontWidget::setUnderlineVisible( bool fl )
{
  ui_->btnunderline->setVisible(fl);
  underlinevisible_ = fl;
}

void FontWidget::setStrikeVisible( bool fl )
{
  ui_->btnstrikeout->setVisible(fl);
  strikevisible_ = fl;
}

void FontWidget::setAllowedAligns(QMap<meteo::Position, bool> aligns)
{
    ui_->btnalign->setAllowedAligns(aligns);
}

void FontWidget::setFont( const QFont& f, int a, QRgb clr )
{
  font_ = f;
  align_ = a;
  rgb_ = clr;
  setUiFromFont();
}

void FontWidget::reloadSpecialFonts()
{
  ui_->cmbfamily->clear();
  QStringList list;
  if ( false == ui_->btnspecial->isChecked() ) {
    QFontDatabase fd;
    list = fd.families();
  }
  else {
    list = meteo::WeatherFont::instance()->specialfonts().keys();
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    ui_->cmbfamily->addItem(list[i]);
  }
}

void FontWidget::setFontFromUi()
{
  disconnectSlots();

  QFont f;

  f.setFamily( ui_->cmbfamily->currentText() );
  f.setPointSize(ui_->btnsize->text().toInt());
  f.setBold( ui_->btnbold->isChecked() );
  f.setItalic( ui_->btnitalic->isChecked() );
  f.setUnderline( ui_->btnunderline->isChecked() );
  f.setStrikeOut( ui_->btnstrikeout->isChecked() );

  bool ch = false;
  bool fchange = false;
  bool clrchange = false;
  if ( f != font_ ) {
    font_ = f;
    fchange = true;
    ch = true;
  }
  bool achange = false;
  if ( ui_->btnalign->align() != align_ ) {
    achange = true;
    align_ = ui_->btnalign->align();
    ch = true;
  }
  if ( ui_->btncolor->color() != rgb_ ) {
    rgb_ = ui_->btncolor->color();
    clrchange = true;
    ch = true;
  }

  ui_->btnsymbol->setFont(font_);
  connectSlots();
  if ( true == achange ) {
    emit alignChanged( align_ );
  }
  if ( true == fchange ) {
    emit fontChanged(f);
  }
  if ( true == clrchange ) {
    emit colorChanged(rgb_);
  }
  if ( true == ch ) {
    emit paramChanged( font_, align_, rgb_ );
  }
}

void FontWidget::setUiFromFont()
{
  disconnectSlots();
  if ( false == meteo::WeatherFont::instance()->specialfonts().contains(font_.family()) ) {
    ui_->btnspecial->setChecked(false);
    reloadSpecialFonts();
  }
  int indx = ui_->cmbfamily->findText( font_.family() );
  if ( -1 == indx ) {
    warning_log << QObject::tr("Шрифт %1 не найден в базе шрифтов").arg(font_.family());
  }
  else {
    ui_->cmbfamily->setCurrentIndex(indx);
  }
  ui_->btnsize->setText( QString::number( font_.pointSize() ) );
  ui_->btnbold->setChecked( font_.bold() );
  ui_->btnitalic->setChecked( font_.italic() );
  ui_->btnunderline->setChecked( font_.underline() );
  ui_->btnstrikeout->setChecked( font_.strikeOut() );
  ui_->btnalign->setAlign( align_ );
  ui_->btnsymbol->setFont(font_);
  ui_->btncolor->setColor(rgb_);
  connectSlots();
}

void FontWidget::connectSlots()
{
  QObject::connect( ui_->cmbfamily, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbFamilyChanged( int ) ) );
  QObject::connect( ui_->btnspecial, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnSpecialToggled( bool ) ) );
  QObject::connect( ui_->btnsize, SIGNAL( clicked() ), this, SLOT( slotBtnSizeClicked() ) );
  QObject::connect( ui_->btnbold, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnBoldToggled( bool ) ) );
  QObject::connect( ui_->btnitalic, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnItalicToggled( bool ) ) );
  QObject::connect( ui_->btnunderline, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnUnderlineToggled( bool ) ) );
  QObject::connect( ui_->btnstrikeout, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnStrikeoutToggled( bool ) ) );
  QObject::connect( ui_->btnalign, SIGNAL( posChanged( int ) ), this, SLOT( slotBtnAlignPosChanged( int ) ) );
  QObject::connect( ui_->btnsymbol, SIGNAL( symbolChanged( const QChar& ) ), this, SLOT( slotBtnSymbolChanged( const QChar& ) ) );
  QObject::connect( ui_->btncolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotBtnColorChanged( QRgb ) ) );
}

void FontWidget::disconnectSlots()
{
  QObject::disconnect( ui_->cmbfamily, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotCmbFamilyChanged( int ) ) );
  QObject::disconnect( ui_->btnspecial, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnSpecialToggled( bool ) ) );
  QObject::disconnect( ui_->btnsize, SIGNAL( clicked() ), this, SLOT( slotBtnSizeClicked() ) );
  QObject::disconnect( ui_->btnbold, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnBoldToggled( bool ) ) );
  QObject::disconnect( ui_->btnitalic, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnItalicToggled( bool ) ) );
  QObject::disconnect( ui_->btnunderline, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnUnderlineToggled( bool ) ) );
  QObject::disconnect( ui_->btnstrikeout, SIGNAL( toggled( bool ) ), this, SLOT( slotBtnStrikeoutToggled( bool ) ) );
  QObject::disconnect( ui_->btnalign, SIGNAL( posChanged( int ) ), this, SLOT( slotBtnAlignPosChanged( int ) ) );
  QObject::disconnect( ui_->btnsymbol, SIGNAL( symbolChanged( const QChar& ) ), this, SLOT( slotBtnSymbolChanged( const QChar& ) ) );
  QObject::disconnect( ui_->btncolor, SIGNAL( colorChanged( QRgb ) ), this, SLOT( slotBtnColorChanged( QRgb ) ) );
}

void FontWidget::slotCmbFamilyChanged( int indx )
{
  Q_UNUSED(indx);
  setFontFromUi();
}

void FontWidget::slotBtnSpecialToggled( bool checked )
{
  Q_UNUSED(checked);
  reloadSpecialFonts();
}

void addSizeAction( int sz, QMenu* menu, const QFont& font )
{
  QAction* a = menu->addAction( QString::number(sz) );
  if ( sz == font.pointSize() ) {
    a->setChecked(true);
  }
}

void FontWidget::slotBtnSizeClicked()
{
  QMenu* menu = new QMenu(ui_->btnsize);
  addSizeAction(5 , menu, font_ );
  addSizeAction(6 , menu, font_ );
  addSizeAction(7 , menu, font_ );
  addSizeAction(8 , menu, font_ );
  addSizeAction(9 , menu, font_ );
  addSizeAction(10, menu, font_ );
  addSizeAction(11, menu, font_ );
  addSizeAction(12, menu, font_ );
  addSizeAction(13, menu, font_ );
  addSizeAction(14, menu, font_ );
  addSizeAction(15, menu, font_ );
  addSizeAction(16, menu, font_ );
  addSizeAction(18, menu, font_ );
  addSizeAction(20, menu, font_ );
  addSizeAction(22, menu, font_ );
  addSizeAction(24, menu, font_ );
  addSizeAction(26, menu, font_ );
  addSizeAction(28, menu, font_ );
  addSizeAction(32, menu, font_ );
  addSizeAction(36, menu, font_ );
  addSizeAction(40, menu, font_ );
  addSizeAction(44, menu, font_ );
  addSizeAction(48, menu, font_ );
  addSizeAction(54, menu, font_ );
  QAction* a = menu->exec( QWidget::mapToGlobal( QPoint(ui_->btnsize->geometry().topLeft().x(),0) ) + QPoint( 0, ui_->btnsize->height() ) );
  if ( 0 != a ) {
    ui_->btnsize->setText(a->text());
    setFontFromUi();
  }
}

void FontWidget::slotBtnBoldToggled( bool checked )
{
  Q_UNUSED(checked);
  setFontFromUi();
}

void FontWidget::slotBtnItalicToggled( bool checked )
{
  Q_UNUSED(checked);
  setFontFromUi();
}

void FontWidget::slotBtnUnderlineToggled( bool checked )
{
  Q_UNUSED(checked);
  setFontFromUi();
}

void FontWidget::slotBtnStrikeoutToggled( bool checked )
{
  Q_UNUSED(checked);
  setFontFromUi();
}

void FontWidget::slotBtnAlignPosChanged( int pos )
{
  Q_UNUSED(pos);
  setFontFromUi();
}

void FontWidget::slotBtnSymbolChanged( const QChar& smb )
{
  emit symbolChanged(smb);
}

void FontWidget::slotBtnColorChanged( QRgb rgb )
{
  rgb_ = rgb;
  emit colorChanged(rgb);
}
