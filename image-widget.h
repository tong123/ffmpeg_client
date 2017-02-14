#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include "wifi-client.h"
class ImageWidget : public QLabel
{
    Q_OBJECT
public:
//    explicit ImageWidget(QLabel *parent = 0);
    explicit ImageWidget(WifiClient *parent );
//    explicit ImageWidget( );

    ~ImageWidget();
    void paintEvent( QPaintEvent * );
    uchar* rgb_to_rgb( QImage &image32, int n_width, int n_height );

signals:

public slots:
    void img_trigged_slot( unsigned int puc_out_buf, int n_data_len );
    void ir_img_trigged_slot( unsigned char *puc_out_buf, int n_data_len );
private:
    QImage m_ir_img;
    WifiClient *mp_client;
    unsigned char *mp_buffer;
    QMutex m_mutex;
    unsigned int n_first_frames;
    qint64 n_first_time;
    QString s_interval_time;
    QMutex mn_data_mutex;
    unsigned char *mp_show_buffer;
    QWaitCondition m_condition;
};

#endif // IMAGEWIDGET_H
