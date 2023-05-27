#include "tcontrolmethods.h"
#include <commons/meteo_data/meteo_data.h>

#include <cross-commons/debug/tlog.h>

#include <qfile.h>
#include <qdir.h>
#include <qdom.h>

TControlMethodBase::TControlMethodBase(const QString& dirName):_isReadSettings(false),_dirName(dirName)
{
}

TControlMethodBase::~TControlMethodBase() 
{
}


bool TControlMethodBase::readSettings()
{
  _isReadSettings = true;

  return readSettings(_dirName);
}

bool TControlMethodBase::readSettings(const QString& fileName)
{
  bool ok = false;
  QFile file(fileName);
  if ( ! file.open(QIODevice::ReadOnly) ) {
    error_log<<QString("Can't open %1 for reading").arg(fileName);
    return ok;
  }

  // debug_log<<"file="<<fileName;

  QDomDocument* xml = new QDomDocument;
  if (xml->setContent(&file)) {
    ok = readSettings(xml);
  }

  file.close();    
  delete xml;
  return ok;
}

void TControlMethodBase::parseNum(const QStringList& strVal, QList<float>& val)
{
  bool ok =false;

  for (int i=0; i<strVal.count(); i++) {
    float v = strVal.at(i).toFloat(&ok);
    if (ok) {
      val.append(v);
    }
  }
}


void TControlMethodBase::parseSegments(const QStringList& vals, QList<QPair<float, float> >& segment)
{
  bool ok = false;

  for (int i=0; i<vals.count(); i++) {
    float min = vals.at(i).section(',',0,0).toFloat(&ok);
    if (!ok) continue;
    float max = vals.at(i).section(',',1,1).toFloat(&ok);
    if (!ok) continue;
    segment.append(QPair<float,float>(min, max));
  }  
}

bool TControlMethodBase::segmentMatch(const QList<QPair<float, float> >& segment, float value)
{
  for (int segm=0; segm < segment.count(); segm++) {
    if (value <= segment.at(segm).second && value >= segment.at(segm).first) {
      return true;
    }
  }

  return false;
}

//! проверка файла на соответствие правилам
bool TControlMethodBase::rulesMatch(const QPair<descr_t,QStringList>& type, TMeteoData* data)
{
  if (!data) return false;
  float num = data->getValueUp(type.first, BAD_METEO_ELEMENT_VAL);
  if (num == BAD_METEO_ELEMENT_VAL) {
    debug_log<<QObject::tr("Нет дескриптора %1").arg(type.first);
    return false;
  }

  // debug_log << type.first << type.second.join(";");
  
  if (type.second.contains(QString::number(int(num)))) {
    return true;
  }

  return false;  
}


//-----------


TControlMethod::TControlMethod()
{
}

TControlMethod::~TControlMethod()
{
  QHashIterator<QString, TControlMethodBase*> it(_methods);
  while (it.hasNext()) {
    it.next();
    //debug_log<<"key="<<it.key()<<it.value()->key();
    
    delete it.value();
  }
}
  
bool TControlMethod::control(TMeteoData* data)
{
  bool ok = true;
  
  QHashIterator<QString, TControlMethodBase*> it(_methods);
  while (it.hasNext()) {
    it.next();
    //debug_log<<"key="<<it.key()<<it.value()->key();
    
    ok &= it.value()->control(data);
  }

  return ok;
}

bool TControlMethod::control(TMeteoData* cur, TMeteoData* prev)
{
  bool ok = true;
  
  QHashIterator<QString, TControlMethodBase*> it(_methods);
  while (it.hasNext()) {
    it.next();
    //debug_log<<"key="<<it.key()<<it.value()->key();
    ok &= it.value()->control(cur, prev);
  }

  return ok;
}

bool TControlMethod::addMethod(const QString& key, TControlMethodBase* method)
{
  //debug_log<<"add method key"<<key;


  _methods.insertMulti(key, method);
  return true;
}



