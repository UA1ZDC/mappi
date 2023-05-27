#ifndef MAPPI_PRETREATMENT_SAVEOBJ_SAVENOTIFY_H
#define MAPPI_PRETREATMENT_SAVEOBJ_SAVENOTIFY_H

#include <mappi/proto/satellite.pb.h>
#include <mappi/global/streamheader.h>

//NOTE если усложнится, то можно разбить на классы, которые отвечают отдельно за ПО, за ТО, а
//потом собирать из них


namespace mappi {

  class SaveNotify;
  class ServiceSaveNotify;
  
  class CreateNotify {
  public:
    enum Type {
      EmptyNotify   = 0, //! не сохранять в БД ПО, не отправлять в ТО (app)
      ServiceNotify = 1, //! Принять параметры сессии, сохранить в БД ПО, отправить в сервис ТО (mappi service)
      StubServiceNotify = 2,  //! Cохранить в БД сессии, в БД ПО, отправить в сервис ТО (fileapp)
      FledgetNotify = 3, //сохранить в БД сессии, в БД ПО, в БД ТО (в момент сохр. картинок), не отправлять в сервис ТО (pak service)
      TypeMax = 3
    };

    static bool isValidType(int type) { return type >= 0 && type <= TypeMax; }
    static SaveNotify* createEmptyNotify();
    static ServiceSaveNotify* createServiceNotify(Type nt);
  };

  //-------
  
  //! не сохранять в БД ПО, не отправлять в ТО (app)
  class SaveNotify {
  public:
    SaveNotify() {}
    virtual ~SaveNotify() {}

    virtual bool rawNotify(const meteo::global::StreamHeader&, const QString&) { return true; }
    virtual bool dataNotify(const meteo::global::PoHeader&, const QString&)    { return true; }
    virtual bool imageNotify(const meteo::global::PoHeader&, const QString&, const QString&, const QString&)   { return true; }
    virtual bool finish() { return true; }
  };
  
  //-------
  
  //! Принять параметры сессии и название файла, сохранить в БД ПО, отправить в сервис ТО (mappi service)
  class ServiceSaveNotify : public SaveNotify {
  public:
    ServiceSaveNotify();
    virtual ~ServiceSaveNotify();

    void setSource(ulong sessionId) { _sessionId = sessionId; }
    
    virtual bool dataNotify(const meteo::global::PoHeader& header, const QString& file);
    virtual bool finish();

  protected:
    void clear();
    
    ulong sessionId() { return _sessionId; }
    QString satname() { return _satname; }
    
  private:
    
    ulong _sessionId = 0;
    QString _satname;
    QDateTime _dtStart;
    QList<mappi::conf::InstrumentType> _instrs;
    
  };

  //-------
    
  //! Cохранить в БД сессии, в БД ПО, отправить в сервис ТО (fileapp)
  class StubServiceSaveNotify : public ServiceSaveNotify {
  public:
    StubServiceSaveNotify() {}
    virtual ~StubServiceSaveNotify() {}

    virtual bool rawNotify(const meteo::global::StreamHeader& header, const QString& file);
  };
  
  //-------
  
  //сохранить в БД сессии, в БД ПО, в БД ТО (в момент сохр. картинок), не отправлять в сервис ТО (pak service)
  class FledgetSaveNotify : public StubServiceSaveNotify {
  public:
    FledgetSaveNotify() {}
    virtual ~FledgetSaveNotify() {}

    virtual bool imageNotify(const meteo::global::PoHeader& header, const QString&, const QString&, const QString& file);
    virtual bool finish() { return SaveNotify::finish(); }
    
  };

   
}

#endif
