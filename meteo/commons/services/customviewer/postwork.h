#ifndef METEO_BANK_SERVICES_CUSTOMVIEWER_POSTWORK_H
#define METEO_BANK_SERVICES_CUSTOMVIEWER_POSTWORK_H
#include <qhash.h>

#include <meteo/commons/proto/customviewer.pb.h>

namespace meteo {

void defaultPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& targetColumn, proto::CustomViewerField* out);

void defaultDatetimeWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& targetColumn, proto::CustomViewerField* out);

void msgviewerPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& targetColumn, proto::CustomViewerField* out);
void msgviewerRoutePostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& column, proto::CustomViewerField* out);

void journalPostWorker(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& column, proto::CustomViewerField* out);
}

#endif // POSTWORK_H
