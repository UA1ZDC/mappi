#include "timesheet.h"
#include "multichoosedlg.h"

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

Timesheet::Timesheet(QWidget *parent) :
  QWidget(parent)
{
    hourBtn_ = new MultiChooseBtn(this);
    hourBtn_->setCols(6);

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
      hourBtn_->addIco(pair);
    }

    minuteBtn_ = new MultiChooseBtn(this);
    minuteBtn_->setCols(10);

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
      minuteBtn_->addIco(pair);
    }
    QGridLayout* layout = new QGridLayout(this);
    QHBoxLayout* l = new QHBoxLayout;
    QLabel* lbl = new QLabel(":");
    hourBtn_->setText(QObject::tr("Ч"));
    minuteBtn_->setText(QObject::tr("М"));
    l->insertWidget(0, hourBtn_);
    l->insertWidget(2, lbl);
    l->insertWidget(2, minuteBtn_);
    layout->addLayout(l, 0, 0, 1, 1);
    layout->setMargin(0);
    setLayout(layout);
}

Timesheet::~Timesheet()
{
  delete hourBtn_; hourBtn_ = 0;
  delete minuteBtn_; minuteBtn_ = 0;
}

QString Timesheet::timesheet()
{
  QStringList h = hourBtn_->indexesToStringList();
  QStringList m = minuteBtn_->indexesToStringList();

  QString resultH = "*";
  if (false == h.isEmpty())
  {
    resultH = h.join(",");
  }
  QString resultM = "*";
  if (false == m.isEmpty())
  {
    resultM = m.join(",");
  }
  QString result = resultM + " " + resultH + " " + "* * *";
  return result;
}

bool Timesheet::setTimesheet(const QString &line) const
{
   if (kDefaultTimesheet == line) { return true; }
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

   hourBtn_->setIndexes(hourIndexes);
   minuteBtn_->setIndexes(minIndexes);

   return true;
}

bool Timesheet::parseTimesheet(const QString &line)
{
  if (kDefaultTimesheet == line) { return true; }
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
