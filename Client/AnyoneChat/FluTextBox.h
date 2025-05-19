#ifndef FLUTEXTBOX_H
#define FLUTEXTBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QColor>
#include <QLabel>

class FluTextBox : public QWidget
{
    Q_OBJECT
public:
    explicit FluTextBox(QWidget* parent = nullptr);

    void setDisabled(bool disabled);
    void setReadOnly(bool readOnly);
    void setIcon(const QIcon& icon);
    void setPlaceholderText(const QString& text);
    void setText(const QString& text);
    void setCleanEnabled(bool enabled);
    void setNormalColor(const QColor& color);
    void setDisableColor(const QColor& color);
    void setPlaceholderNormalColor(const QColor& color);
    void setPlaceholderFocusColor(const QColor& color);
    void setPlaceholderDisableColor(const QColor& color);

    QString text() const;

signals:
    void commit(const QString& text);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void handleCommit();
    void updateClearButton();
    void showContextMenu(const QPoint& pos);

private:
    QLineEdit* m_edit;
    QPushButton* m_clearBtn;
    QLabel* m_iconLabel;
    QMenu* m_menu;
    bool m_cleanEnabled = true;
    QColor m_normalColor;
    QColor m_disableColor;
    QColor m_placeholderNormalColor;
    QColor m_placeholderFocusColor;
    QColor m_placeholderDisableColor;
};
#endif // FLUTEXTBOX_H
