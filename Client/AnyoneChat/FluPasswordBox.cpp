#include "FluPasswordBox.h"
#include <QStyleOption>
#include <QPainter>
#include <QIcon>

FluPasswordBox::FluPasswordBox(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(240);

    m_edit = new QLineEdit(this);
    m_edit->setEchoMode(QLineEdit::Password);
    m_edit->setMinimumHeight(28);
    m_edit->setStyleSheet("border:none;background:transparent;");

    m_revealBtn = new QPushButton(this);
    m_revealBtn->setFixedSize(30, 20);
    m_revealBtn->setCheckable(true);
    m_revealBtn->setCursor(Qt::ArrowCursor);
    m_revealBtn->setStyleSheet("border:none;background:transparent;");
    m_revealBtn->setIcon(QIcon(":/res/visible.png")); // 替换为你的 Reveal 图标

    QHBoxLayout* lay = new QHBoxLayout(this);
    lay->setContentsMargins(7+4, 0, 5, 0); // leftPadding, rightPadding
    lay->setSpacing(4);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_revealBtn, 0);

    // 默认颜色
    m_normalColor = QColor(27,27,27);
    m_disableColor = QColor(160,160,160);
    m_placeholderNormalColor = QColor(96,96,96);
    m_placeholderFocusColor = QColor(141,141,141);
    m_placeholderDisableColor = QColor(160,160,160);

    connect(m_edit, &QLineEdit::returnPressed, this, &FluPasswordBox::handleCommit);
    connect(m_edit, &QLineEdit::editingFinished, this, &FluPasswordBox::editingFinished);

    connect(m_revealBtn, &QPushButton::pressed, [this]{
        m_edit->setEchoMode(QLineEdit::Normal);
    });
    connect(m_revealBtn, &QPushButton::released, [this]{
        m_edit->setEchoMode(QLineEdit::Password);
    });
    connect(m_edit, &QLineEdit::textChanged, this, &FluPasswordBox::updateRevealButton);

    updateRevealButton();
}

void FluPasswordBox::setDisabled(bool disabled)
{
    m_edit->setDisabled(disabled);
    update();
}
void FluPasswordBox::setPlaceholderText(const QString& text)
{
    m_edit->setPlaceholderText(text);
}
void FluPasswordBox::setText(const QString& text)
{
    m_edit->setText(text);
}
QString FluPasswordBox::text() const
{
    return m_edit->text();
}
void FluPasswordBox::setNormalColor(const QColor& color) { m_normalColor = color; update(); }
void FluPasswordBox::setDisableColor(const QColor& color) { m_disableColor = color; update(); }
void FluPasswordBox::setPlaceholderNormalColor(const QColor& color) { m_placeholderNormalColor = color; update(); }
void FluPasswordBox::setPlaceholderFocusColor(const QColor& color) { m_placeholderFocusColor = color; update(); }
void FluPasswordBox::setPlaceholderDisableColor(const QColor& color) { m_placeholderDisableColor = color; update(); }

void FluPasswordBox::handleCommit()
{
    emit commit(m_edit->text());
}

void FluPasswordBox::updateRevealButton()
{
    m_revealBtn->setVisible(!m_edit->text().isEmpty());
}

void FluPasswordBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // 边框和背景
    QRect r = rect();
    QColor border = /*m_edit->hasFocus() ? QColor(0,120,212) :*/ QColor(200,200,200);
    p.setPen(QPen(border, 1));
    p.setBrush(m_edit->isEnabled() ? Qt::white : QColor(245,245,245));
    p.drawRoundedRect(r.adjusted(0,0,-1,-1), 4, 4);
}
