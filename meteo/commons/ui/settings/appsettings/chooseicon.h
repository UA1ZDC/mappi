#ifndef METEO_NOVOST_UI_SETTINGS_CHOOSEICON_H
#define METEO_NOVOST_UI_SETTINGS_CHOOSEICON_H

#include <QDialog>
#include <QHash>
#include <QString>

class QLabel;
class QStringList;

namespace Ui {
  class ChooseIcon;
} // Ui

namespace meteo {

class ChooseIconDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ChooseIconDialog(QWidget* parent = 0);
  ~ChooseIconDialog();

  void init(const QString& icopath);

  QString iconPath() const;

  static QString defaultIcon();

private slots:
  void slotOpenFileDialog();
  void slotLoadIcon();
  void slotSelectPage();

private:
  void previewCurrent();
  void previewDefaultIcons();
  void loadDefaultIcons();

  bool eventFilter(QObject* obj, QEvent* ev);

private:
  Ui::ChooseIcon* ui_;
  QString currentIco_;

  QHash<QLabel*, QString> defaultIcons_;

};


} // meteo

#endif // METEO_NOVOST_UI_SETTINGS_CHOOSEICON_H
