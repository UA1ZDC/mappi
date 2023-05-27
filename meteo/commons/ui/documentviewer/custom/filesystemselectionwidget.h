#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_CUSTOM_FILESYSTEMSELECTIONWIDGET_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_CUSTOM_FILESYSTEMSELECTIONWIDGET_H

#include <qwidget.h>
#include <qfilesystemmodel.h>

namespace Ui {
class FileSystemSelectionWidget;
}

namespace meteo {


class FileSystemSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FileSystemSelectionWidget(QWidget *parent = nullptr);
  ~FileSystemSelectionWidget();

  void setNameFilters( const QStringList& filters );
  const QString& getSelectedFile();

signals:
  void signalFileSelected(const QString& path);

private slots:
  void slotPbFileOpenPressed();
  void slotLwItemDoubleClicked(const QModelIndex & index);

private:
  void openFile();

private:
  Ui::FileSystemSelectionWidget *ui_;
  QFileSystemModel *model_;
  QString selectedFile_;
};

}

#endif
