#ifndef METEO_COMMONS_APPMANAGER_TESTRPC_APPMANAGERTEST_H
#define METEO_COMMONS_APPMANAGER_TESTRPC_APPMANAGERTEST_H

#include <QtTest/QtTest>
#include <meteo/commons/global/global.h>

class Test : public QObject{
  Q_OBJECT
private slots:
  void initTestCase();
  void testNosqlQuery();
  void testSaveLoadDepartments();
  void testCreateUserWithDepartment();
  void testRanksLoading();
  void testRolesLoading();
  void testUserAuth();
  void cleanupTestCase();

private:
  bool dropCollection(const ConnectProp& conf, const QString& kDbName, const QString& collectionName);
  int  countCollection(const ConnectProp& conf, const QString& kDbName, const QString& collection);

  bool dropDepartments();
  bool dropUsers();
  int countDepartments();
  int countUsers();

  const QString getDepartmentName(const QString& departmentId);
  const QString getUserDepartment(const QString& login);
};

#endif
