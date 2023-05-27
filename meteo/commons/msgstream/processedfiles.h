#ifndef METEO_COMMONS_MSGSTREAM_PROCESSEDFILES_H
#define METEO_COMMONS_MSGSTREAM_PROCESSEDFILES_H

#include <qset.h>
#include <qhash.h>
#include <qmap.h>
#include <qdatetime.h>

namespace meteo {

class ProcessedFiles
{
public:
  ProcessedFiles();
  ~ProcessedFiles();

  void addProcessed(const QString& fileName);
  bool isProcessed(const QString& fileName);

  void removeOld();

  bool load(const QString& fileName);
  bool save();

private:
  // параметры
  QString historyFile_;

  // данные
  QMap<QDate,int> count_;
  QHash<QDate,QSet<QByteArray>> hash_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PROCESSEDFILES_H
