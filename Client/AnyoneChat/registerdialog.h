#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "global.h"
#include "notification.h"

#include <QDialog>
#include <functional>
#include <QMap>
#include <QJsonObject>
#include <QSet>
#include <QTimer>
#include <QTimeLine>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void on_sure_btn_clicked();

    void on_return_btn_clicked();

    void on_cancel_btn_clicked();

public slots:
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
private:
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkVarifyValid();
    bool checkConfirmValid();
    bool enableOperation(bool enabled);
    void initHttpHandlers();
    void ChangeTipPage();
    Ui::RegisterDialog *ui;
    void showTip(QString str);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    Notification* notification = Q_NULLPTR;
    QTimeLine* animation = Q_NULLPTR;
    QTimer * _countdown_timer;
    int _countdown;
signals:
    void sigSwitchLogin();
};

#endif // REGISTERDIALOG_H
