#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SRIV512_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SRIV512_H

#include <sys/types.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {
namespace sriv512 {

  enum RecvState { kStart, kInfo, kEnd, kAck,kRR, kError };

  //! Максимальная длина передаваемого сообщения.
  //const uint32_t kMaxLen = std::numeric_limits<uint32_t>::max();
  const int32_t kMaxLen = 512*1024;

  //! Тип пакета.
enum PackType {
  kTcpIpData      = 1,
  kTcpIpDataZ     = 5,
  kTcpIpDataZC    = 0x85,
  kTcpIpAck       = 2,
  kTcpIpEnd       = 4,
  kTcpIpRR        = 6,
  kTcpCompress    = 0x80,
};

//! Структура служебного пакета
struct ServicePackData
{
  int8_t    magic[5]; //!< Идентификатор соединения, составляется из символа 'U' и 4-значного номера порта сервера.
  u_int8_t  type;     //!< Тип пакета, определяет назначение пакета.
  u_int32_t num;      //!< Последовательный номер сообщения при передаче (в бинарном виде).
  u_int32_t len;      //!< Размер текста сообщения в байтах, при передаче в упакованном виде определяет размер сжатого текста.
  u_int8_t  info_len; //!< Размер заголовка информационного пакета.
};


//! Структура заголовка информационного пакета.
struct InfoPackHead
{
  int8_t    format;     //!< Формат метеосообщения.
  uint16_t  text_offset;//!< Смещение от начала информационного пакета на текст сообщения.
  int8_t    ahd[10];    //!< Группы TTAAiiCCCC сокращённого заголовка метеосообщения.
  int8_t    yygg[6];    //!< Группа времени YYGGgg метеосообщения.
  int8_t    bbb[3];     //!< Группа BBB сокращённого заголовка метеосообщения.
};

//!
class ServicePacket
{
public:

  RecvState setPacket(const QByteArray *raw);

  //!
  static bool parse(ServicePacket* pack, const QByteArray& raw);
  //!
  static ServicePacket createRR();

  //!
  ServicePacket();
  //!
  ServicePacket(PackType t, int8_t* magic = 0);

  //!
  void setType(PackType t) { data_.type = t; }
  //!
  void setNum(u_int32_t n) { data_.num = n; }
  //!
  void setLen(u_int32_t l) { data_.len = l; }
  //!
  void setInfoLen(u_int8_t l) { data_.info_len = l; }
  //!
  void setMagic(int8_t* magic);

  //!
  ServicePackData data() const { return data_; }
  //!
  void clear();

  QByteArray toNet() const;

  //!
  bool match(const ServicePacket& pack) const;

private:
  ServicePackData data_;
};

//!
class InfoPacket
{
public:

  bool setPacket(sriv512::ServicePacket *start, const QByteArray *raw);


  //!
  static bool parse(InfoPacket* pack, const QByteArray& head, const QByteArray& tlg);
  //!
  static InfoPacket fromMessage(const ::meteo::tlg::MessageNew& msg);

  //!
  InfoPacket() { clear(); }

  //!
  InfoPackHead head() const { return head_; }
  //!
  ::meteo::tlg::MessageNew msg() const { return msg_; }
  ::meteo::tlg::MessageNew* mutableMsg() {return &msg_;}

  //!
  QByteArray toNet() const;
  //!
  QByteArray rawTlg() const;

  //!
  void clear();

private:
  InfoPackHead head_;
  ::meteo::tlg::MessageNew msg_;
};

struct SendBox
{
  sriv512::ServicePacket start;
  sriv512::InfoPacket info;
  sriv512::ServicePacket end;
  QDateTime date_recv;
};

} // sriv512
} // meteo

TLog& operator <<(TLog& log, const meteo::sriv512::ServicePacket& pack);

TLog& operator <<(TLog& log, const meteo::sriv512::InfoPacket& pack);

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SRIV512_H
