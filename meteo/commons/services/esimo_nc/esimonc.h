#ifndef METEO_COMMONS_SERVICES_ESIMO_NC_ESIMONC_H
#define METEO_COMMONS_SERVICES_ESIMO_NC_ESIMONC_H

#include <QObject>
#include <QTimer>

namespace meteo {

  class ConvertNc;
  
  class EsimoNc : public QObject {
  public:
    EsimoNc(const QString& dirName, bool rmDir);
    ~EsimoNc();
    
    void start();
	      
  public slots:
    bool checkDir();

  private:
    
    
  private:
    QString _dirName; //название сканируемой папки
    bool _rmdir = false;  //удалять ли ответ после обработки
    QTimer _timer;

    ConvertNc* _nc = nullptr;
  };
  
}

#endif
