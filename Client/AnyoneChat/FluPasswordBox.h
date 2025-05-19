#ifndef FLUPASSWORDBOX_H
#define FLUPASSWORDBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColor>
#include <QLabel>

class FluPasswordBox : public QWidget
{
    Q_OBJECT
public:
    explicit FluPasswordBox(QWidget* parent = nullptr);

    void setDisabled(bool disabled);
    void setPlaceholderText(const QString& text);
    void setText(const QString& text);
    QString text() const;
    void setNormalColor(const QColor& color);
    void setDisableColor(const QColor& color);
    void setPlaceholderNormalColor(const QColor& color);
    void setPlaceholderFocusColor(const QColor& color);
    void setPlaceholderDisableColor(const QColor& color);

signals:
    void commit(const QString& text);
    void editingFinished();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void handleCommit();
    void updateRevealButton();

private:
    QLineEdit* m_edit;
    QPushButton* m_revealBtn;
    QColor m_normalColor;
    QColor m_disableColor;
    QColor m_placeholderNormalColor;
    QColor m_placeholderFocusColor;
    QColor m_placeholderDisableColor;
};

#endif // FLUPASSWORDBOX_H
