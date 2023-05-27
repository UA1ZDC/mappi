#include "homepagewidget.h"
#include "ui_homepage.h"

#include <qlayout.h>

namespace meteo {

HomePageWidget::HomePageWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::HomePageForm)
{
  ui_->setupUi(this);
  ui_->help->hide();

  connect( ui_->connectBtn, SIGNAL(clicked()), SIGNAL(removeConnection()) );
  connect( ui_->helpBtn, SIGNAL(clicked()), SLOT(slotHelp()) );
}

void HomePageWidget::slotHelp()
{
  if ( ui_->help->isHidden() ) {
    ui_->help->show();
  }
  else {
    ui_->help->hide();
  }
}

} // meteo
