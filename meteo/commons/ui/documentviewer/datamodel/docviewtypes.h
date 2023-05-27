#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_DOCVIEWTYPES_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_DOCVIEWTYPES_H

#include <qdatetime.h>
#include <meteo/commons/proto/weather.pb.h>

namespace meteo {
namespace documentviewer {

class DocumentHeader{
public:
  /* STATIC */
  static const QHash<QStringList, QString>& getLocalDocumentTypes();
  static const QStringList& getImageSuffixes();
  static const QStringList& getPdfSuffixes();
  static const QStringList& getOfficeDocumentSuffixes();

  static const int documentTypesCount = 5;
  static const QString documentsDisplayNames[documentTypesCount];  

  typedef enum {
    kFax = 0,
    kLocalDocument,
    kSateliteDocument,
    kMapImage,
    kFormalDocument,
    kAmbiguous
  } DocumentClass;



  DocumentHeader();
  DocumentHeader( const QString& id,
                  const QDateTime& dt,
                  const DocumentHeader::DocumentClass& type,
                  const QString& fileId);
  DocumentHeader( const DocumentHeader& other);
  virtual ~DocumentHeader() ;
  const QString& getId() const { return id_; }
  const QDateTime& getDt() const { return dt_; }
  const QString& getFileId() const { return fileId_;}
  virtual DocumentHeader::DocumentClass getClass() { return kAmbiguous; }
  virtual QString displayName() const;


private:
  QString id_;
  QDateTime dt_;
  DocumentHeader::DocumentClass type_;
  QString fileId_;
};

typedef QPair<DocumentHeader::DocumentClass, QString> DocumentUID;

class DocumentFax: public DocumentHeader
{
public:
  DocumentFax();
  DocumentFax( const QString& id,
               const QDateTime& dt,
               const QString& fileId,
               const QString& t1t2,
               const QString& a1a2,
               const QString& cccc,
               int ii
               );
  DocumentFax( const DocumentFax& other);
  virtual ~DocumentFax() override;
  const QString& getT1T2()  const;
  const QString& getA1A2()  const;
  const QString& getCCCC()  const;
  int getII() const;
  virtual DocumentHeader::DocumentClass getClass() override { return kFax; }
  virtual QString displayName() const override;
private:
  QString t1t2_;
  QString a1a2_;
  QString cccc_;
  int ii_;
};

class DocumentLocal : public DocumentHeader
{
public:
  DocumentLocal( const QString& name,
                 const QDateTime& dt
                 );
  virtual DocumentHeader::DocumentClass getClass() override { return kLocalDocument; }
  virtual QString displayName() const override;

};

class DocumentSatelite: public DocumentHeader
{
public:
  DocumentSatelite();
  DocumentSatelite( const QString& oid,
                    const QString& filePath,
                    const QDateTime& dt,
                    const QString& tt,
                    const QString& aa,
                    const QString& cccc,
                    int ii );
  DocumentSatelite(const DocumentSatelite& other);
  const QString& getT1T2() const { return tt_; }
  const QString& getA1A2() const { return aa_; }
  const QString& getCCCC() const { return cccc_; }
  int getII() const { return ii_; }
  virtual DocumentHeader::DocumentClass getClass() override { return kSateliteDocument; }
  virtual QString displayName() const override;
private:
  QString tt_;
  QString aa_;
  QString cccc_;
  int ii_;
};

class DocumentMapImage: public DocumentHeader
{
public:
  DocumentMapImage();
  DocumentMapImage( const QString& name,
                    const QDateTime& dt,
                    int hour,
                    int center,
                    int model,
                    const QString& jobName,
                    const QString& fid );
  DocumentMapImage( const DocumentMapImage& other);
  virtual ~DocumentMapImage() override;
  const QString& getName()  const;
  int getHour()  const;
  int getFullHour() const;
  int getCenter()  const;
  int getModel()  const;
  const QString& getJobName() const;
  virtual DocumentHeader::DocumentClass getClass() override { return kMapImage; }
  virtual QString displayName() const override;
private:
  QString name_;
  int hour_;
  int center_;
  int model_;
  QString jobName_;  
};

class DocumentFormal: public DocumentHeader
{
public:
  DocumentFormal();
  DocumentFormal( const QString& name,
                    const QDateTime& dt,
                    int hour,
                    int center,
                    int model,
                    const QString& jobName,
                    const QString& fid );
  DocumentFormal( const DocumentFormal& other);
  virtual ~DocumentFormal() override;
  const QString& getName()  const;
  int getHour()  const;
  int getFullHour() const;
  int getCenter()  const;
  int getModel()  const;
  const QString& getJobName() const;
  virtual DocumentHeader::DocumentClass getClass() override { return kFormalDocument; }
  virtual QString displayName() const override;
private:
  QString name_;
  int hour_;
  int center_;
  int model_;
  QString jobName_;
};

}
}

inline uint qHash( const QStringList& list, uint seed )
{
  QString str = list.join("@");
  return qHash( str, seed );
}

#endif
