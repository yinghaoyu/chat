#include "FluTextBox.h"
#include "FluTextBox.h"
#include <QStyle>
#include <QAction>
#include <QFocusEvent>
#include <QPalette>
#include <QPainter>
#include <QStyleOption>

FluTextBox::FluTextBox(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(240);

    m_edit = new QLineEdit(this);
    m_clearBtn = new QPushButton(this);
    m_iconLabel = new QLabel(this);

    m_edit->setStyleSheet("border:none;background:transparent;");

    m_clearBtn->setCursor(Qt::ArrowCursor);
    m_clearBtn->setFixedSize(20, 20);
    m_clearBtn->setText("✕");
    m_clearBtn->setVisible(false);
    m_clearBtn->setStyleSheet("border:none;background:transparent;");

    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setStyleSheet("border:none;background:transparent;");
    m_iconLabel->setVisible(false);

    QHBoxLayout* lay = new QHBoxLayout(this);
    lay->setContentsMargins(7+4, 0, 10, 0); // leftPadding, rightPadding
    lay->setSpacing(4);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_clearBtn, 0);
    lay->addWidget(m_iconLabel, 0);

    // 默认颜色
    m_normalColor = QColor(27,27,27);
    m_disableColor = QColor(160,160,160);
    m_placeholderNormalColor = QColor(96,96,96);
    m_placeholderFocusColor = QColor(141,141,141);
    m_placeholderDisableColor = QColor(160,160,160);

    setCleanEnabled(true);

    // 右键菜单
    m_menu = new QMenu(this);
    m_menu->addAction("复制", m_edit, SLOT(copy()));
    m_menu->addAction("粘贴", m_edit, SLOT(paste()));
    m_menu->addAction("剪切", m_edit, SLOT(cut()));
    m_menu->addSeparator();
    m_menu->addAction("全选", m_edit, SLOT(selectAll()));

    m_edit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_edit, &QLineEdit::customContextMenuRequested, this, &FluTextBox::showContextMenu);

    // 清空按钮
    connect(m_clearBtn, &QPushButton::clicked, m_edit, &QLineEdit::clear);
    connect(m_edit, &QLineEdit::textChanged, this, &FluTextBox::updateClearButton);
    connect(m_edit, &QLineEdit::textChanged, this, [this]{ update(); });
    connect(m_edit, &QLineEdit::returnPressed, this, &FluTextBox::handleCommit);

    updateClearButton();
}

void FluTextBox::setDisabled(bool disabled)
{
    m_edit->setDisabled(disabled);
    update();
}
void FluTextBox::setReadOnly(bool readOnly)
{
    m_edit->setReadOnly(readOnly);
    update();
}
void FluTextBox::setIcon(const QIcon& icon)
{
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(16,16));
        m_iconLabel->setVisible(true);
    } else {
        m_iconLabel->setVisible(false);
    }
}
void FluTextBox::setPlaceholderText(const QString& text)
{
    m_edit->setPlaceholderText(text);
}
void FluTextBox::setText(const QString& text)
{
    m_edit->setText(text);
}
QString FluTextBox::text() const
{
    return m_edit->text();
}
void FluTextBox::setCleanEnabled(bool enabled)
{
    m_cleanEnabled = enabled;
    updateClearButton();
}
void FluTextBox::setNormalColor(const QColor& color) { m_normalColor = color; update(); }
void FluTextBox::setDisableColor(const QColor& color) { m_disableColor = color; update(); }
void FluTextBox::setPlaceholderNormalColor(const QColor& color) { m_placeholderNormalColor = color; update(); }
void FluTextBox::setPlaceholderFocusColor(const QColor& color) { m_placeholderFocusColor = color; update(); }
void FluTextBox::setPlaceholderDisableColor(const QColor& color) { m_placeholderDisableColor = color; update(); }

void FluTextBox::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateClearButton();
}

void FluTextBox::handleCommit()
{
    emit commit(m_edit->text());
}

void FluTextBox::updateClearButton()
{
    bool show = m_cleanEnabled && !m_edit->isReadOnly() && !m_edit->text().isEmpty() && m_edit->isEnabled();
    m_clearBtn->setVisible(show);
}

void FluTextBox::showContextMenu(const QPoint& pos)
{
    if (m_edit->echoMode() == QLineEdit::Password)
        return;
    if (m_edit->isReadOnly() && m_edit->text().isEmpty())
        return;
    m_menu->exec(m_edit->mapToGlobal(pos));
}

// 可选：自定义绘制边框和颜色
void FluTextBox::paintEvent(QPaintEvent* event)
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
