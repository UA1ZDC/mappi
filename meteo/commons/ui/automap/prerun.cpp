#include "prerun.h"
#include "ui_prerun.h"

#include <cross-commons/app/options.h>

using commons::ArgParser;

preRun::preRun(const QString & jobname,QWidget *parent) :
  QDialog(parent,Qt::Dialog),
  ui(new Ui::preRun),
  jobname_(jobname)
{
  ui->setupUi(this);
  ui->dateEdit->setDate(QDate::currentDate());
  QTime tm;
  tm.setHMS(QTime::currentTime().hour()+1,0,0);
  ui->timeEdit->setTime(tm);
  //QPushButton()
  // WRONG
  QObject::connect( ui->closeButton, SIGNAL(clicked()),
                    this, SLOT(slotClose()));
  QObject::connect( ui->cancelButton, SIGNAL(clicked()),
                    this, SLOT(reject()));
}

void preRun::slotClose()
{
  QStringList args;
  args << "widget";
  args << "-j";
  args << jobname_;
  args << "-d";
  args << ui->dateEdit->date().toString(Qt::ISODate)+" "+ui->timeEdit->time().toString(Qt::ISODate);

  int argcc = args.size();
  char **argvv = new char*[argcc];

  for(int i =0; i < argcc; ++i)
  {
    argvv[i] = new char[args.at(i).toStdString().size()];
    strcpy(argvv[i],args.at(i).toStdString().data());
    //argvv[i] = args.at(i).toStdString().data();
  }

  ArgParser* options = ArgParser::instance();
  options->clear();
  options->parse(argcc, argvv);
  accept();
}




preRun::~preRun()
{
  delete ui;
}
