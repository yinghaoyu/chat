/********************************************************************************
** Form generated from reading UI file 'userinfopage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERINFOPAGE_H
#define UI_USERINFOPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserInfoPage
{
public:
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_4;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_4;
    QSpacerItem *verticalSpacer_4;
    QHBoxLayout *horizontalLayout;
    QLabel *head_lb;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *up_btn;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *nick_ed;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLineEdit *name_ed;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QLineEdit *desc_ed;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *submit_btn;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QWidget *UserInfoPage)
    {
        if (UserInfoPage->objectName().isEmpty())
            UserInfoPage->setObjectName(QString::fromUtf8("UserInfoPage"));
        UserInfoPage->resize(508, 527);
        horizontalLayout_6 = new QHBoxLayout(UserInfoPage);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_4);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 5, 5, 5);
        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(verticalSpacer_3);

        label_4 = new QLabel(UserInfoPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_2->addWidget(label_4);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(verticalSpacer_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        head_lb = new QLabel(UserInfoPage);
        head_lb->setObjectName(QString::fromUtf8("head_lb"));
        head_lb->setMinimumSize(QSize(200, 200));
        head_lb->setMaximumSize(QSize(200, 200));

        horizontalLayout->addWidget(head_lb);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_6);

        up_btn = new QPushButton(UserInfoPage);
        up_btn->setObjectName(QString::fromUtf8("up_btn"));
        up_btn->setMinimumSize(QSize(120, 30));
        up_btn->setMaximumSize(QSize(120, 30));

        horizontalLayout->addWidget(up_btn);


        verticalLayout_2->addLayout(horizontalLayout);

        verticalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(verticalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(UserInfoPage);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        nick_ed = new QLineEdit(UserInfoPage);
        nick_ed->setObjectName(QString::fromUtf8("nick_ed"));
        nick_ed->setMinimumSize(QSize(200, 30));
        nick_ed->setMaximumSize(QSize(200, 30));

        horizontalLayout_2->addWidget(nick_ed);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(UserInfoPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        name_ed = new QLineEdit(UserInfoPage);
        name_ed->setObjectName(QString::fromUtf8("name_ed"));
        name_ed->setMinimumSize(QSize(200, 30));
        name_ed->setMaximumSize(QSize(200, 30));

        horizontalLayout_3->addWidget(name_ed);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(UserInfoPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        desc_ed = new QLineEdit(UserInfoPage);
        desc_ed->setObjectName(QString::fromUtf8("desc_ed"));
        desc_ed->setMinimumSize(QSize(200, 30));
        desc_ed->setMaximumSize(QSize(200, 30));

        horizontalLayout_4->addWidget(desc_ed);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        submit_btn = new QPushButton(UserInfoPage);
        submit_btn->setObjectName(QString::fromUtf8("submit_btn"));
        submit_btn->setMinimumSize(QSize(100, 30));
        submit_btn->setMaximumSize(QSize(100, 30));

        horizontalLayout_5->addWidget(submit_btn);


        verticalLayout_2->addLayout(horizontalLayout_5);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        horizontalLayout_6->addLayout(verticalLayout_2);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);


        retranslateUi(UserInfoPage);

        QMetaObject::connectSlotsByName(UserInfoPage);
    } // setupUi

    void retranslateUi(QWidget *UserInfoPage)
    {
        UserInfoPage->setWindowTitle(QCoreApplication::translate("UserInfoPage", "Form", nullptr));
        label_4->setText(QCoreApplication::translate("UserInfoPage", "\345\237\272\346\234\254\344\277\241\346\201\257", nullptr));
        head_lb->setText(QString());
        up_btn->setText(QCoreApplication::translate("UserInfoPage", "\344\270\212\344\274\240\346\234\254\345\234\260\347\205\247\347\211\207", nullptr));
        label->setText(QCoreApplication::translate("UserInfoPage", "\346\230\265\347\247\260\357\274\232 ", nullptr));
        label_2->setText(QCoreApplication::translate("UserInfoPage", "\347\224\250\346\210\267\345\220\215:", nullptr));
        label_3->setText(QCoreApplication::translate("UserInfoPage", "\346\217\217\350\277\260:  ", nullptr));
        submit_btn->setText(QCoreApplication::translate("UserInfoPage", "\344\277\235\345\255\230", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserInfoPage: public Ui_UserInfoPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERINFOPAGE_H
