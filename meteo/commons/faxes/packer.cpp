#include "packer.h"
#include "tiffconvert.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/grib/parser/tgribformat.h>
#include <meteo/commons/msgstream/plugins/socketspecial/unimas.h>
#include <meteo/commons/global/global.h>
#include <commons/compresslib/mcompress.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QProcess>
#include <QStringList>
#include <qcryptographichash.h>

#include "extract.h"

const int segmentSize = 15000;

namespace meteo {
namespace faxes {

int Packer::generateFileId(const QString& fileName)
{
  QByteArray nameHash = QCryptographicHash::hash(fileName.toUtf8(), QCryptographicHash::Md5);
  int hash = 0;
  for ( const char& c: nameHash){
    hash = (hash << sizeof(c)) + c;
  }
  return abs(hash) % 65001;
}


bool Packer::createMessages(const QByteArray& content,QList<QByteArray> *messages,int id, FaxSegment::DataType data_type)
{
  QList<FaxSegment> fs;
  bool ret = createSegments(content, &fs,id,data_type);
  if(false == ret) return false;
  for(auto segment: fs){
      QByteArray ba;
      ba +=  segment.getFormatString();
      ba +=  '\r';
      ba +=  '\r';
      ba +=  '\n';
      ba +=  segment.getHeader();
      ba +=  segment.content;
      messages->append(ba);
    }
  return true;
}

QByteArray Packer::createFileNameBuf(const QString& fln) {
  QByteArray content;
  QByteArray fnamebuf = fln.toUtf8();
  int32_t sz = fnamebuf.size();
  content.resize(sizeof(int32_t));
  content = QByteArray ::fromRawData((char*)&sz,sizeof(int32_t));
  content += fnamebuf;
  return content;
}


bool Packer::createFileMessages(const QString& fileName, const QByteArray& content,QList<QByteArray> *messages,int id, FaxSegment::DataType data_type)
{
  QByteArray totalBuf = createFileNameBuf(fileName) + content;
  return createMessages(totalBuf, messages, id, data_type);
}


bool Packer::createSegments(const QByteArray& content, QList<FaxSegment> *fsl, int id, FaxSegment::DataType data_type ){

  int fsize = content.size();
  if (content.isEmpty() == true) {
    error_log << QString::fromUtf8("Получено пустое тело телеграммы id = %1")
                 .arg("");
    return false;
  }
  compress::Compress compressor;

  int cur_num = 1;
  long compr_offs = 0;
  for(long cur_pos =0; cur_pos < fsize; cur_pos+=segmentSize){
      FaxSegment fs = FaxSegment(content.mid(cur_pos, segmentSize));
      fs.segtype = FaxSegment::Type::BOD;
      fs.segnum = cur_num++;
      fs.unpacksize = fs.content.size();
      fs.unpackoffset = cur_pos;
      fs.content = compressor.compress(fs.content);
      fs.packsize = fs.content.size();
      fs.packoffset = compr_offs;
      fs.faxid = id;
      fs.data_type = data_type;
      fs.lpm = FaxSegment::LPM::LPM_120;

      compr_offs = fs.packsize;
      fsl->append(fs);

    }
  fsl->first().segtype = FaxSegment::Type::BEG;
  if(1 < fsl->size()){
      fsl->last().segtype = FaxSegment::Type::END;
    }    
  return true;
}

}
}
