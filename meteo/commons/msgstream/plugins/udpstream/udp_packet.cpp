#include "udp_packet.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <commons/mathtools/mnmath.h>

#include <QFile>
#include <QProcess>
#include <QDataStream>

#include <arpa/inet.h>
#include <cstring>
#include <boost/crc.hpp>


namespace meteo {

UdpPacketMap::UdpPacketMap(const QString &sender,uint32_t magic,
                           const QString &dname)
  : QMap<uint32_t, UdpPacket>(),
    magic_(magic),
    sender_(sender),
    data_name_(dname)
{

}

UdpPacketMap::UdpPacketMap()
  : QMap<uint32_t, UdpPacket>()
{

}

uint32_t UdpPacketMap::getFullDataWithZero(QByteArray *fulldata,
                                           std::vector<pair <uint32_t, uint16_t>> *error_positions)
{
  uint32_t missing_chunks_total = 0;
  fulldata->resize(data_size_);
  fulldata->fill('\0');
  uint32_t cur_shift = 0;
  uint32_t chunk_size = 0;
  uint32_t needed_shift=0;
  uint32_t total=0;

  UdpPacketMap::const_iterator it = begin();
  UdpPacketMap::const_iterator eit = end();
  for(;it!=eit;++it){
    const UdpPacket &up = *it;
    cur_shift = up.getChunkShift();
    chunk_size = up.getChunkSize();
    if(needed_shift != cur_shift ){
      ++(missing_chunks_total);
      error_positions->push_back(std::make_pair(needed_shift,needed_shift+chunk_size));
    }

    needed_shift+=chunk_size;
    fulldata->replace(cur_shift, chunk_size, (const char*) up.chunkData(), chunk_size);
    ++total;
  }
  while(needed_shift < data_size_){
    error_positions->push_back(std::make_pair(needed_shift,needed_shift+chunk_size));
    needed_shift += chunk_size;
    ++total;
    ++missing_chunks_total;
  }
  if(0< missing_chunks_total){
    warning_log << QObject::tr("Пропущено %1 пакетов из %2")
                   .arg(missing_chunks_total).arg(total);
  }
  return missing_chunks_total;
}

void UdpPacketMap::codePayloadAndCreate(const QByteArray& data)
{

 /* QFile file("/tmp/out/"+getDataName()+ ".dec");
     if (file.open(QIODevice::WriteOnly)){
       file.write(data);
     }
  file.close();
*/
  std::vector <uint8_t> encoded_bytes = RS::RS_turbo_code(data.data(), data.size());
  create(QByteArray((char *)encoded_bytes.data(), encoded_bytes.size()),data.size());
}

QByteArray UdpPacketMap::getDecodedPayload(int *corrected,int *fail, bool has_errors)
{
  QByteArray fulldata;
  std::vector<pair <uint32_t, uint16_t>> error_positions;
  if(has_errors){
    getFullDataWithZero(&fulldata,&error_positions);
  } else {
    fulldata  = getPayload();
  }
  std::pair <vector<uint8_t>, RS::decode_result> rs_decode_result;

  rs_decode_result = RS::RS_turbo_decode((char *)fulldata.data(),fulldata.size(),
                                         error_positions);
  rs_decode_result.first.resize(getSrcDataSize());

//  QString filedir = "/tmp/inc/err/";
  if(has_errors){
    if(true == rs_decode_result.second.success){
 //     filedir += "correct/";
      *corrected = 1;
       debug_log<<getDataName() <<"RS decode successful! num of corrected symbol's:"<<rs_decode_result.second.total_corrected;
    }
    else{
 //     filedir += "fail/";
      *fail = 1;
      debug_log <<getDataName()<<"RS decode FAIL! num of corrected symbol's:"<<rs_decode_result.second.total_corrected;
      //return QByteArray();
    }
  /*  QFile file(filedir+getDataName()+ ".code");
       if (file.open(QIODevice::WriteOnly)){
         file.write(getPayload());
       }
    file.close();
    file.setFileName(filedir+getDataName()+ ".dec");
       if (file.open(QIODevice::WriteOnly)){
         QByteArray bdata = QByteArray::fromRawData((const char*)rs_decode_result.first.data(), rs_decode_result.first.size());
         file.write(bdata);
       }
    file.close();*/
  }/* else {
    QFile file("/tmp/inc/full/"+getDataName()+ ".code");
       if (file.open(QIODevice::WriteOnly)){
         file.write(fulldata);
       }
    file.close();
    file.setFileName("/tmp/inc/full/"+getDataName()+ ".dec");
       if (file.open(QIODevice::WriteOnly)){
         QByteArray bdata = QByteArray::fromRawData((const char*)rs_decode_result.first.data(), rs_decode_result.first.size());
         file.write(bdata);
       }
    file.close();
  }*/
  return QByteArray((char *)rs_decode_result.first.data(), rs_decode_result.first.size()) ;
}


QString UdpPacketMap::getDataName()const
{
  return data_name_;
}

uint32_t UdpPacketMap::getMagic() const
{
  return magic_;
}

uint32_t UdpPacketMap::getDataSize() const
{
  return cur_data_size_;
}

uint32_t UdpPacketMap::getSrcDataSize() const
{
  return src_data_size_;
}


bool  UdpPacketMap::takeFirstChunkToSend(QByteArray * chunk)
{
  UdpPacket value;
  bool res = takeFirstChunk(&value);
  value.getDataToSend(chunk);
  return res;
}

bool UdpPacketMap::takeFirstChunk(UdpPacket * avalue)
{
  bool is_end = false;
  if(keys().contains(cur_chunk_shift_)){
    *avalue = take(cur_chunk_shift_);
    cur_chunk_shift_+=avalue->getChunkSize();
    if(avalue->getDataSize() == cur_chunk_shift_){
      is_end = true;
      cur_chunk_shift_=0;
    }
  }
  return is_end;
}

bool  UdpPacketMap::getFirstChunkToSend(QByteArray * chunk)
{
  UdpPacket value;
  bool res = getFirstChunk(&value);
  value.getDataToSend(chunk);
  return res;
}

bool UdpPacketMap::getFirstChunk(UdpPacket * avalue)
{
  bool is_end = false;
  if(keys().contains(cur_chunk_shift_)){
    *avalue = value(cur_chunk_shift_);
    cur_chunk_shift_+=avalue->getChunkSize();
    if(avalue->getDataSize() == cur_chunk_shift_){
      is_end = true;
      cur_chunk_shift_=0;
    }
  }
  return is_end;
}


bool UdpPacketMap::isFull() const
{
 /* QByteArray ba = getPayload();
  uint size = ba.size();
  static boost::crc_32_type crc32_calculator;
  crc32_calculator.reset();
  crc32_calculator.process_bytes(ba.data() , size );
  uint32_t crc32 = crc32_calculator.checksum();
  if(size == data_size_ && getMagic() != crc32){
    debug_log << "bad MagicCrc";
  }
  if(cur_data_size_>data_size_){
    debug_log <<"HHH"<< cur_data_size_ << data_size_;
  }*/
  return(cur_data_size_==data_size_);
}

QByteArray  UdpPacketMap::getPayload() const {
  QByteArray ba;
  UdpPacketMap::const_iterator it = begin();
  UdpPacketMap::const_iterator eit = end();
  for(;it!=eit;++it){
    QByteArray baa;
    it.value().getData(&baa);
    ba+=baa;
  }
  return ba;
}

bool UdpPacketMap::insertOne(const uint32_t &key, const UdpPacket &value)
{
  if(0==size()){
    cur_data_size_=0;
    data_size_=value.getDataSize();
    //chunk_size_ = value.getChunkSize();
    data_name_ = value.getDataName();
    sender_ = value.getSenderName();
    magic_ = value.getMagic();
    src_data_size_ = value.getSrcDataSize();
  }
  if(!contains(key)){
    cur_data_size_+=value.getChunkSize();
    insert(key,value);
  }
  dt_=QDateTime::currentDateTime();
  return true;
}

void UdpPacketMap::setChunkSize(uint32_t as)
{
  chunk_size_ = as;
}

void UdpPacketMap::setSender(const QString &sender)
{
  sender_ = sender;
}

void UdpPacketMap::setMagic(uint32_t magic)
{
  magic_ = magic;
}

void UdpPacketMap::setDataName(const QString &dname)
{
  data_name_ = dname;
}

void UdpPacketMap::reCreate(){
  uint32_t new_size = getDataSize();
  static boost::crc_32_type crc32_calculator;
  QByteArray dc = getPayload();
  crc32_calculator.reset();
  crc32_calculator.process_bytes(dc.data() , dc.size() );
  uint32_t crc32 = crc32_calculator.checksum();
  UdpPacketMap::iterator it = begin();
  UdpPacketMap::iterator eit = end();
  for(;it!=eit;++it){
    it.value().setDataSize(new_size);
    it.value().setMagic(crc32);
  }
}

int UdpPacketMap::create(const QByteArray& data, uint32_t src_size)
{
  uint32_t allsize = data.size();
  QByteArray chunk;
  uint32_t shift = 0;
  while(shift < allsize){
    chunk = data.mid(shift,chunk_size_);
    UdpPacket up;
    up.create(sender_, magic_, allsize,src_size, data_name_, chunk, shift);
    insertOne(shift,up);
    shift+=chunk.size();
  }
  return shift;
}


UdpPacket::UdpPacket( const QByteArray& data)
{
  parse( data );
}

const QDateTime& UdpPacketMap::dt() const
{
  return dt_;
}

UdpPacket::UdpPacket(const UdpPacket& src){
  *this = src;
}


UdpPacket::UdpPacket()
{
}

UdpPacket::~UdpPacket()
{
  clear();
}

bool  UdpPacket::isEndData(){
  return(id_end_[0] == 3 &&
      id_end_[1] == 3 &&
      id_end_[2] == 3  );
}

const uint8_t* UdpPacket::chunkData() const
{
  return chunk_data_;
}


QString UdpPacket::getSenderName()const{
  QString name;
  if(0 < id_sender_length_){
    name=name.fromUtf8((const char*)id_sender_,id_sender_length_);
  }
  return name;
}

QString UdpPacket::getDataName()const{
  QString name;
  if(0 < data_name_length_){
    name = name.fromUtf8((const char*)data_name_,data_name_length_);
  }
  return name;
}


UdpPacket& UdpPacket::operator=(const UdpPacket& p)
{
  if ( this == &p ) {
    return *this;
  }
  //for (int i = 0; i < 3; i++) id_start_[i] = p.id_start_[i];
  std::memcpy( id_start_, p.id_start_, 3);
  msg_size_ = p.msg_size_;
  id_sender_length_ = p.id_sender_length_;
  id_sender_ = new u_int8_t[id_sender_length_];
  // for (int i = 0; i < id_sender_length_; i++) id_sender_[i] = p.id_sender_[i];
  std::memcpy( id_sender_, p.id_sender_, id_sender_length_);
  magic_ = p.magic_;
  data_size_ = p.data_size_;
  data_name_length_ = p.data_name_length_;
  data_name_ = new u_int8_t[data_name_length_];
  //for (int i = 0; i < data_name_length_; i++) data_name_[i] = p.data_name_[i];
  std::memcpy( data_name_, p.data_name_, data_name_length_);
  chunk_size_ = p.chunk_size_;
  chunk_shift_ = p.chunk_shift_;
  chunk_data_ = new u_int8_t[chunk_size_];
  //for (uint i = 0; i < chunk_size_; i++) chunk_data_[i] = p.chunk_data_[i];
  std::memcpy( chunk_data_, p.chunk_data_, chunk_size_);
  //crc32_ = p.crc32_;
  src_data_size_ = p.src_data_size_;
  std::memcpy( id_end_, p.id_end_, 3);
  // for (int i = 0; i < 3; i++) id_end_[i] = p.id_end_[i];

  return *this;
}



void UdpPacket::clear()
{
  if(nullptr != id_sender_){
    delete[] id_sender_;
    id_sender_ = nullptr;
  }
  if(nullptr != data_name_){
    delete[] data_name_;
    data_name_ = nullptr;
  }
  if(nullptr != chunk_data_){
    delete[] chunk_data_;
    chunk_data_ = nullptr;
  }

  msg_size_ = 0;
  id_sender_length_ = 0;
  magic_ = 0;
  data_size_ = 0;
  data_name_length_ = 0;
  chunk_shift_ = 0;
  //crc32_ = 0;
  src_data_size_ = 0;
}


void UdpPacket::create(const QString &sender, uint32_t magic,
                       uint32_t allsize,uint32_t src_size, const QString &dname,
                       const QByteArray& chunk, uint32_t shift)
{
  //Идентификатор начала сообщения.    Последовательность 0x010101
  id_start_[0]=1;
  id_start_[1]=1;
  id_start_[2]=1;
  msg_size_ = 3;
  msg_size_ += 4;//+Размер всего сообщения включая id_start и id_end

  id_sender_length_ = sender.toUtf8().size();
  msg_size_ += 1;
  id_sender_ = new u_int8_t[id_sender_length_];
  memcpy(id_sender_,sender.toUtf8().data(),id_sender_length_);
  msg_size_ += id_sender_length_;
  magic_ = magic;    // Идентификатор передаваемого файла, предназначен для корректной сборки файлов большого размера
  msg_size_ +=4;

  data_size_ = allsize;    //!< Общий размер передаваемого файла
  msg_size_ +=4;
  data_name_length_ = dname.toUtf8().size();//!< Длина имени файла
  msg_size_ +=1;
  // Имя передаваемого файла data_name_length
  if(0<data_name_length_){
    data_name_ = new u_int8_t[data_name_length_];
    memcpy(data_name_,dname.toUtf8().data(),data_name_length_);
    msg_size_ +=data_name_length_;
  }

  chunk_size_ = chunk.size(); // Размер сегмента
  msg_size_ +=4;
  chunk_shift_=shift;    // Смещение сегмента относительно начала файла
  msg_size_ +=4;
  if(0<chunk_size_){
    chunk_data_ = new u_int8_t[chunk_size_];//Передаваемый сегмент[chunk_size]
    memcpy(chunk_data_,chunk.data(),chunk_size_);
    msg_size_ +=chunk_size_;
    //crc32_ = 0;
    src_data_size_ = src_size;
    msg_size_ +=4;
  }
  //Идентификатор окончания сообщения. Последовательность 0x030303

  id_end_[0]=3;
  id_end_[1]=3;
  id_end_[2]=3;

  msg_size_ += 3;//Размер всего сообщения включая id_start и id_end
}
bool UdpPacket::getData(QByteArray* data) const
{
  if(0 >= chunk_size_){
    return false;
  }
  if(0<chunk_size_&&nullptr != chunk_data_){
    data->resize(chunk_size_);
    *data = QByteArray::fromRawData((const char*)chunk_data_,chunk_size_);
    //memcpy(data->data(),(char*)chunk_data_,chunk_size_);
  }
  return true;
}

bool UdpPacket::getDataToSend(QByteArray* dat) const
{
  char* adata = new char[msg_size_] ;
  int data =0;
  int sz = 0;
  sz = sizeof(id_start_);  memcpy(adata+data,&id_start_[0],sz);
  data+=sz;  sz = sizeof(msg_size_); memcpy(adata+data,&msg_size_,sz);
  data+=sz; sz = sizeof(id_sender_length_); memcpy(adata+data,&id_sender_length_,sz);
  if(id_sender_length_){
    data+=sz; sz = id_sender_length_; memcpy(adata+data,(char*)id_sender_,sz);
  }
  data+=sz; sz = sizeof(magic_); memcpy(adata+data,&magic_,sz);
  data+=sz; sz = sizeof(data_size_); memcpy(adata+data,&data_size_,sz);
  data+=sz; sz = sizeof(data_name_length_);  memcpy(adata+data,&data_name_length_,sz);
  if(0<data_name_length_){
    data+=sz; sz = data_name_length_; memcpy(adata+data,(char*)data_name_,sz);
  }
  data+=sz; sz = sizeof(chunk_size_); memcpy(adata+data,&chunk_size_,sz);
  data+=sz; sz = sizeof(chunk_shift_); memcpy(adata+data,&chunk_shift_,sz);
  if(0<chunk_size_){
    data+=sz; sz = chunk_size_; memcpy(adata+data,(char*)chunk_data_,sz);
  }
  //data+=sz; sz = sizeof(crc32_); memcpy(adata+data,&crc32_,sz);
  data+=sz; sz = sizeof(src_data_size_); memcpy(adata+data,&src_data_size_,sz);
  data+=sz; sz = sizeof(id_end_); memcpy(adata+data,&id_end_,sz);
  *dat = QByteArray::fromRawData(adata,msg_size_);
  return true;
}


bool UdpPacket::parse(const QByteArray& dat)
{
  if(1 > dat.size()) return false;
  const char* data = dat.data();
  int sm = 0;
  int sz = 0;
  sz = sizeof(id_start_); memcpy(&id_start_[0],data,sz);
  if(id_start_[0] != 1 ||
     id_start_[1] != 1 ||
     id_start_[2] != 1 )
  {
    return false;
  }
  sm+=sz; sz = sizeof(msg_size_); memcpy(&msg_size_,data+sm,sz);
  sm+=sz; sz = sizeof(id_sender_length_); memcpy(&id_sender_length_,data+sm,sz);
  if(0 < id_sender_length_){
    id_sender_ =new u_int8_t[id_sender_length_];
    sm+=sz; sz = id_sender_length_; memcpy((char*)id_sender_,data+sm,sz);
  }
  sm+=sz; sz = sizeof(magic_); memcpy(&magic_,data+sm,sz);
  sm+=sz; sz = sizeof(data_size_); memcpy(&data_size_,data+sm,sz);
  sm+=sz; sz = sizeof(data_name_length_); memcpy(&data_name_length_,data+sm,sz);

  // Имя передаваемого файла data_name_length
  if(0<data_name_length_){
    data_name_ =new u_int8_t[data_name_length_];
    sm+=sz; sz = data_name_length_; memcpy((char*)data_name_,data+sm,sz);
  }
  sm+=sz; sz = sizeof(chunk_size_); memcpy(&chunk_size_,data+sm,sz);
  sm+=sz; sz = sizeof(chunk_shift_); memcpy(&chunk_shift_,data+sm,sz);
  if(0<chunk_size_){
    chunk_data_ =new u_int8_t[chunk_size_];
    sm+=sz; sz = chunk_size_; memcpy((char*)chunk_data_,data+sm,sz);
  }
  //sm+=sz; sz = sizeof(crc32_); memcpy(&crc32_,data+sm,sz);
  sm+=sz; sz = sizeof(src_data_size_); memcpy(&src_data_size_,data+sm,sz);
  sm+=sz; sz = sizeof(id_end_); memcpy(&id_end_[0],data+sm,sz);
  if(!isEndData())
  {
    clear();
    return false;
  }
  return true;
}



} // meteo
