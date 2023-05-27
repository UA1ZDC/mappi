#include "txmlproto.h"

#include <cross-commons/debug/tlog.h>

#include <QtXml>
#include <QtCore>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


QDomDocument TXmlProto::toXml(const google::protobuf::Message& msg, const QString& tagName)
{
  QDomDocument doc;

  const google::protobuf::Reflection* msgRefl = msg.GetReflection();
  const google::protobuf::Descriptor* msgDesc = msg.GetDescriptor();
  const QString msgName = QString::fromStdString( msgDesc->name() ).toLower();
  const QString rootName = tagName.isNull() ? msgName : tagName;
  QDomElement rootElem = doc.createElement( rootName );
  doc.appendChild( rootElem );

  for( int i=0,isz=msgDesc->field_count(); i<isz; ++i ){
    const google::protobuf::FieldDescriptor* fieldDesc = msgDesc->field(i);
    const QString fieldName = QString::fromStdString( fieldDesc->name() ).toLower();

    bool isMessage  = fieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE;
    bool isRepeated = fieldDesc->is_repeated();

    if( !isRepeated && !msgRefl->HasField(msg, fieldDesc) ){
      continue;
    }

    if( isRepeated && isMessage ){
      int fieldCount = msgRefl->FieldSize( msg, fieldDesc );
      if( 0 == fieldCount ){
        continue;
      }

      QDomElement elem = doc.createElement( fieldName );
      rootElem.appendChild( elem );

      for( int i=0; i<fieldCount; ++i ){
        QDomDocument d = TXmlProto::toXml( msgRefl->GetRepeatedMessage(msg,fieldDesc,i) );
        elem.appendChild( d );
      }
    }
    else if( isRepeated && !isMessage ){
      QStringList vals = TXmlProto::getFieldValues( msg, fieldName );

      if( vals.size() == 0 ){
        continue;
      }

      QDomElement elem = doc.createElement( fieldName );
      rootElem.appendChild( elem );
      foreach( QString val, vals ){
        QDomElement valEelm = doc.createElement( "item" );
        QDomText txt = doc.createTextNode( val );
        valEelm.appendChild( txt );
        elem.appendChild( valEelm );
      }
    }
    else if( !isRepeated && isMessage ){
      QDomDocument d = TXmlProto::toXml( msgRefl->GetMessage(msg,fieldDesc), fieldName );
      rootElem.appendChild( d );
    }
    else if( !isRepeated && !isMessage ){
      QString fieldValue = TXmlProto::getFieldValue( msg, fieldName );
      // если размер поля меньше 20 символов, добавляем как атрибут, иначе -
      // как тег
      if( fieldValue.size() < 20 ){
        rootElem.setAttribute( fieldName, fieldValue );
      }
      else {
        QDomElement elem = doc.createElement( fieldName );
        QDomText txt = doc.createTextNode( fieldValue );
        elem.appendChild( txt );
        rootElem.appendChild( elem );
      }
    }
  }

  return doc;
}

bool TXmlProto::fillProto(google::protobuf::Message* msg,
                          const QDomElement& root)
{
  const google::protobuf::Reflection* msgRefl = msg->GetReflection();
  const google::protobuf::Descriptor* msgDesc = msg->GetDescriptor();

  QDomNamedNodeMap attrs = root.attributes();
  for( int i=0,isz=attrs.count(); i<isz; ++i ){
    QDomAttr n = attrs.item(i).toAttr();
    if( !setValue(msg, n.name(), QStringList(n.value())) ){
      debug_log << "can't set field:" << n.name();
      debug_log << "value:" << n.value();
      return false;
    }
  }

  if( !root.hasChildNodes() ){
    return true;
  }

  QDomElement e = root.firstChildElement();
  while( !e.isNull() ){
    const QString fieldName = e.tagName().toLower();

    const google::protobuf::FieldDescriptor* fieldDesc = 0;
    fieldDesc = msgDesc->FindFieldByLowercaseName( fieldName.toStdString() );
    if( 0 == fieldDesc ){
      debug_log << "field" << fieldName << "not found";
      debug_log << msgDesc->DebugString();
      return false;
    }

    bool isMessage  = fieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE;
    bool isRepeated = fieldDesc->is_repeated();

    if( !isMessage && !isRepeated ){
      QStringList values( e.text() );

      if( !setValue(msg,fieldName,values) ){
        return false;
      }
    }
    else if( !isMessage && isRepeated ){
      QStringList values;

      QDomElement item = e.firstChildElement( "item" );
      while( !item.isNull() ){
        values << item.text();
        item = item.nextSiblingElement( "item" );
      }

      if( !setValue(msg,fieldName,values) ){
        return false;
      }
    }
    else if( isMessage && !isRepeated ){
      google::protobuf::Message* m = msgRefl->MutableMessage( msg, fieldDesc );
      if( !TXmlProto::fillProto(m,e) ){
        return false;
      }
    }
    else if( isMessage && isRepeated ){
      QDomElement chElem = e.firstChildElement();
      while( !chElem.isNull() ){
        google::protobuf::Message* m = msgRefl->AddMessage( msg, fieldDesc );
        if( !TXmlProto::fillProto(m,chElem)){
          return false;
        }
        chElem = chElem.nextSiblingElement();
      }
    }

    e = e.nextSiblingElement();
  }

  return true;
}

QString TXmlProto::getFieldValue(const google::protobuf::Message& msg,
                                 const QString& fieldName)
{
  const google::protobuf::Descriptor* msgDesc = msg.GetDescriptor();
  const google::protobuf::Reflection* msgRefl = msg.GetReflection();
  const google::protobuf::FieldDescriptor* fieldDesc;
  fieldDesc = msgDesc->FindFieldByLowercaseName( fieldName.toLower().toStdString() );
  if( 0 == fieldDesc ){
    debug_log << "field '"+fieldName+"' not found in '" << msgDesc->full_name() << "'";
    debug_log << "message data:\n" << msg.Utf8DebugString();
    return QString();
  }

  if( fieldDesc->is_repeated() ){
    debug_log << __func__ << "don't work with repeated fields";
    return QString();
  }

  QString value;

  switch( fieldDesc->cpp_type() ){
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
      value = msgRefl->GetBool( msg, fieldDesc ) ? "true" : "false";
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:   {
      value = QString::number( msgRefl->GetEnum(msg,fieldDesc)->number() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:  {
      value = QString::number( msgRefl->GetInt32(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:  {
      value = QString::number( msgRefl->GetInt64(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
      value = QString::number( msgRefl->GetUInt32(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
      value = QString::number( msgRefl->GetUInt64(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:  {
      value = QString::number( msgRefl->GetFloat(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
      value = QString::number( msgRefl->GetDouble(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
      value = QString::fromStdString( msgRefl->GetString(msg,fieldDesc) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
      debug_log << __func__ << "don't work with field MESSAGE";
      return QString();
    } break;
  }

  return value;
}

QStringList TXmlProto::getFieldValues(const google::protobuf::Message& msg,
                                      const QString& fieldName )
{
  const google::protobuf::Descriptor* msgDesc = msg.GetDescriptor();
  const google::protobuf::Reflection* msgRefl = msg.GetReflection();
  const google::protobuf::FieldDescriptor* fieldDesc = 0;
  fieldDesc = msgDesc->FindFieldByLowercaseName( fieldName.toLower().toStdString() );
  if( 0 == fieldDesc ){
    debug_log << "field '"+fieldName+"' not found in '" << msgDesc->full_name() << "'";
    debug_log << "message data:\n" << msg.Utf8DebugString();
    return QStringList();
  }

  QStringList values;

  int sz = msgRefl->FieldSize( msg, fieldDesc );
  if( sz > 0 ){
    switch( fieldDesc->cpp_type() ){
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedDouble(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:   {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedFloat(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedUInt32(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:{
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedUInt64(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedInt64(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedInt32(msg, fieldDesc, i) );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:    {
        for( int i=0; i<sz; ++i ){
          values << ( msgRefl->GetRepeatedBool(msg, fieldDesc, i) ? "true" : "false" );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:    {
        for( int i=0; i<sz; ++i ){
          values << QString::number( msgRefl->GetRepeatedEnum(msg, fieldDesc, i)->number() );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        for( int i=0; i<sz; ++i ){
          values << QString::fromStdString(msgRefl->GetRepeatedString(msg, fieldDesc, i));
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        debug_log << __func__ << "don't work with field MESSAGE";
        return QStringList();
      } break;
    }
  }

  return values;
}

bool TXmlProto::setValue(google::protobuf::Message* msg,
                         const QString& fieldName,
                         const QStringList& values)
{
  Q_CHECK_PTR( msg );

  const google::protobuf::Descriptor* msgDesc = msg->GetDescriptor();
  const google::protobuf::FieldDescriptor* fd = 0;
  fd = msgDesc->FindFieldByLowercaseName( fieldName.toLower().toStdString() );
  if( 0 == fd ){
    debug_log << "field '"+fieldName+"' not found in '" << msgDesc->full_name() << "'";
    debug_log << "message data:\n" << msg->Utf8DebugString();
    return false;
  }

  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << __func__ << "don't work with field MESSAGE";
    return false;
  }

  const google::protobuf::Reflection* msgRefl = msg->GetReflection();
  if( fd->is_repeated() ){
    QStringList::const_iterator it  = values.constBegin();
    QStringList::const_iterator end = values.constEnd();
    for( ; it!=end; ++it ){
      switch( fd->cpp_type() ){
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
          msgRefl->AddInt32( msg, fd, (*it).toInt() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
          msgRefl->AddInt64( msg, fd, (*it).toLongLong() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
          msgRefl->AddUInt32( msg, fd, (*it).toUInt() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
          msgRefl->AddUInt64( msg, fd, (*it).toULongLong() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
          msgRefl->AddDouble( msg, fd, (*it).toDouble() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
          msgRefl->AddFloat( msg, fd, (*it).toFloat() );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
          msgRefl->AddBool( msg, fd, (*it) == "true" ? true : false );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
          const google::protobuf::EnumDescriptor* ed = fd->enum_type();
          msgRefl->AddEnum( msg, fd, ed->FindValueByNumber( QString(*it).toInt() ) );
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
          if( !(*it).isNull() ){
            std::string s = (*it).toStdString();
            msgRefl->AddString( msg, fd, s );
          }
        } break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {} break;
      }
    }
  }
  else {
    switch( fd->cpp_type() ){
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        msgRefl->SetInt32( msg, fd, values.first().toInt() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        msgRefl->SetInt64( msg, fd, values.first().toLongLong() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        msgRefl->SetUInt32( msg, fd, values.first().toUInt() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        msgRefl->SetUInt64( msg, fd, values.first().toULongLong() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        msgRefl->SetDouble( msg, fd, values.first().toDouble() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        msgRefl->SetFloat( msg, fd, values.first().toFloat() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        msgRefl->SetBool( msg, fd, values.first() == "true" ? true : false );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        const google::protobuf::EnumDescriptor* ed = fd->enum_type();
        msgRefl->SetEnum( msg, fd, ed->FindValueByNumber( QString(values.first()).toInt() ) );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        if( !values.first().isNull() ){
          std::string s = values.first().toStdString();
          msgRefl->SetString( msg, fd, s );
        }
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {} break;
    }
  }

  return true;
}
