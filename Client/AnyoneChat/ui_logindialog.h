/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <FluButton.h>
#include <FluTextBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <flupasswordbox.h>
#include "clickedlabel.h"

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QWidget *head_widget;
    QGridLayout *gridLayout;
    QLabel *head_label;
    QHBoxLayout *horizontalLayout_2;
    QLabel *email_lb;
    FluTextBox *email_edit;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout_3;
    QLabel *pass_label;
    FluPasswordBox *pass_edit;
    QSpacerItem *horizontalSpacer_8;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    ClickedLabel *reg_label;
    ClickedLabel *forget_label;
    QSpacerItem *horizontalSpacer_7;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_2;
    FluButton *login_btn;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName(QString::fromUtf8("LoginDialog"));
        LoginDialog->resize(300, 450);
        LoginDialog->setMinimumSize(QSize(300, 450));
        LoginDialog->setMaximumSize(QSize(300, 450));
        verticalLayout_3 = new QVBoxLayout(LoginDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 5, 5, 5);
        head_widget = new QWidget(LoginDialog);
        head_widget->setObjectName(QString::fromUtf8("head_widget"));
        head_widget->setMinimumSize(QSize(200, 200));
        gridLayout = new QGridLayout(head_widget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        head_label = new QLabel(head_widget);
        head_label->setObjectName(QString::fromUtf8("head_label"));
        head_label->setMinimumSize(QSize(100, 100));
        head_label->setMaximumSize(QSize(100, 100));

        gridLayout->addWidget(head_label, 1, 0, 1, 1);


        verticalLayout_2->addWidget(head_widget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        email_lb = new QLabel(LoginDialog);
        email_lb->setObjectName(QString::fromUtf8("email_lb"));
        email_lb->setMinimumSize(QSize(25, 25));
        email_lb->setMaximumSize(QSize(25, 25));

        horizontalLayout_2->addWidget(email_lb);

        email_edit = new FluTextBox(LoginDialog);
        email_edit->setObjectName(QString::fromUtf8("email_edit"));
        email_edit->setMinimumSize(QSize(0, 30));
        email_edit->setMaximumSize(QSize(16777215, 30));

        horizontalLayout_2->addWidget(email_edit);

        horizontalSpacer_6 = new QSpacerItem(25, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_6);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pass_label = new QLabel(LoginDialog);
        pass_label->setObjectName(QString::fromUtf8("pass_label"));
        pass_label->setMinimumSize(QSize(25, 25));
        pass_label->setMaximumSize(QSize(25, 25));

        horizontalLayout_3->addWidget(pass_label);

        pass_edit = new FluPasswordBox(LoginDialog);
        pass_edit->setObjectName(QString::fromUtf8("pass_edit"));
        pass_edit->setMinimumSize(QSize(0, 30));
        pass_edit->setMaximumSize(QSize(16777215, 30));

        horizontalLayout_3->addWidget(pass_edit);

        horizontalSpacer_8 = new QSpacerItem(25, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_8);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        reg_label = new ClickedLabel(LoginDialog);
        reg_label->setObjectName(QString::fromUtf8("reg_label"));

        horizontalLayout_4->addWidget(reg_label);

        forget_label = new ClickedLabel(LoginDialog);
        forget_label->setObjectName(QString::fromUtf8("forget_label"));

        horizontalLayout_4->addWidget(forget_label);

        horizontalSpacer_7 = new QSpacerItem(25, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_7);


        verticalLayout_2->addLayout(horizontalLayout_4);

        verticalSpacer_2 = new QSpacerItem(20, 25, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_2);

        login_btn = new FluButton(LoginDialog);
        login_btn->setObjectName(QString::fromUtf8("login_btn"));
        login_btn->setMinimumSize(QSize(100, 30));

        horizontalLayout_5->addWidget(login_btn);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);


        verticalLayout_2->addLayout(horizontalLayout_5);


        verticalLayout_3->addLayout(verticalLayout_2);

        verticalSpacer = new QSpacerItem(20, 25, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);


        retranslateUi(LoginDialog);

        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "Dialog", nullptr));
        head_label->setText(QString());
        email_lb->setText(QString());
        pass_label->setText(QString());
        reg_label->setText(QCoreApplication::translate("LoginDialog", "\346\263\250\345\206\214\350\264\246\345\217\267", nullptr));
        forget_label->setText(QCoreApplication::translate("LoginDialog", "\345\277\230\350\256\260\345\257\206\347\240\201", nullptr));
        login_btn->setText(QCoreApplication::translate("LoginDialog", "\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
