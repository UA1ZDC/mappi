#include "symbdlg.h"

#include <qlayout.h>
#include <qtoolbutton.h>

SymbDlg::SymbDlg(QWidget* parent)
  : QDialog(parent)
{
  setWindowFlags(Qt::Popup);

  QGridLayout* l = new QGridLayout(this);
  l->setMargin(1);
  l->setSpacing(1);

  for ( int i=0; i<16; ++i ) {
    for ( int j=0; j<16; ++j ) {
      QToolButton* btn = new QToolButton(this);
      btn->setMinimumSize(QSize(30,30));
      btn->setMaximumSize(QSize(30,30));
      btn->setAutoRaise(true);
      btn->setStyleSheet("border: 1px solid #999999;");
      buttons_.append(btn);

      l->addWidget(btn, j, i);
    }
  }

  for ( int i=0; i<256; ++i ) {
    if ( 38 == i ) {
      buttons_[i]->setText(QString(QChar(i))+QChar(i));
    }
    else {
      buttons_[i]->setText(QChar(i));
    }
    connect( buttons_[i], SIGNAL(clicked(bool)), SLOT(slotChangeSymb()) );
    connect( buttons_[i], SIGNAL(clicked(bool)), SLOT(accept()) );
  }
}

void SymbDlg::slotSetFont(const QFont& font)
{
  QFont f = font;
  f.setPointSize(10);

  for ( int i=0; i<256; ++i ) {
    buttons_[i]->setFont(f);
  }
}

void SymbDlg::slotChangeSymb()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    return;
  }

  symb_ = btn->text().at(0);
}
