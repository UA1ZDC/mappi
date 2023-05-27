#include "rawparser.h"

#include <arpa/inet.h>
#include <cstring>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/pbtools.h>

namespace meteo {

RawParser::RawParser()
  : byteOrder_(kHostOrder)
{
}

bool RawParser::parse(google::protobuf::Message* msg, QByteArray* rawData)
{
  // очищаем, т.к. заполнение полей происходит по мере разбора, то
  // может возникнуть ситуация когда значения от предыдущего блока данных
  // будет перемешаны со значениями следующего блока данных (принятого частично).
  msg->Clear();

  // перед началом парсинга необходимо иметь список всех маркеров начала и знать размер самого длинного из них,
  // чтобы исключить потерю данных, если в буфере меньше данных, чем размер маркера
  // TODO: перенести данный блок кода в функцию setFormat()
  int begMaxLen = 0;
  QList<QByteArray> begList;
  for ( int i=0,isz=format_.begin_size(); i<isz; ++i ) {
    begList << QByteArray::fromHex(pbtools::fromBytes(format_.begin(i).hex_data()));
    begMaxLen = qMax(begMaxLen, begList.last().size());
  }

  if ( buffer_.size() < begMaxLen ) { return false; }

  // поиск маркера начала данных
  int begIdx = -1;
  while ( buffer_.size() > 0 && !begList.isEmpty() ) {
    for ( int i=0,isz=begList.size(); i<isz; ++i ) {
      if ( buffer_.startsWith(begList[i]) ) {
        begIdx = i;
        break;
      }
    }

    if ( -1 != begIdx ) {
      break;
    }

    buffer_.remove(0,1);
  }

  if ( buffer_.isEmpty() ) { return false; }

  int offset = 0;

  if ( -1 != begIdx ) {
    if ( format_.begin(begIdx).has_path() ) {
      int sz = begList[begIdx].size();
      QStringList path = pbtools::toQString(format_.begin(begIdx).path()).split(".");
      QByteArray raw = buffer_.mid(offset,sz);

      setValue(msg, path, raw);
    }

    offset = begList[begIdx].size();
  }

  for ( int i=0,isz=format_.chunk_size(); i<isz; ++i ) {
    int sz = format_.chunk(i).size();

    if ( buffer_.size() < offset + sz ) {
//      debug_log << "too little data, wait next part...";
      return false;
    }

    offset += sz;

    if ( !format_.chunk(i).has_path() ) {
      continue;
    }

    QStringList path = pbtools::toQString(format_.chunk(i).path()).split(".");
    QByteArray raw = buffer_.mid(offset-sz,sz);

    setValue(msg, path, raw);
  }

  QList<QByteArray> endList;
  for ( int i=0,isz=format_.end_size(); i<isz; ++i ) {
    endList << QByteArray::fromHex(pbtools::fromBytes(format_.end(i).hex_data()));
  }

  int endIdx = -1;
  for ( int i=0,isz=endList.size(); i<isz; ++i ) {
    int sz = endList[i].size();
    QByteArray raw = buffer_.mid(offset,sz);
    if ( raw.startsWith(endList[i]) ) {
      endIdx = i;
      break;
    }
  }

  if ( endList.size() > 0 && -1 == endIdx ) {
    debug_log << QByteArray(buffer_).replace("\r","\\r").replace("\n","\\n");
    debug_log << "end not found";
    return false;
  }

  if ( -1 != endIdx ) {
    if ( format_.end(endIdx).has_path() ) {
      int sz = endList[endIdx].size();
      QStringList path = pbtools::toQString(format_.end(endIdx).path()).split(".");
      QByteArray raw = buffer_.mid(offset,sz);

      setValue(msg, path, raw);
    }

    offset += endList[endIdx].size();
  }

  if ( 0 != rawData ) { *rawData = buffer_.left(offset); }

  buffer_.remove(0, offset);

  return true;
}

bool RawParser::setValue(google::protobuf::Message* msg, const QStringList& path, const QByteArray& raw) const
{
  PbHelper pbh(msg);

  bool hasField = false;
  google::protobuf::FieldDescriptor::CppType cpptype = pbh.cppType(path, &hasField);
  if ( !hasField ) {
    debug_log << "field path '" << path.join(".") << "' not found";
    return false;
  }

  int sz = raw.size();

  switch ( cpptype )
  {
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
    {
      if ( 1 == sz ) {
        uint8_t v;
        std::memcpy( &v, raw.data(), sz);
        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else if ( 2 == sz ) {
        uint16_t v;
        std::memcpy( &v, raw.data(), sz);

        if ( kNetOrder == byteOrder_ ) { v = ntohs(v); }

        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else if ( 4 == sz ) {
        uint32_t v;
        std::memcpy( &v, raw.data(), sz);

        if ( kNetOrder == byteOrder_ ) { v = ntohl(v); }

        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else {
        debug_log << "size of chunk must be 1, 2 or 4 bytes";
        return false;
      }
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
    {
      if ( 1 == sz ) {
        int8_t v;
        std::memcpy( &v, raw.data(), sz);
        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else if ( 2 == sz ) {
        int16_t v;
        std::memcpy( &v, raw.data(), sz);

        if ( kNetOrder == byteOrder_ ) { v = ntohs(v); }

        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else if ( 4 == sz ) {
        int32_t v;
        std::memcpy( &v, raw.data(), sz);

        if ( kNetOrder == byteOrder_ ) { v = ntohl(v); }

        if ( !pbh.setValue(path, QString::number(v)) ) {
          debug_log << "can't set field value:" << path.join(".");
        }
      }
      else {
        debug_log << "size of chunk must be 1, 2 or 4 bytes";
        return false;
      }
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
    {
      char* arr = new char[sz];
      std::memset(arr, '\0', sz);
      std::memcpy(arr, raw.data(), sz);
      if ( !pbh.setValue(path, QByteArray::fromRawData(arr, sz)) ) {
        debug_log << "can't set field value:" << path.join(".");
      }
      delete[] arr;
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
      debug_log << "bool, enum, message, double and float unsupported" << path.join(".");
      return false;
      break;
  }

  return true;
}

} // meteo
