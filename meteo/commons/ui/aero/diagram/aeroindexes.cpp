#include "aeroindexes.h"
#include "ui_aeroindexes.h"
#include "createaerodiagwidget.h"


#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/zond/indexes.h>

using namespace meteo;

enum Columns {
  NAME_COL = 0,
  INDEX_COL = 1,
  ESTIM_COL = 2
};

namespace meteo {
  namespace adiag {
    static QList<QPair<int, QString> > kiEstim = QList<QPair<int, QString> >() 
      << QPair<int, QString>(20, QObject::tr("Без гроз"))
      << QPair<int, QString>(25, QObject::tr("Отдельные изолированные грозы"))
      << QPair<int, QString>(30, QObject::tr("Несколько гроз"))
      << QPair<int, QString>(35, QObject::tr("Рассеянные грозы"))
      << QPair<int, QString>(40, QObject::tr("Многочисленные грозы"))
      << QPair<int, QString>(40, QObject::tr("Грозы повсеместно"));

    static QList<QPair<int, QString> > vtEstim = QList<QPair<int, QString> >() 
         << QPair<int, QString>(28, QObject::tr("Без гроз"))
         << QPair<int, QString>(28, QObject::tr("Высокая неустойчивость"));
 
   static QList<QPair<int, QString> > ctEstim = QList<QPair<int, QString> >() 
      << QPair<int, QString>(18, QObject::tr("Без гроз"))
      << QPair<int, QString>(19, QObject::tr("Умеренная неустойчивость. Слабая грозовая деятельность"))
      << QPair<int, QString>(21, QObject::tr("Высокая неустойчивость. Грозы"))
      << QPair<int, QString>(23, QObject::tr("Энергия неустойчивости при которой возможны сильные грозы"))
      << QPair<int, QString>(25, QObject::tr("Высокая энергия неустойчивости. Сильные грозы"))
      << QPair<int, QString>(25, QObject::tr("Очень высокая энергия неустойчивости. Очень сильные грозы"));

    static QList<QPair<int, QString> > ttEstim = QList<QPair<int, QString> >() 
      << QPair<int, QString>(44, QObject::tr("Без гроз"))
      << QPair<int, QString>(45, QObject::tr("Отдельная гроза или несколько гроз"))
      << QPair<int, QString>(47, QObject::tr("Рассеянные грозовые очаги"))
      << QPair<int, QString>(49, QObject::tr("Значительные количество гроз, отдельные из которых сильные"))
      << QPair<int, QString>(51, QObject::tr("Рассеянные сильные грозовые очаги, отдельные очаги со смерчем"))
      << QPair<int, QString>(55, QObject::tr("Значительное количество очагов сильных гроз, отдельные очаги со смерчем"))
      << QPair<int, QString>(55, QObject::tr("Многочисленные сильные грозы с сильными смерчами"));

    static QList<QPair<int, QString> > sweatEstim = QList<QPair<int, QString> >() 
      << QPair<int, QString>(250, QObject::tr("Без гроз"))
      << QPair<int, QString>(350, QObject::tr("Есть условия для сильных гроз, града и шквалов"))
      << QPair<int, QString>(500, QObject::tr("Есть условия для очень сильных гроз, крупного града, сильных шквалов, смерчей"))
      << QPair<int, QString>(500, QObject::tr("Условия для очень сильных гроз, крупного града, сильных шквалов, сильных смерчей."));

 }
}


adiag::AeroIndexes::AeroIndexes(CreateAeroDiagWidget* parent, Qt::WindowFlags fl):
  QDialog(parent, fl),
  parent_(parent),
  ui_(0)
{
  ui_ = new Ui::AeroIndexes;
  ui_->setupUi(this);

  connect(ui_->cancel, SIGNAL(clicked()), SLOT(close()));
  
  if (0 != parent_) {
    fill();
    QObject::connect(parent, SIGNAL(currentDataChanged(QString)), SLOT(fill()));
  }
}

adiag::AeroIndexes::~AeroIndexes()
{
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}
	      

void adiag::AeroIndexes::fill()
{
  ui_->indexes->clear();

  addIndex(parent_->currentPlaceData().zond(),  QObject::tr("K index (число Вайтинга)"),
	   zond::kiIndex, kiEstim);

  addIndex(parent_->currentPlaceData().zond(),  QObject::tr("Vertical index"),
	   zond::verticalTotalsIndex, vtEstim);

  addIndex(parent_->currentPlaceData().zond(), QObject::tr("Сross index"),
	   zond::crossTotalsIndex, ctEstim);

  addIndex(parent_->currentPlaceData().zond(), QObject::tr("Total index"),
	   zond::totalTotalsIndex, ttEstim);

  addIndex(parent_->currentPlaceData().zond(), QObject::tr("SWEAT index"),
	   zond::sweatIndex, sweatEstim);


  for (int i = 0, sz = ui_->indexes->columnCount(); i < sz; ++i) {
    ui_->indexes->resizeColumnToContents(i);
  }
}

void adiag::AeroIndexes::addIndex(const zond::Zond& zond, const QString& name, FuncInstability func, const QList<QPair<int, QString> >& estim)
{
  float val;
  bool ok = func(zond, &val);
  if (ok) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->indexes);
    item->setText(NAME_COL, name);
    item->setText(INDEX_COL, QString::number(val, 'f', 2));
    item->setText(ESTIM_COL, aeroIndexesEstimate(val, estim));
  }
}


QString adiag::AeroIndexes::aeroIndexesEstimate(float val, const QList<QPair<int, QString> >& estim)
{
  if (estim.isEmpty()) return QString();

  if (val < estim.at(0).first) return estim.at(0).second;
  if (val > estim.last().first) return estim.last().second;

  for (int idx = 1; idx < estim.size(); idx++) {
    if (val >= estim.at(idx-1).first && val <= estim.at(idx).first) {
      return estim.at(idx).second;
    }
  }

  return estim.at(0).second;
}
