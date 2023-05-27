#include "symbolbtn.h"

#include <meteo/commons/global/common.h>
#include "symbdlg.h"

SymbolButton::SymbolButton( QWidget* p )
  : QToolButton(p),
  symbol_( QChar('A') )
{
  QWidget::setMinimumSize( QSize(32,32) );
  QWidget::setMaximumSize( QSize(32,32) );
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );
  setSymbol(symbol_);
}

SymbolButton::~SymbolButton()
{
}

void SymbolButton::setSymbol( const QChar& smb )
{
    if ( 38 == smb.toLatin1() ) {
      QToolButton::setText(QString(smb)+smb);
    }
    else {
      QToolButton::setText(smb);
    }
  if ( symbol_ == smb ) {
    return;
  }
  symbol_ = smb;
  emit symbolChanged(symbol_);
}

void SymbolButton::slotClicked()
{
  SymbDlg* dlg = new SymbDlg(this);
  dlg->move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( 0, height() ) );
  dlg->slotSetFont( font() );
  int res = dlg->exec();
  QChar smb = dlg->currentSymb();
  if ( QDialog::Accepted == res ) {
    setSymbol(smb);
  }
}
