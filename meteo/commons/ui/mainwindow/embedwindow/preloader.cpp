#include "preloader.h"
#include "ui_preloader.h"

#include <qmovie.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace app {

Preloader::Preloader(QWidget *parent) :
  QWidget(parent),
  ui_( new Ui::Preloader)
{
  ui_->setupUi(this);
  //QMovie *movie = new QMovie(":/novost/icons/round_small.gif");
  //ui_->progressImage->setMovie(movie);
  //movie->start();
  ui_->textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui_->textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

Preloader::~Preloader()
{
}

void Preloader::setTextLog(const QString& text)
{
  ui_->textBrowser->append(text);
  ui_->textBrowser->moveCursor( QTextCursor::End );
}

void Preloader::setAppFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
  //ui_->progressText->setVisible(false);
  //ui_->progressImage->setVisible(false);
  ui_->textBrowser->append(QObject::tr("Приложение завершено код завершения %1 статус %2")
                        .arg(exitCode).arg( QProcess::NormalExit == exitStatus ? "Ok" : "Crash" ));
  ui_->textBrowser->moveCursor( QTextCursor::End );
}

void Preloader::resizeEvent(QResizeEvent* e)
{
  ui_->textBrowser->moveCursor( QTextCursor::End );
  QWidget::resizeEvent(e);
}

}
}
