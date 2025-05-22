#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QRegularExpression>
#include "global.h"
#include "httpmgr.h"
#include "FluPasswordBox.h"
#include "FluTextBox.h"
#include <QRegularExpressionValidator>
#include <QRandomGenerator>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),_countdown(5)
{
    ui->setupUi(this);
    //ui->user_edit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]+$")));
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this,
            &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();

    connect(ui->user_edit,&FluTextBox::commit,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &FluTextBox::commit, this, [this](){
        checkEmailValid();
    });

    connect(ui->pass_edit, &FluPasswordBox::commit, this, [this](){
        checkPassValid();
    });

    connect(ui->confirm_edit, &FluPasswordBox::commit, this, [this](){
        checkConfirmValid();
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

    // 创建定时器
    _countdown_timer = new QTimer(this);
    // 连接信号和槽
    connect(_countdown_timer, &QTimer::timeout, [this](){
        if(_countdown==0){
            _countdown_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _countdown--;
        auto str = QString("注册成功，%1 s后返回登录").arg(_countdown);
        ui->tip_lb->setText(str);
    });
}

RegisterDialog::~RegisterDialog()
{
    qDebug()<<"destruct RegDlg";
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    qDebug()<<"receive varify btn clicked ";
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    bool valid = checkEmailValid();
    if(valid){
        //发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                     json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
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


bool RegisterDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        showTip(tr("用户名不能为空"));
        return false;
    }
    return true;
}

bool RegisterDialog::checkEmailValid()
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

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
    auto confirm = ui->confirm_edit->text();

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

    if(pass != confirm){
        //提示密码不匹配
        showTip(tr("密码和确认密码不匹配"));
        return false;
    }
    return true;
}

bool RegisterDialog::checkVarifyValid()
{
    auto pass = ui->varify_edit->text();
    if(pass.isEmpty()){
        showTip(tr("验证码不能为空"));
        return false;
    }
    return true;
}

bool RegisterDialog::checkConfirmValid()
{
    auto pass = ui->pass_edit->text();
    auto confirm = ui->confirm_edit->text();

    if(confirm.length() < 6 || confirm.length() > 15 ){
        //提示长度不准确
        showTip(tr("密码长度应为6~15"));
        return false;
    }

    // 创建一个正则表达式对象，按照上述密码要求
    // 这个正则表达式解释：
    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(confirm).hasMatch();
    if(!match){
        //提示字符非法
        showTip(tr("不能包含非法字符"));
        return false;
    }

    if(pass != confirm){
        //提示密码不匹配
        showTip(tr("确认密码和密码不匹配"));
        return false;
    }
    return true;
}


bool RegisterDialog::enableOperation(bool enabled)
{
    ui->user_edit->setEnabled(enabled);
    ui->email_edit->setEnabled(enabled);
    ui->pass_edit->setEnabled(enabled);
    ui->confirm_edit->setEnabled(enabled);
    ui->varify_edit->setEnabled(enabled);
    return true;
}

void RegisterDialog::initHttpHandlers()
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
            default: showTip(tr("未知错误")); break;
        }
    });

    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        enableOperation(true);
        switch(error)
            {
            case ErrorCodes::Success:
                {
                    auto email = jsonObj["email"].toString();
                    showTip(tr("用户注册成功"));
                    qDebug()<< "email is " << email ;
                    qDebug()<< "user uuid is " <<  jsonObj["uid"].toString();
                    ChangeTipPage();
                    break;
                }
            case ErrorCodes::Error_Json:
                showTip(tr("服务器json解析错误"));
                break;
            case ErrorCodes::PasswdErr:
                showTip(tr("两次密码不一致"));
                break;
            case ErrorCodes::VarifyExpired:
                showTip(tr("验证码已过期"));
                break;
            case ErrorCodes::VarifyCodeErr:
                showTip(tr("验证码错误"));
                break;
            case ErrorCodes::UserExist:
                showTip(tr("用户已存在"));
                break;
            default: showTip(tr("未知错误")); break;
            }
    });
}

void RegisterDialog::ChangeTipPage()
{
    _countdown_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _countdown_timer->start(1000);
}

void RegisterDialog::showTip(QString str)
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

//day11 添加确认槽函数
void RegisterDialog::on_sure_btn_clicked()
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

    valid = checkConfirmValid();
    if(!valid){
        return;
    }

    valid = checkVarifyValid();
    if(!valid){
        return;
    }
    enableOperation(false);
    //day11 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pass_edit->text());
    json_obj["sex"] = 0;

    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int head_i = randomValue % heads.size();

    json_obj["icon"] = heads[head_i];
    json_obj["nick"] = ui->user_edit->text();
    json_obj["confirm"] = xorString(ui->confirm_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                 json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

void RegisterDialog::on_return_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}

void RegisterDialog::on_cancel_btn_clicked()
{
    _countdown_timer->stop();
    emit sigSwitchLogin();
}
