#ifndef METEO_COMMONS_UI_CUSTOM_CHOOSEFILE_H
#define METEO_COMMONS_UI_CUSTOM_CHOOSEFILE_H

#include <QWidget>

namespace Ui {
 class ChooseFile;
}
class ChooseFile : public QWidget
{
  Q_OBJECT
public:
  explicit ChooseFile(QWidget *parent = 0);
  ~ChooseFile();
  const QString file() const;
  void setFile(const QString& file);
signals:
  void changed();
public slots:
  void slotOpenDialog();

private:
  Ui::ChooseFile* ui_;
};

#endif // ChOOSEFILE_H
