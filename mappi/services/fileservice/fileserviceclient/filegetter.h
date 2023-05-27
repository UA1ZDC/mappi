#ifndef MAPPI_SERVICE_FILESERVICE_FILEGETTER_H
#define MAPPI_SERVICE_FILESERVICE_FILEGETTER_H

#include <qobject.h>
#include <qwaitcondition.h>

#include "fileserviceclient.h"

namespace mappi {

/*!
 * \brief The FileGetter class - Получение файла
  Использование:

      FileGetter* fileGetter  = new FileGetter();
      fileGetter->setFilename(fileIt.value());

      QThread *thread = new QThread();
      fileGetter->moveToThread(thread);
      connect(thread,     &QThread::started,      fileGetter, &FileGetter::run);
      connect(fileGetter, &QThread::complete,     this,       &This::getFile);
      connect(fileGetter, &QThread::finished,     fileGetter, &FileGetter::deleteLater);
      connect(fileGetter, &FileGetter::destroyed, thread,     &QThread::quit);
      connect(thread,     &QThread::finished,     thread,     &QThread::deleteLater);
      thread->start();

  При получении файла испускается сигнал complite(QString filename); и finished();
  Состояние объекта можно проверить функцией bool isWorking();
*/

class FileGetter : public QObject
{
  Q_OBJECT

public:
  FileGetter(QObject* parent = nullptr);
  ~FileGetter();

  void setFilename(const QString& filename);
  QString getFilename();

public slots:
  void run();
  bool isWorking();

signals:
  void complete(const QString& filename);
  void finished();

private:
  void gettedFile(const FileServiceResponse& response);

private:
  bool            working_   = false;   //!< Работает ли поток
  QString         gettingFilename_;     //!< Имя получаемого файла
  QString         gettedFilename_;      //!< Имя полученного файла

};

} // mappi

#endif // MAPPI_SERVICE_FILESERVICE_FILEGETTER_H
