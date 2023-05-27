#ifndef METEO_PROGNOZ_GLOBAL_GLOBAL_H
#define METEO_PROGNOZ_GLOBAL_GLOBAL_H

#include <meteo/commons/global/global.h>

namespace prognoz {
namespace global {
  //! Функция для TLog (записывает логи в dbTelegram)
  void logoutToDb(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& m);
} // global
} // prognoz

namespace meteo {

class PrognozGlobal : public Global
{
public:
  PrognozGlobal(){}
  virtual ~PrognozGlobal(){}

  virtual ConnectProp dbConfTelegram()  override;
  virtual ConnectProp dbConfMeteo()     override;
  virtual ConnectProp dbConfObanal()    override;

  virtual ConnectProp mongodbConfTelegrams()  const override;
  virtual ConnectProp mongodbConfMeteo()      const override;
  virtual ConnectProp mongodbConfSprinf()     const override;
  virtual ConnectProp mongodbConfObanal()     const override;

  virtual int serviceTimeout(settings::proto::ServiceCode c)                          const override;
  virtual settings::proto::Service service(settings::proto::ServiceCode c, bool* ok)  const override;
  virtual ::rpc::Address serviceAddress(settings::proto::ServiceCode c, bool* ok = 0) const override;
  virtual ::rpc::Channel* serviceChannel(settings::proto::ServiceCode c)              const override;
  virtual ::rpc::TController* serviceController(settings::proto::ServiceCode c)       const override;
};

} // meteo


#endif // METEO_PROGNOZ_GLOBAL_GLOBAL_H
