#ifndef METEO_NOVOST_UI_SETTINGS_ARGEDIT_H
#define METEO_NOVOST_UI_SETTINGS_ARGEDIT_H

#include <QDialog>

namespace Ui {
  class ArgEdit;
} // Ui

namespace meteo {

class ArgEditDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ArgEditDialog(QWidget* parent = 0);
  ~ArgEditDialog();

  void init(const QString& argkey, const QString& argvalue);

  QString argkey() const;
  QString argvalue() const;

private:
  Ui::ArgEdit* ui_;

};


} // meteo

#endif // METEO_NOVOST_UI_SETTINGS_ARGEDIT_H
