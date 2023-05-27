#ifndef METEO_COMMONS_UI_CUSTOM_SYMBDLG_H
#define METEO_COMMONS_UI_CUSTOM_SYMBDLG_H

#include <qdialog.h>
#include <qlist.h>

class QToolButton;

class SymbDlg : public QDialog
{
  Q_OBJECT
public:
  explicit SymbDlg(QWidget* parent = 0);

  QChar currentSymb() const { return symb_; }

public slots:
  void slotSetFont(const QFont& font);

private slots:
  void slotChangeSymb();

private:
  QList<QToolButton*> buttons_;

  QChar symb_;
};

#endif
