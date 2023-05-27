#include "intervaledit.h"
#include "ui_intervaledit.h"
#include <QComboBox>
#include <QToolButton>
#include <QDateEdit>
#include <QSpacerItem>
#include <meteo/commons/global/dateformat.h>


DateIntervalEdit::DateIntervalEdit(QWidget* parent /*=*/) :
    QWidget(parent),
    m_ui_(new Ui::DateIntervalEdit),
    firstYear_(QDate::currentDate().year() - 10)
{
    m_ui_->setupUi(this);
    m_ui_->deBegin->setDisplayFormat(meteo::dtHumanFormatDateOnly);
    m_ui_->deEnd->setDisplayFormat(meteo::dtHumanFormatDateOnly);

    m_ui_->cbRange->insertItem(WEEK, QString::fromUtf8("неделя"));
    m_ui_->cbRange->insertItem(MONTH, QString::fromUtf8("месяц"));
    m_ui_->cbRange->insertItem(YEAR, QString::fromUtf8("год"));
    m_ui_->cbRange->insertItem(OTHER, QString::fromUtf8("произвольный"));

    
    m_ui_->cbMonth->insertItem(JAN, QString::fromUtf8("январь"));
    m_ui_->cbMonth->insertItem(FEB, QString::fromUtf8("февраль"));
    m_ui_->cbMonth->insertItem(MAR, QString::fromUtf8("март"));
    m_ui_->cbMonth->insertItem(APR, QString::fromUtf8("апрель"));
    m_ui_->cbMonth->insertItem(MAY, QString::fromUtf8("май"));
    m_ui_->cbMonth->insertItem(JUN, QString::fromUtf8("июнь"));
    m_ui_->cbMonth->insertItem(JUL, QString::fromUtf8("июль"));
    m_ui_->cbMonth->insertItem(AUG, QString::fromUtf8("август"));
    m_ui_->cbMonth->insertItem(SEP, QString::fromUtf8("сентябрь"));
    m_ui_->cbMonth->insertItem(OCT, QString::fromUtf8("октябрь"));
    m_ui_->cbMonth->insertItem(NOV, QString::fromUtf8("ноябрь"));
    m_ui_->cbMonth->insertItem(DEC, QString::fromUtf8("декабрь"));
    //m_ui_->cbMonth->setCurrentIndex(QDate::currentDate().month() - 1);

    QDate date = QDate(firstYear_, 1, 1);
    for (int i = 0; i <= 20; ++i)
    {
        m_ui_->cbYear->insertItem(i, date.toString("yyyy"));
        date = date.addYears(1);
    }


    QObject::connect(m_ui_->tbPrev, SIGNAL(clicked()), this, SLOT(onPrev()));
    QObject::connect(m_ui_->tbNext, SIGNAL(clicked(bool)), this, SLOT(onNext()));

    QObject::connect(m_ui_->cbRange, SIGNAL(activated(int)), this, SLOT(onActivatedRange(int)));
    QObject::connect(m_ui_->cbRange, SIGNAL(currentIndexChanged(int)), this, SLOT(onActivatedRange(int)));
    QObject::connect(m_ui_->cbMonth, SIGNAL(activated(int)), this, SLOT(onActivatedMonth(int)));
    QObject::connect(m_ui_->cbYear, SIGNAL(activated(int)), this, SLOT(onActivatedYear(int)));

    QObject::connect(m_ui_->deBegin, SIGNAL(dateChanged(QDate)), this, SLOT(slotBeginDateChanged(const QDate&)));
    QObject::connect(m_ui_->deBegin, &QDateEdit::dateChanged, this, &DateIntervalEdit::signalIntervalChanged);
    QObject::connect(m_ui_->deEnd, &QDateEdit::dateChanged, this, &DateIntervalEdit::signalIntervalChanged);

    setHorisontal(true);

    setDateInterval(QDate::currentDate(), QDate::currentDate().addDays(1));
    m_ui_->cbRange->setCurrentIndex(MONTH);
    this->onActivatedRange(MONTH);
}

int DateIntervalEdit::getRangeType()
{
    return m_ui_->cbRange->currentIndex();
}

void DateIntervalEdit::slotBeginDateChanged(const QDate& newDate)
{
    QDate dateBegin;
    QDate dateEnd;
    switch (getRangeType()) {
    case WEEK:
        {
            dateBegin = newDate.addDays(1 - newDate.dayOfWeek());
            dateEnd = dateBegin.addDays(6);
            break;
        }
    default:
        {
            dateBegin = newDate;
            dateEnd = getSecondDate();
            if (dateBegin > dateEnd) dateEnd = dateBegin.addDays(1);
            break;
        }
    }
    this->setDateInterval(dateBegin, dateEnd);
}



bool DateIntervalEdit::isHorisontal() const
{
  return isHor_;
}

void DateIntervalEdit::setHorisontal(bool hor){
  delete layout();
  this->isHor_ = hor;
  QBoxLayout* layout = nullptr;
  if ( true == hor ) {
    layout = new QHBoxLayout(this);
  }
  else {
    layout = new QVBoxLayout(this);
    layout->setSpacing(1);
  }
  layout->setMargin(0);

  layout->addWidget(m_ui_->layoutRange);
  layout->addWidget(m_ui_->layoutMMYY);
  layout->addWidget(m_ui_->layoutDeBegin);
  layout->addWidget(m_ui_->layoutDeEnd);
}

DateIntervalEdit::~DateIntervalEdit()
{
    delete m_ui_;
    m_ui_ = 0;
}

QDate DateIntervalEdit::getFirstDate() const
{
    return m_ui_->deBegin->date();
}

QDate DateIntervalEdit::getSecondDate() const
{
    return m_ui_->deEnd->date();
}

void DateIntervalEdit::setDateRange(const QDate& first, const QDate& second, DateIntervalEdit::type_t type)
{
    this->setDateInterval(first, second);
    onActivatedRange(type);
}

void DateIntervalEdit::setDateInterval(const QDate& begin, const QDate& end)
{
  m_ui_->deBegin->blockSignals(true);
  m_ui_->deEnd->blockSignals(true);
  m_ui_->deBegin->setDate(begin);
  m_ui_->deEnd->setDate(end);
  m_ui_->cbMonth->setCurrentIndex(begin.month() - 1);
  m_ui_->cbYear->setCurrentIndex(begin.year() - firstYear_);
  m_ui_->deBegin->blockSignals(false);
  m_ui_->deEnd->blockSignals(false);
  emit signalIntervalChanged();
}

void DateIntervalEdit::onActivatedRange(int index)
{
    const QDate& date = this->getFirstDate();    
    switch (index)
    {
        case WEEK :
        {
            deEnabled(true, false);
            rangeEditorEnabled(true, false, false);
            const QDate& newBegin = date.addDays(1 - date.dayOfWeek());
            const QDate& newEnd = newBegin.addDays(6);
            this->setDateInterval(newBegin, newEnd);
            break;
        }
        case MONTH :
        {
            deEnabled(false);
            rangeEditorEnabled(true);
            const QDate& newBegin = QDate(date.year(), date.month(), 1);
            const QDate& newEnd = QDate(date.year(), date.month() + 1, 1).addDays(-1);
            setDateInterval(newBegin, newEnd);
            break ;
        }
        case YEAR :
        {
            deEnabled(false);
            rangeEditorEnabled(true);
            m_ui_->cbMonth->setEnabled(false);
            const QDate& newBegin = QDate(date.year(), 1, 1);
            const QDate& newEnd = QDate(date.year(), 12, 31);
            setDateInterval(newBegin, newEnd);
            break ;
        }
        case OTHER :
        {
            deEnabled(true);
            rangeEditorEnabled(false);
            break ;
        }
    }
}

void DateIntervalEdit::onActivatedMonth(int index)
{
    const QDate& oldDate = getFirstDate();
    const QDate& dateBegin = QDate(oldDate.year(), index + 1, oldDate.day());
    const QDate& dateEnd = dateBegin.addMonths(1).addDays(-1);
    this->setDateInterval(dateBegin, dateEnd);
    //onActivatedRange(m_ui_->cbRange->currentIndex());
}

void DateIntervalEdit::onActivatedYear(int index)
{
    Q_UNUSED(index)
    const QDate& prewBegin = this->getFirstDate();
    const QDate& prewEnd = this->getSecondDate();
    const QDate& dateBegin = QDate(firstYear_ + index, prewBegin.month(), prewBegin.day());
    const QDate& dateEnd = QDate(firstYear_ + index, prewEnd.month(), prewEnd.day());
    this->setDateInterval(dateBegin, dateEnd);

    //onActivatedRange(m_ui_->cbRange->currentIndex());
}

void DateIntervalEdit::deEnabled(bool isEnabled)
{
    deEnabled(isEnabled, isEnabled);
}

void DateIntervalEdit::deEnabled(bool isBeginEnabled, bool isEndEnabled)
{
    m_ui_->deBegin->setEnabled(isBeginEnabled);
    m_ui_->deEnd->setEnabled(isEndEnabled);
}

void DateIntervalEdit::rangeEditorEnabled(bool isEnabled)
{
    rangeEditorEnabled(isEnabled, isEnabled, isEnabled);
}

void DateIntervalEdit::rangeEditorEnabled(bool isButtonsNextPrewEnabled, bool isMonthEnabled, bool isYearEnabled)
{
    m_ui_->tbPrev->setEnabled(isButtonsNextPrewEnabled);
    m_ui_->tbNext->setEnabled(isButtonsNextPrewEnabled);
    m_ui_->cbMonth->setEnabled(isMonthEnabled);
    m_ui_->cbYear->setEnabled(isYearEnabled);
}

void DateIntervalEdit::onPrev()
{
    switch (m_ui_->cbRange->currentIndex())
    {
        case WEEK:
        {
            const QDate& newBegin = this->getFirstDate().addDays(-7);
            const QDate& newEnd = this->getSecondDate().addDays(-7);
            this->setDateInterval(newBegin, newEnd);
            break;
        }
        case MONTH :
        {
            const QDate& newBegin = this->getFirstDate().addMonths(-1);
            const QDate& newEnd = this->getSecondDate().addMonths(-1);
            this->setDateInterval(newBegin, newEnd);
            break ;
        }

        case YEAR :
        {
            const QDate& newBegin = this->getFirstDate().addYears(-1);
            const QDate& newEnd = this->getSecondDate().addYears(-1);
            this->setDateInterval(newBegin, newEnd);
            break ;
        }
        default :
        {
            //TODO Some error message
            break ;
        }

    }
}

void DateIntervalEdit::onNext()
{
    switch (m_ui_->cbRange->currentIndex())
    {
        case WEEK:
        {
            const QDate& newBegin = this->getFirstDate().addDays(7);
            const QDate& newEnd = this->getSecondDate().addDays(7);
            this->setDateInterval(newBegin, newEnd);
            break;
        }
        case MONTH :
        {
            const QDate& newBegin = this->getFirstDate().addMonths(1);
            const QDate& newEnd = this->getSecondDate().addMonths(1);
            this->setDateInterval(newBegin, newEnd);
            break ;
        }

        case YEAR :
        {
            const QDate& newBegin = this->getFirstDate().addYears(1);
            const QDate& newEnd = this->getSecondDate().addYears(1);
            this->setDateInterval(newBegin, newEnd);
            break ;
        }
        default :
        {
            //TODO Some error message
            break ;
        }

    }
}
