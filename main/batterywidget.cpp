#include "batterywidget.h"
#include <QPainter>

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget{parent}
{
    m_bgImage.load(":/res/battery.png");
}

void BatteryWidget::setBatteryValue(int value)
{
    if (value < 0)
    {
        m_batteryValue = 0;
    }
    else if (value > 100)
    {
        m_batteryValue = 100;
    }
    else
    {
        m_batteryValue = value;
    }
    update();
}

void BatteryWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QRect widgetRect = rect();

    // 画背景图
    painter.drawPixmap(rect(), m_bgImage);

    // 画电量
    int leftPadding = 6;
    int rightPadding = 8;
    int topPadding = 20;
    int bottomPadding = 10;
    QRect clientRect;
    clientRect.setX(widgetRect.x()+leftPadding);
    clientRect.setWidth(widgetRect.width()-leftPadding-rightPadding);
    clientRect.setY(widgetRect.y()+topPadding);
    clientRect.setHeight(widgetRect.height()-topPadding-bottomPadding);    

    int space = 5; // 每格电池之间的距离
    int items = 5; // 共5格电池
    int height = (clientRect.height() - (items-1)*space) / items; // 每格的高度
    int batteryValue = m_batteryValue;    
    int offsetBottom = 0;
    QRect itemRect = clientRect;
    while(batteryValue > 0)
    {
        int itemHeight = height;
        if (batteryValue < 20)
        {
            itemHeight = (int)(height*batteryValue/20.0f);
        }
        itemRect.setY(clientRect.bottom()-offsetBottom-itemHeight);
        itemRect.setHeight(itemHeight);

        QColor color = QColor(87, 222, 60);
        painter.setBrush(color);
        painter.setPen(color);
        painter.drawRect(itemRect);

        batteryValue -= 20;
        offsetBottom += itemHeight + space;
    }
}
