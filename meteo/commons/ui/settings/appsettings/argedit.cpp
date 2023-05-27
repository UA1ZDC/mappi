#include "argedit.h"
#include "ui_argedit.h"

namespace meteo {

ArgEditDialog::ArgEditDialog(QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::ArgEdit())
{
  ui_->setupUi(this);

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

ArgEditDialog::~ArgEditDialog()
{
  delete ui_;
  ui_ = 0;
}

void ArgEditDialog::init(const QString& argkey, const QString& argvalue)
{
  ui_->keyLineEdit->setText(argkey);
  ui_->valueLineEdit->setText(argvalue);
}

QString ArgEditDialog::argkey() const
{
  return ui_->keyLineEdit->text();
}

QString ArgEditDialog::argvalue() const
{
  return ui_->valueLineEdit->text();
}

} // meteo
