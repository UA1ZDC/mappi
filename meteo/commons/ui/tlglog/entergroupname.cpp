#include "entergroupname.h"
#include "ui_entergroupname.h"


EnterGroupName::EnterGroupName(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::EnterGroupName)
{
  ui->setupUi(this);
  connect(ui->buttonBox_2,SIGNAL(accepted()),this,SLOT(accept()) );
  connect(ui->buttonBox_2,SIGNAL(rejected()),this,SLOT(reject()) );
}

EnterGroupName::~EnterGroupName()
{
  delete ui;
}

void EnterGroupName::accept()
{
  if(ui->lineEdit->text().isEmpty()){
    return;
  }
  group_name_ = ui->lineEdit->text();

  QDialog::accept();
}
