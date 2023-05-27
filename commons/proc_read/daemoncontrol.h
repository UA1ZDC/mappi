#ifndef COMMONS_PROC_READ_DAEMONCONTROL_H
#define COMMONS_PROC_READ_DAEMONCONTROL_H

#include <cross-commons/app/paths.h>

class QCoreApplication;

namespace ProcControl {

//! \note В windows для создания демона (сервиса) необходимо использовать утилиту sc. Данная функция
//! работает также как и \fn isAlreadyRun(), праметр \a daemon игнорируется.
int daemonizeSock(const QString& name = MnCommon::applicationName(), bool daemon = true );

//! \note В windows тоже что \fn isAlreadyRun().
int singleInstanceSock(const QString&);
int checkSockInUse( const QString& sokname );

//! \note Данную функцию необходимо вызывать после создания QApplication, т.к. используется объект QSharedMemory,
//! который должен быть удалён перед завершением приложения, чтобы освободить память.
bool isAlreadyRun(const QString& appId);

}

#endif//COMMONS_PROC_READ_DAEMONCONTROL_H

