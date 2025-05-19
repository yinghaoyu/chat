#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include "notification.h"
#include "global.h"

#include <QDialog>
#include <QTimeLine>

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();

private slots:
    void on_return_btn_clicked();

    void on_varify_btn_clicked();

    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_sure_btn_clicked();

private:
    bool checkUserValid();
    bool checkPassValid();
    void showTip(QString str);
    bool checkEmailValid();
    bool checkVarifyValid();
    bool enableOperation(bool enabled);
    void initHandlers();
    Ui::ResetDialog *ui;
    Notification* notification = Q_NULLPTR;
    QTimeLine* animation = Q_NULLPTR;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
signals:
    void switchLogin();
};

#endif // RESETDIALOG_H
