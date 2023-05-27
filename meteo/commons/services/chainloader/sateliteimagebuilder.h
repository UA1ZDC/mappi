#ifndef SATELITEIMAGEBUILDER_H
#define SATELITEIMAGEBUILDER_H


#include <meteo/commons/services/chainloader/genericchainbuilder.h>

class SateliteImageBuilder : public meteo::GenericChainBuilder
{
  Q_OBJECT
public:
  SateliteImageBuilder();
  ~SateliteImageBuilder();

protected:
  virtual QStringList subscribeMsgTypes() override;
  virtual bool saveChain(const ChunkStats& chain, const QByteArray& data) override;
  virtual QString loaderUid() override;
  virtual QString buildObjectName() override;
private:
  QString gridfsPreffix();

  QString saveImageToGridFs( const QByteArray& data, const QString& fileName, bool* isDup );
  bool saveImage( const meteo::tlg::Header& header, const QByteArray &data);
};

#endif
