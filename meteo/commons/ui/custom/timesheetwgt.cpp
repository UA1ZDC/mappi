#include "timesheetwgt.h"

#include <qgridlayout.h>
#include <qlayout.h>
#include <qicon.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <cross-commons/debug/tlog.h>

const int kHoursMax = 23;
const int kMinutesMax = 59;
const QRegExp kCronRegExp("^([1-5][0-9]|[0-9])(([,][1-5][0-9]|[,][0-9])*)(\\s)([1-2][0-9]|[0-9])(([,][1-2][0-9]|[,][0-9])*)\\s[*]\\s[*]\\s[*]$");
const QRegExp kCronRegExpH("^[*]\\s([1-2][0-9]|[0-9])(([,][1-2][0-9]|[,][0-9])*)\\s[*]\\s[*]\\s[*]$");
const QRegExp kCronRegExpM("^([1-5][0-9]|[0-9])(([,][1-5][0-9]|[,][0-9])*)\\s[*]\\s[*]\\s[*]\\s[*]$");

TimesheetWgt::TimesheetWgt(QWidget *parent) :
  QWidget(parent)
{
    hour_ = new MultiChooseWgt(this);
    hour_->setCols(6);

    for ( int i=0; i<=kHoursMax; ++i ) {
      QPair< QString, QString> pair;
      if (i < 10)
      {
         pair.first = QObject::tr("0") + QString::number(i) + QObject::tr(":XX");
      }
      else
      {
         pair.first = QString::number(i) + QObject::tr(":XX");
      }
      pair.second = QString::number(i)+" ч.";
      hour_->addIco(pair);
    }

    minute_ = new MultiChooseWgt(this);
    minute_->setCols(10);

    for ( int i=0; i<=kMinutesMax; ++i ) {
      QPair< QString, QString> pair;
      if (i < 10)
      {
        pair.first = QObject::tr("XX:0") + QString::number(i);
      }
      else
      {
        pair.first = QObject::tr("XX:") + QString::number(i);
      }
      pair.second = QString::number(i)+" мин.";
      minute_->addIco(pair);
    }

    QGridLayout* layout = new QGridLayout(this);
    QGridLayout* l = new QGridLayout;
    QLabel* hlbl = new QLabel(QObject::tr("Часы:"));
    QLabel* mlbl = new QLabel(QObject::tr("Минуты:"));
    hlbl->setAlignment(Qt::AlignCenter);
    mlbl->setAlignment(Qt::AlignCenter);
    l->addWidget(hlbl, 0, 0);
    l->addWidget(mlbl, 0, 1);
    l->addWidget(hour_, 1, 0);
    l->addWidget(minute_, 1, 1);
    layout->addLayout(l, 0, 0, 1, 1);
//    layout->setMargin(0);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(hour_,SIGNAL(changed()),this,SIGNAL(changed()));
    connect(minute_,SIGNAL(changed()),this,SIGNAL(changed()));
}

TimesheetWgt::~TimesheetWgt()
{
  delete hour_; hour_ = 0;
  delete minute_; minute_ = 0;
}

QString TimesheetWgt::timesheet()
{
  QStringList h = hour_->indexesToStringList();
  QStringList m = minute_->indexesToStringList();

  QString resultH = "*";
  if ((false == h.isEmpty()) && (kHoursMax + 1 != h.size()))
  {
    resultH = h.join(",");
  }
  QString resultM = "*";
  if ((false == m.isEmpty()) && (kMinutesMax + 1 != m.size()))
  {
    resultM = m.join(",");
  }
  QString result = resultM + " " + resultH + " " + "* * *";
  return result;
}

bool TimesheetWgt::setTimesheet(const QString &line) const
{
   bool exp = kCronRegExp.exactMatch(line);
   bool expH = kCronRegExpH.exactMatch(line);
   bool expM = kCronRegExpM.exactMatch(line);
   if (kDefaultTimesheetWgt != line)
   {

     if (!exp && !expH && !expM)
     {
       debug_log << QObject::tr("Задано неверное значение периода");
       return false;
     }
   }
   QString str = line;
   QStringList tm = str.remove(" *").remove("* ").split(" ");
   if (0 == tm.size())
   {
     debug_log << QObject::tr("Задано неверное значение периода");
     return false;
   }

   QStringList h;
   QStringList m;
   if (exp)
   {
     if (tm.size() > 1)
     {
       h = tm[1].split(",");
     }
     m = tm[0].split(",");
   }
   else if (expH)
   {
     h = tm[0].split(",");
   }
   else if (expM)
   {
     m = tm[0].split(",");
   }
   QList<int> minIndexes;
   QList<int> hourIndexes;
   for (int j = 0, jsz = m.count(); j < jsz; ++j)
   {
     int min = m.at(j).toInt();
     if (min <= kMinutesMax)
     {
      minIndexes.append(min);
     }
     else
     {
       debug_log << "Задано неверное значение периода";
       return false;
     }
   }
   for (int k = 0, ksz = h.count(); k < ksz; ++k)
   {
     int hour = h.at(k).toInt();
     if (hour <= kHoursMax)
     {
      hourIndexes.append(hour);
     }
     else
     {
      debug_log << "Задано неверное значение периода";
      return false;
     }
   }
   hour_->setIndexes(hourIndexes);
   minute_->setIndexes(minIndexes);
   minute_->setBtns();
   hour_->setBtns();
   return true;
}

bool TimesheetWgt::parseTimesheet(const QString &line)
{
  if (kDefaultTimesheetWgt == line) { return true; }
  bool exp = kCronRegExp.exactMatch(line);
  bool expH = kCronRegExpH.exactMatch(line);
  bool expM = kCronRegExpM.exactMatch(line);
  if (!exp && !expH && !expM)
  {
    debug_log << QObject::tr("Задано неверное значение периода");
    return false;
  }
  QString str = line;
  QStringList tm = str.remove(" *").remove("* ").split(" ");
  if (0 == tm.size())
  {
    debug_log << QObject::tr("Задано неверное значение периода");
    return false;
  }

  QStringList h;
  QStringList m;
  if (exp)
  {
    if (tm.size() > 1)
    {
      h = tm[1].split(",");
    }
    m = tm[0].split(",");
  }
  else if (expH)
  {
    h = tm[0].split(",");
  }
  else if (expM)
  {
    m = tm[0].split(",");
  }
  QList<int> minIndexes;
  QList<int> hourIndexes;
  for (int j = 0, jsz = m.count(); j < jsz; ++j)
  {
    int min = m.at(j).toInt();
    if (min <= kMinutesMax)
    {
     minIndexes.append(min);
    }
    else
    {
      debug_log << "Задано неверное значение периода";
      return false;
    }
  }
  for (int k = 0, ksz = h.count(); k < ksz; ++k)
  {
    int hour = h.at(k).toInt();
    if (hour <= kHoursMax)
    {
     hourIndexes.append(hour);
    }
    else
    {
     debug_log << "Задано неверное значение периода";
     return false;
    }
  }
  return true;
}
