#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMAS_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMAS_H

#include <QByteArray>

#include <sys/types.h>

namespace meteo {


  //! Тип пакета.
  enum PackType {
    kTcpIpData      = 1,//!< пакет описания (начала передачи)  сообщения;
    kTcpIpDataZ     = 5,//!< пакет описания (начала передачи)
                        //!< сообщения при использовании режима оптимального сжатия,
                        //!< в этом случае также применяется бит сжатия данных
                        //!< TCP_COMPRESS
    kTcpIpAck       = 2,//!< пакет подтверждения приема сообщения;
    kTcpIpEnd       = 4,//!< пакет окончания передачи сообщения;
    kTcpIpRR        = 6, //!< пакет готовности к работе.
    kTcpCompress    = 0x80,
  };


typedef struct
{
  u_int8_t  type;   //!< тип пакета, определяет назначение пакета
  int32_t   ijp;    //!< идентификатор сообщения  в системе  MTS;
  u_int16_t len;    //!< размер сообщения в байтах, при передаче в упакованном виде определяет размер сжатого текста;
  u_int16_t num;    //!< последовательный номер сообщения при передаче
  int8_t    ahd[12];//!< сокращенный заголовок сообщения AHD;
  int8_t    pri;    //!< приоритет сообщения, определенный для данного вида информации.
} TcpServicePacket;


//! структура идентификатора сообщения в системе  UniMAS (UTCP_MODE)
typedef struct
{
  int16_t   jno;    //!< номер журнала
  int32_t   ijp;    //!< адрес сообщения в журнале
  u_int8_t  textno; //!< номер текста сообщения
} jinfo;


typedef struct
{
  u_int8_t  type;   //!< тип пакета, определяет назначение пакета
  u_int8_t  format;   //!< формат сообщения
  jinfo     ijp;      //!< идентификатор сообщения  в системе  UniMAS
  u_int16_t text_offs;//!< смещение от начала заголовка сообщения (SOH) на текст сообщения в байтах
  u_int16_t len;    //!< размер сообщения в байтах, при передаче в упакованном виде определяет размер сжатого текста;
  u_int16_t num;    //!< последовательный номер сообщения при передаче
  int8_t    ahd[12];//!< сокращенный заголовок сообщения AHD;
  int8_t    pri;    //!< приоритет сообщения, определенный для данного вида информации.

} UtcpServicePacket;


class TServicePacket
{
public:
  enum Mode { TCP, UTCP };

  u_int8_t&  type;
  u_int8_t&  utcp_format;
  jinfo&     utcp_ijp;
  u_int16_t& utcp_text_offs;
  int32_t&   tcp_ijp;
  u_int16_t& len;
  u_int16_t& num;
  int8_t*    ahd;
  int8_t&    pri;

  TcpServicePacket tcp;
  UtcpServicePacket utcp;

public:
  TServicePacket(Mode m, const char* data = nullptr);
  TServicePacket(const TServicePacket& p);
  int parse(const QByteArray& ba);
  void clear();

  TServicePacket toAck() const;
  QByteArray toNet() const;
  inline bool isTcpMode() const  { return TCP == mode_; }
  TServicePacket& operator=(const TServicePacket& p);
  bool operator!=(const TServicePacket& p) const;
  bool operator==(const TServicePacket& p)const ;
  void printDebug() const;
  int packetSize() {return packetSize_;}
private:
  void copyFrom(const char* data);
private:
  Mode mode_;
  int  packetSize_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMAS_H
