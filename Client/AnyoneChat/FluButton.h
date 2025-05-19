#ifndef FLUBUTTON_H
#define FLUBUTTON_H

#include <QPushButton>
#include <QColor>
#include <QEvent>

// 普通按钮
class FluButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FluButton(QWidget* parent = nullptr);

    bool isDisabled() const;
    void setDisabled(bool disabled);

    QString contentDescription() const;
    void setContentDescription(const QString& desc);

    QColor normalColor() const;
    void setNormalColor(const QColor& color);

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    QColor disableColor() const;
    void setDisableColor(const QColor& color);

    QColor dividerColor() const;
    void setDividerColor(const QColor& color);

    QColor textColor() const;
    void setTextColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_disabled = false;
    QString m_contentDescription;
    QColor m_normalColor = QColor(254,254,254);
    QColor m_hoverColor = QColor(246,246,246);
    QColor m_disableColor = QColor(251,251,251);
    QColor m_dividerColor = QColor(233,233,233);
    QColor m_textColor = QColor(0,0,0);
};

#endif // FLUBUTTON_H
