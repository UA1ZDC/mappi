#ifndef METEO_COMMONS_MSGPARSER_TMSGRECV_H
#define METEO_COMMONS_MSGPARSER_TMSGRECV_H

#include <qdir.h>
#include <qlist.h>
#include <qobject.h>
#include <qstringlist.h>

#include "tmsgformat.h"
//#include <meteocontrol.h> //in editing
//#include <meta_data.h>
#include <commons/meteo_data/meteo_data.h> //include in meteocontrol.h

class TDecodeData;
class QFileInfo;

class TMsgRecv : public QObject
{
  Q_OBJECT

private:
  static const QString ERROR_EXT;
  static const QString DONE_EXT;
  static const QString UNK_EXT;
  static const QStringList PROC_EXT_LIST;

public:
  enum Error
  {
    kNoError = 0,
    kErrLoadCodeforms,
    kErrLoadDecoders,
    kErrLoadMsgmts
  };
  TMsgRecv( QObject* parent = 0 );
  virtual ~TMsgRecv();

  int error() const { return error_; }

  void setMark( bool mark ) { mark_ = mark; }
  void setOnce( bool once ) { once_ = once; }
  void setExts( const QStringList& exts ) { exts_ = exts; }
  void setUnk( bool unk ) { unk_ = unk; }

  void start();
  void stop();

  void addDirectory( const QString& path );

protected:
  void timerEvent( QTimerEvent* event );

private:
  int error_;
  int timerid_;
  //QHash< int, QHash< int, TMeteoData > > datahash_;
  // TMetaData metadata_;
  // TDecodeData* decodata_;
  QList<QDir> msgdirs_;
  //    MeteoControl mc_; //include in meteocontrol.h
  int64_t meteodatasize_;
  bool mark_;
  bool once_;
  QStringList exts_;
  bool unk_;

  void processDirs();
  void processDir( const QDir& dir );
  void processFile( const QFileInfo& fi );

  //void processTextMessage( const QByteArray& buf, const QDateTime& dt, int* datasizeintlg );
  //void processBinMessage( const QString& type, const QString& filename );
  QString processMessage( const QByteArray& buf, const QString& type, const QDateTime& dt );

  void markAsTreated( const QFileInfo& fi, const QString& status );
  bool isTreated( const QFileInfo& fi );
  QString treatedFileName( const QString& n, const QString& s ) const { return n + "." + s; }

private slots:
  void slotTimeout();

signals:
  void messageParsed( const QString& msgfile, const QByteArray& msg, const QString& type, const QDateTime& dt );
  //   void messageSaved( const QString& msgfile, const QString& type );

};

#endif
