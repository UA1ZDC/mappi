#include "tgradaciidb.h"
#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/global.h>

namespace meteo{
  namespace forecast{

    bool TGradaciiDB::getGradacii(int level, int levelType, int descr, int hour, meteo::forecast::Gradacii* responce)
    {
      std::unique_ptr<Dbi> db(meteo::global::dbForecast());
      if ( nullptr == db.get() ) {
	error_log << QObject::tr("Ошибка при подключении к базе данных mongodbConfForecast");
	return false;
      }
      
      auto query = db->queryptrByName("get_gradacii_by_level_leveltype_descr");
      if(nullptr == query) {return false;}
      query->arg("descriptor",descr);
      query->arg("level",level);
      query->arg("type_level",levelType);
      query->arg("hour",hour);

      QString error;
      if(false == query->execInit( &error)){
	//error_log << error;
	return false;
      }
      
      int cnt = 0;
      while ( true == query->next()) {
	++cnt;
	const DbiEntry& doc = query->entry();
	int dopuskGrad = doc.valueInt32("dopusk_grad");
	responce->set_dopusk_grad(dopuskGrad);
	if (doc.hasField("delta")){
	  double delta = doc.valueDouble("delta");
	  responce->set_delta(delta);
	}
	if (doc.hasField("time_delta")){
	  double timeDelta = doc.valueDouble("time_delta");
	  responce->set_time_delta(timeDelta);

	}
	if (doc.hasField("gradacii")){
	  auto gradacii = doc.valueArray("gradacii");

	  meteo::Document one;
	  while (gradacii.next()){
	    gradacii.valueDocument(&one);
	    double min = one.valueDouble("min");
	    double max = one.valueDouble("max");

	    responce->add_gradacii_min(min);
	    responce->add_gradacii_max(max);
	  }
	}
	if (doc.hasField("delta")){
	  double delta = doc.valueDouble("delta");
	  responce->set_delta(delta);
	}
      }
      return true;
    }

  }
}
