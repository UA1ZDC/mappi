#ifndef METEO_COMMONS_UI_JOURNALPLUGIN_JOURNALLEFTPANEL_H
#define METEO_COMMONS_UI_JOURNALPLUGIN_JOURNALLEFTPANEL_H

#include <qwidget.h>

namespace Ui {
class JournalLeftPanel;
}

namespace meteo {
namespace journal {

class JournalLeftPanel : public QWidget
{
  Q_OBJECT

public:
  explicit JournalLeftPanel(QWidget *parent = nullptr);
  ~JournalLeftPanel();

  QStringList selectedSources();
  void unsetAllCheckboxes();
  void setList(const QStringList& list);
signals:
  void signalFilterChanged();

private slots:
  void slotColumnChecked();

private:
  Ui::JournalLeftPanel *ui;
};


}
}

#endif
