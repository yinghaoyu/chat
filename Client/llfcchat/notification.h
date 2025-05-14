#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QWidget>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
class Notification : public QWidget
{
    Q_OBJECT

  public:
    Notification(QWidget* parent);
    ~Notification();
  public:
    void setText(const QString& _text);
  protected:
    void paintEvent(QPaintEvent*)Q_DECL_OVERRIDE;
  private:
    QGraphicsDropShadowEffect* shadow = Q_NULLPTR;
    QLabel* text = Q_NULLPTR;
};

#endif  // NOTIFICATION_H
