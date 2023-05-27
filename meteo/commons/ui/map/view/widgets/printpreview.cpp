#include "printpreview.h"
#include "printpreview_ui.h"

#include <cross-commons/debug/tlog.h>

#include <qlineedit.h>
#include <QKeyEvent>

using namespace meteo;
using namespace map;

PrintPreview::PrintPreview(QPrinter* printer, QWidget *parent, Qt::WindowFlags flags):
  QMainWindow(parent, flags),
  ui_(0)
{
  // setTitle(QObject::tr("Предварительный просмотр"));
  // setPixmap(QPixmap(":/meteo/icons/map/print.png"));

  ui_ = new Ui::PrintPreview;
  ui_->setupUi(this, printer);


  static const short factors[] = { 25, 50, 75, 100, 150, 200, 250 };
  for (int i = 0; i < int(sizeof(factors) / sizeof(factors[0])); ++i)
    ui_->zoomFactor->addItem(QString("%1%").arg(factors[i]));
  
  setupActions();

  connect(ui_->preview, SIGNAL(paintRequested(QPrinter*)), SIGNAL(paintRequested(QPrinter*)));
  connect(ui_->preview, SIGNAL(previewChanged()), SLOT(previewChanged()));
  connect(ui_->actFirst, SIGNAL(triggered(bool)), SLOT(firstPage()));
  connect(ui_->actLast, SIGNAL(triggered(bool)), SLOT(lastPage()));
  connect(ui_->actNext, SIGNAL(triggered(bool)), SLOT(nextPage()));
  connect(ui_->actPrev, SIGNAL(triggered(bool)), SLOT(prevPage()));
  connect(ui_->actZoomIn, SIGNAL(triggered(bool)), SLOT(zoomIn()));
  connect(ui_->actZoomOut, SIGNAL(triggered(bool)), SLOT(zoomOut()));
  connect(ui_->actSinglePage, SIGNAL(triggered(bool)), SLOT(setSinglePage()));
  connect(ui_->actAllPages, SIGNAL(triggered(bool)), SLOT(setAllPages()));
  connect(ui_->actRedraw, SIGNAL(triggered(bool)), ui_->preview, SLOT(updatePreview()));
  connect(ui_->zoomFactor->lineEdit(), SIGNAL(editingFinished()), SLOT(zoomFactorChanged()));
  connect(ui_->zoomFactor, SIGNAL(currentIndexChanged(int)), SLOT(zoomFactorChanged()));
  connect(ui_->curPageNum, SIGNAL(editingFinished()), SLOT(pageNumChanged()));

  ui_->preview->setFocus();

  ui_->centralwidget->addAction(ui_->actFirst);
  ui_->centralwidget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

PrintPreview::~PrintPreview()
{
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
}

void PrintPreview::setupActionIcon(QAction *action, const QLatin1String &name)
{
    QLatin1String imagePrefix(":/meteo/icons/tools/print/");
    QIcon icon;
    // icon.addFile(imagePrefix + name + QLatin1String("-24.png"), QSize(24, 24));
    icon.addFile(imagePrefix + name + QLatin1String(".png"), QSize(32, 32));
    action->setIcon(icon);
}

void PrintPreview::setupActions()
{
  setupActionIcon(ui_->actNext, QLatin1String("go-next"));
  setupActionIcon(ui_->actPrev, QLatin1String("go-previous"));
  setupActionIcon(ui_->actFirst, QLatin1String("go-first"));
  setupActionIcon(ui_->actLast, QLatin1String("go-last"));
  setupActionIcon(ui_->actZoomIn, QLatin1String("zoom-in"));
  setupActionIcon(ui_->actZoomOut, QLatin1String("zoom-out"));
  setupActionIcon(ui_->actSinglePage, QLatin1String("view-page-one"));
  setupActionIcon(ui_->actAllPages, QLatin1String("view-page-multi"));

  QLatin1String imagePrefix(":/meteo/icons/tools/print/refresh");
  QIcon icon;
  // icon.addFile(imagePrefix + QLatin1String("-24.png"), QSize(24, 24));
  icon.addFile(imagePrefix + QLatin1String(".png"), QSize(32, 32));
   ui_->actRedraw->setIcon(icon);
}

void PrintPreview::previewChanged()
{
  updateNavigateActions();
 
  //zoom
  ui_->zoomFactor->setEditText(QString().sprintf("%.1f%%", ui_->preview->zoomFactor()*100));

  //номер страницы
  int numPages = ui_->preview->pageCount();
  ui_->pagesCnt->setText(QString::fromLatin1("/ %1").arg(numPages));
  ui_->curPageNum->setValidator(new QIntValidator(1, numPages, ui_->curPageNum));
}

void PrintPreview::updateNavigateActions()
{
  if (ui_->preview->viewMode() != QPrintPreviewWidget::AllPagesView) {
    int curPage = ui_->preview->currentPage();
    int numPages = ui_->preview->pageCount();
    ui_->actNext->setEnabled(curPage < numPages);
    ui_->actPrev->setEnabled(curPage > 1);
    ui_->actFirst->setEnabled(curPage > 1);
    ui_->actLast->setEnabled(curPage < numPages);
  }
  ui_->curPageNum->setText(QString::number(ui_->preview->currentPage()));
}

void PrintPreview::firstPage()
{ 
  ui_->preview->setCurrentPage(1);
  updateNavigateActions();
}

void PrintPreview::lastPage()
{
  ui_->preview->setCurrentPage(ui_->preview->pageCount());
  updateNavigateActions();
}

void PrintPreview::nextPage()
{
  ui_->preview->setCurrentPage(ui_->preview->currentPage() + 1);
  updateNavigateActions();
}

void PrintPreview::prevPage()
{
  ui_->preview->setCurrentPage(ui_->preview->currentPage() - 1);
  updateNavigateActions();
}

void PrintPreview::zoomIn()
{  
  ui_->preview->zoomIn();
  previewChanged();
}

void PrintPreview::zoomOut()
{
  ui_->preview->zoomOut();
  previewChanged();
}

void PrintPreview::zoomFactorChanged()
{
  QString text =  ui_->zoomFactor->lineEdit()->text();
  bool ok;
  qreal factor = text.remove('%').toFloat(&ok);
  factor = qMax(qreal(1.0), qMin(qreal(1000.0), factor));
  if (ok) {
     ui_->preview->setZoomFactor(factor/100.0);
     ui_->zoomFactor->setEditText(QString("%1%").arg(factor));
  }
}

void PrintPreview::pageNumChanged()
{
  bool ok = false;
  int res = ui_->curPageNum->text().toInt(&ok);
  if (ok)
    ui_->preview->setCurrentPage(res);
}

void PrintPreview::setSinglePage()
{
  ui_->actFirst->setEnabled(true);
  ui_->actLast->setEnabled(true);
  ui_->actNext->setEnabled(true);
  ui_->actPrev->setEnabled(true);
  ui_->curPageNum->setEnabled(true);
  ui_->preview->setViewMode(QPrintPreviewWidget::SinglePageView);
}

void PrintPreview::setAllPages()
{
  ui_->actFirst->setEnabled(false);
  ui_->actLast->setEnabled(false);
  ui_->actNext->setEnabled(false);
  ui_->actPrev->setEnabled(false);
  ui_->curPageNum->setEnabled(false);
  ui_->preview->fitInView();
  ui_->preview->setViewMode(QPrintPreviewWidget::AllPagesView);
}


void PrintPreview::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    QMainWindow::close();
  }
}

void PrintPreview::closeEvent(QCloseEvent* e)
{
  emit closed();
  QMainWindow::closeEvent(e);
}

void PrintPreview::updatePreview()
{
  ui_->preview->updatePreview();
}
