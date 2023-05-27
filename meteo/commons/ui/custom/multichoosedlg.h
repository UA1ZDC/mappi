#ifndef METEO_COMMONS_UI_CUSTOM_MULTICHOOSEDLG_H
#define METEO_COMMONS_UI_CUSTOM_MULTICHOOSEDLG_H

#include <qdialog.h>
#include <qlist.h>
#include <qpair.h>
#include <qicon.h>

class QToolButton;
class QGridLayout;

const int kIconMinWidth = 32;
const int kIconMinHeight = 32;
class MultiChooseDlg: public QDialog
{
  Q_OBJECT
  public:
    explicit MultiChooseDlg(QWidget* parent = 0);
    ~MultiChooseDlg();
    void setIcons(const QList<QPair<QString, QString> > &icons );
    void setCols(int cols);
    void setIndexes(const QList<int>& list);
    QList<int> indexes() const { return indexes_; }
    QString currentIcon() const ;

  private slots:
    void slotChooseIcon();
    void slotSelectAll(bool select);
    void slotSelectRow(bool select);
    void slotSelectCol(bool select);
  private:
    QList<int> indexes_;
    QList< QPair< QString, QString> > icons_;
    QList<QToolButton*> buttons_;
    QList<QToolButton*> hbtns_;
    QList<QToolButton*> vbtns_;
    QGridLayout* layout_;
    int cols_;
    QToolButton* allbtn_;
};

#endif
