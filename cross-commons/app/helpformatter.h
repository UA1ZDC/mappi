#ifndef CROSS_COMMONS_APP_HELPFORMATTER_H
#define CROSS_COMMONS_APP_HELPFORMATTER_H

#include <cmath>
#include <iostream>

#include <qmap.h>
#include <qstringlist.h>

class HelpFormatter
{
public:
  HelpFormatter& operator<<( const QStringList& t )
  {
    if ( t.size() == 1 ) {
      _one.append("");
      _word.append("");
      _text.append(t.at(0));
    }
    else if ( t.size() == 2 ) {
      _one.append("");
      _word.append(t.at(0));
      _text.append(t.at(1));
    }
    else if ( t.size() == 3 ) {
      _one.append(t.at(0));
      _word.append(t.at(1));
      _text.append(t.at(2));
    }

    return *this;
  }

  HelpFormatter& operator<<( const QList<QStringList>& l )
  {
    for ( const QStringList& t : l ) {
      *this << t;
    }

    return *this;
  }

  void print() const
  {
    int maxOne = 0;
    int maxWord = 0;

    foreach ( const QString& s, _one ) {
      maxOne = std::max(maxOne, s.size());
    }
    foreach ( const QString& s, _word ) {
      maxWord = std::max(maxWord, s.size());
    }

    QStringList lines;

    for ( int i=0,isz=_one.size(); i<isz; ++i ) {
      QString one = _one[i].isEmpty() ? QString().leftJustified(maxOne + 4, ' ') : " -" + QString(_one[i]).leftJustified(maxOne, ' ') + ", ";
      QString word = _word[i].isEmpty() ? QString().leftJustified(maxWord + 2, ' ') : "--" + QString(_word[i]).leftJustified(maxWord, ' ');
      lines.append(one + word + "   " + _text[i]);
    }

    std::cout << std::endl;
    foreach ( const QString& l, lines ) {
      std::cout << l.toLocal8Bit().constData() << std::endl;
    }
    std::cout << std::endl;
  }

private:
  QStringList _one;
  QStringList _word;
  QStringList _text;
};

#endif // CROSS_COMMONS_APP_HELPFORMATTER_H
