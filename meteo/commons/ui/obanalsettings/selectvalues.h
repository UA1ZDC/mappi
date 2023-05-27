#ifndef METEO_COMMONS_UI_OBANALSETTINGS_SELECTVALUES_H
#define METEO_COMMONS_UI_OBANALSETTINGS_SELECTVALUES_H

#include <QDialog>
#include <QStringList>

class QTreeWidget;

namespace Ui {
  class SelectValuesWidget;
}

namespace meteo {

class SelectValuesWidget : public QDialog
{
  Q_OBJECT

public:
  explicit SelectValuesWidget(QWidget* parent = 0);
  virtual ~SelectValuesWidget();

  void init();
  void init(const QString& values);

  QString valuesToString() const;

protected:
  virtual QStringList loadAllValuesList() = 0;
  virtual QStringList loadValuesList(const QString& str) = 0;
  virtual QString getValues() const = 0;

  const QStringList& availableValues() const;
  const QStringList& selectedValues() const;

private slots:
  void slotSearch(const QString& str);
  void slotSelectAll();
  void slotMoveToAll();
  void slotMoveToSelected();

private:
  void installValuesList(QTreeWidget* target, const QStringList& values);
  void resetValuesList(QTreeWidget* target, const QStringList& values);

private:
  Ui::SelectValuesWidget* ui_;

  QStringList availableValues_;
  QStringList selectedValues_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_SELECTVALUES_H
