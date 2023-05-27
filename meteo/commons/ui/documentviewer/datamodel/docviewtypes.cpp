#include "datamodel/docviewtypes.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/dateformat.h>

namespace meteo {
namespace documentviewer {

static const QStringList imageTypes = { "png", "bmp", "jpg", "jpeg", "tiff" };
static const QStringList officeDocumentTypes = { "odt", "doc", "ods" };
static const QStringList pdfType = { "pdf" };


static QHash<QStringList, QString> fill_LocalHocumentFilterOptions()
{
  QHash<QStringList, QString> allOptions;
  allOptions.insert( { "*.pdf" }, "Документы PDF");
  allOptions.insert( { "*.odt", "*.doc" }, "Документы Office");
  allOptions.insert( { "*.png", "*.bmp", "*.jpg", "*.jpeg", "*.tiff" }, "Изображения");
  return allOptions;
}

const QStringList& DocumentHeader::getImageSuffixes()
{
  return imageTypes;
}

const QStringList& DocumentHeader::getOfficeDocumentSuffixes()
{
  return officeDocumentTypes;
}

DocumentHeader::DocumentHeader(): id_(QString()), dt_(QDateTime()), type_(kFax), fileId_(QString())
{

}

const QStringList& DocumentHeader::getPdfSuffixes()
{
  return pdfType;
}


const QHash<QStringList, QString>& DocumentHeader::getLocalDocumentTypes()
{
  static const QHash<QStringList, QString> st_LocalDocumentFilterOptions = fill_LocalHocumentFilterOptions();
  return st_LocalDocumentFilterOptions;
}

const QString DocumentHeader::documentsDisplayNames[DocumentHeader::documentTypesCount] = {
  QObject::tr("Факсимильные карты"),
  QObject::tr("Локальные документы"),
  QObject::tr("Спутниковые изображения"),
  QObject::tr("Растровые изображения"),
  QObject::tr("Формализованные документы")
};

DocumentHeader::DocumentHeader( const QString& id,
                                const QDateTime& dt,
                                const DocumentHeader::DocumentClass& type,
                                const QString& fileId) : id_(id), dt_(dt), type_(type), fileId_(fileId)
{

}

DocumentHeader::DocumentHeader(const DocumentHeader &other) : id_(other.id_), dt_(other.dt_), type_(other.type_), fileId_(other.fileId_)
{

}

DocumentHeader::~DocumentHeader()
{

}

DocumentFax::DocumentFax() : DocumentHeader (),t1t2_(QString()),a1a2_(QString()),cccc_(QString()),ii_(0)
{

}

DocumentFax::DocumentFax( const QString& id,
                          const QDateTime& dt,
                          const QString& fileId,
                          const QString& t1t2,
                          const QString& a1a2,
                          const QString& cccc,
                          int ii
                          ) :
  DocumentHeader (id, dt, DocumentHeader::kFax, fileId),
  t1t2_(t1t2),
  a1a2_(a1a2),
  cccc_(cccc),
  ii_(ii)
{

}

DocumentFax::DocumentFax(const DocumentFax &other):
  DocumentHeader (other),
  t1t2_(other.t1t2_),
  a1a2_(other.a1a2_),
  cccc_(other.cccc_),
  ii_(other.ii_)
{

}

DocumentFax::~DocumentFax()
{

}

const QString& DocumentFax::getT1T2() const
{
  return this->t1t2_;
}

const QString& DocumentFax::getA1A2() const
{
  return this->a1a2_;
}

const QString& DocumentFax::getCCCC() const
{
  return this->cccc_;
}

int DocumentFax::getII() const
{
  return this->ii_;
}

DocumentLocal::DocumentLocal(const QString& path, const QDateTime &dt) :
  DocumentHeader (path, dt, DocumentHeader::kLocalDocument, path)
{

}


DocumentSatelite::DocumentSatelite():DocumentHeader (),tt_(QString()),aa_(QString()),cccc_(QString()),ii_(0)
{

}

DocumentSatelite::DocumentSatelite( const QString& oid,
                                    const QString& filePath,
                                    const QDateTime& dt,
                                    const QString& tt,
                                    const QString& aa,
                                    const QString& cccc,
                                    int ii ) :
  DocumentHeader ( oid, dt, DocumentHeader::kSateliteDocument, filePath ),
  tt_(tt),
  aa_(aa),
  cccc_(cccc),
  ii_(ii)
{

}

DocumentSatelite::DocumentSatelite(const DocumentSatelite &other):
  DocumentHeader ( other ),
  tt_(other.tt_),
  aa_(other.aa_),
  cccc_(other.cccc_),
  ii_(other.ii_)
{

}

DocumentMapImage::DocumentMapImage():DocumentHeader (),name_(QString()),hour_(0),center_(0),model_(0),jobName_(QString())
{

}

DocumentMapImage::DocumentMapImage( const QString& name,
                                    const QDateTime& dt,
                                    int hour,
                                    int center,
                                    int model,
                                    const QString& jobName,
                                    const QString& fid
                                    ) :
  DocumentHeader( fid, dt, DocumentHeader::kMapImage, fid ),
  name_(name),
  hour_(hour),
  center_(center),
  model_(model),
  jobName_(jobName)
{
}

DocumentMapImage::DocumentMapImage(const DocumentMapImage &other):
  DocumentHeader ( other ),
  name_(other.name_),
  hour_(other.hour_),
  center_(other.center_),
  model_(other.model_),
  jobName_(other.jobName_)
{

}

const QString& DocumentMapImage::getName() const
{
  return this->name_;
}

int DocumentMapImage::getHour() const
{  
  return (this->hour_/3600);
}

int DocumentMapImage::getCenter() const
{
  return this->center_;
}

int DocumentMapImage::getModel() const
{
  return this->model_;
}

const QString& DocumentMapImage::getJobName() const
{
  return this->jobName_;
}

QString DocumentHeader::displayName() const
{
  return this->getFileId();
}

QString DocumentFax::displayName() const
{
      return QObject::tr("%1_%2_%3_%4_%5")
          .arg(dateToHumanFull(this->getDt()))
          .arg(this->getT1T2())
          .arg(this->getA1A2())
          .arg(this->getCCCC())
          .arg(this->getII());
    }

QString DocumentLocal::displayName() const
{
  return this->getFileId();
}

QString DocumentMapImage::displayName() const
{
      return QObject::tr("%1_%2_%3_%4_%5_%6")
          .arg(this->getName())
          .arg(this->getJobName())
          .arg(this->getHour())
          .arg(this->getCenter())
          .arg(this->getModel())
          .arg(dateToHumanFull(this->getDt()));
}


QString DocumentSatelite::displayName() const
{
      return QObject::tr("%1_%2_%3_%4_%5_%6")
          .arg(dateToHumanFull(this->getDt()))
          .arg(this->getT1T2())
          .arg(this->getA1A2())
          .arg(this->getCCCC())
          .arg(this->getII());
}

DocumentMapImage::~DocumentMapImage()
{

}


DocumentFormal::DocumentFormal():DocumentHeader (),name_(QString()),hour_(0),center_(0),model_(0),jobName_(QString())
{

}

DocumentFormal::DocumentFormal( const QString& name,
                                const QDateTime& dt,
                                int hour,
                                int center,
                                int model,
                                const QString& jobName,
                                const QString& fid ) :
  DocumentHeader( fid, dt, DocumentHeader::kFormalDocument, fid ),
  name_(name),
  hour_(hour),
  center_(center),
  model_(model),
  jobName_(jobName)
{
}

DocumentFormal::DocumentFormal(const DocumentFormal &other):
  DocumentHeader (other),
  name_(other.name_),
  hour_(other.hour_),
  center_(other.center_),
  model_(other.model_),
  jobName_(other.jobName_)
{

}

DocumentFormal::~DocumentFormal()
{

}

const QString& DocumentFormal::getName()  const
{
  return this->name_;
}

int DocumentFormal::getHour() const
{
  return (this->hour_/3600);
}

int DocumentFormal::getCenter()  const
{
  return this->center_;
}

int DocumentFormal::getModel()  const
{
  return this->model_;
}

const QString& DocumentFormal::getJobName() const
{
  return this->jobName_;
}

QString DocumentFormal::displayName() const
{
  return QObject::tr("%1_%2_%3_%4_%5_%6")
      .arg(this->getName())
      .arg(this->getJobName())
      .arg(this->getHour())
      .arg(this->getCenter())
      .arg(this->getModel())
      .arg(dateToHumanFull(this->getDt()));
}

int DocumentFormal::getFullHour() const
{
  return this->hour_;
}

int DocumentMapImage::getFullHour() const
{
  return this->hour_;
}


}
}
