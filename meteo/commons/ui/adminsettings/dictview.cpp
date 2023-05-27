#include "dictview.h"
#include "ui_dictview.h"


namespace meteo {

namespace ui {

// class DictView
DictView::DictView(const internal::TUserSettings::dict_t& dict,
  QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QDialog(parent, f),
  ui_(new Ui::DictView()),
  clickCount_(0),
  sequence_(1),
  model_(new DictModel(dict, this))
{
  ui_->setupUi(this);

  ui_->tView->setAlternatingRowColors(true);
  ui_->tView->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->tView->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui_->tView->setModel(model_);
  ui_->tView->horizontalHeader()->setSectionResizeMode(DictModel::NAME, QHeaderView::Stretch);
  ui_->tView->hideColumn(DictModel::ID);

  ui_->leName->setVisible(false);
  ui_->leName->setPlaceholderText(QObject::tr("Введите новое наименование"));

  QObject::connect(ui_->pbUpdate, SIGNAL(clicked()), this, SLOT(updateDict()));
  QObject::connect(ui_->pbAppend, SIGNAL(clicked()), this, SLOT(appendDict()));
  QObject::connect(ui_->pbCancel, SIGNAL(clicked()), this, SLOT(cancel()));

  initSequence(dict);
  if (0 < model_->rowCount())
    setCurrentIndex(0);
  else
    ui_->pbUpdate->setEnabled(false);
}

DictView::~DictView()
{
  delete ui_;
  ui_ = nullptr;
}

void DictView::initSequence(const internal::TUserSettings::dict_t& dict)
{
  if (dict.isEmpty())
    return ;

  QList<int> keys = dict.keys();
  qSort(keys.begin(), keys.end());
  sequence_ = keys.last() + 1;
}

void DictView::setCurrentIndex(int row)
{
  if (0 <= row && row < model_->rowCount())
    ui_->tView->setCurrentIndex(model_->index(row, 0));
}

QModelIndex DictView::currentIndex(int column /*=*/)
{
  QModelIndex index = ui_->tView->currentIndex();
  return model_->index(index.row(), column);
}

void DictView::cancel()
{
  switch (clickCount_) {
    case 0 :
      reject();
      break ;

    case 1 :
      ++clickCount_;
      ui_->leName->setVisible(false);
      ui_->leName->clear();
      ui_->pbAppend->setEnabled(true);
      ui_->pbUpdate->setEnabled(true);
      ui_->tView->setEnabled(true);
      clickCount_ = 0;
  }
}

void DictView::updateDict()
{
  switch (clickCount_) {
    case 0 :
      ui_->leName->setVisible(true);
      ui_->leName->setFocus(Qt::OtherFocusReason);
      ui_->leName->setText(model_->data(currentIndex(1)).toString());
      ui_->pbAppend->setEnabled(false);
      ui_->tView->setEnabled(false);
      ++clickCount_;
      break ;

    case 1 : {
      QModelIndex index = currentIndex();

      QString name = ui_->leName->text();
      if (!name.isEmpty() && put(model_->data(index).toInt(), name)) {
        model_->update();
        setCurrentIndex(index.row());
      }

      ui_->leName->setVisible(false);
      ui_->leName->clear();
      ui_->pbAppend->setEnabled(true);
      ui_->tView->setEnabled(true);
      clickCount_ = 0;
    }
  }
}

void DictView::appendDict()
{
  switch (clickCount_) {
    case 0 :
      ui_->leName->setVisible(true);
      ui_->leName->setFocus(Qt::OtherFocusReason);
      ui_->pbUpdate->setEnabled(false);
      ui_->tView->setEnabled(false);
      ++clickCount_;
      break ;

    case 1 : {
      QString name = ui_->leName->text();
      if (!name.isEmpty() && put(sequence_, name)) {
        model_->update();
        setCurrentIndex(sequence_ - 1);
        ++sequence_;
      }

      ui_->leName->setVisible(false);
      ui_->leName->clear();
      ui_->pbUpdate->setEnabled(true);
      ui_->tView->setEnabled(true);
      clickCount_ = 0;
    }
  }
}


// class DepartmentView
DepartmentView::DepartmentView(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    DictView(TUserSettings::instance()->departments(), parent, f)
{
  setWindowTitle(QObject::tr("Перечень подразделений"));
}

DepartmentView::~DepartmentView()
{
}

bool DepartmentView::put(int id, const QString& name)
{
  return TUserSettings::instance()->putDepartment(id, name);
}


// class RankView
RankView::RankView(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    DictView(TUserSettings::instance()->ranks(), parent, f)
{
  setWindowTitle(QObject::tr("Перечень званий"));
}

RankView::~RankView()
{
}

bool RankView::put(int id, const QString& name)
{
  return TUserSettings::instance()->putRank(id, name);
}

}

}
