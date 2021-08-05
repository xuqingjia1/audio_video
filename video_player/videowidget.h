#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QObject>

class VideoWidget : public QObject
{
    Q_OBJECT
public:
    explicit VideoWidget(QObject *parent = nullptr);

signals:

};

#endif // VIDEOWIDGET_H
