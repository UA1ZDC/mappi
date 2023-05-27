#include "colorbtn.h"

#include <meteo/commons/global/common.h>
#include <cross-commons/debug/tlog.h>
#include "colordlg.h"

ColorButton::ColorButton( QWidget* p )
  : QToolButton(p),
  rgb_( 0xFF000000 )
{
  QWidget::setAutoFillBackground(true);
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );
  setColor(rgb_);
}

ColorButton::~ColorButton()
{
}

void ColorButton::setColor( QRgb rgb )
{
  QColor clr = QColor::fromRgba(rgb);
  QPalette pal = palette();
  pal.setColor( QPalette::Button, clr );
  QToolButton::setPalette(pal);
  QToolButton::setStyleSheet(
      QString("background-color: rgb(%1,%2,%3);")
      .arg( clr.red() )
      .arg( clr.green() )
      .arg( clr.blue() )
      );
  if ( rgb_ == rgb ) {
    return;
  }
  rgb_ = rgb;
  emit colorChanged(rgb_);
}

void ColorButton::slotClicked()
{
//  ColorDlg* dlg = new ColorDlg(this);
  ColorDlg* dlg = new ColorDlg;
  dlg->move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( 0, height() ) );
  dlg->slotSetColor(rgb_);
  int res = dlg->exec();
  QRgb clr = dlg->currentColor();
  if ( QDialog::Accepted == res ) {
    setColor(clr);
  }
  delete dlg;
}
