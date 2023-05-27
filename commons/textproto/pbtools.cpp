#include "pbtools.h"

#include <cross-commons/debug/tlog.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

bool PbHelper::setValue(const QStringList& fieldPath, const QStringList& values)
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;

  if ( !findField(&m, &fd, fieldPath) ) {
    return false;
  }

  return setValue(m, fd, values);
}

bool PbHelper::setValue(const QStringList& fieldPath, const QString& value)
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;

  if ( !findField(&m, &fd, fieldPath) ) {
    return false;
  }

  return setValue(m, fd, value);
}

bool PbHelper::setValue(const QStringList& fieldPath, const QByteArray& value)
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;

  if ( !findField(&m, &fd, fieldPath) ) {
    return false;
  }

  return setValue(m, fd, value);
}

bool PbHelper::addValue(const QStringList& fieldPath, const QString& value)
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;

  if ( !findField(&m, &fd, fieldPath) ) {
    return false;
  }

  return addValue(m, fd, value);
}

bool PbHelper::addValue(const QStringList& fieldPath, const QByteArray& value)
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;

  if ( !findField(&m, &fd, fieldPath) ) {
    return false;
  }

  return addValue(m, fd, value);
}

bool PbHelper::hasField(const QStringList& fieldPath) const
{
  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;
  return findField(&m, &fd, fieldPath);
}

google::protobuf::FieldDescriptor::CppType PbHelper::cppType(const QStringList& fieldPath, bool* ok) const
{
  if ( 0 != ok ) { *ok = false; }

  google::protobuf::Message* m = 0;
  const google::protobuf::FieldDescriptor* fd = 0;
  if ( !findField(&m, &fd, fieldPath) ) {
    return google::protobuf::FieldDescriptor::CPPTYPE_INT32;
  }

  if ( 0 != ok ) { *ok = true; }

  return fd->cpp_type();
}

bool PbHelper::findField(google::protobuf::Message** resMsg,
           const google::protobuf::FieldDescriptor** resFd,
                                 const QStringList& path) const
{
  google::protobuf::Message* msg = msg_;
  const google::protobuf::FieldDescriptor* fd = 0;

  msg = msg_;
  foreach ( const QString& field, path ) {
    const google::protobuf::Descriptor* desc = msg->GetDescriptor();

    fd = desc->FindFieldByName(pbtools::toString(field));
    if ( 0 == fd ) {
      debug_log << QObject::tr("Поле %1 (%2) не найдено в %3.")
                              .arg(field)
                              .arg(path.join("."))
                              .arg(pbtools::toQString(desc->full_name()));
      return false;
    }

    if ( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ) {
      const google::protobuf::Reflection* refl = msg->GetReflection();
      msg = refl->MutableMessage(msg, fd);
    }
  }

  *resMsg = msg;
  *resFd  = fd;

  return true;
}

bool PbHelper::setValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                                const QStringList& values) const
{
  const google::protobuf::Reflection* refl = msg->GetReflection();

  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if( !fd->is_repeated() ){
    debug_log << QObject::tr("Список значение можно устанавливать только для repeated полей.");
    return false;
  }

  refl->ClearField(msg, fd);

  QStringList::const_iterator it  = values.constBegin();
  QStringList::const_iterator end = values.constEnd();
  for( ; it!=end; ++it ){
    if ( (*it).isNull() ) { continue; }

    switch( fd->cpp_type() ){
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        refl->AddInt32( msg, fd, (*it).toInt() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        refl->AddInt64( msg, fd, (*it).toLongLong() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        refl->AddUInt32( msg, fd, (*it).toUInt() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        refl->AddUInt64( msg, fd, (*it).toULongLong() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        refl->AddDouble( msg, fd, (*it).toDouble() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        refl->AddFloat( msg, fd, (*it).toFloat() );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        refl->AddBool( msg, fd, (*it) == "true" ? true : false );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        const google::protobuf::EnumDescriptor* ed = fd->enum_type();
        refl->AddEnum( msg, fd, ed->FindValueByName(pbtools::toString(*it)) );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        refl->AddString( msg, fd, pbtools::toString(*it) );
      } break;
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {} break;
    }
  }

  return true;
}

bool PbHelper::setValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                          const QList<QByteArray>& values) const
{
  const google::protobuf::Reflection* refl = msg->GetReflection();

  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if( !fd->is_repeated() ){
    debug_log << QObject::tr("Список значение можно устанавливать только для repeated полей.");
    return false;
  }

  refl->ClearField(msg, fd);

  if ( fd->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING
       && fd->type() == google::protobuf::FieldDescriptor::TYPE_BYTES
       )
  {
    for ( int i=0,isz=values.size(); i<isz; ++i ) {
      if ( values.at(i).isNull() ) { continue; }

      msg->GetReflection()->AddString( msg, fd, pbtools::toBytes(values.at(i)) ) ;
    }

    return true;
  }

  QStringList list;
  for ( int i=0,isz=values.size(); i<isz; ++i ) {
    list += QString(values.at(i));
  }

  return setValue(msg, fd, list);
}

bool PbHelper::setValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                                    const QString& value) const
{
  const google::protobuf::Reflection* refl = msg->GetReflection();

  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if ( value.isNull() ) {
    return true;
  }

  if( fd->is_repeated() ){
    return setValue(msg, fd, QStringList(value));
  }

  switch ( fd->cpp_type() ) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
      refl->SetInt32( msg, fd, value.toInt() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
      refl->SetInt64( msg, fd, value.toLongLong() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
      refl->SetUInt32( msg, fd, value.toUInt() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
      refl->SetUInt64( msg, fd, value.toULongLong() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
      refl->SetDouble( msg, fd, value.toDouble() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
      refl->SetFloat( msg, fd, value.toFloat() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
      refl->SetBool( msg, fd, value == "true" ? true : false );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
      const google::protobuf::EnumDescriptor* ed = fd->enum_type();
      refl->SetEnum( msg, fd, ed->FindValueByName(pbtools::toString(value)) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
      if ( !value.isNull() ) {
        refl->SetString( msg, fd, pbtools::toString(value) );
      }
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {} break;
  }

  return true;

}

bool PbHelper::setValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                                 const QByteArray& value) const
{
  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if( fd->is_repeated() ){
    return setValue(msg, fd, QList<QByteArray>() << value);
  }

  if ( value.isNull() ) {
    return true;
  }

  if ( fd->cpp_type() ==  google::protobuf::FieldDescriptor::CPPTYPE_STRING ) {
    if ( fd->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ) {
      msg->GetReflection()->SetString( msg, fd, pbtools::toBytes(value) ) ;
      return true;
    }
  }

  return setValue(msg, fd, QStringList(QString(value)));
}

bool PbHelper::addValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                                    const QString& value) const
{
  const google::protobuf::Reflection* refl = msg->GetReflection();

  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if( !fd->is_repeated() ){
    debug_log << QObject::tr("Добавлять значение можно только для repeted полей.");
    return false;
  }

  if ( value.isNull() ) {
    return true;
  }

  switch( fd->cpp_type() ){
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
      refl->AddInt32( msg, fd, value.toInt() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
      refl->AddInt64( msg, fd, value.toLongLong() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
      refl->AddUInt32( msg, fd, value.toUInt() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
      refl->AddUInt64( msg, fd, value.toULongLong() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
      refl->AddDouble( msg, fd, value.toDouble() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
      refl->AddFloat( msg, fd, value.toFloat() );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
      refl->AddBool( msg, fd, value == "true" ? true : false );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
      const google::protobuf::EnumDescriptor* ed = fd->enum_type();
      refl->AddEnum( msg, fd, ed->FindValueByName(pbtools::toString(value)) );
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
      if( !value.isNull() ){
        refl->AddString( msg, fd, pbtools::toString(value) );
      }
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {} break;
  }

  return true;
}

bool PbHelper::addValue(google::protobuf::Message* msg,
          const google::protobuf::FieldDescriptor* fd,
                                 const QByteArray& value) const
{
  if( fd->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ){
    debug_log << QObject::tr("Невозможно установить значение для поля типа MESSAGE.");
    return false;
  }

  if( !fd->is_repeated() ){
    debug_log << QObject::tr("Добавлять значение можно только для repeted полей.");
    return false;
  }

  if ( value.isNull() ) {
    return true;
  }

  if ( fd->cpp_type() ==  google::protobuf::FieldDescriptor::CPPTYPE_STRING ) {
    if ( fd->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ) {
      msg->GetReflection()->AddString( msg, fd, pbtools::toBytes(value) ) ;
      return true;
    }
  }

  return addValue(msg, fd, QString(value));
}
