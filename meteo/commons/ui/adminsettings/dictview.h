#pragma once

#include "dictmodel.h"
#include <QDialog>


namespace Ui {
  class DictView;
}

namespace meteo {

namespace ui {

class DictView :
  public QDialog
{
  Q_OBJECT

public :
  explicit DictView(const internal::TUserSettings::dict_t& dict,
    QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  virtual ~DictView();

protected :
  virtual bool put(int id, const QString& name) = 0;

private :
  void initSequence(const internal::TUserSettings::dict_t& dict);

  void setCurrentIndex(int row);
  QModelIndex currentIndex(int column = 0);

private slots :
  void cancel();
  void updateDict();
  void appendDict();

private :
  Ui::DictView* ui_;
  int clickCount_;

  int sequence_;
  DictModel* model_;
};


class DepartmentView :
  public DictView
{
public :
  explicit DepartmentView(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~DepartmentView();

protected :
  virtual bool put(int id, const QString& name);
};


class RankView :
  public DictView
{
public :
  explicit RankView(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~RankView();

protected :
  virtual bool put(int id, const QString& name);
};

}

}
