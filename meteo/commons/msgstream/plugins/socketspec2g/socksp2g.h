#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_H

#include <sys/types.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {
namespace socksp2g {

enum RecvState { kStart, kInfoMsg, kInfoFile, kEnd, kAck,kRR, kError };

//! Максимальная длина передаваемого сообщения.
//const uint32_t kMaxLen = std::numeric_limits<uint32_t>::max();
const int32_t kMaxLen = 512*1024;

//! Тип пакета.
enum PackType {
  kTcpIpData      = 0x01,
  kTcpIpFile      = 0x08,
  kTcpIpAck       = 0x02,
  kTcpIpEnd       = 0x04,
  kTcpIpRR        = 0x06
};
#pragma pack(push, 1)
//! Структура служебного пакета
struct ServicePackData
{
  int8_t    magic[5]; //!< Идентификатор соединения, составляется из символа 'U' и 4-значного номера порта сервера.
  u_int8_t  type;     //!< Тип пакета, определяет назначение пакета (PackType).
  u_int32_t num;      //!< Последовательный номер сообщения при передаче (в бинарном виде).
  u_int32_t len;      //!< Размер текста сообщения в байтах, при передаче в упакованном виде определяет размер сжатого текста.
  int16_t  info_len; //!< Размер заголовка информационного пакета.
};
const int SERV_PACKET_SIZE = 16;

//! Структура заголовка информационного пакета при передаче ообщений.
struct InfoMsgPackHead
{
  int8_t    format;     //!< Формат метеосообщения. formatType
  uint16_t  text_offset;//!< Смещение от начала информационного пакета на текст сообщения.
  int8_t    ahd[10];    //!< Группы TTAAiiCCCC сокращённого заголовка метеосообщения.
  int8_t    yygg[6];    //!< Группа времени YYGGgg метеосообщения.
  int8_t    bbb[3];     //!< Группа BBB сокращённого заголовка метеосообщения.
};

//! Структура заголовка информационного пакета при передаче файла.
struct InfoFilePackHead
{
  uint64_t  size;     //!< размер файла
  char      file_name[512];    //!< имя файла.
};

#pragma pack(pop)

//! Формат метеосообщения.
enum formatType {
  kWMOFormat      = 0x01,
  kGMSFormat      = 0x02,
};

const int INFO_MSG_PACKET_SIZE = 22;



const int INFO_FILE_NAME_PACKET_SIZE = 8;
const int INFO_FILE_PACKET_SIZE = 520;

//!
class ServicePacket
{
public:

  RecvState setPacket(const QByteArray *raw);

  //!
  bool parse(const QByteArray& raw);
  //!
  static ServicePacket createRR(int8_t* );

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

  int setPacket(socksp2g::ServicePacket *start, const QByteArray *raw, bool compressed);
  //!
  //! \brief parseMsg
  //! \param head
  //! \param tlg
  //! \return
  //!
  bool parseMsg(const QByteArray& head, const QByteArray& tlg);
  //!
  //! \brief parseFile
  //! \param head
  //! \param tlg
  //! \return
  //!

  bool parseFile(const QByteArray& head, const QByteArray& tlg);

  //!
  //! \brief fromMessage
  //! \param msg
  //! \return
  //!
  static InfoPacket fromMessage(const ::meteo::tlg::MessageNew& msg);

  //!
  //! \brief InfoPacket
  //!
  InfoPacket() { clear(); }

  //!
  //! \brief head
  //! \return
  //!
  InfoMsgPackHead head() const { return msghead_; }
  //!

  ::meteo::tlg::MessageNew msg() const { return msg_; }
  ::meteo::tlg::MessageNew* mutableMsg() {return &msg_;}

  //!
  QByteArray toNet() const;
  //!
  QByteArray rawTlg() const;

  //!
  void clear();

  void setOutDir(const QString& outd){outdir_ = outd;}
  const QString& outDir(){ return outdir_;}
private:

  InfoMsgPackHead msghead_;
  InfoFilePackHead filehead_;
  ::meteo::tlg::MessageNew msg_;
  QString outdir_;
};

struct SendBox
{
  socksp2g::ServicePacket start;
  socksp2g::InfoPacket info;
  socksp2g::ServicePacket end;
  QDateTime date_recv;
};

} // socksp2g
} // meteo

TLog& operator <<(TLog& log, const meteo::socksp2g::ServicePacket& pack);

TLog& operator <<(TLog& log, const meteo::socksp2g::InfoPacket& pack);

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SRIV512_H
