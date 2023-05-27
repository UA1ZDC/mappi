#ifndef COMMONS_PROC_READ_CHECKPS_H
#define COMMONS_PROC_READ_CHECKPS_H

#include <qlist.h>

/* #define PSERR_RUNPROC  0 */
/* #define PSERR_NOFILE  -1 */
/* #define PSERR_BADFILE -2 */
/* #define PSERR_NOPROC  -3 */
/* #define PSERR_BADREAD -4 */
/* #define PSERR_BADPROC -5 */
/* #define PSERR_BADPARAM -6 */
/* #define PSERR_NODIR   -7 */
/* #define PSERR_BADDIR -8 */

namespace ProcRead {

  struct ProcIds {
    unsigned tgid;
    unsigned pid;
    unsigned ppid;
    unsigned threads;
  };

  struct ProcInfo {
    qint64 pid;
    qint64 ppid;
  };

  enum ProcListFlag {
    kPID          = 0x01,
    kPPID         = 0x02,
  };

  //! Получить список всех процессов.
  QList<ProcInfo> procList(int flags = kPID);
  //! Получить список дочерних процессов для процесса с идентификатором pid
  QList<ProcInfo> procChildList(qint64 pid, bool recursive = true);
}

#endif //COMMONS_PROC_READ_CHECKPS_H

