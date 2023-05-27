#ifndef METEO_COMMONS_MSGPARSER_TMSGFORMAT_H
#define METEO_COMMONS_MSGPARSER_TMSGFORMAT_H

#include <qregexp.h>
#include <qlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>
#include <qbytearray.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/singleton/tsingleton.h>

class QDomElement;

namespace msgformat
{

class TMsgFormat
{
  static const QStringList bitnames;
  public:
    struct Section
    {
      QString name;
      QRegExp exp;
      QMap< int, QString > headerparts;
    };
    struct MsgRule
    {
      static const QByteArray endline;
      QString name;
      bool exclusive;
      bool endless;
      bool onlybody;
      QString msgtype;
      QMap< QString, QRegExp > bits;
      Section startline;
      QList<Section> headers;
      
      bool loadRule( const QDomElement& rule );

      private:
      bool loadRuleBit( const QDomElement& rule, const QString& bitname, QRegExp* bit );
      bool loadRuleHeaders( const QDomElement& rule, QList<Section>* headers );
    };
    struct SizeRule : public QPair< int /*startbyte*/, int /*lengthbyte*/>
    {
      int byteorder;
      int addbytes;
    };
    struct BinRule
    {
      enum Order {
        BE_ORDER =      0,
        LE_ORDER =      1 
      };
      QString name;
      QByteArray begin;
      QByteArray end;
      int editionbyte;
      QString path;
      QMap< int /*edition*/, SizeRule > sizerules;
      
      bool loadRule( const QDomElement& rule );
      private:
      bool loadEdition( const QDomElement& edition, SizeRule* sizerule );
    };

  public:
    TMsgFormat();
    ~TMsgFormat();

//    const QList<MsgRule>& rules() const { return rules_; } 
    const QMap< QString, MsgRule >& rules() const { return rules_; } 
    const QMap< QString, BinRule >& binrules() const { return binrules_; } 

  private:
//    QList<MsgRule> rules_;
    QMap< QString,  MsgRule > rules_;
    QMap< QString, BinRule > binrules_;

    bool loadRules();
    bool loadRule( const QString& filename );
};

}

typedef TSingleton<msgformat::TMsgFormat> TMsgFormat;

#endif
