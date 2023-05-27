#include "appstartargsview.h"
#include "ui_appstartargsview.h"

namespace meteo {

AppStartArgsViewWidget::AppStartArgsViewWidget(const QString& txt, QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::ArgsView())
{
  ui_->setupUi(this);

  connect(ui_->okButton, SIGNAL(clicked()), SLOT(reject()));

  ui_->textEdit->setPlainText(txt);
}

AppStartArgsViewWidget::~AppStartArgsViewWidget()
{
  delete ui_;
  ui_ = 0;
}

} // meteo
