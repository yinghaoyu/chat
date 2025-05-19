#include "FluButton.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QVBoxLayout>

FluButton::FluButton(QWidget* parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(30);
    setMinimumWidth(30);
}

bool FluButton::isDisabled() const { return m_disabled; }
void FluButton::setDisabled(bool disabled) {
    m_disabled = disabled;
    QPushButton::setDisabled(disabled);
    update();
}

QString FluButton::contentDescription() const { return m_contentDescription; }
void FluButton::setContentDescription(const QString& desc) { m_contentDescription = desc; }

QColor FluButton::normalColor() const { return m_normalColor; }
void FluButton::setNormalColor(const QColor& color) { m_normalColor = color; update(); }

QColor FluButton::hoverColor() const { return m_hoverColor; }
void FluButton::setHoverColor(const QColor& color) { m_hoverColor = color; update(); }

QColor FluButton::disableColor() const { return m_disableColor; }
void FluButton::setDisableColor(const QColor& color) { m_disableColor = color; update(); }

QColor FluButton:: dividerColor() const { return m_dividerColor; }
void FluButton:: setDividerColor(const QColor& color) { m_dividerColor = color; update(); }

QColor FluButton::textColor() const { return m_textColor; }
void FluButton::setTextColor(const QColor& color) { m_textColor = color; update(); }

void FluButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    // Draw BackGround
    QColor bgColor = m_disabled ? m_disableColor : (m_hovered ? m_hoverColor : m_normalColor);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(bgColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 4, 4);

    // Draw Divider
    QPen boderPan(Qt::SolidLine);
    boderPan.setColor(m_dividerColor);
    p.setPen(boderPan);
    p.drawRoundedRect(rect(), 4, 4);

    // Draw text
    p.setPen(m_textColor);
    p.setFont(font());
    p.drawText(rect(), Qt::AlignCenter, text());

    // Focus rectangle
    if (hasFocus()) {
        QPen focusPen(Qt::DashLine);
        focusPen.setColor(Qt::black);
        p.setPen(focusPen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(2,2,-2,-2), 4, 4);
    }
}

void FluButton::enterEvent(QEvent* event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void FluButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QPushButton::leaveEvent(event);
}

void FluButton::mousePressEvent(QMouseEvent* event)
{
    m_pressed = true;
    update();
    QPushButton::mousePressEvent(event);
}

void FluButton::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}
