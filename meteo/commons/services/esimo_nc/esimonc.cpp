#include "esimonc.h"
#include "convert_nc.h"

#include <cross-commons/debug/tlog.h>

#include <QDir>

using namespace meteo;

//dirName - папка, в которой лежат папки с ответами (названия, которых соответсвуют esimoId)
//remove  - если true - удалить после обработки
EsimoNc::EsimoNc(const QString& dirName, bool remove):
  _dirName(dirName),
  _rmdir(remove)
{
  _nc = new ConvertNc;

  _timer.setInterval(30 * 1000);
  QObject::connect(&_timer, &QTimer::timeout,
                   this, &EsimoNc::checkDir);
}


EsimoNc::~EsimoNc()
{
  delete _nc;
  _nc = nullptr;
}

void EsimoNc::start()
{
  checkDir();
  _timer.start();
}

bool EsimoNc::checkDir()
{
  if (_nc == nullptr) return false;
  
  info_log << tr("Сканирование директории '%1'").arg(_dirName);
  
  QDir dir(_dirName);
  dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
  QStringList dirlist = dir.entryList();
  //var(dirlist);

  for (auto esimoid : dirlist) {
    meteo::surf::DataType datatype = meteo::surf::kUnknownDataType;
    meteo::EsimoType esimonc = meteo::kEsMeteo;
    NetType net_type = NO_TYPE;
    bool ok = _nc->esimoid2datatype(esimoid, &datatype, &esimonc, &net_type);
    if (!ok) {
      error_log << QObject::tr("Ошибка преобразования id в тип данных");
      continue;
    }

    debug_log << esimoid << datatype << esimonc;
   
    QString path = dir.absolutePath() + "/" +  esimoid;
    QDir diresimo(path);
    
    ok = _nc->assimilateDir(diresimo, datatype, esimonc, net_type);

    if (_rmdir) {
      debug_log << "rm" << path;
      
      ok = diresimo.removeRecursively();
      if (!ok) {
	error_log << QObject::tr("Ошибка удаления директории '%1'").arg(path);
      }
    }
  }

  return true;
}
