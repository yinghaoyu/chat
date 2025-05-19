#ifndef TIMERBTN_H
#define TIMERBTN_H
#include "FluButton.h"
#include <QTimer>

class TimerBtn : public FluButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~ TimerBtn();

    // 重写mouseReleaseEvent
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QTimer  *_timer;
    int _counter;
};

#endif // TIMERBTN_H
