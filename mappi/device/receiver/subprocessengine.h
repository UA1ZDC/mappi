#pragma once

#include "engine.h"
#include <qprocess.h>
#include <QNetworkAccessManager>


namespace mappi {

namespace receiver {

/**
 * Движок запускает внешнее приложение в дочернем процессе.
 * В данном случае запускается python script (gnu radio).
 * raw-файл пишет непосредственно внешнее приложение.
 */
class SubProcessEngine :
  public Engine
{
  Q_OBJECT
public :
  explicit SubProcessEngine(QObject* parent = nullptr);
  virtual ~SubProcessEngine();

  virtual const char* type() const { return "sub process engine"; }

  virtual bool configure(const Configuration& conf);
  virtual bool start(Satellite* satellite, const Parameters& param);
  virtual bool stop();

  virtual void changeVariable(const QString name, const double value);
  virtual void updateFreq(const float freq);

private slots :
  void fromOutput();
  void fromError();

private :
  QNetworkAccessManager* networkManager_;
  QProcess* app_;
  QString appFolder_;
  QString rawFile_;
  QUrl requestURL_;
  bool separateHeader_;

  void deleteOldFiles(const QString &directoryPath);

  const QString XMLRPCServer = "http://%1:%2/RPC2";
  const QString XMLRPC_Request = "<?xml version='1.0'?>\n"
                                 "<methodCall>\n"
                                 "<methodName>%1</methodName>\n"
                                 "<params>\n"
                                 "<param>\n"
                                 "<value><double>%2</double></value>\n"
                                 "</param>\n"
                                 "</params>\n"
                                 "</methodCall>";
};

}

}
