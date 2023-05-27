#include "selectlevels.h"

#include <meteo/commons/proto/obanal.pb.h>

#include <QRegExp>
#include <QString>
#include <QStringList>

namespace {

QStringList heigthLevels()
{
  QStringList result;
  result << QString::fromUtf8("0 м")
         << QString::fromUtf8("1000 м")
         << QString::fromUtf8("2000 м")
         << QString::fromUtf8("3000 м")
         << QString::fromUtf8("4000 м")
         << QString::fromUtf8("5000 м")
         << QString::fromUtf8("6000 м")
         << QString::fromUtf8("7000 м")
         << QString::fromUtf8("8000 м")
         << QString::fromUtf8("9000 м")
         << QString::fromUtf8("10000 м")
         << QString::fromUtf8("11000 м")
         << QString::fromUtf8("12000 м")
         << QString::fromUtf8("13000 м")
         << QString::fromUtf8("14000 м")
         << QString::fromUtf8("15000 м");
  return result;
}

QStringList pressureLevels()
{
  QStringList result;
  result << QString::fromUtf8("1000 мбар")
         << QString::fromUtf8("925 мбар")
         << QString::fromUtf8("850 мбар")
         << QString::fromUtf8("700 мбар")
         << QString::fromUtf8("500 мбар")
         << QString::fromUtf8("400 мбар")
         << QString::fromUtf8("300 мбар")
         << QString::fromUtf8("250 мбар")
         << QString::fromUtf8("200 мбар")
         << QString::fromUtf8("150 мбар")
         << QString::fromUtf8("100 мбар")
         << QString::fromUtf8("70 мбар")
         << QString::fromUtf8("50 мбар")
         << QString::fromUtf8("30 мбар")
         << QString::fromUtf8("20 мбар")
         << QString::fromUtf8("10 мбар")
         << QString::fromUtf8("5 мбар");
  return result;
}

QStringList deepLevels()
{
  QStringList result;
  result << QString::fromUtf8("0 м")
            ;
//         << QString::fromUtf8("10 м")
//         << QString::fromUtf8("20 м")
//         << QString::fromUtf8("25 м")
//         << QString::fromUtf8("30 м")
//         << QString::fromUtf8("50 м")
//         << QString::fromUtf8("75 м")
//         << QString::fromUtf8("100 м")
//         << QString::fromUtf8("125 м")
//         << QString::fromUtf8("150 м")
//         << QString::fromUtf8("200 м")
//         << QString::fromUtf8("300 м")
//         << QString::fromUtf8("400 м")
//         << QString::fromUtf8("500 м")
//         << QString::fromUtf8("600 м")
//         << QString::fromUtf8("800 м")
//         << QString::fromUtf8("1000 м")
//         << QString::fromUtf8("1200 м")
//         << QString::fromUtf8("1500 м")
//         << QString::fromUtf8("2000 м")
//         << QString::fromUtf8("2500 м")
//         << QString::fromUtf8("3000 м")
//         << QString::fromUtf8("3500 м")
//         << QString::fromUtf8("4000 м")
//         << QString::fromUtf8("4500 м")
//         << QString::fromUtf8("5000 м");
  return result;
}

}

namespace meteo {

SelectLevelsWidget::SelectLevelsWidget(int levelType, QWidget* parent) :
  SelectValuesWidget(parent),
  levelType_(levelType)
{
  setWindowTitle(QString::fromUtf8("%1: Уровни").arg(windowTitle()));
}

QList<int> SelectLevelsWidget::allLevelsList(int levelType)
{
  SelectLevelsWidget wgt(levelType);
  QList<int> result;
  QRegExp re("[\\d]+");
  foreach (const QString& each, wgt.loadAllValuesList()) {
    if (re.indexIn(each) > -1) {
      bool ok = false;
      int v = re.cap().toInt(&ok);
      if (ok) {
        result.append(v);
      }
    }
  }
  return result;
}

QStringList SelectLevelsWidget::loadAllValuesList()
{
  if (levelType_ == meteo::obanalsettings::kAero) {
    return ::pressureLevels();
  }
  else if (levelType_ == meteo::obanalsettings::kRadar) {
    return ::heigthLevels();
  }
  else if (levelType_ == meteo::obanalsettings::kOcean) {
    return ::deepLevels();
  }
  else if (levelType_ == meteo::obanalsettings::kGrib) {
    return ::pressureLevels();
  }

  return QStringList();
}

QStringList SelectLevelsWidget::loadValuesList(const QString& str)
{
  typedef QStringList::iterator Iter;

  QStringList result = loadAllValuesList();

  QStringList included;
  foreach (QString each, str.split(",")) {
    if (each.startsWith('[') == true ||
        each.endsWith(']') == true) {
      each.remove('[').remove(']');
    }
    included.append(each.trimmed());
  }

  Iter it = result.begin();
  while (it != result.end()) {
    bool founded = false;
    for (int i = 0, sz = included.size(); i < sz; ++i) {
      const QRegExp re(included[i] + " ");
      if (re.indexIn(*it) > -1) {
        founded = true;
        break;
      }
    }
    if (founded == true) {
      ++it;
    }
    else {
      it = result.erase(it);
    }
  }
  return result;
}

QString SelectLevelsWidget::getValues() const
{
  if (availableValues().isEmpty() == true ||
      selectedValues().isEmpty() == true) {
    return QString();
  }

  typedef QStringList::const_iterator Iter;

  const QRegExp re("[\\d]+");
  QStringList levels;
  for (Iter it = selectedValues().constBegin(), end = selectedValues().constEnd(); it != end; ++it) {
    if (re.indexIn(*it) > -1) {
      levels.append(re.cap());
    }
  }
  return QString("[%1]").arg(levels.join(", "));
}

} // meteo
