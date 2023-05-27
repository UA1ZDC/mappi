#ifndef METEO_MAPPI_CLEANER_CLEARTABLEDIRTASK_H
#define METEO_MAPPI_CLEANER_CLEARTABLEDIRTASK_H

#include <qdir.h>
#include <qstringlist.h>

#include <meteo/commons/planner/planner.h>

namespace mappi {
namespace cleaner {

class ClearTableDirTask : public meteo::PlannerTask
{
  Q_OBJECT
public:
  ClearTableDirTask(QObject* parent = 0) : PlannerTask(parent), mac_(-1) {}
  virtual ~ClearTableDirTask(){}

  void setConnectionName(const QString& connectionName)
  { connectionName_ = connectionName; }

  void addQuery(const QString& query)
  { _queries.append(query); }

  void setDtFormat(const QString& dtFormat)
  { _dtFormat = dtFormat; }

  void addPath(const QString& path)
  { paths_.append(QDir::cleanPath(path)); }

  void addFileTemplate(const QString& fileTemplate)
  { fileTemplateList_ << fileTemplate; }

  void setMacLevel(int m)
  { mac_ = m; }

protected:
  virtual void run();

private:
  bool rmPath(const QString& path) const;
  void rmEmptyDir(const QString& path) const;
  QStringList dirList(const QStringList& paths, int maxDepth = -1) const;
  QStringList pathListByFilter(const QStringList& patterns);

  void levelUp();
  void levelDown();

private:
  QString connectionName_;
  QStringList _queries;
  QString     _dtFormat;
  QStringList paths_;
  QStringList fileTemplateList_;
  int mac_;
};

} // cleaner
} // mappi
#endif // METEO_MAPPI_CLEANER_CLEARTABLEDIRTASK_H
