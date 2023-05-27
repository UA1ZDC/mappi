#ifndef METEO_COMMONS_UI_CUSTOM_CHOOSEDIR_H
#define METEO_COMMONS_UI_CUSTOM_CHOOSEDIR_H

#include <QWidget>

namespace Ui {
 class ChooseDir;
}
class ChooseDir : public QWidget
{
  Q_OBJECT
public:
  explicit ChooseDir(QWidget *parent = 0);
  ~ChooseDir();
  const QString dir() const;
  void setDir(const QString& dir);
signals:
  void changed();
public slots:
  void slotOpenDialog();

private:
  Ui::ChooseDir* ui_;
};

#endif // CHOOSEDIR_H
