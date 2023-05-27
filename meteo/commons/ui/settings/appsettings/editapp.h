#ifndef METEO_NOVOST_UI_SETTINGS_EDITAPP_H
#define METEO_NOVOST_UI_SETTINGS_EDITAPP_H

#include <QDialog>
#include <QString>

namespace Ui {
  class EditApp;
} // Ui

namespace meteo {

class EditAppDialog : public QDialog
{
  Q_OBJECT

public:
  explicit EditAppDialog(const QStringList& spoNames, QWidget* parent = nullptr);
  ~EditAppDialog();

  void init(const QString& apptitle, const QString& appico);

  QString appTitle() const;
  QString iconPath() const;
  QString spoName() const;

private slots:
  void slotOpenIconDialog();

private:
  void loadIcon();

private:
  Ui::EditApp* ui_;
  QString icopath_;

};


} // meteo

#endif // METEO_NOVOST_UI_SETTINGS_EDITAPP_H
