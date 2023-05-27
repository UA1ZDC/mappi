#include "colordlg.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcolordialog.h>

ColorDlg::ColorDlg(QWidget* parent)
  : QDialog(parent)
{
  setWindowFlags(Qt::Popup);

  QGridLayout* l = new QGridLayout(this);
  l->setMargin(1);
  l->setSpacing(1);

  for ( int i=0; i<10; ++i ) {
    for ( int j=0; j<10; ++j ) {
      QColor clr = QColor( QRgb( 167772*(i*10+j) ) );

      QToolButton* btn = new QToolButton(this);
      btn->setMinimumSize(QSize(15,15));
      btn->setMaximumSize(QSize(15,15));
      btn->setAutoRaise(true);
      btn->setStyleSheet(
          QString("border: 1px solid #999999; background-color: rgb(%1,%2,%3);")
          .arg( clr.red() )
          .arg( clr.green() )
          .arg( clr.blue() )
          );
      buttons_.append(btn);

      l->addWidget(btn, j, i);
      QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(slotChangeColor()) );
      QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(accept()) );
    }
  }
  QToolButton* btn = new QToolButton(this);
  btn->setMinimumSize(QSize(15*10,15));
  btn->setMaximumSize(QSize(15*10,15));
  btn->setStyleSheet("border: 1px solid #999999;");
  btn->setText( QObject::tr("Вся палитра") );
  btn->setAutoRaise(true);
  l->addWidget( btn, 10, 0, 1, 10, Qt::AlignHCenter );
  QObject::connect( btn, SIGNAL( clicked() ), this, SLOT( slotCustomColor() ) );
}

void ColorDlg::slotSetColor( QRgb r )
{
  rgb_ = r;
}

void ColorDlg::slotChangeColor()
{
  QToolButton* btn = qobject_cast<QToolButton*>( sender() );
  if ( 0 == btn ) {
    return;
  }

  rgb_ = btn->palette().color( QPalette::Button ).rgba();
}

void ColorDlg::slotCustomColor()
{
  QColorDialog* dlg = new QColorDialog( QColor(rgb_), this );
  dlg->setOption( QColorDialog::ShowAlphaChannel, true );
  int res = dlg->exec();
  if ( QDialog::Accepted != res ) {
    delete dlg;
    return;
  }
  QRgb clr = dlg->selectedColor().rgba();
  delete dlg;
  slotSetColor( clr );
  accept();
}
