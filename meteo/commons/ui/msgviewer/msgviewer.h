#ifndef MSGVIEWER_H
#define MSGVIEWER_H

#include <QtGui>
#include <meteo/commons/ui/customviewer/customviewer.h>
#include <meteo/commons/ui/customviewer/contentwidget.h>
#include  <meteo/commons/ui/msgviewer/leftpanel.h>

namespace meteo {

class MsgViewer : public CustomViewer {
Q_OBJECT
public:
  MsgViewer(QWidget* parent = nullptr);
  virtual ~MsgViewer() override;
  virtual bool init() override;

protected:
  virtual void setDefaultSectionSize() override;
  virtual void setDefaultSectionOrder() override;
  virtual void setDefaultSectionVisible() override;

protected slots:
  virtual void slotUpdate() override;

private:
  QMenu* tableMenu_;
  ContentWidget* contentWidget_;  
  QToolButton* tableBtn_;

  bool getTelegram(const QString& file, meteo::tlg::MessageNew* tlgout);

private slots:

  void slotFiltersChanged();

  void slotTableMenuClicked();
  void slotItemDoubleClicked(QTableWidgetItem* item);

  virtual void slotPanelClosed() override;
  virtual void slotChangeTable(QAction*);
  void slotOpen();
};

}

#endif
