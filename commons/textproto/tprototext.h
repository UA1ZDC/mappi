#ifndef PROTOTEXT_TPROTOTEXT_H
#define PROTOTEXT_TPROTOTEXT_H

#include <qmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qobject.h>

#include <cross-commons/debug/tlog.h>

class QString;

namespace google {
namespace protobuf {
  class Message;
}
}

class TProtoText
{
  public:
    TProtoText(){}
    ~TProtoText(){}

    static bool fillProto( const QString& text, google::protobuf::Message* message );

    static QByteArray toText( const google::protobuf::Message& message );
    static bool fillProto( const QByteArray& text, google::protobuf::Message* message );

    static bool toFile( const google::protobuf::Message& message, const QString& filename );
    static bool fromFile( const QString& filename, google::protobuf::Message* message, bool replacelf = false );


    template<class T> static bool loadProtosFromDirectory( const QString& path, QMap<QString /*filename*/, T>* protos );
    
};

template<class T> bool TProtoText::loadProtosFromDirectory( const QString& path, QMap<QString, T>* protos )
{
  if ( 0 == protos ) {
    error_log << QObject::tr("Нулевой указатель на лист в функции %1. Путь к директории с протофайлами %2")
      .arg(__PRETTY_FUNCTION__)
      .arg(path);
    return false;
  }
  QDir dir(path);
  if ( false == dir.exists() ) {
    error_log << QObject::tr("Директория с протофайлами не существует.");
    return false;
  }
  QStringList files = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  int sz = files.size();
  if ( 0 == sz ) {
    error_log << QObject::tr("В директории с протофайлами не найден ни один протофайл.");
    return false;
  }
  const int k5MBSize( 5*1024*1024 );
  for ( int i = 0; i < sz; ++i ) {
    const QString& filename = files[i];
    QFile file( dir.absoluteFilePath( filename ) );
    if ( false == file.open( QIODevice::ReadOnly ) ) {
      error_log << QObject::tr("Не удалось открыть файл %1. Причина = %2")
        .arg( filename )
        .arg( file.errorString() );
      continue;
    }
    if ( k5MBSize < file.size() ) {
      warning_log << QObject::tr("Не прочитан файл с прото %1, потому что его размер больше %2 байт")
        .arg( file.fileName() )
        .arg(k5MBSize);
      continue;
    }
    QByteArray arr = file.readAll();
    if ( 0 == arr.size() ) {
      warning_log << QObject::tr("Прочитано ноль байт в файле %1")
        .arg( file.fileName() );
      continue;
    }
    T t;
    if ( false == TProtoText::fillProto( QString::fromUtf8(arr), &t ) ) {
      error_log << QObject::tr("Не удалось заполнить прото из файла %1")
        .arg( file.fileName() );
      continue;
    }
    protos->insert( file.fileName(), t );
  }
  if ( 0 == protos->size() ) {
    error_log << QObject::tr("Загружено ноль шаблонов карт.");
    return false;
  }
  return true;
}

#endif
