#ifndef __LINUX_IO__
#define __LINUX_IO__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

//Consts
#define BLKSIZE 4096
#define BUFFER_SIZE (4 * BLKSIZE)//(16 * 1024)

size_t f_read(int, char*);

/**
 * Reads a file specified by it's file descriptor and stores contents into a PRE-ALLOCATED buffer
 * Return value: total number of bytes read
 */
size_t f_read(int fd, char *buffer)
{
  //Get file descriptor by 'open' system call. -1 indicates error
  if(fd == -1)
    return fd;
  else
  {
    #ifdef __linux__
    //Advise the kernel on read for optimizations
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    
    size_t bytes_read;
    size_t total_bytes = 0;
    
    while((bytes_read = read(fd, buffer + total_bytes, BUFFER_SIZE)))
    {
      if(!bytes_read)
        break;
      total_bytes += bytes_read;
    }
    #endif
    return total_bytes;
    
  }
  
}

#endif

#include <stdlib.h>

#include <iostream>

#include <qapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qlist.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/novost/global/global.h>
#include <meteo/novost/settings/tmeteosettings.h>
#include <meteo/novost/proto/meteosettings.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/novost/dbjournal/tdbjournal.h>

#include <meteo/novost/service/obanal_service/tobanal.h>


rpc::Address address;
int main(int argc, char**argv) 
{
  TAPPLICATION_NAME("meteo");
  QApplication* app = new QApplication( argc, argv, false );
  Q_UNUSED(app);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QTime ttt; ttt.start();
  
  QByteArray ba(50*1024*1024,'f');
  QString fn = "/home/ivan/tmp/test.doc";
  struct stat st;
  
 /* QFile file(fn);
//  file.open(QIODevice::WriteOnly);
//  file.write(ba);
//  file.close();
  if (!file.open(QIODevice::ReadOnly)){
    //if (!file.open(QIODevice::ReadOnly|QIODevice::Unbuffered)){
    return false;
  }
  ba = file.readAll();
  file.close();

 debug_log << QString::fromUtf8("Время чтенияфайла : %1 мсек").arg(ttt.elapsed());
 size_t l;
  int fd;
 
  fd = open(fn.toAscii(), O_RDONLY);
  if (-1 == fd ) {
    return false;
  }
  ttt.start();
  fstat( fd, &st);
  ba.resize(st.st_size);
  // debug_log<<"size: "<<st.st_size;
  l = f_read( fd,ba.data() );
//  debug_log << "Bytes read: "<<l;
  close( fd ); 
  debug_log << QString::fromUtf8("Время чтенияфайла : %1 мсек").arg(ttt.elapsed())<<l<<st.st_blksize;
 */
  obanal::TField *fid = new obanal::TField();
  RegionParam reg_par(-90.,-180.,90.,180.);
  fid->setNet(reg_par,STEP_0500x0500);
  QFile file1( fn );
  if (!file1.open(QIODevice::WriteOnly)) {
    error_log << QObject::tr("Ошибка при открытии файла") << fn;
    return false;
 }
 ttt.start();
 fid->getBuffer(&ba);
 file1.write(ba);
 //QDataStream str(&file1);
 
// str << *fid;
 file1.close();

 
 int fd = open(fn.toAscii(), O_RDONLY);
 if (-1 == fd ) {
   return false;
 }
 ttt.start();
 fstat( fd, &st);
 ba.resize(st.st_size);
 debug_log<<"size: "<<st.st_size;
int  l = f_read( fd,ba.data() );
   debug_log << "Bytes read: "<<l;
 close( fd ); 
 
 ttt.start();
 
 obanal::TField *fd2 = new obanal::TField();
 //QByteArray arr( fresp.fielddata().data(), fresp.fielddata().size() );
 ttt.start();
 fd2->fromBuffer(&ba);
 
 
}


int main1(int argc, char**argv) 
{
  TAPPLICATION_NAME("meteo");
  QApplication* app = new QApplication( argc, argv, false );
  Q_UNUSED(app);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  if( false == meteo::settings::TMeteoSettings::instance()->load() ){
    error_log << meteo::settings::TMeteoSettings::instance()->errorString();
    return EXIT_FAILURE;
  }
  
  
if(false)
{
  if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kSrcDataService], 500, &address ) ) {
    if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kSrcDataService], 500, &address ) ) {
        error_log << QObject::tr("Сервис %1 не найден")
        .arg(kServiceNames[kSrcDataService]);
        return EXIT_FAILURE;
      }
    } 
  rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
  if ( 0 == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
    .arg( address.host() )
    .arg( address.port() );
    return EXIT_FAILURE;
  }
  rpc::TController ctrl(ch);
 
  meteo::surf::DataRequest request1;
  request1.add_meteo_descr(21001);
  request1.set_date_start("2016-01-07 00:00:00");
  request1.set_level_h(8000);
  request1.set_type(6);
  meteo::surf::ManyMrlValueReply * reply =
  ctrl.remoteCall( &meteo::surf::SurfaceService::GetMrlDataOnAllStation, request1,  30000, true);
  if ( 0 == reply ) {
    error_log << QObject::tr(" от сервиса данных не получен");
    return false;
  }
  debug_log << reply->DebugString();
  delete reply; reply = 0;
  delete ch; ch = 0;
  
  }
  

  if (true){

    if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kFieldService], 500, &address ) ) {
      if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kFieldService], 500, &address ) ) {
        error_log << QObject::tr("Сервис %1 не найден")
        .arg(kServiceNames[kFieldService]);
        return EXIT_FAILURE;
      }
    } 
    rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
    if ( 0 == ch ) {
      error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
      .arg( address.host() )
      .arg( address.port() );
      return EXIT_FAILURE;
    }
    rpc::TController ctrl(ch);
    
    
    meteo::field::DataRequest request;
  
  ::meteo::surf::Point* p =  request.add_coords();
  p->set_fi( 60. /180.*M_PI);
  p->set_la(30. /180.*M_PI);
  request.set_date_start("2016-01-19 00:00:00");
  request.set_date_end("2016-01-20 00:00:00");
  //request.set_is_df(true);
  
  meteo::field::ProfileDataReply * reply =
  ctrl.remoteCall( &meteo::field::FieldService::GetProfile, request,  30000, true);
  if ( 0 == reply ) {
    error_log << QObject::tr("При поытке выполнить анализ  данных в коде GRIB ответ от сервиса данных не получен");
    return false;
  }
  debug_log<<reply->DebugString();
  debug_log <<"Kol profile "<< reply->zonds().data_size();
  delete reply; reply = 0;
  delete ch; ch = 0;
  
  }
  
  
  if(true){
    
    if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kFieldService], 500, &address ) ) {
      if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kFieldService], 500, &address ) ) {
        error_log << QObject::tr("Сервис %1 не найден")
        .arg(kServiceNames[kFieldService]);
        return EXIT_FAILURE;
      }
    } 
    rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
    if ( 0 == ch ) {
      error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
      .arg( address.host() )
      .arg( address.port() );
      return EXIT_FAILURE;
    }
    rpc::TController ctrl(ch);
    
    
    meteo::field::DataRequest request;
    
    request.set_date_start("2016-01-14 00:00:00");
   // request.set_date_start("2015-11-01 00:00:00");
//    request.set_date_end("2016-01-15 00:00:00");
    
    //request.add_center(34);
    request.add_level(0);
    request.add_type_level(1);
    
    request.add_hour(0);
    request.add_hour(6*3600);
    request.add_hour(12*3600);
    request.add_hour(18*3600);
    request.add_hour(24*3600);
    
    request.add_meteo_descr(12101);
    request.set_need_field_descr(true);
    
    ::meteo::surf::Point* p =  request.add_coords();
    p->set_fi( 60. /180.*M_PI);
    p->set_la(30. /180.*M_PI);
    
    meteo::field::ValueDataReply * reply =
    ctrl.remoteCall( &meteo::field::FieldService::GetValues, request,  30000, true);
    if ( 0 == reply ) {
      error_log << QObject::tr("При поытке выполнить анализ  данных в коде GRIB ответ от сервиса данных не получен");
      return false;
    }
    debug_log<<reply->DebugString();
    debug_log <<"Kol values "<< reply->data_size();
    delete reply; reply = 0;
    delete ch; ch = 0;
    
  }
  
  
  
  if(true)
  {
    
    if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kFieldService], 500, &address ) ) {
      if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kFieldService], 500, &address ) ) {
        error_log << QObject::tr("Сервис %1 не найден")
        .arg(kServiceNames[kFieldService]);
        return EXIT_FAILURE;
      }
    } 
    rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
    if ( 0 == ch ) {
      error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
      .arg( address.host() )
      .arg( address.port() );
      return EXIT_FAILURE;
    }
    rpc::TController ctrl(ch);
    
    
    meteo::field::AdvectDataRequest request;
    
    request.set_date("2016-01-15 00:00:00");
    // request.set_date_start("2015-11-01 00:00:00");
    //  request.set_date_end("2015-11-11 00:00:00");
    
    request.set_adv_time(24*3600);
   request.set_center(34);
    request.set_need_field_descr(true );
    
    ::meteo::surf::Point* p =  request.mutable_coord();
    p->set_fi( 20. /180.*M_PI);
    p->set_la(20. /180.*M_PI);
    
    meteo::field::AdvectDataReply * reply =
    ctrl.remoteCall( &meteo::field::FieldService::GetAdvectPoints, request,  30000, true);
    if ( 0 == reply ) {
      error_log << QObject::tr("При поытке выполнить анализ  данных в коде GRIB ответ от сервиса данных не получен");
      return false;
    }
    debug_log<<reply->DebugString();
    debug_log <<"Kol values "<< reply->coord_size();
    delete reply; reply = 0;
    delete ch; ch = 0;
    
  }
  
if (false){
  
  if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kSrcDataService], 500, &address ) ) {
    if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kSrcDataService], 500, &address ) ) {
      error_log << QObject::tr("Сервис %1 не найден")
      .arg(kServiceNames[kFieldService]);
      return EXIT_FAILURE;
    }
  } 
  rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
  if ( 0 == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
    .arg( address.host() )
    .arg( address.port() );
    return EXIT_FAILURE;
  }
  rpc::TController ctrl(ch);
  
  
  meteo::surf::DataRequest request;
  
  request.set_date_start("2016-01-15 00:00:00");
  // request.set_date_start("2015-11-01 00:00:00");
  //  request.set_date_end("2015-11-11 00:00:00");
  
  request.set_level_p(850);
  request.add_meteo_descr(12101);
  request.set_type(meteo::surf::kAeroType);
  request.add_station(26063);
  
  meteo::surf::ManyZondValueReply * reply =
  ctrl.remoteCall( &meteo::surf::SurfaceService::GetZondDataOnLevel, request,  30000, true);
  if ( 0 == reply ) {
    error_log << QObject::tr("При поытке  ответ от сервиса данных не получен");
    return false;
  }
  debug_log<<reply->Utf8DebugString();
  delete reply; reply = 0;
  delete ch; ch = 0;
  
}

if(false){
  
  if ( false == rpc::ServiceLocation::instance()->findLocalService( kServiceNames[kSrcDataService], 500, &address ) ) {
    if ( false == rpc::ServiceLocation::instance()->findService( kServiceNames[kSrcDataService], 500, &address ) ) {
      error_log << QObject::tr("Сервис %1 не найден")
      .arg(kServiceNames[kFieldService]);
      return EXIT_FAILURE;
    }
  } 
  rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
  if ( 0 == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
    .arg( address.host() )
    .arg( address.port() );
    return EXIT_FAILURE;
  }
  rpc::TController ctrl(ch);
  
  
  meteo::surf::DataRequest request;
  
  request.set_date_start("2016-01-07 00:00:00");
  // request.set_date_start("2015-11-01 00:00:00");
  //  request.set_date_end("2015-11-11 00:00:00");
  
//  request.set_level_p(850);
  request.set_type(meteo::surf::kAeroType);
  request.add_station(26063);
  meteo::surf::ZondValueReply * reply =
  ctrl.remoteCall( &meteo::surf::SurfaceService::GetZondDataOnStation, request,  30000, true);
  if ( 0 == reply ) {
    error_log << QObject::tr("При поытке  ответ от сервиса данных не получен");
    return false;
  }
  debug_log<<reply->Utf8DebugString();
  delete reply; reply = 0;
  delete ch; ch = 0;
  
}
  
  return 0;
}
