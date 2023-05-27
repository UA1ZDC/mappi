#ifndef FAXBUILDER_H
#define FAXBUILDER_H

#include <meteo/commons/services/chainloader/genericchainbuilder.h>

class FaxBuilder : public meteo::GenericChainBuilder
{
public:
  FaxBuilder();

protected:
  virtual QStringList subscribeMsgTypes() override;
  virtual bool saveChain(const ChunkStats& chain, const QByteArray& data) override;
  virtual QString loaderUid() override;
  virtual QString buildObjectName() override;

private:
  bool saveFaxInfo(const QDateTime &dt,
                   const meteo::tlg::Header &header,
                   int magic,
                   const QString& imagePath);
  bool saveFaxDataToGridFs(const QString& fileName, const QByteArray& data, const QString &prefixName );
  QString generateFileName(const QDateTime &dt,const meteo::tlg::Header &header);

};

#endif
