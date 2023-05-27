#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_UDP_PACKET_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_UDP_PACKET_H

#include <QByteArray>
#include <qmap.h>
#include <qdatetime.h>

#include <sys/types.h>
#include "reeds.h"

namespace meteo {

class UdpPacket
{
  public:

    UdpPacket(const QByteArray& );
    UdpPacket(const UdpPacket&);
    UdpPacket();
    ~UdpPacket();

    void clear();

    UdpPacket& operator=(const UdpPacket& p);

   // bool operator!=(const UdpPacket& p) const;
   // bool operator==(const UdpPacket& p)const ;
    void setChunkSize( uint32_t  chunk_size){chunk_size_ = chunk_size;}    //!< Размер сегмента
    void setDataSize( uint32_t  data_size){data_size_ = data_size;}    //!< Размер данных
    void setMagic( uint32_t  magic){magic_ = magic;}    //!< CRC32 всех данных

    uint32_t getChunkShift() const {return chunk_shift_;}
    uint32_t getDataSize() const {return data_size_;}
    uint32_t getSrcDataSize() const{return src_data_size_;}
    uint32_t getChunkSize() const {return chunk_size_;}
    uint32_t getMagic() const {return magic_;}
    QString getDataName() const;
    bool getDataToSend(QByteArray*) const;
    bool getData(QByteArray*) const;
    const uint8_t* chunkData()const;
    QString getSenderName()const;
    bool isEndData();

    bool parse(const QByteArray& );

    void create(const QString &sender, uint32_t magic,
                uint32_t allsize, uint32_t src_size, const QString &fname,
                const QByteArray& chunk, uint32_t shift);

  private:

    u_int8_t  id_start_[3];   //!< Идентификатор начала сообщения.    Последовательность 0x010101
    uint32_t  msg_size_;   //!<Размер всего сообщения включая id_start и id_end
    u_int8_t  id_sender_length_;      //!< Длина идентификатора отправителя
    u_int8_t  *id_sender_=nullptr;//!< Идентификатор отправителя [id_sender_length]
    uint32_t  magic_;    //!< Идентификатор передаваемого файла, предназначен для корректной сборки файлов большого размера
    uint32_t  data_size_;    //!< Общий размер передаваемого файла с кодированием
    uint8_t   data_name_length_;//!< Длина имени файла
    uint8_t   *data_name_=nullptr;//!< Имя передаваемого файла file_name_length
    uint32_t  chunk_size_ = 256;    //!< Размер сегмента
    uint32_t  chunk_shift_;    //!< Смещение сегмента относительно начала файла
    uint8_t   *chunk_data_=nullptr;//!<Передаваемый сегмент[chunk_size]
    //uint32_t  crc32_;//!<Crc32(chunk_data) Не используется. При передаче кодированных данных передается размер исходных данных
    uint32_t  src_data_size_;//!<При передаче кодированных данных передается размер исходных данных
    uint8_t   id_end_[3];//!<Идентификатор окончания сообщения. Последовательность 0x030303

    const int min_size_ = 3+4+1+1+4+4+1+1+4+4+1+4+1;
    u_int8_t magic_start_[3] = { 1, 1, 1};
    u_int8_t magic_end_[3] = { 3, 3, 3};


};


class UdpPacketMap :public QMap <uint32_t, UdpPacket >
{
  public:
    explicit UdpPacketMap();
    explicit UdpPacketMap(const QString& sender, uint32_t magic, const QString& dname);
    bool insertOne(const uint32_t &key, const UdpPacket &value);


    uint32_t getFullDataWithZero(QByteArray *fulldata,
                             vector<pair<uint32_t, uint16_t> > *error_positions);
    QByteArray  getPayload() const;
    QByteArray getDecodedPayload(int *corrected, int *fail, bool has_errors);
   // QByteArray getPayloadWhithoutCorrection();
    QString getDataName() const;
    uint32_t getDataSize() const;
    uint32_t getSrcDataSize() const;

    uint32_t getMagic() const;
    const QDateTime& dt() const;

    void setSender(const QString &);
    void setMagic(uint32_t );
    void setDataName(const QString &);
    void setChunkSize(uint32_t as);
    void reCreate( );

    bool isFull()const;
    bool takeFirstChunkToSend(QByteArray * chunk);
    bool getFirstChunkToSend(QByteArray * chunk);
    int create(const QByteArray& data, uint32_t src_size);
    void codePayloadAndCreate(const QByteArray& data);

    std::vector<uint8_t> getFullData();


  private:
    bool getFirstChunk(UdpPacket * chunk);
    bool takeFirstChunk(UdpPacket * chunk);

    QList<uint32_t> chunk_shifts_;
    QDateTime dt_=QDateTime::currentDateTime();

    uint32_t data_size_=0;
    uint32_t cur_data_size_=0;
    uint32_t src_data_size_=0;
    uint32_t chunk_size_=0;
    uint32_t cur_chunk_shift_=0;
    uint32_t magic_=0;

   // uint32_t max_missing_chunks_in_order_ = 10;
   // uint32_t max_missing_chunks_total_ = 20;



    QString sender_;
    QString data_name_;
    QByteArray alldata_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMAS_H
