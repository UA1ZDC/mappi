#ifndef	MNIO_H
#define	MNIO_H

#include <stdio.h>
#include <stdlib.h>

#include <qtcpsocket.h>
#include <qstring.h>
#include <qobject.h>
#include <qbytearray.h>

namespace MnCommon {

  /*! 
   * \brief Неблокирующее чтение из QTcpSocket
   * \param socket - указатель на tcp-сокет
   * \param incomingBuffer - буфер, в который будет записана прочитанная из сокета информация.
   *                         буфер увеличивается до необходимого размера внутри функции
   * \param bytes - количество байт, которые должны быть прочитаны
   * \return количество прочитанных байт
   */
  qint64 readFromSocket(QTcpSocket *socket, QByteArray* incomingBuffer, qint64 bytes);

  /*! 
   * \brief - неблокирующая запись в QTcpSocket
   * \param socket - указатель на tcp-сокет
   * \param outgoingBuffer - данные, предназначенные для записи в сокет
   * \return - количество записанных байт
   */
  qint64 writeToSocket(QTcpSocket *socket, const QByteArray &outgoingBuffer);

  /*! 
   * \brief функция предназначена для установки параметров сокета, которые обеспечивают контроль наличия соединения
   * функция не работает в ms windows
   * \param s - номер дескриптора сокета
   * \param fl - SO_KEEPALIVE (true - выполнять периодическую отправку тестовых пакетов, false - не выполнять)
   * \param maxidle - период проверки соединения в секундах
   * \param cnt - максимальное количество проверок до сброса соединения
   * \param intrvl - интервал между проверочными пакетами в секундах.
   * Проверена работоспособность программы со сл. значениями функции - fl = true, maxidle 10, cnt = 3, intrvl = 2
   */
  void setSocketOptions(int s, int32_t fl = 1 ,int32_t maxidle = 10,int32_t cnt = 3,int32_t intrvl = 2);

  /*!
   * \brief функция предназначена для получения MAC-уровня POSIX-сокета. На ОС Debian всегда возвращает 0
   * \param sock - номер дескриптора, для которого требуется получить MAC-level
   */
  uint8_t socket_mac_level(int sock);

  /**
   * @brief process_self_mac_level
   * @return Mac-уровень текущего процесса(0 для Debian)
   */
  uint8_t process_self_mac_level();
}

#endif

