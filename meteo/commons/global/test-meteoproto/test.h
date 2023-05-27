#ifndef TEST_H
#define TEST_H

#include <QtTest/QtTest>
#include <meteo/commons/global/global.h>

class GetMeteoDataWrapTest: public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void doMeteodataTest();

private:
  QString readFile(const QString& filename);
  bool dropCollection(const QString& kDbName, const QString& collectionName);
  int countCollection(const QString& kDbName, const QString& collection);
};

#endif
