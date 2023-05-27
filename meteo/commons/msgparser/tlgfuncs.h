#ifndef METEO_COMMONS_MSGPARSER_TLGFUNCS_H
#define METEO_COMMONS_MSGPARSER_TLGFUNCS_H

#include <qstring.h>
#include <qdatetime.h>

namespace meteo {

//! TODO: необходимо реализовать работу не только с tar, чтобы работало в windows
QByteArray readTelegram(const QString& fileName, bool* aOk = nullptr, QString* aError = nullptr);

//!
QDateTime convertedDateTime(const QDateTime& metaDt, const QString& yygggg);

} // meteo

#endif // METEO_COMMONS_MSGPARSER_TLGFUNCS_H
