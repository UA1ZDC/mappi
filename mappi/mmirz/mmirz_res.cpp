#include "mmirz_res.h"
#include "ui_mmirz_res.h"


//Added by qt3to4:
#include <Q3ValueList>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/custom/coordedit.h>
#include <meteo/commons/ui/custom/choosefile.h>
//#include <commons/geobasis/geopoint.h>

#include <sat_schedule.h>
#include <mappi_defines.h>
#include <coords.h>
#include <mnmath.h>
#include <QDebug>

#include <QKeyEvent>


using namespace mappi;

//-----------


MmirzRes::MmirzRes(QWidget* parent):
  QDialog(parent)
{
  _ui = new Ui::MmirzRes;
  _ui->setupUi(this);
  hide();
  connect(_ui->cancelBtn, SIGNAL(clicked()), SLOT(close()));
}

MmirzRes::~MmirzRes()
{
  if (0 != _ui) {
    delete _ui;
    _ui = 0;
  }
}

void MmirzRes::setColumns(QStringList& str)
{
  _ui->tableWidget->setColumnCount(str.count());
  _ui->tableWidget->setHorizontalHeaderLabels(str);  
}

void MmirzRes::setRowCount(int num)
{
  _ui->tableWidget->setRowCount(num);
}

void MmirzRes::setRows(QStringList& str)
{
  _ui->tableWidget->setVerticalHeaderLabels(str);
}

void MmirzRes::setKA(const QString& str)
{
  _ui->label->setText(str);
}

void MmirzRes::addItem(int row, const QStringList& list)
{
  int isz = _ui->tableWidget->model()->columnCount();
  if (list.count() <  _ui->tableWidget->model()->columnCount()) { isz = list.count(); }

  for (int i=0; i<isz; ++i)
  {
    _ui->tableWidget->model()->setData(_ui->tableWidget->model()->index(row,i),list.at(i));
//    _ui->tableWidget->model()->setData(_ui->tableWidget->model()->index(row,i), Qt::AlignCenter, Qt::DecorationRole);
    _ui->tableWidget->item(row,i)->setTextAlignment(Qt::AlignCenter);
  }
//  for(int i = 0; i < _ui->tableWidget->columnCount(); i++)
//  {
//    _ui->tableWidget->resizeColumnToContents(i);
//  }
}

void MmirzRes::setDt(const QDateTime &dtbeg, const QDateTime &dtend)
{
  _ui->label_2->setText(dtbeg.toString("dd.MM.yyyy HH:mm"));
  _ui->label_5->setText(dtend.toString("dd.MM.yyyy HH:mm"));
}
