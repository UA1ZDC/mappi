#include "editapp.h"
#include "ui_editapp.h"

#include "chooseicon.h"

#include <QFile>
#include <QMessageBox>
#include <QPixmap>

namespace meteo {

EditAppDialog::EditAppDialog(const QStringList& spoNames, QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::EditApp())
{
  ui_->setupUi(this);
  ui_->cbSpoNames->addItems(spoNames);

  loadIcon();

  connect(ui_->selectIconButton, SIGNAL(clicked()), SLOT(slotOpenIconDialog()));
  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

EditAppDialog::~EditAppDialog()
{
  delete ui_;
  ui_ = 0;
}

void EditAppDialog::init(const QString& apptitle, const QString& appico)
{
  ui_->titleLineEdit->setText(apptitle);
  icopath_ = appico;
  loadIcon();
}

QString EditAppDialog::appTitle() const
{
  return ui_->titleLineEdit->text();
}

QString EditAppDialog::iconPath() const
{
  return icopath_;
}

QString EditAppDialog::spoName() const
{
  return this->ui_->cbSpoNames->currentText();
}

void EditAppDialog::slotOpenIconDialog()
{
  ChooseIconDialog dlg;
  dlg.init(icopath_);
  if (dlg.exec() == QDialog::Accepted) {
    icopath_ = dlg.iconPath();
    loadIcon();
  }
}

void EditAppDialog::loadIcon()
{
  if (icopath_.isEmpty() == true ||
      QFile::exists(icopath_) == false) {
    icopath_ = ChooseIconDialog::defaultIcon();
  }

  QPixmap px(icopath_);
  if (px.isNull() == true) {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                         QString::fromUtf8("Не удалось загрузить изображение из файла: %1").arg(icopath_),
                         QMessageBox::Ok);
    icopath_ = ChooseIconDialog::defaultIcon();
    px.load(icopath_);
  }
  ui_->iconLabel->setPixmap(px.scaledToWidth(ui_->iconLabel->width()));
}

} // meteo
