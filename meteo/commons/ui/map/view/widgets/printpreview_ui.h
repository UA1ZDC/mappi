#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_UI_PRINTPREVIEW_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_UI_PRINTPREVIEW_H

#include <qvariant.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qheaderview.h>
#include <qmainwindow.h>
#include <qprintpreviewwidget.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

class Ui_PrintPreview
{
public:
    QAction *actFirst;
    QAction *actLast;
    QLineEdit *curPageNum;
  QLabel *pagesCnt;
    QAction *actNext;
    QAction *actPrev;
    QAction *actZoomIn;
    QAction *actZoomOut;
    QComboBox *zoomFactor;
    QActionGroup* modeGroup;
    QAction *actSinglePage;
    QAction *actAllPages;
    QAction *actRedraw;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QPrintPreviewWidget *preview;
    QToolBar *toolBar;

  void setupUi(QMainWindow *PrintPreview, QPrinter* printer)
    {
        if (PrintPreview->objectName().isEmpty())
            PrintPreview->setObjectName(QString::fromUtf8("PrintPreview"));
        PrintPreview->resize(887, 452);
        actFirst = new QAction(PrintPreview);
        actFirst->setObjectName(QString::fromUtf8("actFirst"));
        actLast = new QAction(PrintPreview);
        actLast->setObjectName(QString::fromUtf8("actLast"));
        actNext = new QAction(PrintPreview);
        actNext->setObjectName(QString::fromUtf8("actNext"));
        actPrev = new QAction(PrintPreview);
        actPrev->setObjectName(QString::fromUtf8("actPrev"));
	actZoomIn = new QAction(PrintPreview);
        actZoomIn->setObjectName(QString::fromUtf8("actZoomIn"));
        actZoomOut = new QAction(PrintPreview);
        actZoomOut->setObjectName(QString::fromUtf8("actZoomOut"));
	zoomFactor = new QComboBox;
	zoomFactor->setEditable(true);
	zoomFactor->setMinimumContentsLength(7);
	zoomFactor->setInsertPolicy(QComboBox::NoInsert);
	curPageNum = new QLineEdit;
	curPageNum->setAlignment(Qt::AlignRight);
	curPageNum->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	int cyphersWidth = PrintPreview->fontMetrics().width(QString().fill(QLatin1Char('8'), 3));
	int maxWidth = curPageNum->minimumSizeHint().width() + cyphersWidth;
	curPageNum->setMinimumWidth(maxWidth);
	curPageNum->setMaximumWidth(maxWidth);
	pagesCnt = new QLabel;

	modeGroup = new QActionGroup(PrintPreview);
        actSinglePage = modeGroup->addAction(QString::fromUtf8("Одна"));
        actSinglePage->setObjectName(QString::fromUtf8("actSinglePage"));
	actSinglePage->setCheckable(true);
        actAllPages = modeGroup->addAction(QString::fromUtf8("Все"));
        actAllPages->setObjectName(QString::fromUtf8("actAllPages"));
	actAllPages->setCheckable(true);

	actRedraw = new QAction(PrintPreview);

        centralwidget = new QWidget(PrintPreview);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        preview = new QPrintPreviewWidget(printer, centralwidget);
        preview->setObjectName(QString::fromUtf8("preview"));

        gridLayout->addWidget(preview, 0, 0, 1, 1);

        PrintPreview->setCentralWidget(centralwidget);
        toolBar = new QToolBar(PrintPreview);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        PrintPreview->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actFirst);
        toolBar->addAction(actPrev);
	toolBar->addWidget(curPageNum);
	toolBar->addWidget(pagesCnt);
        toolBar->addAction(actNext);
	toolBar->addAction(actLast);
        toolBar->addSeparator();
	toolBar->addWidget(zoomFactor);
	toolBar->addAction(actZoomIn);
	toolBar->addAction(actZoomOut);
        toolBar->addSeparator();
	toolBar->addAction(actSinglePage);
	toolBar->addAction(actAllPages);
        toolBar->addSeparator();
	toolBar->addAction(actRedraw);

        retranslateUi(PrintPreview);

        QMetaObject::connectSlotsByName(PrintPreview);
    } // setupUi

    void retranslateUi(QMainWindow *PrintPreview)
    {
      PrintPreview->setWindowTitle(QObject::tr("Предварительный просмотр"));

      actFirst->setText(QObject::tr("Первая"));
      actFirst->setShortcut(QApplication::translate("PrintPreview", "Ctrl+Home", 0));
#ifndef QT_NO_TOOLTIP
      actFirst->setToolTip(QObject::tr("Первая страница (Ctrl+Home)"));
#endif 
      actLast->setText( QObject::tr("Последняя"));
      actLast->setShortcut(QApplication::translate("PrintPreview", "Ctrl+End", 0));
#ifndef QT_NO_TOOLTIP
      actLast->setToolTip(QObject::tr("Последняя страница (Ctrl+End)"));
#endif 
      actNext->setText(QObject::tr("Следующая"));
      actNext->setShortcut(QApplication::translate("PrintPreview", "Right", 0));
#ifndef QT_NO_TOOLTIP
      actNext->setToolTip(QObject::tr("Следующая страница (Стрелка вправо)"));
#endif
      actPrev->setText(QObject::tr("Предыдущая"));
      actPrev->setShortcut(QApplication::translate("PrintPreview", "Left", 0));
#ifndef QT_NO_TOOLTIP
      actPrev->setToolTip(QObject::tr("Предыдущая станица (Стрелка влево)"));
#endif
    
      actZoomIn->setShortcuts(QKeySequence::ZoomIn);
      actZoomIn->setText(QObject::tr("Увеличить"));
#ifndef QT_NO_TOOLTIP
      actZoomIn->setToolTip(QObject::tr("Увеличить (Ctrl+Plus)"));
#endif
      actZoomOut->setShortcuts(QKeySequence::ZoomOut);
      actZoomOut->setText(QObject::tr("Уменьшить"));
#ifndef QT_NO_TOOLTIP
      actZoomOut->setToolTip(QObject::tr("Уменьшить (Ctrl+Minus)"));
#endif
      
      actAllPages->setText(QObject::tr("Все"));
#ifndef QT_NO_TOOLTIP
      actAllPages->setToolTip(QObject::tr("Все страницы"));
#endif
      actSinglePage->setText(QObject::tr("Одна"));
#ifndef QT_NO_TOOLTIP
      actSinglePage->setToolTip(QObject::tr("Одна страница"));
#endif

      actRedraw->setShortcuts(QKeySequence::Refresh);
      actRedraw->setText(QObject::tr("Обновить"));
#ifndef QT_NO_TOOLTIP
      actRedraw->setToolTip(QObject::tr("Обновить"));
#endif

      toolBar->setWindowTitle(QApplication::translate("PrintPreview", "toolBar", 0));
    }

};

namespace Ui {
    class PrintPreview: public Ui_PrintPreview {};
} // namespace Ui

QT_END_NAMESPACE

#endif
