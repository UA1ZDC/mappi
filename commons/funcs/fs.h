#ifndef COMMONS_FUNCS_FS_H
#define COMMONS_FUNCS_FS_H

#include <qstringlist.h>

namespace meteo {

//! Возвращает список абсолютных путей ко всем вложенным директориям до уровня maxDepth.
QStringList dirList(const QStringList& paths, int maxDepth = -1);

QStringList fileList(const QString& path, const QStringList& extensions = QStringList() );
QStringList fileList(const QString& path, const QString& extension );

//!< Возвращает файлы, соответствующие шаблону
QStringList fileFilteredList(const QString& path, const QString& wildcard );

}

#endif
