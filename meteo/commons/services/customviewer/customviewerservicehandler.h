#ifndef METEO_COMMONS_SERVICES_CUSTOMVIEWER_CUSTOMVIEWERSERVICEHANDLER_H
#define METEO_COMMONS_SERVICES_CUSTOMVIEWER_CUSTOMVIEWERSERVICEHANDLER_H

#include <meteo/commons/proto/customviewer.pb.h>
#include <sql/dbi/dbientry.h>
#include <qstringlist.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

namespace meteo {

typedef QString LoaderFunction(const QList<Condition> &conditions, const QString &sortColumn, Qt::SortOrder order, int skip, int limit, const QString &);
typedef qint64 CounterFunction(const QList<Condition> &conditions);
typedef void PostWorkerFunction(const proto::ViewerConfig* conf, const QHash<QString, QString>& row, const QString& targetColumn, proto::CustomViewerField* out);

class CustomViewerServiceHandler
{
public:
  CustomViewerServiceHandler();
  ~CustomViewerServiceHandler();

  void LoadData(const ::meteo::proto::CustomviewerDataRequest* request,
                ::meteo::proto::CustomviewerDataResponce* response);

  void LoadConfig(const ::meteo::proto::ViewerConfigRequest* request,
                  ::meteo::proto::ViewerConfig* response);

  void CountData(const ::meteo::proto::CustomViewerCountRequest* request,
                 ::meteo::proto::CustomViewerCountResponce* response);
  void GetLastTlgId(const ::meteo::proto::Dummy* request,
                    ::meteo::proto::GetLastTlgIdResponce* response);
  void GetAvailableTT(const ::meteo::proto::Dummy* request,
                      ::meteo::proto::GetAvailableTTResponce* response);

  void GetTypecAreas(const ::meteo::proto::Dummy* request,
                     ::meteo::proto::GetTypecAreasResponce* response);

  void GetIcon(const ::meteo::proto::CustomViewerIconRequest* request,
               ::meteo::proto::CustomViewerIconResponce* response);

  void GetTelegramDetails(const ::meteo::proto::MsgViewerDetailsRequest* request,
                       ::meteo::proto::MsgViewerDetailsResponce* response);

public:
  static proto::ViewerType columnType(const proto::ViewerConfig *conf, const QString& type);
  static QStringList columnsForCollection(const proto::ViewerConfig *conf);

private:
  const proto::ViewerConfig* configForViewer(const meteo::proto::CustomViewerId table) const;  

  QHash<QString, QString> dataFromDocument(const meteo::DbiEntry &doc, const meteo::proto::CustomViewerId id);

  QStringList columnsForCollection(const meteo::proto::CustomViewerId table) const;
  proto::ViewerType columnType(const meteo::proto::CustomViewerId id, const QString& type) const;


  PostWorkerFunction* postWorkFunctionForColumn(const meteo::proto::CustomViewerId id, const QString& col);

  ConnectProp connectPropForDatabase(const meteo::proto::CustomViewerId table);


  void fillBufrTlg(const tlg::MessageNew& msg, proto::MsgViewerDetailsResponce* resp);

  QString getNameByT1(const QString& t1);
  QString getNameByT1T2(const QString& t1, const QString& t2);
  QString getNameCyrByNameSMB(const QString& nameSmb);

  bool initContentWidget(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding);
  bool loadBinMessage(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding);
  bool loadTextMessage(const meteo::tlg::MessageNew& tlg, proto::MsgViewerDetailsResponce* resp, const QString& encoding);

  QString replaceContent(const QByteArray& data, bool val);
  QString decodeMsg(const QByteArray& raw) const;
  void loadDecodedContent(proto::MsgViewerDetailsResponce* resp, const QByteArray& tlg, const QString& msgtype, const QDateTime& dt, const QString& encoding);


private:
  QHash< proto::CustomViewerId, proto::ViewerConfig*> viewerConfigs_;
  QHash< proto::CustomViewerId, LoaderFunction*> loaders_;
  QHash< proto::CustomViewerId, CounterFunction*> counters_;
  QHash< proto::CustomViewerId, ConnectProp> props_;
  QHash< QString, PostWorkerFunction*> postWork_;
};

}

#endif
