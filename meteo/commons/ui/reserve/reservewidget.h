#ifndef RESERVEWIDGET_H
#define RESERVEWIDGET_H
#include <qthread.h>
#include <qdialog.h>
#include <qsyntaxhighlighter.h>

namespace Ui {
  class ReserveWidget;
}

namespace meteo{

class DataSaver;

class TextHightlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  TextHightlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
  {}

  virtual void highlightBlock(const QString& text)
  {
    if( text.contains("[E]") ){
//      setFormat(0, text.length(), Qt::red);
    }else if( text.contains("[W]") ){
      setFormat(0, text.length(), Qt::yellow);
    }else if(text.contains("[I]")){
      setFormat(0, text.length(), Qt::green);
    }
  }
};

class ReserveWidget : public QDialog
{
  Q_OBJECT
public:
  explicit ReserveWidget(QWidget *parent = 0);
  ~ReserveWidget();

private:
  Ui::ReserveWidget *ui;
  QThread* saverThread = nullptr;
  QThread* restoreThread = nullptr;
  QString saveDir_;
  QString restoreDir_;

protected:
  virtual void closeEvent(QCloseEvent *);

signals:

private slots:
  void save();
  void restore();
  void log(const QString& text);
  void slotSaveComplete();
  void slotRestoreComplete();
  void slotChangeSaveDir();
  void slotChangeRestoreDir();
  void slotClose();
};

}

#endif //RESERVEWIDGET
