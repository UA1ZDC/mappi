#include "mmirz_graph.h"
#include "ui_mmirz_graph.h"


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

#include <qcustomplot.h>
#include <widget.h>

using namespace mappi;

//-----------


MmirzGraph::MmirzGraph(QWidget* parent):
  QDialog(parent)
{
  _ui = new Ui::MmirzGraph;
  _ui->setupUi(this);
  wgt = new Widget(_ui->frame);
  wgt->show();
  hide();
  connect(_ui->cancelBtn, SIGNAL(clicked()), SLOT(close()));
}

MmirzGraph::~MmirzGraph()
{
  if (0 != _ui) {
    delete _ui;
    _ui = 0;
  } 
}

void MmirzGraph::setKA(const QString& str)
{
  _ui->label->setText(str);
}

void MmirzGraph::grafik(QVector<QStringList> data, QStringList times, QVector<QString> cities)
{
  wgt->plot(data, times, cities, _ui->label->text());
}

void MmirzGraph::setDt(const QDateTime &dtbeg, const QDateTime &dtend)
{
  _ui->label_2->setText(dtbeg.toString("dd.MM.yyyy HH:mm"));
  _ui->label_5->setText(dtend.toString("dd.MM.yyyy HH:mm"));
}

void MmirzGraph::setupBars(QVector<QString> cities)
{
  wgt->setupBars(cities);
}




