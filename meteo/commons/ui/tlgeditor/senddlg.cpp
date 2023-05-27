#include "senddlg.h"
#include "ui_senddlg.h"

#include <commons/funcs/tlgtextobject.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/msgparser/tlgfuncs.h>

namespace meteo {

SendDlg::SendDlg(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::SendDlg)
{
  ui_->setupUi(this);

  ui_->yyggggEdit->setText(QDateTime::currentDateTime().toString("ddhh00"));
}

SendDlg::~SendDlg()
{
  delete ui_;
}

QByteArray SendDlg::ahead() const
{
  QByteArray ahd;

  ahd += TlgTextObject::kSpecChars["<SOH>"];
  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<LF>"];

  QString num = ui_->numEdit->text();

  if ( num.size() == 3 ) { ahd += num.toUtf8(); }

  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<LF>"];

  QString tt = ui_->ttEdit->text();
  QString aa = ui_->aaEdit->text();
  QString ii = ui_->iiEdit->text();
  QString cccc = ui_->ccccEdit->text();
  QString yygggg = ui_->yyggggEdit->text();
  QString bbb = ui_->bbbEdit->text();

  if ( tt.size() == 2 ) { ahd += tt.toUtf8(); }
  if ( aa.size() == 2 ) { ahd += aa.toUtf8(); }
  if ( ii.size() == 2 ) { ahd += ii.toUtf8(); }

  ahd += " ";
  if ( cccc.size() == 4 ) { ahd += cccc.toUtf8(); }

  ahd += " ";
  if ( yygggg.size() == 6 ) { ahd += yygggg.toUtf8(); }

  if ( bbb.size() == 3 ) { ahd += " "; ahd += bbb.toUtf8(); }

  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<CR>"];
  ahd += TlgTextObject::kSpecChars["<LF>"];

  return ahd;
}

QDateTime SendDlg::dateTime() const
{
  return convertedDateTime(QDateTime::currentDateTime(), ui_->yyggggEdit->text());
}

} // meteo
