#include <cross-commons/debug/tlog_debug.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>
#include <meteo/novost/global/global.h>
//#include <tdbusservice_adaptor.h>
//#include <trequest.h>
//#include <tzond_service.h>
//#include <tzond.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>


//#include <meteo/commons/services/srcdata/tzond_data_service.h>
#include <meteo/commons/zond/zond.h>
#include <oldzond/taero.h>

//#include <commons/meteo_data/meteo_data.h>

#include <QtGui/QApplication>
#include <qtextcodec.h>

#include <unistd.h>

#include <qdebug.h> //TODO

//#define SERVICE_FILE  "zond.xml"  //MnCommon::etcPath("meteo") + "/service/zond.xml"
#define APP_NAME "tzond"





int main(int argc, char** argv)
{
//  Subscription::registerMetaType();
  TAPPLICATION_NAME(APP_NAME);

  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  bool isdaemon = false;
  int opt;

  while ((opt = getopt(argc, argv, "dh")) != -1) {
    switch (opt) {
    case 'd':
      isdaemon = true;
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-d - start as daemon\n";
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (isdaemon) {
    if ( 0 != ProcControl::daemonizeSock()) {
      error_log << "Such process is already running.";
      return -1;
    }
    TLog::setMsgHandler( TLog::syslogOut );
    info_log << "Start OK";
  } else {
    if (0 != ProcControl::daemonizeSock(MnCommon::applicationName(), false)) {
      error_log << "Such process is already running.";
      return -1;
    }
  }

  QCoreApplication app(argc, argv);


//  TZondService* service = new TZondService(SERVICE_FILE);
//  ServiceAdaptor* adaptor = new ServiceAdaptor(service);
//  meteo::ServiceDBusOpt optDbus;
//  optDbus.setRemote("192.168.1.49");
//  service->setup(optDbus);

//!< поиск службы источника данных и соединенеие с ним  
  rpc::TController* ctrl_src;

  rpc::Address address;

//  if( ( true == rpc::ServiceLocation::instance()->findLocalService( meteo::global::kServiceNames[meteo::global::kSrcDataService], 500, &address ) ) 
//    || ( true == rpc::ServiceLocation::instance()->findService( meteo::global::kServiceNames[meteo::global::kSrcDataService], 500, &address ) ) )
//  {
      address = rpc::Address(/*"127.0.0.1"*/"192.168.52.130",38765);
      rpc::Channel* ch_src = rpc::ServiceLocation::instance()->serviceClient(address);
//      rpc::Channel* ch_src = rpc::ServiceLocation::instance()->serviceClient( rpc::Address("127.0.0.1",38765));
      if ( 0 != ch_src ) {
        ctrl_src = new  rpc::TController(ch_src);
      } else {
        error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
        .arg( address.host() )
        .arg( address.port() );
      }
//    } else {
//    error_log << QObject::tr("Сервис %1 не найден")
//    .arg(meteo::global::kServiceNames[meteo::global::kSrcDataService]);
//  }

//!< формирование запроса на получение данных
  //объекты для хранения данных
  aero::TAeroDataAll aerodata;
  aero::TAero aerofun; 
  
  zond::Zond zonddata;

  //исходные данные
  QDateTime dt(QDate(2003,04,07)); //2016,01,27
  dt.setTime(QTime(15,0));
//  TMeteoDataKey key(dt,22217,1000);
  //int level = 850; 
  int index = 26063; //91413; Yap Island (Micronezia)
                     //91765; Pago Pago Int, Airp (US)
                     //94299; Willis Island
                     //94403; Geraldton Airport (Australia)
                     //94203; //26063;
                     //22113  Мурманск
                     //26038  - ошибка 2016 02 13 0:0
                     //96011  Банда Ачех (Индонезия)
                     //96441  Бинтулу (Малайзия)
  const char cname[] = "";//Willis Island";//"Broome Airport";//"Sent-Petersbourg";
  
  //запрос
  meteo::surf::DataRequest src_request; 
  src_request.set_date_start(dt.toString("yyyy-MM-dd hh:mm:ss").toStdString());
  src_request.add_station(index);
  //src_request.set_level_p(level);
  src_request.clear_meteo_descr();
  src_request.set_type(meteo::surf::kAeroType);

//!< получение данных
  int timeout = 30000;
  meteo::surf::ZondValueReply* reply = ctrl_src->remoteCall( &meteo::surf::SurfaceService::GetZondDataOnStation, src_request, timeout, true );
  if ( 0 == reply ) {
    error_log << QObject::tr("Ответ от сервиса аэрологических данных не получен");
  }
  if(!reply->IsInitialized()) {
    error_log << QObject::tr("Данные от сервиса аэрологических данных не получены");
  }
//!< сохранение данных в TAeroDataAll
  aerodata.Index = reply->data(0).station();
  strcpy(aerodata.Name, cname);
  aerodata.KolDan = (reply->data(0).ur_size() <= 200)? reply->data(0).ur_size() : 200;
  aerodata.Koord[0] = static_cast<int>(reply->data(0).point().fi());
  aerodata.Koord[1] = 60*(reply->data(0).point().fi()-aerodata.Koord[0]);
  aerodata.Koord[2] = static_cast<int>(reply->data(0).point().la());
  aerodata.Koord[3] = 60*(reply->data(0).point().la()-aerodata.Koord[2]);
  qDebug() << "N=" <<  aerodata.KolDan;
  
  
   
//!< сохранение данных в Zond
  zonddata.setData(reply->data(0));
  zond::Uroven ur;
  
////getUrPoP() проверка данных
qDebug() << "getUrPoP() проверка данных"; 
  
  int j;
  
  for(int i = 0; i< aerodata.KolDan; ++i) {
   j= aerodata.KolDan - i-1; //aerodata уровни упорядочены по высоте 
   ur.reset(); 
   if(! zonddata.getUrPoP(reply->data(0).ur(i).p().value(), &ur)) continue;
//    ur = zonddata.getUrPoKey(reply->data(0).ur(i).p().value());
    aerodata.data[j].P = reply->data(0).ur(i).p().value();
    aerodata.data[j].pok_kach[0] = (reply->data(0).ur(i).p().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "P=" << reply->data(0).ur(i).p().value()
                << "Paero=" <<  aerodata.data[j].P 
                << "Pzond=" << ur.value(zond::UR_P)
		<< "Q=" << reply->data(0).ur(i).p().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[0]
		<< "Qzond=" <<ur.isGood(zond::UR_P);
		

    aerodata.data[j].H = reply->data(0).ur(i).h().value()*10.; //перевод из декаметров в метры
    aerodata.data[j].pok_kach[1] = (reply->data(0).ur(i).h().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "H=" << reply->data(0).ur(i).h().value()*10. //перевод из декаметров в метры
                << "Haero=" <<  aerodata.data[j].H 
                << "Hzond=" << ur.value(zond::UR_H)
		<< "Q=" << reply->data(0).ur(i).h().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[1]
		<< "Qzond=" <<ur.isGood(zond::UR_H);
    
    aerodata.data[j].T = reply->data(0).ur(i).t().value()*10.; //такой формат для хранения в int десятых
    aerodata.data[j].pok_kach[2] = (reply->data(0).ur(i).t().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "T=" << reply->data(0).ur(i).t().value()
                << "Taero=" <<  aerodata.data[j].T*0.1 //перевод в нормальный формат 
                << "Tzond=" << ur.value(zond::UR_T)
		<< "Q=" << reply->data(0).ur(i).t().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[2]
		<< "Qzond=" <<ur.isGood(zond::UR_T);

  
    aerodata.data[j].D = reply->data(0).ur(i).d().value()*10.; //такой формат для хранения в int десятых
    aerodata.data[j].pok_kach[3] = (reply->data(0).ur(i).d().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "D=" << reply->data(0).ur(i).d().value()
                << "Daero=" <<  aerodata.data[j].D*0.1 // перевод в нормальный формат 
                << "Dzond=" << ur.value(zond::UR_D)
		<< "Q=" << reply->data(0).ur(i).d().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[3]
		<< "Qzond=" <<ur.isGood(zond::UR_D);

    aerodata.data[j].D = reply->data(0).ur(i).d().value()*10.; //такой формат для хранения в int десятых
    aerodata.data[j].pok_kach[3] = (reply->data(0).ur(i).td().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "Td=" << reply->data(0).ur(i).td().value()
                << "Tdaero=" <<  (aerodata.data[j].T - aerodata.data[j].D)*0.1 // перевод в нормальный формат 
                << "Tdzond=" << ur.value(zond::UR_Td)
		<< "Q=" << reply->data(0).ur(i).td().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[3]
		<< "Qzond=" <<ur.isGood(zond::UR_Td);

 
//  aerodata.data[i].D = reply->data(0)->ur(i).d().value();
  
    aerodata.data[j].dd = reply->data(0).ur(i).dd().value();
    aerodata.data[j].pok_kach[4] = (reply->data(0).ur(i).dd().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "dd=" << reply->data(0).ur(i).dd().value()
                << "ddaero=" <<  aerodata.data[j].dd 
                << "ddzond=" << ur.value(zond::UR_dd)
		<< "Q=" << reply->data(0).ur(i).dd().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[4]
		<< "Qzond=" <<ur.isGood(zond::UR_dd);

  
    aerodata.data[j].ff = reply->data(0).ur(i).ff().value();
    aerodata.data[j].pok_kach[5] = (reply->data(0).ur(i).ff().quality() < control::MISTAKEN) ?  '0' : '9';
  qDebug() << i << "ff=" << reply->data(0).ur(i).ff().value()
                << "ffaero=" <<  aerodata.data[j].ff 
                << "ffzond=" << ur.value(zond::UR_ff)
		<< "Q=" << reply->data(0).ur(i).ff().quality()
		<< "Qaero=" << aerodata.data[j].pok_kach[5]
		<< "Qzond=" <<ur.isGood(zond::UR_ff);

    
//  aerodata.data[i].dd = reply->data(0)->ur(i).u().value();
//  aerodata.data[i].ff = reply->data(0)->ur(i).v().value();
    if(reply->data(0).ur(i).level_type() == 100/*zond::Gr_Standart*/){ 
      aerodata.data[j].pok_kach[6] = '0';  
    }
    if(reply->data(0).ur(i).level_type() == 1/*zond::Gr_Station*/){ 
      aerodata.data[j].pok_kach[6] = '1';  
    }
//    if(reply->data(0).ur(i).group_type() == 4 /*zond::Gr_Tempr*/){ 
//      aerodata.data[j].pok_kach[6] = '2';  
//    }
//    if(reply->data(0).ur(i).group_type() == 5 /*zond::Gr_Wind*/){ 
//      aerodata.data[j].pok_kach[6] = '3';  
//    }
    if(reply->data(0).ur(i).level_type() == 7 /*zond::Gr_Tropo*/){ 
      aerodata.data[j].pok_kach[6] = '4';  
    }
    if(reply->data(0).ur(i).level_type() == 6 /*zond::Gr_MaxWind*/){ 
      aerodata.data[j].pok_kach[6] = '5';  
    }
    
    
  }  
  aerofun.setPoTAeroDataAll(aerodata); //передача данных в обработчик
  
  
////getUrPoH()  
qDebug() << "getUrPoH() проверка данных";   

  for(double Pval = 100.; Pval <= 1000.; Pval += 10.){
    ur.reset();
    if(!zonddata.getUrPoP(Pval, &ur)){
      qDebug() << "Pval =" << Pval << "getUrPoP is false";
    } else {
    qDebug() << "Pval =" << Pval
             << "Pzond=" << ur.value(zond::UR_P)
             << "QP=" << ur.isGood(zond::UR_P)
             << "Hzond=" << ur.value(zond::UR_H)
             << "QH=" << ur.isGood(zond::UR_H)
             << "Tzond=" << ur.value(zond::UR_T)
             << "QT=" << ur.isGood(zond::UR_T)
             << "Dzond=" << ur.value(zond::UR_D)
             << "QD=" << ur.isGood(zond::UR_D)
             << "ddzond=" << ur.value(zond::UR_dd)
             << "Qdd=" << ur.isGood(zond::UR_dd)   
             << "ffzond=" << ur.value(zond::UR_ff)
             << "Qff=" << ur.isGood(zond::UR_ff);
    }
  }

  double gam_t, gam_dd, gam_ff, Ri, t_sr, dudz, dvdz;
  double prev_t = 0, prev_dd = 0., prev_ff = 0., prev_u = 0., prev_v = 0.;
  
  for(float Hval = /*ur.value(zond::UR_H)*/100.; Hval <= /*ur.value(zond::UR_H)*/17000.; Hval += 100.){
    if(!zonddata.getUrPoH(Hval , &ur)){
      qDebug() << "Hval =" << Hval << "getUrPoH is false";
    } else {
              
             gam_t =   -(ur.value(zond::UR_T) - prev_t)*10.; //градиент на км
             gam_dd = (ur.value(zond::UR_dd) - prev_dd)*10.; //градиент на км
             gam_ff = (ur.value(zond::UR_ff) - prev_ff)*10.; //градиент на км
                          
             t_sr = 0.5*(ur.value(zond::UR_T) + prev_t)+273.15;
             dudz = (ur.value(zond::UR_u) - prev_u)/100.;
             dvdz = (ur.value(zond::UR_v) - prev_v)/100.;
             Ri = (dudz*dudz+dvdz*dvdz == 0. ) ? 9.8*(0.0098 - gam_t/1000.)/1.e-6:
                                                 9.8*(0.0098 - gam_t/1000.)/t_sr/(dudz*dudz+dvdz*dvdz);
             
    qDebug() << "Hval =" << Hval
             << "Pzond=" << ur.value(zond::UR_P)
             << "QP=" << ur.isGood(zond::UR_P)
             << "Hzond=" << ur.value(zond::UR_H)
             << "QH=" << ur.isGood(zond::UR_H)
             << "Tzond=" << ur.value(zond::UR_T)
             << "QT=" << ur.isGood(zond::UR_T)
             << "Dzond=" << ur.value(zond::UR_D)
             << "QD=" << ur.isGood(zond::UR_D)
             << "ddzond=" << ur.value(zond::UR_dd)
             << "Qdd=" << ur.isGood(zond::UR_dd)   
             << "ffzond=" << ur.value(zond::UR_ff)
             << "Qff=" << ur.isGood(zond::UR_ff)
             << "u_zond=" << ur.value(zond::UR_u)
             << "v_zond=" << ur.value(zond::UR_v)
             << "gradT_zond=" << gam_t
             << "graddd_zond=" << gam_dd
             << "gradff_zond=" << gam_ff
             << "Ri_zond=" << Ri;

        prev_t = ur.value(zond::UR_T);
        prev_dd = ur.value(zond::UR_dd);
        prev_ff = ur.value(zond::UR_ff);
        prev_u = ur.value(zond::UR_u);
        prev_v = ur.value(zond::UR_v);

    }
  }

 
////Tsost() проверка данных   
qDebug() << "Tsost() проверка данных"; 
int i = 0;
ur.reset();
for(i = 0; i < aerodata.KolDan; ++i){
 double Tsost1, Tsost2;
 double Psost = aerodata.data[i].P;
 zonddata.getUrPoP(Psost, &ur);
 if(!zonddata.oprTsost(Psost, Tsost1)){
   Tsost1 = -9999;
 }
 if(!aerofun.oprTsost(Psost, Tsost2)){
   Tsost2 = -9999.;
 }
 qDebug() << "i=" << i 
          << "P=" << Psost << "ZondTsost="  << Tsost1
                           << "AeroTsost="  << Tsost2
                           << "deltaTzond=" << Tsost1 - ur.value(zond::UR_T)
                           << "deltaTaero=" << Tsost2 - aerodata.data[i].T*0.1;

}
////oprGranKNS() проверка данных 
qDebug() << "oprGranKNS() проверка данных"; 

QList<double> p_kns;
 bool a = zonddata.oprGranKNS(p_kns);
 int count;
 double *mass_kns;
 bool b = aerofun.oprGranKNS(count, mass_kns);

 qDebug() << "TZond KNScount=" << p_kns.size();
 qDebug() << "TAero KNScount=" << count;
 
for(i = 0; i < p_kns.size(); ++i){
  if(a) {
    qDebug() << "i=" << i << "TZond KNS" << p_kns[i];
  } else {
    qDebug() << "No zond KNS";  
  }
}
for(i = 0; i < count; ++i){
  if(b){
    qDebug() << "i=" << i << "TAero KNS" << mass_kns[i];
  } else {
    delete mass_kns;
    qDebug() << "No aero KNS";
  }
}

////oprKNS()
qDebug() << "oprKNS() проверка данных"; 

 double p; 
 double Tmax;
 if(zonddata.getTz(Tmax)){
  Tmax += 10.;
  if(zonddata.oprKNS(p, Tmax)){
    qDebug() << "Tmax=" << Tmax << "Pkns=" << p;
  }
 }
 
////oprConvecLevel()
qDebug() << "oprConvecLevel() проверка данных"; 

 if(zonddata.getTz(Tmax)){
  Tmax += 5.;
  if(zonddata.averConvecLevel(p, Tmax)){
    qDebug() << "Tmax=" << Tmax << "Pconvec=" << p;
  }
 }
 
 
//oprPPoParam()
qDebug() << "oprPPoParam() проверка данных"; 

QList<double> p_levels;

if(zonddata.oprPPoParam(zond::UR_T, -10., p_levels)){
  qDebug() << "PZond Pcount=" << p_levels.size();
  qDebug() << "PZond P[0]=" << p_levels;

}

double **mass_levels = new double*[1];

if(aerofun.oprPPoParam('T', -10., mass_levels, count )){;  
  qDebug() << "PAero Pcount=" << count;
  qDebug() << "PAero P[0]=" << (*mass_levels)[0];
}
delete []mass_levels;

//oprHPoParam()
qDebug() << "oprHPoParam() проверка данных"; 

p_levels.clear();

if(zonddata.oprHPoParam(zond::UR_T, -10., p_levels)){
  qDebug() << "HZond Hcount=" << p_levels.size();
  qDebug() << "HZond H[0]=" << p_levels;

}

//oprPrizInv()
qDebug() << "oprPprizInv() проверка данных"; 
double t, td, h;


//if(zonddata.oprPprizInv(&p, &t, &td, &h )){
ur.reset();
if(zonddata.getPprizInvHi(&ur)){
  qDebug() << "PZond_inv=" << ur.value(zond::UR_P)
           << "TZond_inv=" << ur.value(zond::UR_T)
           << "TdZond_inv=" << ur.value(zond::UR_Td)
           << "HZond_inv=" << ur.value(zond::UR_H);
           
}

if(aerofun.oprPprizInv(p, t, td, h)){
  qDebug() << "PAero_inv=" << p
           << "TAero_inv=" << t
           << "TdAero_inv=" << td
           << "HAero_inv=" << h;
           
}


//oprPkondens()
qDebug() << "oprPkondens() проверка данных"; 
if(zonddata.oprPkondens(p,t)){
  qDebug() << "PZond_kond=" << p
           << "TZond_kond=" << t;
}
if(aerofun.oprPkondens(p,t)){
  qDebug() << "PAero_kond=" << p
           << "TAero_kond=" << t;
}

//oprPsostRaznAllFast
qDebug() << "oprPsostRaznAllFast() проверка данных"; 
double deltaT;
if(zonddata.oprPsostRaznAllFast(p, deltaT)){ //быстрый по основным и особым точкам;
  qDebug() << "PZond_max=" << p
           << "deltaTZond_max=" << deltaT;
}

if(aerofun.oprPsostRaznAll(p, deltaT)){ //быстрый по основным и особым точкам;
  qDebug() << "PAero_max=" << p
           << "deltaTAero_max=" << deltaT;
}

//oprHmaxVFast
qDebug() << "oprHmaxVFast() проверка данных"; 
double Vmax;
if(zonddata.oprHmaxVFast(h, Vmax)){
  qDebug() << "HZond=" << h
           << "VmaxZond=" << Vmax;
}
if(aerofun.oprHmaxV(h, Vmax)){
  qDebug() << "HAero=" << h
           << "VmaxAero=" << Vmax;
}

//oprIlinaFast   
qDebug() << "oprIlinaFast() проверка данных"; 
if(zonddata.oprIlinaFast(p, Vmax)){
  qDebug() << "PZond=" << p
           << "VmaxZond=" << Vmax;
}

if(aerofun.oprIlina(p, Vmax)){
  qDebug() << "PAero=" << p
           << "VmaxAero=" << Vmax;
}     
     
//oprH1Fast
qDebug() << "oprH1Fast() проверка данных"; 
double h1,h2;
if(zonddata.oprH1Fast(h1, h2)){
  qDebug() << "H1Zond=" << h1
           << "H2Zond=" << h2;
}

if(aerofun.oprH1(h1, h2)){
  qDebug() << "H1Aero=" << h1
           << "H2Aero=" << h2;
}

     
//oprSkorDpKNS
qDebug() << "oprSkorDpKNS() проверка данных"; 
QList<double> w, dp;
if(zonddata.oprSkorDpKNS(w, dp)){
  qDebug() << "Zond:w=" << w;
  qDebug() << "Zond:dp=" << dp;
}

double* w_mass;
double* dp_mass;

double*& wwm= w_mass;
double*& dpm = dp_mass;

if(aerofun.oprSkorDpKNS(count, wwm, dpm)){
  if(count !=0 ){
    qDebug() << "Aero:w=" << wwm[0];
    qDebug() << "Aero:dp=" << dpm[0];
  }
}

     
//oprSrKonvSkor
qDebug() << "oprSrKonvSkor() проверка данных"; 
double p1 = 1000;
double p2 = 100;
double w_konv;
if(zonddata.oprSrKonvSkor( p1, p2, w_konv)){
  qDebug() << "Zond:w_konv=" << w_konv;
}

if(aerofun.oprSrKonvSkor(w_konv,  p1, p2)){
  qDebug() << "Aero:w_konv=" << w_konv;
}

     
//oprGranOblak
qDebug() << "oprGranOblak() проверка данных"; 
QList<double> p_oblak;
 if(zonddata.oprGranOblak(&p_oblak)){  //анализ облачных слоев 
  qDebug() << "Zond:p_oblak=" << p_oblak;
 }
 
//oprGranObled
qDebug() << "oprGranObled() проверка данных"; 
QList<double> p_obled;
 if(zonddata.oprGranObled(&p_obled)){  //анализ обледенения
  qDebug() << "Zond:p_obled=" << p_obled;
 }
     
//oprGranBoltan
qDebug() << "oprGranBoltan() проверка данных"; 
QList<double> p_boltan;
 if(zonddata.oprGranBoltan(&p_boltan)){//анализ болтанки
  qDebug() << "Zond:p_boltan=" << p_boltan;
 }
 
 
//oprGranTrace
qDebug() << "oprGranTrace() проверка данных"; 
QList<double> p_trace;
 if(zonddata.oprGranTrace(&p_trace)){  //анализ конденсационных следов
  qDebug() << "Zond:p_trace=" << p_trace;
 }

//getUrz, getTropo()
qDebug() << "getUrz(), getTropo(), getUrMaxWind проверка данных"; 
zond::Uroven urZ, urTr, urMaxW;
 if(zonddata.getUrz(&urZ)){
  qDebug() << "urZ =" << urZ.value(zond::UR_P);
 }
 if(zonddata.getTropo(&urTr)){
  qDebug() << "urTropo =" << urTr.value(zond::UR_P);
 }
 if(zonddata.getUrMaxWind(&urMaxW)){
  qDebug() << "urMaxW =" << urMaxW.value(zond::UR_P);
 }

//getSloiInver()
qDebug() << "getSloiInver() проверка данных"; 
QVector<zond::InvProp> layers;
 if(zonddata.getSloiInver(&layers)){
  qDebug() << "InvCount =" << layers.count();
 }
for(i = 0; i < layers.count(); i++){
  qDebug() << "Inv[" << i << "] =" << layers.at(i).h_lo;
}
 
//oprSrParamPoPH
qDebug() << "oprSrParamPoPH() проверка данных"; 
double ph1 = 27.8114;//1010;
double ph2 = 115;//1000;
double sr_par;
 if(zonddata.oprSrParamPoPH(zond::UR_T, zond::UR_H, ph1, ph2, sr_par)){
  qDebug() << "Zond:sr_par=" << sr_par;
 }

double znach, znach1;
bool type_p = false; 
 if(aerofun.oprSrParamPoPH('T', znach, znach1, ph1, ph2, type_p)){
  qDebug() << "Aero:sr_par=" << znach;
 }

//oprIntLivnOrl
qDebug() << "oprIntLivnOrl() проверка данных"; 
double intens;
 if(zonddata.oprIntLivnOrl(intens)){
  qDebug() << "Zond:intens=" << intens;
 }
 
 if(aerofun.oprIntLivnOrl(intens)){
  qDebug() << "Aero:intens=" << intens;
 }

 
//oprZnakEnergy
qDebug() << "oprZnakEnergy() проверка данных"; 
 int energy = zonddata.oprZnakEnergy(850.);
 if(energy != 0) qDebug() << "Zond: energy" << energy; 
 energy =  aerofun.oprZnakEnergy(850.);
 if(energy != 0) qDebug() << "Aero: energy" << energy; 

//oprZnakEnergy
qDebug() << "getAverWindUV() проверка данных"; 
float ha1 = 7;
float ha2 = 1500;
float u;
float v;

 if(zonddata.getAverWindUV(ha1,  ha2,  u,  v)){;
    qDebug() << "Zond: getAverWindUV() u=" << u  << "v=" << v; 
 }
 
 

//!<проверка функций аэрологического анализа   



  int res = app.exec();
  
  if(ctrl_src) delete ctrl_src;

  return res;
}
