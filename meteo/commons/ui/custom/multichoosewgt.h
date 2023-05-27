#ifndef METEO_COMMONS_UI_CUSTOM_MULTICHOOSEWGT_H
#define METEO_COMMONS_UI_CUSTOM_MULTICHOOSEWGT_H

#include <qwidget.h>
#include <qlist.h>
#include <qpair.h>
#include <qicon.h>

class QToolButton;
class QGridLayout;

class MultiChooseWgt: public QWidget
{
  Q_OBJECT
  public:
    explicit MultiChooseWgt(QWidget* parent = 0);
    ~MultiChooseWgt();
    void setBtns();
    void setCols(int cols);
    void setIndexes(const QList<int>& list);
    QList<int> indexes() const { return indexes_; }
    QString currentIcon() const ;

    MultiChooseWgt* addIco( const QPair< QString, QString >& pair );
    QStringList indexesToStringList() const;
    void init();
  signals:
    void changed();
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
