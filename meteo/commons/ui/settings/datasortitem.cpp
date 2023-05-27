#include "datasortitem.h"

static bool compareAsInt(QVariant data1, QVariant data2, bool* result)
{
  bool ok1 = false, ok2 = false;
  int value1 = data1.toInt(&ok1),
      value2 = data2.toInt(&ok2);
  if ( false == ok1 || false ==ok2 ){
    return false;
  }

  *result = value1 < value2;
  return true;
}

static bool compareAsDouble(QVariant data1, QVariant data2, bool* result)
{
  bool ok1 = false, ok2 = false;
  double value1 = data1.toDouble(&ok1),
      value2 = data2.toDouble(&ok2);
  if ( false == ok1 || false ==ok2 ){
    return false;
  }

  *result = value1 < value2;
  return true;
}

static bool compareAsString(QVariant data1, QVariant data2)
{
  auto value1 = data1.toString(),
      value2 = data2.toString();
  return value1 < value2;
}



bool DataSortItem::operator<(const QTreeWidgetItem &other) const {
  int column = treeWidget()->sortColumn();
  auto data = this->data(column, Qt::UserRole);
  auto otherData = other.data(column, Qt::UserRole);
  bool result = false;
  if ( true == compareAsDouble(data, otherData, &result)){
    return result;
  }
  if ( true == compareAsInt(data, otherData, &result ) ){
    return result;
  }

  return compareAsString(data, otherData);
}
