#include "astrodatawidget.h"
#include "ui_astrodatawidget.h"

#include <meteo/commons/astrodata/defines.h>
#include <meteo/commons/astrodata/exception.h>
#include <meteo/commons/astrodata/observerpoint.h>
#include <meteo/commons/astrodata/astrodatafile.h>
#include "astrodatacolorproxymodel.h"
#include <meteo/commons/astrodata/astrodatadocument.h>
#include <meteo/commons/astrodata/header.h>
#include "headerview.h"
#include <meteo/commons/ui/custom/geopointeditor.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/custom/intervaledit.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>

#include <meteo/commons/ui/custom/filedialogrus.h>

#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QPainter>

using namespace astro;
static const QString kName = QObject::tr("astro_diag_action");
static const QString kNameStationWgt = QObject::tr("astro");


AstroDataWidget::AstroDataWidget(QWidget *parent) :
  QMainWindow (parent),
  m_ui(new Ui::AstroDataWidget),
  m_model(new AstroDataModel(&m_calendar, this))
{
    m_ui->setupUi(this);
    m_ui->toolBar->addWidget(m_ui->stationWidget);
    m_ui->toolBar->addWidget(m_ui->wgtSaveLoadBtns);
    initStationWidget();
    QObject::connect(this->m_ui->pbOpen, &QPushButton::clicked, this, &AstroDataWidget::onOpenCalendar);
    QObject::connect(this->m_ui->pbSave, &QPushButton::clicked, this, &AstroDataWidget::onSaveCalendar);
    QObject::connect(this->m_ui->pbPrint, &QPushButton::clicked, this, &AstroDataWidget::onPrintCalendar);

    AstroDataColorProxyModel *colorModel = new AstroDataColorProxyModel(this);
    colorModel->setSourceModel(m_model);

    m_ui->tvCalendar->setModel(colorModel);
    m_ui->tvCalendar->setFont(QFont("Liberation Mono", 10));

    QObject::connect(m_ui->pbMakeUp, SIGNAL(clicked()), this, SLOT(makeUp()));
    QObject::connect(m_ui->cmbtimespec, SIGNAL(currentIndexChanged( int ) ), this, SLOT( slotCmbTimeSpecChanged() ) );

    ObserverPoint &point = m_calendar.observerPoint();
    m_ui->dateIntervalEdit->setDateRange(point.getFirstDateTime().date(),
                           point.getSecondDateTime().date()
                          );


    HeaderItem item1(new QStandardItem(RU("Дата")), true);

    HeaderItem item2(new QStandardItem(RU("Солнце")));
    HeaderItem item21(new QStandardItem(RU("восход")));
    HeaderItem item22(new QStandardItem(RU("заход")));
    item2.appendChild(item21);
    item2.appendChild(item22);

    HeaderItem item3(new QStandardItem(RU("День")));

    HeaderItem item4(new QStandardItem(RU("Ночь")));

    HeaderItem item5(new QStandardItem(RU("Луна")));
    HeaderItem item51(new QStandardItem(RU("восход")));
    HeaderItem item52(new QStandardItem(RU("заход")));
    HeaderItem item53(new QStandardItem(RU("фаза")));
    item5.appendChild(item51);
    item5.appendChild(item52);
    item5.appendChild(item53);

    HeaderItem item6(new QStandardItem(RU("Гражданские сумерки")));
    HeaderItem item61(new QStandardItem(RU("утренние")));
    HeaderItem item62(new QStandardItem(RU("вечерние")));
    item6.appendChild(item61);
    item6.appendChild(item62);

    HeaderItem item7(new QStandardItem(RU("Навигационные сумерки")));
    HeaderItem item71(new QStandardItem(RU("утренние")));
    HeaderItem item72(new QStandardItem(RU("вечерние")));
    item7.appendChild(item71);
    item7.appendChild(item72);

    HeaderItem item8(new QStandardItem(RU("Астрономические сумерки")));
    HeaderItem item81(new QStandardItem(RU("утренние")));
    HeaderItem item82(new QStandardItem(RU("вечерние")));
    item8.appendChild(item81);
    item8.appendChild(item82);

    Header header;
    header.appendItem(item1);
    header.appendItem(item2);
    header.appendItem(item3);
    header.appendItem(item4);
    header.appendItem(item5);
    header.appendItem(item6);
    header.appendItem(item7);
    header.appendItem(item8);

    HorizontalSpanHeader *hw = new HorizontalSpanHeader(m_ui->tvCalendar);
    hw->setHeaderInfo(header);
    m_ui->tvCalendar->setHorizontalHeader(hw);
    setWindowTitle("Астрономические данные");
    makeUp();
}

AstroDataWidget::~AstroDataWidget()
{
    delete m_ui;
    m_ui = nullptr;
}

bool AstroDataWidget::initStationWidget(){
  static const QList<meteo::sprinf::MeteostationType> stations_to_load = {
    meteo::sprinf::MeteostationType::kStationSynop,
    meteo::sprinf::MeteostationType::kStationAirport,
    meteo::sprinf::MeteostationType::kStationAerodrome,
    meteo::sprinf::MeteostationType::kStationAero,
    meteo::sprinf::MeteostationType::kStationHydro,
    meteo::sprinf::MeteostationType::kStationOcean,
    meteo::sprinf::MeteostationType::kStationRadarmap,
    meteo::sprinf::MeteostationType::kStationGeophysics };
  return this->m_ui->stationWidget->loadStation(stations_to_load);
}


QString AstroDataWidget::calendarInfo() const
{
    return m_calendar.info();
}

void AstroDataWidget::makeUp()
{
    ObserverPoint &point = m_calendar.observerPoint();
    auto stWgt = this->m_ui->stationWidget;
    point.setNumber(stWgt->stationIndex());
    point.setNameRU(stWgt->ruName());
    point.setNameENG(stWgt->enName());
    auto coord = stWgt->coord();
    point.setLatitude(coord.latDeg());
    point.setLongitude(coord.lonDeg());
    int alt = qRound(coord.alt());
    point.setHeight(alt);

    point.setFirstDateTime(QDateTime(m_ui->dateIntervalEdit->getFirstDate(), QTime()));
    point.setSecondDateTime(QDateTime(m_ui->dateIntervalEdit->getSecondDate(), QTime()));

    m_calendar.clear();
    m_calendar.makeUp();

    m_model->update();
    if ( 0 < m_model->rowCount() ) {
        m_ui->tvCalendar->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    }
   setWindowTitle(RU("Астрономические данные %1").arg(point.getNameRU()));
}

void AstroDataWidget::slotCmbTimeSpecChanged()
{
  if ( 0 == m_ui->cmbtimespec->currentIndex() ) {
    m_calendar.setTimeSpec(Qt::UTC);
  }
  else {
    m_calendar.setTimeSpec(Qt::LocalTime);
  }
}

void AstroDataWidget::openCalendar(const QString &filePath)
{
    m_calendar.clear();
    AstroDataFile file(filePath);
    try {
        file.load(m_calendar);
    } catch (const Exception &exp) {
        QMessageBox::warning(this,
                             RU("Ошибка"),
                             RU("Ошибка открытия файла, причина :\n%1").arg(exp.what()),
                             QMessageBox::Ok
                            );
    }

    const ObserverPoint &point = m_calendar.observerPoint();
    meteo::GeoPoint gp = meteo::GeoPoint::fromDegree(point.getLatitude(), point.getLongitude(),
                                                     point.getHeight());
    m_ui->stationWidget->setStation( point.getNumber(), gp);
    m_ui->dateIntervalEdit->setDateRange(point.getFirstDateTime().date(),
                           point.getSecondDateTime().date()
                          );

    Qt::TimeSpec tms = m_calendar.timeSpec();
    if ( Qt::UTC == tms ) {
      m_ui->cmbtimespec->setCurrentIndex(0);
    }
    else {
      m_ui->cmbtimespec->setCurrentIndex(1);
    }

    m_model->update();
    if (0 < m_model->rowCount()) {
        m_ui->tvCalendar->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    }
}

void AstroDataWidget::saveCalendar(const QString &filePath)
{
    AstroDataFile file(
        QString("%1%2")
        .arg(filePath)
        .arg(DATA_FILE_EXT)
    );

    try {
        file.save(m_calendar);
    } catch (const Exception &exp) {
        QMessageBox::warning(this,
                             RU("Ошибка"),
                             RU("Ошибка сохранения файла, причина :\n%1").arg(exp.what()),
                             QMessageBox::Ok
                            );
    }
}

void AstroDataWidget::printCalendar()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    printer.setPageMargins(0.4, 0.9, 0.4, 0.4, QPrinter::Inch);
    QPrintDialog dialog(&printer);
    dialog.setWindowTitle(RU("Печать"));
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter;
        painter.begin(&printer);

        QTextDocument view;
        view.setTextWidth(printer.pageRect().width());
        AstroDataDocument document(m_calendar);
        for (int page = 0; page < document.pageCount(); ++page) {
            if (page != 0) {
                printer.newPage();
            }
            view.setHtml(document.pageToHtml(page));
            view.drawContents(&painter);
        }
        painter.end();
    }
}

void AstroDataWidget::onOpenCalendar()
{
  QString filePath = meteo::FileDialog::getOpenFileName(
            this,
            RU("Открыть календарь"),
            QDir::homePath(),
            QString("*%1").arg(DATA_FILE_EXT)
    );
    if (!filePath.isEmpty())
    {
        this->openCalendar(filePath);
    };
}

void AstroDataWidget::onSaveCalendar()
{
    this->saveCalendar(
           meteo::FileDialog::getSaveFileName(
            this,
            RU("Сохранить календарь"),
            QString("%1/%2").arg(QDir::homePath()).arg(this->calendarInfo()),
            QString("*%1").arg(DATA_FILE_EXT)
        ).replace(QString(DATA_FILE_EXT), QString())
    );
}

void AstroDataWidget::onPrintCalendar()
{
    this->printCalendar();
}
