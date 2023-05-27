#ifndef METEO_COMMONS_UI_TLGLOG_CHECKPANEL_H
#define METEO_COMMONS_UI_TLGLOG_CHECKPANEL_H

#include <QtWidgets>

class CheckPanel : public QWidget
{
  Q_OBJECT
public:
  explicit CheckPanel(QWidget *parent = 0);
  void addCheckBox(QCheckBox* check);

private:
  QToolButton* btn_;
  QHBoxLayout* hbl_;

protected:
  void closeEvent(QCloseEvent *);

signals:
  void panelClosed();

public slots:

};

#endif // CHECKPANEL_H
