#pragma once

#include <commons/geobasis/coords.h>
#include <sat-commons/satellite/satellite.h>
#include <qobject.h>
#include <qprocess.h>
#include <qfile.h>


namespace mappi {

namespace receiver {
/*
class FileReader :
  public QObject
{
  Q_OBJECT
public :
  explicit FileReader(const QString& file, QObject* parent = nullptr);
  virtual ~FileReader();

  bool open();
  void close();

signals :
  void read(const QByteArray& buf);

protected :
  virtual void timerEvent(QTimerEvent* event);

private :
  QFile* file_;
  int timerId_;
};
*/

class GnuRadio :
  public QObject
{
  Q_OBJECT
public :
  struct param_t {
    Coords::GeoCoord site;
    float freq;
    float gain;
    float rate;
    bool adjust_freq;
  };

public :
  static const char* SCRIPT_STUB;

public :
  // explicit GnuRadio(const QString& rawFile, QObject* parent = nullptr);
  explicit GnuRadio(QObject* parent = nullptr);
  virtual ~GnuRadio();

  bool start(const QString& script, Satellite* satellite, const param_t& param);
  bool stop();

signals :
  void readyRead(const QByteArray& buf);

protected :
  virtual void timerEvent(QTimerEvent* event);

private slots :
  void fromOutput();
  void fromError();

private :
  // FileReader* reader_;
  QProcess* pyapp_;
  Satellite* satellite_;
  int timerId_;
  param_t param_;
};

}

}
