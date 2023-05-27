#ifndef _DATEINTERVAL_EDIT_H_
#define _DATEINTERVAL_EDIT_H_

#include <QWidget>
#include <qboxlayout.h>
#include <QDateTime>


namespace Ui
{
    class DateIntervalEdit;
}

/** @brief */
class DateIntervalEdit :
    public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool isHorisontal READ isHorisontal WRITE setHorisontal)
    
public :
    enum type_t
    {
        WEEK = 0,
        MONTH,
        YEAR,
        OTHER
    };

    enum month_t
    {
        JAN = 0,
        FEB,
        MAR,
        APR,
        MAY,
        JUN,
        JUL,
        AUG,
        SEP,
        OCT,
        NOV,
        DEC
    };

public :
    explicit DateIntervalEdit(QWidget* parent = 0);
    virtual ~DateIntervalEdit();

    QDate getFirstDate() const;
    QDate getSecondDate() const;

    void setDateRange(const QDate& first, const QDate& second, type_t type = MONTH);
    bool isHorisontal() const;
    void setHorisontal(bool hor);
    
private slots :
    void onActivatedRange(int index);
    void onActivatedMonth(int index);
    void onActivatedYear(int index);
    void onPrev();
    void onNext();    
    void slotBeginDateChanged(const QDate &newDate);

private :
    int getRangeType();
    void deEnabled(bool isEnabled);
    void deEnabled(bool isBeginEnabled, bool isEndEnabled);

    void rangeEditorEnabled(bool isEnabled);
    void rangeEditorEnabled(bool isButtonsNextPrewEnabled, bool isMonthEnabled, bool isYearEnabled);
    void setDateInterval(const QDate& begin, const QDate& end);

signals:
    void signalIntervalChanged();

private :
    Ui::DateIntervalEdit* m_ui_;
    bool isHor_;
    const int firstYear_;

};



#endif //_DATE_EDIT_H_
