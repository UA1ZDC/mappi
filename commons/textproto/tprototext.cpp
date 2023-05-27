#include "tprototext.h"

#include <cross-commons/debug/tlog.h>

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

#include <qstring.h>
#include <string>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "pbtools.h"

QByteArray TProtoText::toText( const google::protobuf::Message& message )
{
  std::string output;
  google::protobuf::TextFormat::Printer printer;
  printer.SetUseUtf8StringEscaping(true);
  bool res = printer.PrintToString( message, &output );
  if ( false == res ) {
    error_log
      << QObject::tr("Не удалось преобразовать proto-сообщение в строку. Сообщение =") << '\n'
      << message.Utf8DebugString();
  }
//  std::string unescaped_output = google::protobuf::modified::UnescapeCEscapeString(output);
  return QByteArray( output.data(), output.size() );
}

bool TProtoText::fillProto( const QString& text, google::protobuf::Message* output )
{
  return fillProto( text.toUtf8(), output );
}

bool TProtoText::fillProto( const QByteArray& text, google::protobuf::Message* output )
{
  std::string input = std::string( text.data(), text.size() );
  bool res = google::protobuf::TextFormat::ParseFromString( input, output );
  if ( false == res ) {
    error_log
      << QObject::tr("Не удалось преобразовать текст в proto-сообщение. Сообщение =") << '\n'
      << input
      << "proto =" << output->Utf8DebugString();
  }
  return res;
}

bool TProtoText::toFile( const google::protobuf::Message& message, const QString& filename )
{
  if ( true == filename.isEmpty() ) {
    error_log << QObject::tr("Имя файла для сохранения proto-сообщения не указано.");
    return false;
  }
  QFileInfo fi(filename);
  if ( true == fi.exists() && true == fi.isDir() ) {
    error_log << QObject::tr("%1 - это директория. Сохранить proto-сообщение невозможно")
      .arg(filename);
    return false;
  }
  if ( false == fi.exists() ) {
    QDir dir = fi.dir();
    if ( false == dir.exists() ) {
      if ( false == dir.mkpath( dir.absolutePath() ) ) {
        error_log << QObject::tr("Не удалось создать директорию %1")
          .arg( dir.absolutePath() );
        return false;
      }
    }
  }
  QFile file(filename);
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log << QObject::tr("Не удалось октрыть файл для записи %1. Причина - %2")
      .arg(filename)
      .arg( file.errorString() );
    return false;
  }
  QByteArray text = TProtoText::toText(message);
  int64_t sz = file.write( text.data(), text.size() );
  if ( text.size() != sz ) {
    warning_log << QObject::tr("proto-сообщение записано в файл %1 не полностью. Записано %2 из %3.\n\tПричина - %4")
      .arg( filename )
      .arg( sz )
      .arg( text.size() )
      .arg( file.errorString() );
    return false;
  }
  file.flush();
  file.close();
  return true;
}

bool TProtoText::fromFile( const QString& filename, google::protobuf::Message* message, bool replacelf )
{
  QFile file(filename);
  if ( false == file.open( QIODevice::ReadOnly ) ) {
//     error_log << QObject::tr("Не удалось открыть файл %1 для чтения\n\tПричина - %2")
//       .arg( filename )
//       .arg( file.errorString() );
    return false;
  }
  if ( 5*1024*1024 < file.size() ) {
    error_log << QObject::tr("Файл %1 слишком боьшой для proto-сообщения в текстовом виде. Размер = %2")
      .arg( filename )
      .arg( file.size() );
    return false;
  }
  QByteArray txt = file.readAll();
  if ( true == replacelf ) {
    txt = txt.replace(0x0A,0x20);
  }
  if ( false == TProtoText::fillProto( txt, message ) ) {
    return false;
  }
  file.close();
  return true;
}
