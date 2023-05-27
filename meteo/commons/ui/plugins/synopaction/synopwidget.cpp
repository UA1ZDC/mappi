#include "synopwidget.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options = 0 )
  {
    Q_UNUSED(options);
    SynopWidget* dlg = new SynopWidget( parent );
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "synopwidget", createDialog );
  }

static bool res = registerDialog();
}
SynopWidget::SynopWidget(QWidget *parent):
  QDialog(parent),
  labelHeader1_(nullptr),
  labelSit_(nullptr),
  labelHeader2_(nullptr),
  labelFenom_(nullptr)
{
  setMinimumSize(QSize(250, 300));

//  setFixedSize(QSize(200, 300));
  setMaximumSize(QSize(250, 300));
  setStyleSheet("background-color:white;");

  layout_ = new QVBoxLayout(this);
//  layout_->setSizeConstraint(QLayout::SetMinimumSize);
//  layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
  layout_->setSizeConstraint(QLayout::SetDefaultConstraint);



  layout_->setSpacing(0);
//  layout_->setContentsMargins(2,2,2,12);

  labelHeader1_ = new QLabel;
  labelSit_ = new QLabel;
  labelHeader2_ = new QLabel;
  labelFenom_ = new QLabel;

//  labelHeader1_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  labelHeader1_->setWordWrap(true);

//  labelSit_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  labelSit_->setWordWrap(true);

//  labelHeader2_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  labelHeader2_->setWordWrap(true);

//  labelFenom_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  labelFenom_->setWordWrap(true);


  layout_->addWidget(labelHeader1_);
  layout_->addWidget(labelSit_);
  layout_->addWidget(labelHeader2_);
  layout_->addWidget(labelFenom_);

  //labelHeader1_->setText();
  labelHeader2_->setText("<b>Ожидаемые опасные явления погоды:</b>");
  setWindowTitle("Синоптическая ситуация");
//  QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}


void SynopWidget::setSit(const QString &text)
{
  if( nullptr == labelSit_){
    return;
  }
  labelSit_->setText(text);
//  adjustSize();
}

void SynopWidget::setHeader1(const QString &text)
{
  if( nullptr == labelHeader1_){
    return;
  }
  labelHeader1_->setText(text);
//  adjustSize();
}

void SynopWidget::setFenom(const QString &text)
{
  if( nullptr == labelFenom_){
    return;
  }
  labelFenom_->setText(text);
  adjustSize();
}




}
}
