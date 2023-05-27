#include "tmsgformat.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qdom.h>

#include <cross-commons/debug/tlog.h>

template<> msgformat::TMsgFormat* TMsgFormat::_instance = 0;

namespace msgformat
{

const QStringList TMsgFormat::bitnames = QStringList()
  << "startline"
  << "header"
  << "end";

const QByteArray TMsgFormat::MsgRule::endline = QByteArray("\r\r\n\003");

bool TMsgFormat::MsgRule::loadRule( const QDomElement& rule )
{
  name = rule.attribute("name");
  if ( true == rule.hasAttribute("exclusive") && "true" == rule.attribute("exclusive") ) {
    exclusive = true;
  }
  else {
    exclusive = false;
  }
  if ( true == rule.hasAttribute("onlybody") && "true" == rule.attribute("onlybody") ) {
    onlybody = true;
  }
  else {
    onlybody = false;
  }
  if ( true == rule.hasAttribute("msgtype") && false == rule.attribute("msgtype").isEmpty() ) {
    msgtype = rule.attribute("msgtype");
  }
  else {
    msgtype = QString();
  }
  if ( true == rule.hasAttribute("endless") && "true" == rule.attribute("endless") ) {
    endless = true;
  }
  else {
    endless = false;
  }
  for ( int i = 0, sz = bitnames.size(); i < sz; ++i ) {
    QRegExp exp;
    const QString& bn = bitnames[i];
    if ( "header" == bn ) {
      loadRuleHeaders( rule, &headers );
    }
    else {
      if ( false == loadRuleBit( rule, bitnames[i], &exp ) ) {
        continue;
      }
      bits.insert( bitnames[i], exp );
    }
  }
  return true;
}

bool TMsgFormat::MsgRule::loadRuleBit( const QDomElement& rule, const QString& bitname, QRegExp* bit )
{
  QDomElement el = rule.firstChildElement(bitname);
  if ( true == el.isNull() || false == el.hasAttribute("regex") ) {
//    info_log << QObject::tr("В описании формата сообщений =") << name
//      << QObject::tr("\n\t* не найдено правило для поиска секции сообщения =") << bitname;
    return false;
  }
  bit->setPattern( el.attribute("regex") );
  if ( true == bit->isEmpty() ) {
    error_log << QObject::tr("Ошибка. В описании формата сообщений =") << name
      << QObject::tr("\n\t* пустое правило для поиска секции сообщения =") << bitname;
    return false;
  }
  if ( false == bit->isValid() ) {
    error_log << QObject::tr("Ошибка. В описании формата сообщений =") << name
      << QObject::tr("\n\t* неверное правило для поиска секции сообщения =") << bitname;
    return false;
  }
  if ( "startline" == bitname ) {
    startline.name = el.attribute("name");
    startline.exp.setPattern( el.attribute("regex") );
    QDomNodeList groups = el.elementsByTagName("group");
    for ( int i = 0, sz = groups.size(); i < sz; ++i ) {
      QDomElement el = groups.item(i).toElement();
      if ( true == el.isNull() ) {
        continue;
      }
      QStringList strlist = el.attribute("order").split( ',', QString::SkipEmptyParts );
      for ( int j = 0, jsz = strlist.size(); j < jsz; ++j ) {
        startline.headerparts.insert( strlist[j].toInt(), el.attribute("name") );
      }
    }
  }
  return true;
}

bool TMsgFormat::MsgRule::loadRuleHeaders( const QDomElement& rule, QList<Section>* headers )
{
  QDomElement el = rule.firstChildElement("header");
  while ( false == el.isNull() ) {
    if ( false == el.hasAttribute("regex") ) {
      continue;
    }
    Section h;
    h.exp.setPattern( el.attribute("regex") );
    if ( true == h.exp.isEmpty() ) {
      error_log << QObject::tr("Ошибка. В описании формата сообщений =") << name
        << QObject::tr("\n\t* пустое правило для поиска секции сообщения =") << "header";
      continue;
    }
    if ( false == h.exp.isValid() ) {
      error_log << QObject::tr("Ошибка. В описании формата сообщений =") << name
        << QObject::tr("\n\t* неверное правило для поиска секции сообщения =") << "header";
      continue;
    }
    h.name = el.attribute("name");
    QDomNodeList groups = el.elementsByTagName("group");
    for ( int i = 0, sz = groups.size(); i < sz; ++i ) {
      QDomElement el = groups.item(i).toElement();
      if ( true == el.isNull() ) {
        continue;
      }
      QStringList strlist = el.attribute("order").split( ',', QString::SkipEmptyParts );
      for ( int j = 0, jsz = strlist.size(); j < jsz; ++j ) {
        h.headerparts.insert( strlist[j].toInt(), el.attribute("name") );
      }
    }
    headers->append(h);
    el = el.nextSiblingElement();
  }
  if ( 0 == headers->size() ) {
    return false;
  }
  return true;
}

bool TMsgFormat::BinRule::loadRule( const QDomElement& rule )
{
  name = rule.attribute("name");
  begin = rule.attribute("begin").toUtf8();
  if ( 0 == begin.indexOf("\\x") ) {
    begin = QByteArray::fromHex(begin);
  }
  end = rule.attribute("end").toUtf8();
  if ( 0 == end.indexOf("\\x") ) {
    end = QByteArray::fromHex(end);
  }
  editionbyte = rule.attribute( "editionbyte", "-1" ).toInt();
  path = rule.attribute( "path", MnCommon::varPath() + "/binmsg/UNKFMT/" );
  if ( '/' != path[0] ) {
    path = MnCommon::varPath() + path;
  }

  QDomElement elem = rule.firstChildElement("edition");
  while ( false == elem.isNull() ) {
    SizeRule sizerule;
    if ( true == loadEdition( elem, &sizerule ) ) {
      sizerules.insert( elem.attribute( "version", 0 ).toInt(), sizerule );
    }
    elem = elem.nextSiblingElement("edition");
  }
  if ( 0 == sizerules.size() ) {
    error_log << QObject::tr("Ошибка В описании бинарного формата =") << name;
    return false;
  }
  return true;
}

bool TMsgFormat::BinRule::loadEdition( const QDomElement& edition, SizeRule* sizerule )
{
  QStringList list = edition.attribute("sizebyte").split( "-", QString::SkipEmptyParts );
  if ( 2 != list.size() ) {
    warning_log << QObject::tr("Ошибка в описании бинарного формата =") << name;
    return false;
  }
  bool ok;
  int startbyte = list[0].toInt(&ok);
  if ( false == ok ) {
    warning_log << QObject::tr("Ошибка в описании бинарного формата =") << name;
    return false;
  }
  int bytelength = list[1].toInt(&ok) - startbyte + 1;
  if ( false == ok ) {
    warning_log << QObject::tr("Ошибка в описании бинарного формата =") << name;
    return false;
  }
  sizerule->first = startbyte;
  sizerule->second = bytelength;
  QString bostring = edition.attribute( "byteorder", "be" );
  sizerule->byteorder = ( "le"== bostring ) ? LE_ORDER : BE_ORDER;
  sizerule->addbytes = edition.attribute( "add", 0 ).toInt();
  return true;
}

TMsgFormat::TMsgFormat()
{
  if ( false == loadRules() ) {
    error_log << QObject::tr("Ошибка. Не удалось загрузить форматы сообщений");
  }
}

TMsgFormat::~TMsgFormat()
{
}

bool TMsgFormat::loadRules()
{
  QString path( MnCommon::etcPath() + "/parserules/" );
  QDir dir( path );
  if ( false == dir.exists() ) {
    error_log << QObject::tr("Ошибка. Директория =") << path << QObject::tr("не cуществует.");
    return false;
  }
  QFileInfoList filist = dir.entryInfoList( QStringList("*.xml"), QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0, sz = filist.size(); i < sz; ++i ) {
    loadRule( filist[i].absoluteFilePath() );
  }

  if ( 0 == rules_.size() ) {
    error_log << QObject::tr("Ошибка. Форматы сообщений не найдены.");
    return false;
  }
  return true;
}

bool TMsgFormat::loadRule( const QString& filename )
{
  QFile file( filename );
  if ( false == file.exists() ) {
    error_log << QObject::tr("Ошибка. файл =") << filename << QObject::tr("не cуществует.");
    return false;
  }
  QString err;
  int errline = 0;
  int errclmn = 0;
  QDomDocument dom;
  if ( false == dom.setContent( &file, &err, &errline, &errclmn ) ) {
    error_log << QObject::tr("Ошибка при загрузке xml-файла =") << filename
      << "\n\t* " << err
      << QObject::tr("Номер строки =") << errline
      << QObject::tr("Номер колонки =") << errclmn;
    return false;
  }
  QDomNodeList msgs = dom.elementsByTagName("msgformat");
  if ( 0 == msgs.count() ) {
    error_log << QObject::tr("Ошибка. В файле =") << filename << QObject::tr("не найдены форматы сообщений.");
    return false;
  }
  for ( int i = 0, sz = msgs.count(); i < sz; ++i ) {
    QDomNode node = msgs.at(i);
    if ( false == node.isElement() ) {
      continue;
    }
    QDomElement el = node.toElement();
    if ( false == el.hasAttribute("name") ) {
      warning_log << QObject::tr("В файле") << filename << QObject::tr("обнаружен формат сообщений без наименования");
    }
    MsgRule rule;
    if ( false == rule.loadRule(el) ) {
      error_log << QObject::tr("Ошибка при загрузке формата сообщений =" ) << el.attribute("name") << QObject::tr("из файла =") << filename;
    }
    else {
      rules_.insert( rule.name, rule );
//      rules_.append(rule);
    }
  }
  msgs = dom.elementsByTagName("binformat");
  for ( int i = 0, sz = msgs.count(); i < sz; ++i ) {
    QDomNode node = msgs.at(i);
    if ( false == node.isElement() ) {
      continue;
    }
    QDomElement el = node.toElement();
    if ( false == el.hasAttribute("name") ) {
      warning_log << QObject::tr("В файле") << filename << QObject::tr("обнаружен бинарный формат без наименования");
    }
    BinRule rule;
    if ( false == rule.loadRule(el) ) {
      error_log << QObject::tr("Ошибка при загрузке бинарного формата =" ) << el.attribute("name") << QObject::tr("из файла =") << filename;
    }
    else {
      binrules_.insert( rule.name, rule );
    }
  }
 
  return true;
}

}
