#include "base_data_service.h"

#include <meteo/commons/global/global.h>

namespace meteo {
namespace surf {

TBaseDataService::TBaseDataService(google::protobuf::Closure* done, int serviceType)
  : done_(done), serviceType_(serviceType){ }

TBaseDataService::~TBaseDataService() {

}

} // surf
} // meteo
