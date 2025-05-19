#include "resetdialog.h"
#include "ui_resetdialog.h"
#include <QDebug>
#include <QRegularExpression>
#include "global.h"
#include "httpmgr.h"
#include "FluTextBox.h"
#include "FluPasswordBox.h"

ResetDialog::ResetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetDialog)
{
    ui->setupUi(this);

    connect(ui->user_edit,&FluTextBox::commit,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &FluTextBox::commit, this, [this](){
        checkEmailValid();
    });

    connect(ui->pwd_edit, &FluPasswordBox::commit, this, [this](){
        checkPassValid();
    });


    connect(ui->varify_edit, &FluTextBox::commit, this, [this](){
         checkVarifyValid();
    });

    this->notification = new Notification(this);
    this->notification->setGeometry(QRect(QPoint(this->rect().center().x() - (this->notification->width() / 2), this->rect().top() - (this->notification->height())), QSize(this->notification->size())));
    this->notification->hide();

    this->animation = new QTimeLine(500, this);
    this->animation->setUpdateInterval(0);
    this->animation->setFrameRange(this->rect().top() - (this->notification->height()), 20);

    connect(this->animation, &QTimeLine::frameChanged, this, [=](int frame) {
        this->notification->move(this->notification->geometry().x(), frame);
    });

    //连接reset相关信号和注册处理回调
    initHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish, this,
            &ResetDialog::slot_reset_mod_finish);

}


ResetDialog::~ResetDialog()
{
    delete ui;
}

void ResetDialog::on_return_btn_clicked()
{
    qDebug() << "sure btn clicked ";
    emit switchLogin();
}

void ResetDialog::on_varify_btn_clicked()
{
    qDebug()<<"receive varify btn clicked ";
    auto email = ui->email_edit->text();
    auto bcheck = checkEmailValid();
    if(!bcheck){
        return;
    }

    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                                        json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::RESETMOD);
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::Success){
        showTip(tr("网络请求错误"));
        enableOperation(true);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"));
        enableOperation(true);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"));
        enableOperation(true);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}

bool ResetDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"));
        return false;
    }
    return true;
}


bool ResetDialog::checkPassValid()
{
    auto pass = ui->pwd_edit->text();

    if(pass.length() < 6 || pass.length()>15){
        //提示长度不准确
        showTip(tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        showTip(tr("不能包含非法字符"));
        return false;;
    }
    return true;
}



bool ResetDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        showTip(tr("邮箱地址不正确"));
        return false;
    }
    return true;
}

bool ResetDialog::checkVarifyValid()
{
    auto pass = ui->varify_edit->text();
    if(pass.isEmpty()){
        showTip(tr("验证码不能为空"));
        return false;
    }
    return true;
}

bool ResetDialog::enableOperation(bool enabled)
{
    ui->user_edit->setEnabled(enabled);
    ui->email_edit->setEnabled(enabled);
    ui->pwd_edit->setEnabled(enabled);
    ui->varify_edit->setEnabled(enabled);
    ui->varify_edit->setEnabled(enabled);
    return true;
}

void ResetDialog::initHandlers()
{
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        switch(error)
        {
            case ErrorCodes::Success:
            {
                auto email = jsonObj["email"].toString();
                showTip(tr("验证码已发送到邮箱，注意查收"));
                qDebug()<< "email is " << email ;
                break;
            }
            case ErrorCodes::Error_Json:
                showTip(tr("服务器json解析错误"));
                break;
            default: showTip(tr("参数错误")); break;
        }
    });

    // 重置用户回包逻辑
    _handlers.insert(ReqId::ID_RESET_PWD, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        enableOperation(true);
        switch(error)
        {
            case ErrorCodes::Success:
            {
                auto email = jsonObj["email"].toString();
                showTip(tr("重置成功"));
                qDebug()<< "email is " << email ;
                qDebug()<< "user uuid is " <<  jsonObj["uuid"].toString();
                break;
            }
            case ErrorCodes::Error_Json:
                showTip(tr("服务器json解析错误"));
                break;
            case ErrorCodes::VarifyExpired:
                showTip(tr("验证码已过期"));
                break;
            case ErrorCodes::VarifyCodeErr:
                showTip(tr("验证码错误"));
                break;
            case ErrorCodes::EmailNotMatch:
                showTip(tr("邮箱和用户名不匹配"));
                break;
            case ErrorCodes::UserExist:
                showTip(tr("用户已存在"));
                break;
            case ErrorCodes::PasswdUpFailed:
                showTip(tr("服务器数据库错误，更新密码失败"));
                break;
            default: showTip(tr("参数错误")); break;
        }
    });
}

void ResetDialog::showTip(QString str)
{
    this->notification->setText(str);
    this->notification->setGeometry(QRect(QPoint(this->rect().center().x() - (this->notification->width() / 2), this->rect().top() - (this->notification->height())), QSize(this->notification->size())));
    this->notification->show();

    this->animation->setDirection(QTimeLine::Forward);
    this->animation->start();
    QTimer::singleShot(1000, this, [=]() {
        this->animation->setDirection(QTimeLine::Backward);
        this->animation->start();
    });
}

void ResetDialog::on_sure_btn_clicked()
{
    bool valid = checkUserValid();
    if(!valid){
        return;
    }

    valid = checkEmailValid();
    if(!valid){
        return;
    }

    valid = checkPassValid();
    if(!valid){
        return;
    }

    valid = checkVarifyValid();
    if(!valid){
        return;
    }
    enableOperation(false);
    //发送http重置用户请求
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pwd_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/reset_pwd"),
                 json_obj, ReqId::ID_RESET_PWD,Modules::RESETMOD);
}
