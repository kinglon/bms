#ifndef BATTERYWIDGET_H
#define BATTERYWIDGET_H

#include <QWidget>

class BatteryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BatteryWidget(QWidget *parent = nullptr);

public:
    void setBatteryValue(int value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // 电池电量
    int m_batteryValue = 20;

    QPixmap m_bgImage;
};

#endif // BATTERYWIDGET_H
