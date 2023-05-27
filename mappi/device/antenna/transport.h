#pragma once

#include "config.h"
#include "protocol.h"
#include "profile.h"
#include <qobject.h>
#include <QSerialPort>


namespace mappi {

namespace antenna {

/**
 * Транспортный уровень, имеет входной буфер и занимается фрагментацией входного потока.
 * В демонстрационном режиме порт всегда открывается успешно.
 */
class Transport :
  public QObject
{
  Q_OBJECT
public :
  explicit Transport(Protocol* protocol, QObject* parent = nullptr);
  virtual ~Transport();

  bool init(const Configuration& conf);
  bool isDemoMode() const { return demoMode_; }

  bool open();
  bool isOpen() const;
  void close();

  QString pinoutSignals() const;

signals :
  void recv(const QByteArray& buf);

public slots :
  bool send(const QByteArray& buf);

private slots :
  void onRead();
  void onError(QSerialPort::SerialPortError error);

private :
  Protocol* protocol_;
  QSerialPort* port_;

  bool demoMode_;
  Configuration::serialport_t param_;

  QByteArray buf_rx_;
};

}

}
