#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <qdir.h>
#include <qfile.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_errdefs.h>

#include "checkps.h"

#define PATHLEN   100

typedef unsigned int uint;

namespace ProcRead {

QList<ProcInfo> procList(int flags)
{
  QList<ProcInfo> list;

  QDir dir("/proc");
  QStringList items = dir.entryList();
  foreach ( const QString& i, items ) {
    bool ok = false;
    unsigned pid = i.toUInt(&ok);

    if ( !ok ) { continue; }

    ProcInfo info;
    if ( flags & kPID ) {
      info.pid = pid;
    }
    if ( flags & kPPID  ) {
      QFile f(QString("/proc/%1/status").arg(i));
      if ( !f.open(QFile::ReadOnly) ) {
        debug_log << QObject::tr("Не удалось открыть файл '%1'.").arg(f.fileName())
                  << f.errorString();;
      }
      else {

        QByteArray ba = f.readAll();

        QList<QByteArray> lines = ba.split('\n');
        for ( int i=0,isz=lines.size(); i<isz; ++i ) {
          if ( !lines[i].startsWith("PPid:") ) { continue; }

          lines[i] = lines[i].remove(0, 5).trimmed();
          info.ppid = lines[i].toLongLong();
          break;
        }
      }
    }
    list << info;
  }

  return list;
}

QList<ProcInfo> procChildList(qint64 pid, bool recursive)
{
  QHash<unsigned,ProcInfo> childs;

  QList<ProcInfo>  list = procList(kPID|kPPID);
  foreach ( const ProcInfo& i, list ) {
    if ( pid == i.ppid ) {
      childs.insert(i.pid,i);
    }
  }

  bool childFound = true;
  while ( recursive && childFound ) {
    childFound = false;
    foreach ( const ProcInfo& i, list ) {
      if ( !childs.contains(i.pid) && childs.contains(i.ppid) ) {
        childs.insert(i.pid,i);
        childFound = true;
      }
    }
  }

  return childs.values();
}

} // ProcRead

