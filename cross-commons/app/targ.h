#ifndef CROSS_COMMONS_APP_TARG_H
#define CROSS_COMMONS_APP_TARG_H

#include <qstringlist.h>
#include <qmap.h>

/*! \brief Класс для работы с агрументами командной строки.

    Распознает аргументы следующих форматов:
      * однобуквенные ключи (-s -H)
      * группу однобуквенных ключей (-sHpd )
      * однобуквенные ключи с параметром (-u arm -H -d db_name -T 'let's go' -T "just do it")
      * многобуквенные ключи (--set --cpp-out)
      * многобуквенные ключи с параметрами (--set=90 --text='hello world!' --go="to the right")
 */

const QString kDefaultRegExp = "^([-]{1,2})([\\-a-zA-z0-9]+)(=(.*))?";
class TArg
{
public:
  enum CompareFlag {
    Flag_All,         //!< все
    Flag_Any          //!< любой
  };

public:
  TArg(int argc, char** argv);
  TArg(const QStringList& args);

  //! \note Если ключ связан с несколькими значениями, возвращается первое
  QString value( const QString& key, const QString& defaultValue = QString() ) const;
  //! Возвращает первое значение для группы ключей.
  QString value( const QStringList& keys, const QString& defaultValue = QString() ) const;
  QStringList values( const QString& key, const QStringList& defaultValue = QStringList() ) const;
  //! Возвращает все значения для группы ключей
  QStringList values( const QStringList& keys, const QStringList& defaultValue = QStringList() ) const;
  //! Возвращает все ключи
  QStringList keys() const;

  bool contains( const QString& key ) const;
  bool contains( const QStringList& keys, CompareFlag flag = Flag_Any  ) const;

  int keyCount() const;

private:
  void parse(const QStringList& list);
  void append( const QString& key, const QString& value = QString() );

private:
  QMap<QString,QString>     singleValues_;
  QMap<QString,QStringList> multiValues_;
};

#endif // CROSS_COMMONS_APP_TARG_H
