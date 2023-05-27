#ifndef CheckPanel_H
#define CheckPanel_H

#include <QtWidgets>

class CheckPanel : public QWidget
{
  Q_OBJECT
public:
  explicit CheckPanel(QWidget *parent = nullptr);
  void addCheckBox(QCheckBox* check);
  void clearPanel();
  QList<QCheckBox*> getCheckboxList();
  bool hasAnyCheckBoxs();

private:
  QToolButton* btn_;
  QHBoxLayout* hbl_;  

protected:
  void closeEvent(QCloseEvent *);

signals:
  void panelClosed();

public slots:

};

#endif // CheckPanel_H
