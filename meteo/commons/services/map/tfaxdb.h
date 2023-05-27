#ifndef METEO_COMMONS_SERVICES_MAP_TFAXDB_H
#define METEO_COMMONS_SERVICES_MAP_TFAXDB_H

#include <meteo/commons/proto/document_service.pb.h>
#include <qbytearray.h>
#include <qstring.h>
#include <qdatetime.h>

namespace meteo {
namespace tfaxdb {

bool getAllFaxes(const QDateTime& dtStart, const QDateTime &dtEnd, meteo::map::proto::FaxReply* reply);
bool getFaxById(const QString& id, QByteArray *bytes);


}
}

#endif
