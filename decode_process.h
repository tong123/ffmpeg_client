#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QImage>
#include "video_decode_api.h"
#include "wifi-client.h"

class WifiClient;
class Decode_H264 : public QObject
{
    Q_OBJECT

public:
    friend class WifiClient;
    explicit Decode_H264(QObject *parent = 0);
    ~Decode_H264();

    int cb_data_out( unsigned char *puc_out_buf, unsigned int ui_data_len, int width, int height, void *p_user_data );

private slots:
public slots:
    void on_decode_h264_packet(unsigned char *p_data_encode, unsigned int dw_data_len);

protected:
//    void paintEvent(QPaintEvent *event);

private:
    unsigned int *mp_dw_bits;
    unsigned char *mp_buffer;

    int m_recv_len;
    int m_recv_len_one;
    int m_frame;

    bool b_connect;
    YHANDLE m_handle;
    int m_decode_len;
    WifiClient *mp_wifi_client;
public:
    void put_data();
signals:
    void set_ir_img_show( unsigned char *puc_out_buf, unsigned int ui_data_len );

};

#endif // WIDGET_H
