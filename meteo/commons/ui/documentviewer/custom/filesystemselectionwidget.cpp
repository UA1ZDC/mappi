#include "filesystemselectionwidget.h"
#include "ui_filesystemselectionwidget.h"

namespace meteo {

FileSystemSelectionWidget::FileSystemSelectionWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new ::Ui::FileSystemSelectionWidget),
  model_(nullptr)
{  
  ui_->setupUi(this);
  ui_->twFileSystem->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  ui_->twFileSystem->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  model_ = new QFileSystemModel();
  if ( nullptr != model_ ){
    model_->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    model_->setNameFilterDisables(false);
    static const QString defaultPath = QDir::homePath();
    model_->setRootPath(defaultPath);
    ui_->twFileSystem->setModel(model_);
    ui_->twFileSystem->setRootIndex( model_->index(defaultPath) );
  }

  QObject::connect( this->ui_->twFileSystem, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotLwItemDoubleClicked(const QModelIndex &)) );  
}

FileSystemSelectionWidget::~FileSystemSelectionWidget()
{
  delete ui_;
}

void FileSystemSelectionWidget::setNameFilters(const QStringList& filters)
{
  if ( nullptr != this->model_ ){
    this->model_->setNameFilters(filters);
  }
}

void FileSystemSelectionWidget::slotLwItemDoubleClicked(const QModelIndex &)
{
  this->openFile();
}

void FileSystemSelectionWidget::slotPbFileOpenPressed()
{
  this->openFile();
}

void FileSystemSelectionWidget::openFile()
{
  auto indexes = ui_->twFileSystem->selectionModel()->selectedRows();
  if ( 1 == indexes.count() ) {    
    this->selectedFile_ = model_->filePath(indexes.first());
    QFileInfo info(selectedFile_);
    if ( false == info.isDir()) {
      emit signalFileSelected(selectedFile_);
    }
  }
}

const QString& FileSystemSelectionWidget::getSelectedFile()
{
  return this->selectedFile_;
}

}
