#include "obanaltfp.h"
#include "temp_func.h"

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>

#include <meteo/commons/zond/diagn_func.h>

#include <commons/obanal/tfield.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>


#include <qdatetime.h>

using namespace meteo;

ObanalTfp::ObanalTfp()
{
  _oservice = new meteo::TObanalService;
  _db = new TObanalDb;
}


ObanalTfp::~ObanalTfp()
{
  delete _oservice;
  _oservice = nullptr;

  delete _db;
  _db = nullptr;
}

bool ObanalTfp::runField(const QDateTime& dt)
{
  bool ok =false;
  QList<int> net_types = QList<int>() << 4 << 3 << 7; //надо учитывать шаг (правильный коэффициент)
  // STEP_100x100 = 4,  //!< через 1 градус по широте и долготе
  // STEP_125x125 = 3,  //!< через 1.25 градуса по широте и долготе
  // STEP_0500x0500 = 7, //!< через .5 градуса по широте и долготе
  
  
  for (auto net_type : net_types) {
    
    var(net_type);

    // ok = runField_Renard_Clark(dt, net_type);
    // var(ok);

    ok = runField_Renard_ClarkQpot(dt, net_type);
    var(ok);


 /*   ok = runField_Renard_ClarkKm(dt, net_type);
    var(ok);*/
 
    
    // ok = runField_HuberPock_Kress(dt, net_type);
    // var(ok);
  //   ok = runField_HuberPock_KressKm(dt, net_type);
  //   var(ok);


/*
    ok = runField_JapanAgency(dt, net_type);
    var(ok);

    //ok = runField_JapanAgencyKm(dt, net_type);
    //var(ok);
    
    ok = runField_Hewson(dt, net_type);
    var(ok);
*/

    ok = runField_Hewson6(dt, net_type);
    var(ok);
    /*
    ok = runField_Hewson6_m1(dt, net_type);
    var(ok);
    */
    
    ok = runField_Hewson6_m2(dt, net_type);
    var(ok);

    
  }

  
  return ok;
}

//Renard & Clark (1965)
//potential temperature
bool ObanalTfp::runField_Renard_Clark(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid;

  
  if(! getAvailable(dt, "T", 850, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }

  
  //var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t850 = std::make_unique<obanal::TField>();
    bool ok = true;
    if (!it.value().empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(it.value(), t850.get(), &fdescr);
      if (ok) {
	t850->mnoz(powf(1000.f/850.f, zond::kKap)); //потенциальная темпа
	std::unique_ptr<obanal::TField>	tfp = calcTFP(*t850);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPrc"));
	fdescr.set_level(850);
	saveField(tfp.get(), fdescr);
      }
      
    }
    
  }

  return true;
}


bool ObanalTfp::runField_Renard_ClarkKm(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid;

  
  if(! getAvailable(dt, "T", 850, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }

  
  //var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t850 = std::make_unique<obanal::TField>();
    bool ok = true;
    if (!it.value().empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(it.value(), t850.get(), &fdescr);
      if (ok) {
	t850->mnoz(powf(1000.f/850.f, zond::kKap)); //TODO потенциальная темпа. Точно просто коэф?
	std::unique_ptr<obanal::TField>	tfp = calcTFPKm(*t850);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPrckm"));
	fdescr.set_level(850);
	saveField(tfp.get(), fdescr);
      }
      
    }
   
  }

  return true;
}

//Renard & Clark (1965)
//potential temperature
bool ObanalTfp::runField_Renard_ClarkQpot(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> tkey, humkey;
  bool isSpecific = true;
  
  if(! getAvailable(dt, "T", 850, net_type, &tkey)) {
    info_log << QObject::tr("Нет данных T");
    return false;
  }

  if(! getAvailable(dt, "q", 850, net_type, &humkey)) {
    info_log << QObject::tr("Нет данных q");

    isSpecific = false;
    if(! getAvailable(dt, "U", 850, net_type, &humkey)) {
      info_log << QObject::tr("Нет данных U");
      return false;
    }  
  }
  
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(tkey);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t850 = std::make_unique<obanal::TField>();
    auto hum850 = std::make_unique<obanal::TField>();
    bool ok = true;
    
    if (!it.value().empty() && !humkey.value(it.key()).empty()) {
      // debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(tkey.value(it.key()), t850.get(), &fdescr);
      ok &= getField(humkey.value(it.key()), hum850.get(), &fdescr);
      if (ok) {
	toQe(t850.get(), *hum850, isSpecific);
	//	tfp = calcTFP(*t850.get());
	std::unique_ptr<obanal::TField>	tfp = calcTFP(*t850);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPrckm"));
	fdescr.set_level(850);
	saveField(tfp.get(), fdescr);
      }
      
    }

  }


  
  return true;
}



//Huber-Pock & Kress (1989)
//equivalent thickness
bool ObanalTfp::runField_HuberPock_Kress(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> hh850key, hh500key;
  QMap<FieldsKey, std::string> T850key, T500key, T700key;
;

  if(! getAvailable(dt, "hh", 850, net_type, &hh850key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
    
  if(! getAvailable(dt, "hh", 500, net_type, &hh500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  if(! getAvailable(dt, "T", 850, net_type, &T850key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  if(! getAvailable(dt, "T", 700, net_type, &T700key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  if(! getAvailable(dt, "T", 500, net_type, &T500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  //var(hh850key.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(hh850key);
  while (it.hasNext()) {
    it.next();
    if (! hh500key.contains(it.key()) ||
	! T500key.contains(it.key()) ||
	! T700key.contains(it.key()) ||
	! T850key.contains(it.key()) ||
	it.value().empty() ||
	hh500key.value(it.key()).empty() ||
	T500key.value(it.key()).empty()  ||
	T700key.value(it.key()).empty()  ||
	T850key.value(it.key()).empty()) {
      continue;
    }
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto h850 = std::make_unique<obanal::TField>();
    auto h500 = std::make_unique<obanal::TField>();
    auto T850 = std::make_unique<obanal::TField>();
    auto T700 = std::make_unique<obanal::TField>();
    auto T500 = std::make_unique<obanal::TField>();
    

    bool ok = true;
    //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();
    
    ok &= getField(it.value(), h850.get(), &fdescr);
    ok &= getField(hh500key.value(it.key()), h500.get(), &fdescr);
    ok &= getField(T850key.value(it.key()), T850.get(), &fdescr);
    ok &= getField(T700key.value(it.key()), T700.get(), &fdescr);
    ok &= getField(T500key.value(it.key()), T500.get(), &fdescr);
    
      if (ok) {
	auto Zte = calcZte(*h850, *h500, *T850, *T700, *T500);
	//h500->minus(h850, &Zte);
	if (nullptr != Zte) {
	  std::unique_ptr<obanal::TField> tfp = calcTFP(*Zte);
	  fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPhp"));
	  fdescr.set_level(850);
	  saveField(tfp.get(), fdescr);
	}
      }
    
  }

  return true;
}

//Huber-Pock & Kress (1989)
//equivalent thickness
bool ObanalTfp::runField_HuberPock_KressKm(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> hh850key, hh500key;
  QMap<FieldsKey, std::string> T850key, T500key, T700key;
;

  if(! getAvailable(dt, "hh", 850, net_type, &hh850key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
    
  if(! getAvailable(dt, "hh", 500, net_type, &hh500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  if(! getAvailable(dt, "T", 850, net_type, &T850key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  if(! getAvailable(dt, "T", 700, net_type, &T700key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  if(! getAvailable(dt, "T", 500, net_type, &T500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  //var(hh850key.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(hh850key);
  while (it.hasNext()) {
    it.next();
    if (! hh500key.contains(it.key()) ||
	! T500key.contains(it.key()) ||
	! T700key.contains(it.key()) ||
	! T850key.contains(it.key()) ||
	it.value().empty() ||
	hh500key.value(it.key()).empty() ||
	T500key.value(it.key()).empty()  ||
	T700key.value(it.key()).empty()  ||
	T850key.value(it.key()).empty()) {
      continue;
    }
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto h850 = std::unique_ptr<obanal::TField>();
    auto h500 = std::unique_ptr<obanal::TField>();
    auto T850 = std::unique_ptr<obanal::TField>();
    auto T700 = std::unique_ptr<obanal::TField>();
    auto T500 = std::unique_ptr<obanal::TField>();
    

    bool ok = true;

    //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();
    
    ok &= getField(it.value(), h850.get(), &fdescr);
    ok &= getField(hh500key.value(it.key()), h500.get(), &fdescr);
    ok &= getField(T850key.value(it.key()), T850.get(), &fdescr);
    ok &= getField(T700key.value(it.key()), T700.get(), &fdescr);
    ok &= getField(T500key.value(it.key()), T500.get(), &fdescr);
    
      if (ok) {
	auto Zte = calcZte(*h850, *h500, *T850, *T700, *T500);
	//h500->minus(h850, &Zte);
	if (nullptr != Zte) {
	  std::unique_ptr<obanal::TField> tfp = calcTFPKm(*Zte);
	  fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPjakm"));
	  fdescr.set_level(850);
	  saveField(tfp.get(), fdescr);
	}
      }
    
  }

  return true;
}



//Japan Meteorological Agency (1988)
//thickness
bool ObanalTfp::runField_JapanAgency(const QDateTime& dt, int net_type)
{
  trc;
  Q_UNUSED(net_type);
  QMap<FieldsKey, std::string> hh1000key, hh500key;

  
  if(! getAvailable(dt, "hh", 1000, net_type, &hh1000key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
    
  if(! getAvailable(dt, "hh", 500, net_type, &hh500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  //var(hh1000key.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(hh1000key);
  while (it.hasNext()) {
    it.next();
    if (! hh500key.contains(it.key()) ||
	it.value().empty() ||
	hh500key.value(it.key()).empty()) {
      continue;
    }
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto h1000 = std::unique_ptr<obanal::TField>();
    auto h500 = std::unique_ptr<obanal::TField>();
    
    obanal::TField* Z = new obanal::TField;   // thickness    
    bool ok = true;

    //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();
    
    ok &= getField(it.value(), h1000.get(), &fdescr);
    ok &= getField(hh500key.value(it.key()), h500.get(), &fdescr);
    
    if (ok) {
      h500->minus(h1000.get(), &Z);      
      if (nullptr != Z) {
	Z->mnoz(10);
	std::unique_ptr<obanal::TField> tfp = calcTFP(*Z);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPja"));
	fdescr.set_level(850);
	saveField(tfp.get(), fdescr);
      }
    }
        
    delete Z; Z = nullptr;
  }

  return true;
}


bool ObanalTfp::runField_JapanAgencyKm(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> hh1000key, hh500key;

  
  if(! getAvailable(dt, "hh", 1000, net_type, &hh1000key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
    
  if(! getAvailable(dt, "hh", 500, net_type, &hh500key)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }
  
  //var(hh1000key.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(hh1000key);
  while (it.hasNext()) {
    it.next();
    if (! hh500key.contains(it.key()) ||
	it.value().empty() ||
	hh500key.value(it.key()).empty()) {
      continue;
    }
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto h1000 = std::unique_ptr<obanal::TField>();
    auto h500 = std::unique_ptr<obanal::TField>();
    
    obanal::TField* Z = nullptr;   // thickness
    bool ok = true;

    //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();
    
    ok &= getField(it.value(), h1000.get(), &fdescr);
    ok &= getField(hh500key.value(it.key()), h500.get(), &fdescr);
    
    if (ok) {
      h500->minus(h1000.get(), &Z);      
      if (nullptr != Z) {
	Z->mnoz(10);
	std::unique_ptr<obanal::TField>	tfp = calcTFPKm(*Z);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPjakm"));
	fdescr.set_level(850);
	saveField(tfp.get(), fdescr);
      }
    }
        
    delete Z; Z = nullptr;
  }

  return true;
}

//Hewson (1997)
//wet-bulb potential temperature
bool ObanalTfp::runField_Hewson(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid;

  int level = 850; //TODO в доке 900
  
  if(! getAvailable(dt, "T", level, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }

  
  //  var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t900 = std::make_unique<obanal::TField>();
    bool ok = true;
    if (!it.value().empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(it.value(), t900.get(), &fdescr);
      if (ok) {
	auto Tpot = calcTpot(*t900, level);
	std::unique_ptr<obanal::TField>	tfp = calcTFP_Hewson(*Tpot);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPh"));
	fdescr.set_level(level);
	saveField(tfp.get(), fdescr);
      }
      
    }
   
  }

  return true;
}


//Hewson (1997)
//wet-bulb potential temperature, критерий M1
bool ObanalTfp::runField_Hewson6_m1(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid;

  int level = 850; //TODO в доке 900
  
  if(! getAvailable(dt, "T", level, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных");
    return false;
  }

  
  //  var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t900 = std::make_unique<obanal::TField>();
    bool ok = true;
    if (!it.value().empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(it.value(), t900.get(), &fdescr);
      if (ok) {
	auto Tpot = calcTpot(*t900, level);
	std::unique_ptr<obanal::TField> tfp = calcTFP(*Tpot);
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPh6m1"));
	fdescr.set_level(level);
	saveField(tfp.get(), fdescr);
      }
      
    }
  }

  return true;
}

bool ObanalTfp::runField_Hewson6_m2(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid, humkey;
  int level = 850; //TODO в доке 900
  bool isSpecific = true;
  
  if(! getAvailable(dt, "T", level, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных T");
    return false;
  }

  if(! getAvailable(dt, "q", 850, net_type, &humkey)) {
    info_log << QObject::tr("Нет данных q");
    
    isSpecific = false;
    if(! getAvailable(dt, "U", 850, net_type, &humkey)) {
      info_log << QObject::tr("Нет данных U");
      return false;
    }  
  }
  
  //  var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t900 = std::make_unique<obanal::TField>();
    auto hum850 = std::make_unique<obanal::TField>();    
    bool ok = true;
    if (!it.value().empty() && !humkey.value(it.key()).empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();
      ok &= getField(fileid.value(it.key()), t900.get(), &fdescr);
      ok &= getField(humkey.value(it.key()), hum850.get(), &fdescr);
      
      if (ok) {
	toQe(t900.get(), *hum850, isSpecific);	
	std::unique_ptr<obanal::TField> tfp = calcHewsomM2(*t900);
	
	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPh6m2"));
	fdescr.set_level(level);
	saveField(tfp.get(), fdescr);
      }
      
    }
    
  }

  return true;
}


//Hewson (1997)
//wet-bulb potential temperature
// bool ObanalTfp::runField_Hewson6(const QDateTime& dt, int net_type)
// {
//   trc;
//   QMap<FieldsKey, std::string> fileid;

//   int level = 850; //TODO в доке 900
  
//   if(! getAvailable(dt, "T", level, net_type, &fileid)) {
//     info_log << QObject::tr("Нет данных");
//     return false;
//   }

  
//   var(fileid.size());
  
//   int cnt = 0;
//   QMapIterator<FieldsKey, std::string> it(fileid);
//   while (it.hasNext()) {
//     it.next();
    
//     cnt++;
    
//     meteo::field::DataDesc fdescr;

//     obanal::TField* t900 = new obanal::TField;
//     obanal::TField* Tpot = nullptr;
//     obanal::TField* tfp = nullptr;
//     bool ok = true;
//     if (!it.value().empty()) {
//       //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
//       ok &= getField(it.value(), t900, &fdescr);
//       if (ok) {
// 	Tpot = calcTpot(*t900, level);
// 	tfp = calcTFP_Hewson6(*Tpot);

//  	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPh6"));
// 	fdescr.set_level(level);
// 	saveField(tfp, fdescr);
//       }
      
//     }
    
//     delete t900;
//     delete Tpot;
//     delete tfp;
//   }

//   return true;
// }


bool ObanalTfp::runField_Hewson6(const QDateTime& dt, int net_type)
{
  trc;
  QMap<FieldsKey, std::string> fileid, humkey;
  bool isSpecific = true;
  int level = 850; //TODO в доке 900
  
  if(! getAvailable(dt, "T", level, net_type, &fileid)) {
    info_log << QObject::tr("Нет данных T");
    return false;
  }

  if(! getAvailable(dt, "q", 850, net_type, &humkey)) {
    info_log << QObject::tr("Нет данных q");
    
    isSpecific = false;
    if(! getAvailable(dt, "U", 850, net_type, &humkey)) {
      info_log << QObject::tr("Нет данных U");
      return false;
    }  
  }
  
  var(fileid.size());
  
  int cnt = 0;
  QMapIterator<FieldsKey, std::string> it(fileid);
  while (it.hasNext()) {
    it.next();
    
    cnt++;
    
    meteo::field::DataDesc fdescr;

    auto t900 = std::make_unique<obanal::TField>();
    auto hum850 = std::make_unique<obanal::TField>();    

    bool ok = true;
    if (!it.value().empty() && !humkey.value(it.key()).empty()) {
      //debug_log << it.key().model << it.key().center << it.key().net_type << it.value();  
      ok &= getField(it.value(), t900.get(), &fdescr);
      ok &= getField(humkey.value(it.key()), hum850.get(), &fdescr);
      if (ok) {
	toQe(t900.get(), *hum850, isSpecific);
	auto tfp = calcTFP_Hewson6(*t900);

 	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("TFPh6"));
	fdescr.set_level(level);
	saveField(tfp.get(), fdescr);
      }
      
    }
    
  }

  return true;
}


std::unique_ptr<obanal::TField> ObanalTfp::calcTFP(const obanal::TField& field)
{
  int sizeFi = field.kolFi();
  int sizeLa = field.kolLa();

  //debug_log << "size" << sizeFi << sizeLa;
  
  std::unique_ptr<obanal::TField> tfp(field.getCopy());
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {

      float grad_fi, grad_la;
      float grad_fi2, grad_la2;
      
      bool ok = field.gradient(fi_idx, la_idx, &grad_fi, &grad_la);
      if (!ok) {
	//debug_log << QObject::tr("err grad") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      // float temp;
      // field.getData(fi_idx, la_idx, &temp);
      // debug_log << "value" << temp;
      
      float grad_modul = sqrt(grad_fi*grad_fi+grad_la*grad_la);

      if (grad_modul < 1e-10) {
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }
      
      grad_fi /= grad_modul;
      grad_la /= grad_modul;
      
      ok = field.gradient2(fi_idx, la_idx, &grad_fi2, &grad_la2);
      if (!ok) {
	//debug_log << QObject::tr("err grad2") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      //debug_log << "grad2" << grad_fi2 << grad_la2;
      
      float tfp_val = (grad_fi*grad_fi2 + grad_la*grad_la2)*1e10;

      if (field.getFi(fi_idx)*180/3.14 > 10  && field.getFi(fi_idx)*180/3.14 < 80 ) { 
	if (field.getLa(la_idx)*180/3.14 > 0  && field.getLa(la_idx)*180/3.14 < 70 ) {
	  /*debug_log << "temp" << field.getData(fi_idx, la_idx);
	  debug_log << "grad" << grad_fi << grad_la << grad_modul; 
	  debug_log << "tfp" << tfp_val << field.getFi(fi_idx)*180/3.14 << field.getLa(la_idx)*180/3.14;
	  */
	}
      }
       
      tfp->setData(fi_idx, la_idx, tfp_val, true);
      
    }
  }
  
  return tfp;  
}

std::unique_ptr<obanal::TField> ObanalTfp::calcTFPKm(const obanal::TField& field)
{
  int sizeFi = field.kolFi();
  int sizeLa = field.kolLa();
  float step = 100; //км

  //debug_log << "size" << sizeFi << sizeLa;
  
  std::unique_ptr<obanal::TField> tfp(field.getCopy());
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {

      float grad_fi, grad_la;
      float grad_fi2, grad_la2;
      
      bool ok = field.gradientKm(fi_idx, la_idx, step, &grad_fi, &grad_la);
      if (!ok) {
	//debug_log << QObject::tr("err grad") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      // float temp;
      // field.getData(fi_idx, la_idx, &temp);
      // debug_log << "value" << temp;
      
      float grad_modul = sqrt(grad_fi*grad_fi+grad_la*grad_la);
      //debug_log << "grad" << grad_fi << grad_la << grad_modul; 
      
      grad_fi /= grad_modul;
      grad_la /= grad_modul;
      
      ok = field.gradient2Km(fi_idx, la_idx, step, &grad_fi2, &grad_la2);
      if (!ok) {
	//debug_log << QObject::tr("err grad2") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      //debug_log << "grad2" << grad_fi2 << grad_la2;
      

      float tfp_val = (grad_fi*grad_fi2 + grad_la*grad_la2)*1e11; //TFP > 1.5 для RC
        
      //debug_log << "tfp" << tfp_val << field.getFi(fi_idx)*180/3.14 << field.getLa(la_idx)*180/3.14;
       
      tfp->setData(fi_idx, la_idx, tfp_val, true);
      
    }
  }
  
  return tfp;  
}

//Objective fronts, Hewson
//формула 7
std::unique_ptr<obanal::TField> ObanalTfp::calcTFP_Hewson(const obanal::TField& field)
{
  trc;
  int sizeFi = field.kolFi();
  int sizeLa = field.kolLa();

  //debug_log << "size" << sizeFi << sizeLa;

  std::unique_ptr<obanal::TField> tfp(field.getCopy());
  
  obanal::TField* mu_modul = field.getCopy();
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {

      float grad_fi2, grad_la2;
      bool ok = field.gradient2(fi_idx, la_idx, &grad_fi2, &grad_la2);
      if (!ok) {
	mu_modul->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      float grad_modul = sqrt(grad_fi2*grad_fi2+grad_la2*grad_la2);
      //debug_log << "mu grad" << grad_fi2 << grad_la2 << grad_modul; 
      float mu_val =  grad_modul;
      //      var(mu_val);
       
      mu_modul->setData(fi_idx, la_idx, mu_val, true);      
    }    
  }

  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {
      float grad_fi, grad_la;
      float grad_fi2, grad_la2;
      
      bool ok = mu_modul->gradient(fi_idx, la_idx, &grad_fi, &grad_la);
      if (!ok) {
	//debug_log << QObject::tr("err grad") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      //debug_log << "grad mu" << grad_fi << grad_la; 

      ok = field.gradient2(fi_idx, la_idx, &grad_fi2, &grad_la2); //mu
      if (!ok) {
	//debug_log << QObject::tr("err grad2") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      float grad_modul2 = sqrt(grad_fi2*grad_fi2+grad_la2*grad_la2);
      //debug_log << "grad2" << grad_fi2 << grad_la2 << grad_modul2;
      
      grad_fi2 /= grad_modul2;
      grad_la2 /= grad_modul2;
      //debug_log << "grad2/modul2" << grad_fi2 << grad_la2;
      
      float tfp_val = (grad_fi*grad_fi2 + grad_la*grad_la2)*1e16;
  
      // debug_log << "tfp" << tfp_val << field.getFi(fi_idx)*180/3.14 << field.getLa(la_idx)*180/3.14;
      tfp->setData(fi_idx, la_idx, tfp_val, true);
    }
  }
  

  delete mu_modul;
  return tfp;  
}

//Objective fronts, Hewson
//формула 6
std::unique_ptr<obanal::TField> ObanalTfp::calcTFP_Hewson6(const obanal::TField& field)
{
  trc;
  auto vecModul = [](float dx, float dy) {
    return sqrt(dx*dx + dy*dy);
  };


  std::unique_ptr<obanal::TField> tfp(field.getCopy());

  float step;
  if (! field.stepFi(&step)) {
    error_log << QObject::trUtf8("Ошибка определения шага сетки %1").arg(step);
    return tfp;
  }
  step *= 6.371e6f; //на радиус земли, для перевода в длину дуги в км
 
  int sizeFi = field.kolFi();
  int sizeLa = field.kolLa();

  //debug_log << "size" << sizeFi << sizeLa;

  
  obanal::TField* modul = field.getCopy();
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {

      float grad_fi, grad_la;
      bool ok = field.gradient(fi_idx, la_idx, &grad_fi, &grad_la);
      if (!ok) {
	modul->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      float grad_modul = vecModul(grad_fi, grad_la);
      //debug_log << "grad" << grad_fi << grad_la << grad_modul; 

      if (grad_modul < 1e-10) {
	modul->setData(fi_idx, la_idx, -9999, false);
      } else {
	modul->setData(fi_idx, la_idx, grad_modul, true);
	// tfp->setData(fi_idx, la_idx, grad_modul*1e5, true);
      }

    }    
  }

  //return tfp;

  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {
      float cosFi = cosf(modul->getFi(fi_idx));
      if(MnMath::isZero(cosFi)){
	tfp->setData(fi_idx, la_idx, -9999, false);	
	continue;
      }
      
      float grad_fi_x, grad_la_x;
      float grad_fi_a, grad_la_a;
      float grad_fi_b, grad_la_b;
      float grad_fi_c, grad_la_c;
      float grad_fi_d, grad_la_d;
                 
      bool ok = modul->gradient(fi_idx, la_idx, &grad_fi_x, &grad_la_x);
      ok |= modul->gradient(fi_idx,   la_idx-1, &grad_fi_a, &grad_la_a);
      ok |= modul->gradient(fi_idx,   la_idx+1, &grad_fi_b, &grad_la_b);
      ok |= modul->gradient(fi_idx+1, la_idx,   &grad_fi_c, &grad_la_c);
      ok |= modul->gradient(fi_idx-1, la_idx,   &grad_fi_d, &grad_la_d);
      if (!ok) {
	//debug_log << QObject::tr("err grad") << fi_idx << la_idx;
	tfp->setData(fi_idx, la_idx, -9999, false);
	continue;
      }
      float s_betta, s_length;
      deriveMeanAxis(grad_fi_x, grad_la_x, grad_fi_a, grad_la_a,
		     grad_fi_b, grad_la_b, grad_fi_c, grad_la_c,
		     grad_fi_d, grad_la_d, &s_betta, &s_length);


      //float grad_fi_x1 = -grad_la_x*sin(s_betta) + grad_fi_x*cos(s_betta);
      //float grad_la_x1 =  grad_la_x*cos(s_betta) + grad_fi_x*sin(s_betta);
     
 
      float grad_fi_a1 = -grad_la_a*sin(s_betta) + grad_fi_a*cos(s_betta);
      float grad_la_a1 =  grad_la_a*cos(s_betta) + grad_fi_a*sin(s_betta);      
      //float grad_fi_b1 = -grad_la_b*sin(s_betta) + grad_fi_b*cos(s_betta);
      float grad_la_b1 =  grad_la_b*cos(s_betta) + grad_fi_b*sin(s_betta);      
      float grad_fi_c1 = -grad_la_c*sin(s_betta) + grad_fi_c*cos(s_betta);
      float grad_la_c1 =  grad_la_c*cos(s_betta) + grad_fi_c*sin(s_betta);      
      float grad_fi_d1 = -grad_la_d*sin(s_betta) + grad_fi_d*cos(s_betta);
      float grad_la_d1 =  grad_la_d*cos(s_betta) + grad_fi_d*sin(s_betta);
      
      //float totalDiverg = (grad_fi_c1 - grad_fi_d1) / (2*step) / cos(s_betta)  + (grad_la_b1 - grad_la_a1) / (2*step*sinT) / cos(s_betta);
      float step_fi = 2 * step * cos(s_betta);
      float step_la = 2 * step * cosFi * cos(s_betta);

      Q_UNUSED(grad_fi_a1);
      Q_UNUSED(grad_la_c1);
      Q_UNUSED(grad_la_d1);

      float modA = vecModul(grad_la_a, grad_fi_a);
      float modB = vecModul(grad_la_b, grad_fi_b);
      float modC = vecModul(grad_la_c, grad_fi_c);
      float modD = vecModul(grad_la_d, grad_fi_d);

      float df_dx = ((modB - modA)*cosFi - (modC - modD)*tan(s_betta)) / ( 2 * step * cos(s_betta) * (cosFi*cosFi + tan(s_betta)*tan(s_betta)));
      float df_dy = ((modA - modB)*cosFi*tan(s_betta)  + modD - modC) / (2*step * cos(s_betta) * (cosFi*cosFi*tan(s_betta)*tan(s_betta)  + 1));



     if (field.getFi(fi_idx)*180/3.14 > 10  && field.getFi(fi_idx)*180/3.14 < 80 ) {
       if (field.getLa(la_idx)*180/3.14 > 0  && field.getLa(la_idx)*180/3.14 < 70 ) {
 
      // debug_log << "grad_fi" << grad_fi_c << grad_fi_d << "la" << grad_la_b << grad_la_a; 
      // debug_log << "grad_fi" << grad_fi_c1 << grad_fi_d1 << "la" << grad_la_b1 << grad_la_a1; 
      // debug_log << "s_betta" << s_betta << step_fi << step_la 
      //           << 2. * step * cosFi << 2. * step;     

     }
    }

     float totalDiverg = (grad_fi_c1 - grad_fi_d1) / step_fi  + (grad_la_b1 - grad_la_a1) /step_la;

// -	grad_fi_x1 / R * tan(modul->getFi(fi_idx));
      
     float tfp_val = totalDiverg * 1e15;

     float tfp_val1 = ((grad_fi_c - grad_fi_d) / (2*step)  + (grad_la_b - grad_la_a) / (2*step*cosFi)) * 1e15;
     tfp_val = tfp_val1; //todo этот вариант вроде симпатичнее других
    
     tfp_val1 = (df_dx + df_dy) * 1e15; //todo что-то пошло не так
     Q_UNUSED(tfp_val1);

     if (field.getFi(fi_idx)*180/3.14 > 10  && field.getFi(fi_idx)*180/3.14 < 80 ) { 
       if (field.getLa(la_idx)*180/3.14 > 0  && field.getLa(la_idx)*180/3.14 < 70 ) {

   //    debug_log << "totalDiverg" << totalDiverg << tfp_val << tfp_val1  
//                 << "df" << df_dx << df_dy
// 		<< field.getFi(fi_idx)*180/3.14 << field.getLa(la_idx)*180/3.14;
// //		<< s_betta << s_length << "dop" <<  grad_fi_x1 / R * tan(modul->getFi(fi_idx));
     }
     }

      tfp->setData(fi_idx, la_idx, tfp_val, true);
    }
  }
  

  delete modul;
  return tfp;  
}


std::unique_ptr<obanal::TField> ObanalTfp::calcHewsomM2(const obanal::TField& field)
{
  int sizeFi = field.kolFi();
  int sizeLa = field.kolLa();

  //debug_log << "size" << sizeFi << sizeLa;
  
  std::unique_ptr<obanal::TField> M2(field.getCopy());

  float step;
  if (! field.stepFi(&step)) {
    error_log << QObject::trUtf8("Ошибка определения шага сетки %1").arg(step);
    return M2;
  }
  step *= 6.371e6f; //на радиус земли, для перевода в длину дуги в км

  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {

      float grad_fi, grad_la;
      float grad_fi2, grad_la2;
      
      bool ok = field.gradient(fi_idx, la_idx, &grad_fi, &grad_la);
      if (!ok) {
	//debug_log << QObject::tr("err grad") << fi_idx << la_idx;
	M2->setData(fi_idx, la_idx, -9999, false);
	continue;
      }

      // float temp;
      // field.getData(fi_idx, la_idx, &temp);
      // debug_log << "value" << temp;
      
      float grad_modul = sqrt(grad_fi*grad_fi+grad_la*grad_la);
      //debug_log << "grad" << grad_fi << grad_la << grad_modul;     
      
      ok = field.gradient2(fi_idx, la_idx, &grad_fi2, &grad_la2);
      if (!ok) {
	//debug_log << QObject::tr("err grad2") << fi_idx << la_idx;
	M2->setData(fi_idx, la_idx, -9999, false);
	continue;
      }
      
      //debug_log << "grad2" << grad_fi2 << grad_la2;
      float grad_modul2 = sqrt(grad_fi2*grad_fi2+grad_la2*grad_la2);
      
      float m2_val = (grad_modul +  grad_modul2/(sqrt(2))*step)*1e5;
        
      //debug_log << "tfp" << tfp_val << field.getFi(fi_idx)*180/3.14 << field.getLa(la_idx)*180/3.14;
       
      M2->setData(fi_idx, la_idx, m2_val, true);
      
    }
  }
  
  return M2;
}


void ObanalTfp::deriveMeanAxis(float fi_x, float la_x, float fi_a, float la_a,
		               float fi_b, float la_b, float fi_c, float la_c,
		               float fi_d, float la_d, float *betta, float *length)
{
  float num = 5;

  auto convert = [](float fi, float la, float* betta, float* d) {
      *betta = atan2(fi, la);
      *d = sqrt(fi*fi + la*la);
  };

  //float betta_x = atan2(fi_x, la_x);
 // float d_x = sqrt(fi_x*fi_x + la_x*la_x);

  float betta_x, d_x, betta_a, d_a, betta_b, d_b, betta_c, d_c, betta_d, d_d;

  convert(fi_x, la_x, &betta_x, &d_x);
  convert(fi_a, la_a, &betta_a, &d_a);
  convert(fi_b, la_b, &betta_b, &d_b);
  convert(fi_c, la_c, &betta_c, &d_c);
  convert(fi_d, la_d, &betta_d, &d_d);

  float P = d_x*cos(2*betta_x) + d_a*cos(2*betta_a) + d_b*cos(2*betta_b) + d_c*cos(2*betta_c) + d_d*cos(2*betta_d);
  float Q = d_x*sin(2*betta_x) + d_a*sin(2*betta_a) + d_b*sin(2*betta_b) + d_c*sin(2*betta_c) + d_d*sin(2*betta_d);

  *betta = 0.5 * atan2(Q, P);
  *length = 1./num * sqrt(P*P + Q*Q);
}




//https://www.villasmunta.it/OpenGrADS/TFP.htm
std::unique_ptr<obanal::TField> ObanalTfp::calcZte(const obanal::TField& h850, const obanal::TField& h500,
						   const obanal::TField& T850, const obanal::TField& T700,
						   const obanal::TField& T500)
{
  int sizeFi = h850.kolFi();
  int sizeLa = h850.kolLa();
  
  //debug_log << "size" << sizeFi << sizeLa;
  
  std::unique_ptr<obanal::TField> Zte(h850.getCopy());
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {
      
      float thick = (h500.getData(fi_idx, la_idx) - h850.getData(fi_idx, la_idx))*10;
      //var(thick);
      float Tave1 = zond::G * thick  / (log(850./500.) * zond::Rc); //кельвин
      //Correction for moisture (specific humidity). Use Equivalent temperature definition
      // var(h850.getData(fi_idx, la_idx));
      // var(h500.getData(fi_idx, la_idx));
      // var(T850.getData(fi_idx, la_idx));
      // var(T700.getData(fi_idx, la_idx));
      // var(T500.getData(fi_idx, la_idx));
      // var(Tave1);
      float q850 = zond::kPars * zond::EP(T850.getData(fi_idx, la_idx)) / 850.;
      float q700 = zond::kPars * zond::EP(T700.getData(fi_idx, la_idx)) / 700.;
      float q500 = zond::kPars * zond::EP(T500.getData(fi_idx, la_idx)) / 500.;
    
      float q1 = (850-700) * (q850 + q700) / 2.;
      float q2 = (700-500) * (q700 + q500) / 2.;
      float qave = (q1+q2) / (850 - 500);

      // var(qave);
      
      float Tave = Tave1 + (2.5e+6/zond::Cp)*qave;
      //var(Tave);
      float eqth = Tave*log(850./500.)*zond::Rc/zond::G; //equivalent thickness
      //var(eqth);
      Zte->setData(fi_idx, la_idx, eqth, true);	
      //Zte->setData(fi_idx, la_idx, Tave, true);	
   }
  }

  return Zte;
}


//wet-bulb potential temperature
std::unique_ptr<obanal::TField> ObanalTfp::calcTpot(const obanal::TField& T, float level)
{
  int sizeFi = T.kolFi();
  int sizeLa = T.kolLa();
  
  //debug_log << "size" << sizeFi << sizeLa;
  
  std::unique_ptr<obanal::TField> Tpot(T.getCopy());
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {
      float temp;
      bool ok = zond::oprTPot(level, T.getData(fi_idx, la_idx), &temp);
      if (!ok) {
	Tpot->setData(fi_idx, la_idx, -9999, false);	
      } else {
	ok = Tpot->setData(fi_idx, la_idx, temp, true);
      }
      //debug_log << T.getData(fi_idx, la_idx) << Tpot->getData(fi_idx, la_idx) << temp << ok;

    }
  }



  return Tpot;
}

bool ObanalTfp::toQe(obanal::TField* t850, const obanal::TField& hum850, bool isSpecific)
{
  int sizeFi = t850->kolFi();
  int sizeLa = t850->kolLa();
  
  //debug_log << "size" << sizeFi << sizeLa;
  
  for (int fi_idx = 0; fi_idx < sizeFi; fi_idx++) {
    for (int la_idx = 0; la_idx < sizeLa; la_idx++) {
      if (!t850->getMask(fi_idx, la_idx) ||
	  !hum850.getMask(fi_idx, la_idx)) {
	t850->setData(fi_idx, la_idx, -9999, false);
      } else {
	float q = hum850.getData(fi_idx, la_idx);
	if (!isSpecific) {
	  q = relToSpecHumidity(q, 85000);
	}
	float temp = equivalentPotentialTemperature(t850->getData(fi_idx, la_idx) + 273.15, 85000, q);
	//	debug_log << t850->getMask(fi_idx, la_idx) << q850->getMask(fi_idx, la_idx)
	//	      << t850->getData(fi_idx, la_idx) << hum850.getData(fi_idx, la_idx) << temp;
	
	
	t850->setData(fi_idx, la_idx, temp-273.15, true);
	
	
      }
      //debug_log << t850->getData(fi_idx, la_idx); 
    }
  }
  
  return true;
}



bool ObanalTfp::saveField(obanal::TField* fd, const meteo::field::DataDesc& fdescr)
{
  trc;
  // debug_log << "no save";
  //return false;
  
  if ( nullptr == _db || nullptr == fd) {
    return false;
  }
  if (fd->isEmpty()) {
    debug_log << "save error : empty field";
    return false;
  }

  fd->setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  fd->setHour(fdescr.hour());
  fd->setModel(fdescr.model());
  fd->setCenter(fdescr.center());
  fd->setDate(QDateTime::fromString(QString::fromStdString(fdescr.date()), Qt::ISODate));
  if(fdescr.has_center_name()) {
    fd->setCenterName(QString::fromStdString(fdescr.center_name()));
  }
  fd->setLevel(fdescr.level());
  
  bool ok = _db->saveField(*fd, fdescr);
  if (ok) {
    info_log << QObject::tr("Сохранение поля %1 срок %2 центр %3 модель %4").
      arg(fdescr.meteodescr()).arg(fdescr.hour()).arg(fdescr.center()).arg(fdescr.model());
  }
  var(ok);
  return ok;
}

bool ObanalTfp::getField(const std::string &id, obanal::TField *fd, meteo::field::DataDesc* fdescr /*= nullptr*/)
{
  if (nullptr == fd) {
    return false;
  }
  
  meteo::field::SimpleDataRequest request;
  request.add_id(id);

  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    return false;
  }
  
  meteo::field::DataReply* reply = ctrl->remoteCall( &meteo::field::FieldService::GetFieldDataPoID, request,  30000);
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
    return false;
  }

  bool result = false;
  reply->fielddata();
  if(false ==  reply->has_fielddata()){
    error_log << QObject::tr("Нет данных") << request.DebugString();
    result = false;
  } else {
    //var(reply->fielddata().size());
    QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
    if( !fd->fromBuffer(&arr)){
      error_log << QObject::tr("Не удалось получить поле из сериализованного сообщения");
      result = false;
    } else {
      if (nullptr != fdescr) {
	fdescr->CopyFrom(reply->descr());	
      }
      result = true;
    }
  }

  delete reply;
  
  return result;
}

bool ObanalTfp::getAvailable(const QDateTime& dt, const QString& descrname, int level, int net_type, QMap<FieldsKey, std::string>* fileid)
{
  Q_UNUSED(net_type);
  if (fileid == nullptr) return false;
  
  meteo::field::DataRequest req;
  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
  req.set_date_end(dt.toString(Qt::ISODate).toStdString());
  req.add_type_level(::meteodescr::kIsobarLevel);
  req.add_hour(0*3600);
//  req.add_hour(6*3600);
//  req.add_hour(12*3600);
//  req.add_hour(18*3600);


//  req.add_center(74);//Брекнелл, сетка 1 градус была при 0ч, 0.5 при 6ч
  //  req.add_center(4); //Москва, сетка 1.25
 // req.add_center(98); //ЕЦСПП, Рединг. сетка 0.5
  req.set_net_type(net_type);
  req.set_is_df(1);

  meteo::field::DataDescResponse* resp = requestAvailable(&req, descrname, level);
  if (nullptr == resp) return false;

  // var(req.Utf8DebugString());
  // var(resp->Utf8DebugString());
  
  bool ok = false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    fileid->insert(key, one.id());
    ok = true;
    //var(resp->DebugString());
  }
  delete resp;
  
  return ok;
}


meteo::field::DataDescResponse* ObanalTfp::requestAvailable(meteo::field::DataRequest* req, const QString& descrname, int level)
{
  if (nullptr == req) return nullptr;
  
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Ошибка подключения к сервису доступа к результатам ОА");
    return nullptr;
  }

  req->clear_meteo_descr();
  req->clear_level();
  req->add_meteo_descr(TMeteoDescriptor::instance()->descriptor(descrname));
  req->add_level(level);

  meteo::field::DataDescResponse* resp = ctrl->remoteCall( &meteo::field::FieldService::GetAvailableData, *req, 300000 );
  if (nullptr == resp) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
  }

  return resp;
}


