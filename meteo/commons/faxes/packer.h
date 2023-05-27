#ifndef METEO_COMMONS_FILE_PACKER_H
#define METEO_COMMONS_FILE_PACKER_H

#include <QByteArray>
#include <QString>
#include "extract.h"

template<typename T> class QList;

namespace meteo {
namespace faxes {



class Packer
{
public:
  static int generateFileId(const QString& fileName);
  bool createMessages(const QByteArray& content,QList<QByteArray> *messages,int id, FaxSegment::DataType data_type);
  bool createFileMessages(const QString& fileName, const QByteArray& content,QList<QByteArray> *messages,int id, FaxSegment::DataType data_type);
  static QByteArray createFileNameBuf(const QString& fln);


private:
  bool createSegments(const QByteArray& content, QList<FaxSegment> *fsl, int id, FaxSegment::DataType data_type );
};


}
}

#endif // METEO_MASLO_FAXES_EXTRACT_H
