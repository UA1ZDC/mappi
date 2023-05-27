#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_SYMBDLG_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_SYMBDLG_H

#include <qdialog.h>
#include <qlist.h>

class QToolButton;

namespace meteo {
namespace map {

//!
class SymbDlg : public QWidget
{
  Q_OBJECT
public:
  explicit SymbDlg(QWidget* parent = 0);

  QChar currentSymb() const { return symb_; }

public slots:
  void slotSetFont(const QFont& font);

private slots:
  void slotChangeSymb();

signals:
  void symbChanged();

private:
  QList<QToolButton*> buttons_;

  QChar symb_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_SYMBDLG_H
