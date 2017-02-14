#include "decode_process.h"
//#include "ui_Decode_H264.h"
#include <QDebug>
#include <QPainter>
#include <QRect>
#include <QByteArray>
#include <QThread>
#include "video_decode.h"

Decode_H264 *g_p_main_window;

int callback_vi_decode_out_data(unsigned char *puc_out_buf, unsigned int ui_data_len, int width, int height, void *p_user_data)
{
//    qDebug()<<"hello callback_vi_decode_out_data";
//    if ( g_p_main_window != NULL ) {
        g_p_main_window->cb_data_out( puc_out_buf, ui_data_len, width, height, p_user_data );
//    }
    return 0;
}

int Decode_H264::cb_data_out( unsigned char *puc_out_buf, unsigned int ui_data_len, int width, int height, void *p_user_data )
{
    //qDebug("[%s][%s:%d]ui_data_len=%d\n", __FILE__, __FUNCTION__, __LINE__, ui_data_len);
    qDebug()<<width<<height;
//    memcpy( mp_dw_bits, puc_out_buf, ui_data_len);
    m_decode_len = ui_data_len;
    qDebug()<<"ui_data_len: "<<ui_data_len;
    emit set_ir_img_show( (unsigned char*)puc_out_buf, ui_data_len );
//    repaint();
//    QThread::msleep(10);
//    qDebug()<<"puc_out_buf:"<<puc_out_buf;
//    qDebug() << "after decode:"<<ui_data_len;
//    m_frame++;
//    update();
    return 0;
}

Decode_H264::Decode_H264(QObject *parent) :
    QObject(parent)
{

    g_p_main_window = this;

    video_decode_init(&m_handle, 1024*1024, callback_vi_decode_out_data, NULL);

    mp_dw_bits = new unsigned int[ 640 * 480 ];
    mp_buffer = new unsigned char[ 1000000  ];

    b_connect = false;
    m_recv_len = 0;
    m_recv_len_one = 0;
    m_frame = 0;

    mp_wifi_client = (WifiClient*)parent;
    connect( (VidConv*)m_handle, SIGNAL( imaged_data( unsigned char*, unsigned int ) ), mp_wifi_client, SLOT(set_ir_img_data(unsigned char*,uint ) ) );
    connect( this, SIGNAL(set_ir_img_show(unsigned char*,uint ) ), mp_wifi_client, SLOT(set_ir_img_data(unsigned char*,uint )) );
}

Decode_H264::~Decode_H264()
{

}

//void Decode_H264::paintEvent( QPaintEvent *event )
//{
//    QPainter painter;
//    painter.begin(this);

//    QImage image( ( unsigned char* )mp_dw_bits, 720, 480, QImage::Format_RGB32 );
//    painter.drawImage( 1, 1, image );

//    painter.end();
//}

void Decode_H264::on_decode_h264_packet( unsigned char *p_data_encode, unsigned int dw_data_len )
{
    m_recv_len = 0;
    memset( mp_buffer, 0, sizeof(mp_buffer));
    QByteArray arr_buff =  QByteArray( (char*)p_data_encode );
    if( !arr_buff.isNull() ) {
//        qDebug()<<"arr_buff---";
       memcpy( mp_buffer+m_recv_len, p_data_encode, dw_data_len );
    }
//    qDebug()<<"hello world on_decode_h264_packet"<<arr_buff.length()<<dw_data_len;
    m_recv_len_one = dw_data_len;

    m_recv_len += dw_data_len;

    MONITOR_HEAD_S *pst_head = (MONITOR_HEAD_S *) mp_buffer;
//    qDebug()<<"hello1"<<m_recv_len<<pst_head->ui_Length;

    if( m_recv_len >= pst_head->ui_Length ) {
//        qDebug()<<"hello2";

        int ui_len = 0;

        while( ui_len < m_recv_len ) {
//            qDebug()<<"hello3";

           if (ui_len + sizeof ( MONITOR_HEAD_S ) >= m_recv_len ||
                   ui_len + pst_head->ui_Length > m_recv_len ){

                unsigned char *tmp_buf = new unsigned char [m_recv_len - ui_len];

                memcpy (tmp_buf, mp_buffer + ui_len, m_recv_len - ui_len);
                memcpy (mp_buffer, tmp_buf, m_recv_len - ui_len);
                m_recv_len -= ui_len;
                delete tmp_buf;
                break;
           }
//            qDebug()<<"hello";
           qDebug()<<"pst_head->ui_Length: "<<pst_head->ui_Length;
           video_put_data( m_handle, mp_buffer + ui_len, pst_head->ui_Length );

           ui_len += pst_head->ui_Length;
           pst_head = (MONITOR_HEAD_S *)( mp_buffer + ui_len );

        }
//        memcpy( p_data_decode, mp_dw_bits, m_decode_len );
    }

}




