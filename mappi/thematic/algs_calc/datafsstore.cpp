#include "datafsstore.h"

#include <mappi/settings/mappisettings.h>

using namespace mappi::to;

DataFsStore::DataFsStore()
{
  QString satdata = QString::fromStdString(::mappi::inter::Settings::instance()->reception().file_storage().root());
  _dirname = satdata + "/pretreatment/";// + dateTime.toString("yyyyMMdd_hhmm");
}


DataFsStore::~DataFsStore()
{
}

      
bool DataFsStore::getFiles(const QDateTime& , const QString& , mappi::conf::InstrumentType ,
         const QList<std::string>& , QMap<std::string, QString>* chfiles)
{
  if (nullptr == chfiles) {
    return false;
  }
  
  // QDir dir(_dirname);
  
  // QFileInfoList flist = dir.entryInfoList("*.po", QDir::Files);
  // if (flist.isEmpty) {
  //   return false;
  // }

  
  return true;
}


bool DataFsStore::save(const struct meteo::global::PoHeader& , conf::ThemType , const std::string &,
                       const QString& , const std::string &)
{
  return false;
}



// bool DataFsStore::getAvailableInstruments(const QDateTime& start, const QString& satname,
// 					  QList<conf::InstrumentType>* instruments)
// {
//   if (nullptr == instruments) {
//     return false;
//   }

//   QDir dir(_dirname);
  
//   QFileInfoList flist = dir.entryInfoList("*.po", QDir::Files);
//   if (flist.isEmpty) {
//     return false;
//   }

//   for (auto filename : flist) {
//     QRegularExpression rx("_(\\d+)\\d+.po");
//     QString stype = rx.match(filename).captured(1);
//     var(stype);

//     conf::InstrumentType itype = conf::InstrumentType(stype.toInt(&ok));
//     if (ok && !instruments->contains(itype)) {
//       instruments.append(itype);
//     }
//   }
//   var(instruments);
  
//   return true;
// }
