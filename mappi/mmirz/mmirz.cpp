#include "mmirz.h"
#include "ui_mmirz.h"
#include "mmirz_res.h"
#include "mmirz_graph.h"


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


MmirzWidget::MmirzWidget(QWidget* parent):
  QDialog(parent)
{
  _ui = new Ui::Mmirz;
  _ui->setupUi(this);

  _uires = new mappi::MmirzRes;

  _uigraph = new mappi::MmirzGraph;
//  _uigraph->show();
//  _uires->show();

  initWidgets();

  connect(_ui->cancelBtn, SIGNAL(clicked()), SLOT(cancel()));
  connect(_ui->applyBtn, SIGNAL(clicked()), SLOT(apply()));
  connect(_ui->addBtn, SIGNAL(clicked()), SLOT(slotAddCity()));
  connect(_ui->delBtn, SIGNAL(clicked()), SLOT(slotRmCity()));
  connect(_ui->radioButton_2, SIGNAL(clicked(bool)), _ui->groupBox_4, SLOT(setEnabled(bool)));
  connect(_ui->radioButton, SIGNAL(clicked(bool)), _ui->groupBox_4, SLOT(setDisabled(bool)));
  connect(_ui->pushButton_4, SIGNAL(clicked()), SLOT(slotAddTime()));
  connect(_ui->delTimeBtn, SIGNAL(clicked()), SLOT(slotRmTime()));
}

MmirzWidget::~MmirzWidget()
{
  if (0 != _ui) {
    delete _ui;
    _ui = 0;
  }
  if (0 != _uires) {
    delete _uires;
    _uires = 0;
  }
  if (0 != _uigraph) {
    delete _uigraph;
    _uigraph = 0;
  }
  clear();
}


void MmirzWidget::slotAddTime()
{
  if (_ui->treeWidget->model()->rowCount()>5)
  {
    return;
  }
  debug_log << "addTime";
  QString tbegin = _ui->timeEdit->text();
  QString tend = _ui->timeEdit_2->text();

  QTreeWidgetItem *item = new QTreeWidgetItem(_ui->treeWidget);
  item->setData(0, Qt::DisplayRole, tbegin);
  item->setData(1, Qt::DisplayRole, tend);
}

void MmirzWidget::slotRmCity()
{
    int row = _ui->cityList->selectionModel()->currentIndex().row();
    _ui->cityList->model()->removeRow(row);
}

void MmirzWidget::slotRmTime()
{
  int row = _ui->treeWidget->selectionModel()->currentIndex().row();
  _ui->treeWidget->model()->removeRow(row);
}

void MmirzWidget::slotAddCity()
{
  debug_log << "addCity";
  QString cityName = _ui->cityName->text();
  QString lat = _ui->latEdit->text();
  QString lon = _ui->lonEdit->text();
  QString h = QString::number(_ui->heightVal->value());

  QTreeWidgetItem *item = new QTreeWidgetItem(_ui->cityList);
  item->setData(0, Qt::DisplayRole, cityName);
  item->setData(1, Qt::DisplayRole, lat);
  item->setData(2, Qt::DisplayRole, lon);
  item->setData(3, Qt::DisplayRole, h);
}

void MmirzWidget::clear()
{
  _uires->hide();
  _uigraph->hide();
}

void  MmirzWidget::initWidgets()
{
  _ui->dateTimeEdit->setTimeSpec(Qt::UTC);
  _ui->dateTimeEdit->setDateTime(QDateTime::currentDateTimeUtc());
  _ui->dateTimeEdit_2->setTimeSpec(Qt::UTC);
//  _ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTimeUtc().addSecs(3600));
  _ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTimeUtc().addDays(1));

//  _ui->file->setFile(QString("/home/marina/build_dir/var/mappi/weather.txt"));
  //_ui->file->setFile(QString("/home/marina/weather.txt"));
  _ui->file->setFile(QCoreApplication::applicationDirPath() + QString("/weather.txt"));
}

void MmirzWidget::cancel()
{
  clear();
  close();
}

void MmirzWidget::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    cancel();
  }
}

void MmirzWidget::closeEvent(QCloseEvent* e)
{
  Q_UNUSED(e);
  cancel();
}

void MmirzWidget::apply()
{
  if (_uires != 0)
  {
    delete _uires; _uires = 0;
    _uires = new mappi::MmirzRes;
  }
  if (_uigraph != 0)
  {
    delete _uigraph; _uigraph = 0;
    _uigraph = new mappi::MmirzGraph;
  }

  if (_ui->cityList->model()->rowCount() <= 0) { debug_log << "empty"; return; }
  if (_ui->cityList->model()->columnCount() < 4) { debug_log << "empty"; return; }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QString sat = _ui->comboBox->currentText(); //спутник
  QDateTime dt = QDateTime(_ui->dateTimeEdit->dateTime());
  QDateTime dt2 = _ui->dateTimeEdit_2->dateTime();
  debug_log << QObject::tr("Спутник: %1").arg(sat);
  if (_ui->buttonGroup->checkedButton() == _ui->buttonGroup->buttons().at(0))
  {
        int isz= _ui->cityList->model()->rowCount();
        if (isz == 0) { debug_log << "empty"; return; }
        QVector<QString> labels;
        QVector<QStringList> data;

        QStringList str = QStringList();
        str << QObject::tr("Всего за период");
        if (_ui->checkBox->isChecked() ==true)
        {

         _uires->setColumns(str);
         _uires->setKA(sat);
         _uires->setDt(dt, dt2);
         _uires->setRowCount(isz);
        }

        QStringList rows;
        for (int i = 0; i < isz; ++i)
        {
          QString text = _ui->cityList->model()->data(_ui->cityList->model()->index(i,0)).toString();
          if (_ui->checkBox->isChecked() == true) { rows << text; }
          if (_ui->checkBox_2->isChecked() == true) { labels << text; }
          QStringList list1 = dmsFromString(_ui->cityList->model()->data(_ui->cityList->model()->index(i,1)).toString());
          QStringList list2 = dmsFromString(_ui->cityList->model()->data(_ui->cityList->model()->index(i,2)).toString());
          float alt = _ui->cityList->model()->data(_ui->cityList->model()->index(i,3)).toFloat(); // высота?
          Coords::GeoCoord site(MnMath::deg2rad(toDecDeg(list1)), MnMath::deg2rad(toDecDeg(list2)), alt);
          QStringList list = calc(site, dt, dt2, sat, text);
          if (_ui->checkBox->isChecked() ==true)
          {
            int num = i;
            _uires->addItem(num, list);
          }
          data.append(list);
        }

        if (_ui->checkBox_2->isChecked() == true)
        {
          _uigraph->setKA(sat);
          _uigraph->setDt(dt, dt2);
          _uigraph->grafik(data,str,labels);
          _uigraph->show();
        }

        if (_ui->checkBox->isChecked() == true)
        {
         _uires->setRows(rows);
         _uires->show();
        }
  }
  else
  {
      int isz= _ui->cityList->model()->rowCount();
      if (isz == 0) { debug_log << "empty"; return; }
      QVector<QString> labels;
      QVector<QStringList> data;
      QList<QTime> times;

      QStringList str = QStringList();

      for (int j = 0; j < _ui->treeWidget->model()->rowCount(); ++j)
      {
        QTime t1 = QTime::fromString(_ui->treeWidget->model()->data(_ui->treeWidget->model()->index(j,0)).toString(),"H:mm");
        QTime t2 = QTime::fromString(_ui->treeWidget->model()->data(_ui->treeWidget->model()->index(j,1)).toString(),"H:mm");
        times << t1;
        times << t2;
        str << _ui->treeWidget->model()->data(_ui->treeWidget->model()->index(j,0)).toString() + " - " + _ui->treeWidget->model()->data(_ui->treeWidget->model()->index(j,1)).toString();
       }
      str << QObject::tr("Всего");
      if (_ui->checkBox->isChecked() ==true)
      {

       _uires->setColumns(str);
       _uires->setKA(sat);
       _uires->setDt(dt, dt2);
       _uires->setRowCount(isz);
      }

      QStringList rows;
      for (int i = 0; i < isz; ++i)
      {
        QString text = _ui->cityList->model()->data(_ui->cityList->model()->index(i,0)).toString();
        if (_ui->checkBox->isChecked() == true) { rows << text; }
        if (_ui->checkBox_2->isChecked() == true) { labels << text; }
        QStringList list1 = dmsFromString(_ui->cityList->model()->data(_ui->cityList->model()->index(i,1)).toString());
        QStringList list2 = dmsFromString(_ui->cityList->model()->data(_ui->cityList->model()->index(i,2)).toString());
        float alt = _ui->cityList->model()->data(_ui->cityList->model()->index(i,3)).toFloat(); // высота?
        Coords::GeoCoord site(MnMath::deg2rad(toDecDeg(list1)), MnMath::deg2rad(toDecDeg(list2)), alt);
        QStringList list = calcCustom(site, dt.toUTC(), dt2.toUTC(), sat, text, times);
        if (_ui->checkBox->isChecked() ==true)
        {
          int num = i;
          _uires->addItem(num, list);
        }
        data.append(list);
      }

      if (_ui->checkBox_2->isChecked() == true)
      {
        _uigraph->setKA(sat);
        _uigraph->setDt(dt, dt2);
        _uigraph->grafik(data,str,labels);
        _uigraph->show();
      }

      if (_ui->checkBox->isChecked() == true)
      {
       _uires->setRows(rows);
       _uires->show();
      }
  }
   QApplication::restoreOverrideCursor();
}

QStringList MmirzWidget::calc(const Coords::GeoCoord& site, const QDateTime& dtBeg,  const QDateTime& dtEnd, const QString& satName, const QString& text/*, int h1, int h2, int h3, int h4*/)
{
    Q3ValueList<SpSatellite::Schedule> schedule;
    SatSchedule sched(_ui->file->file(), site);

    float thr = MnMath::deg2rad(1);
    float culmin = MnMath::deg2rad(1);

    //printf("%s ", text.ascii());
    debug_log << text;

    QDateTime dt = dtBeg;
    QDateTime dt2 = dtEnd;
    float hours = dt.secsTo(dt2)/3600.0;
    debug_log << "hours" << hours;
    sched.getSchedule( dt, satName, hours, thr, culmin, schedule);

    int cnt = schedule.count();
    QStringList list = QStringList() << QString::number(cnt);
    return list;
}

QStringList MmirzWidget::calcCustom(const Coords::GeoCoord& site, const QDateTime& dt,  const QDateTime& dt2, const QString& satName, const QString& text,  QList<QTime> times)
{
    Q3ValueList<SpSatellite::Schedule> schedule;
    SatSchedule sched(_ui->file->file(), site);

    float thr = MnMath::deg2rad(1);
    float culmin = MnMath::deg2rad(1);

    debug_log << text;

//    QDateTime dt = dtBeg;
//    QDateTime dt2 = dtEnd;
    float hours = dt.secsTo(dt2)/3600.0;
        debug_log << "hours" << hours;


    debug_log << "Начало расчета:" << dt;
    debug_log << "КА:" << satName;
    debug_log << "Количество часов:" << hours;
    debug_log << "thr:" << thr;
    debug_log << "culmin:" << culmin;

    debug_log << "dt.toUTC()" << dt.toUTC();

    sched.getSchedule( dt, satName, hours, thr, culmin, schedule);
    debug_log << "schedule" << schedule.count();
    for (int m=0,msz=schedule.count();m<msz;++m)
    {
    debug_log << schedule[m].aos.toUTC();
    }
    QList<int> cnts;
    for (int k=0; k<times.count(); k=k+2)
    {
      cnts.append(0);
    }
    int cnt = 0;
    for (int i=0; i<schedule.count(); ++i ) {
      for (int j=0; j<times.count()-1; ++j)
      {
        QTime t1 = times[j];
        QTime t2 = times[j+1];
        //debug_log << t1.toString("H:mm") << t2.toString("H:mm")<< schedule[i].aos.time().toString("H:mm");

        if ((t1.secsTo(t2) >0 && schedule[i].aos.time().secsTo(t1)<=0 && schedule[i].aos.time().secsTo(t2)>0) ||
               (t1.secsTo(t2)<0 && (schedule[i].aos.time().secsTo(t2)<=0 || schedule[i].aos.time().secsTo(t1)>0)))
        {
           int num = j/2;
           //debug_log << "number" << num;
           if (cnts.count()>num)
           {
             cnts[num]++;
             cnt++;
           }
        }
        ++j;
      }
    }
    QStringList list = QStringList();
    for (int k=0; k<cnts.count(); ++k)
    {
      //printf("%d",cnts[k]);
      list << QString::number(cnts[k]);
    }
    list << QString::number(cnt);
    debug_log <<"------------";
    return list;
}

QStringList MmirzWidget::dmsFromString(const QString& s)
{
  QString ss = s.trimmed();

  QRegExp splitRx;

  if ( ss.contains(QChar(0x00B0)) ) {
    splitRx.setPattern(QString::fromUtf8("[°'\"]{1}"));
    ss.remove(QRegExp("\\s"));
  }
  else {
    splitRx.setPattern("\\s+");
  }

  QStringList list = ss.split(splitRx, QString::KeepEmptyParts);
  return list;
}

float MmirzWidget::toDecDeg(QStringList list)
{
  if (list.count() < 3) { return 0; }
  const int k = 1000000;
  int64_t D = list[0].toInt() * k;
  int64_t M = list[1].toInt() * k;
  int64_t S = list[2].toInt() * k;

  float dd = D + (M / 60.0) + (S / 3600.0);

  dd = dd / k;

  return dd;
}
