#include "image-widget.h"
#include <QPainter>
#include <QDateTime>
#include <QFont>

ImageWidget::ImageWidget( WifiClient *parent ):mp_client( parent )
{
    setFixedSize( 1280, 1600 );
    mp_buffer = new unsigned char[640*480*4];
    mp_show_buffer = new unsigned char[640*480*4];

    QImage img( ":/wheat.jpg" );
    m_ir_img = img;
    n_first_frames = 0;
    n_first_time =  QDateTime::currentMSecsSinceEpoch();

//    connect( mp_client, SIGNAL( image_changed( unsigned int, int) ), this, SLOT(img_trigged_slot( unsigned int, int)) );
    connect( mp_client, SIGNAL( ir_image_changed( unsigned char*, int) ), this, SLOT(ir_img_trigged_slot( unsigned char*, int)) );

}

ImageWidget::~ImageWidget()
{

}

void ImageWidget::paintEvent(QPaintEvent *)
{
//    m_condition.wait( &m_mutex );
    m_mutex.lock();
    QPainter painter( this );
    QRect a(0,0,1280,1600);

    m_ir_img = m_ir_img.scaled( 1280, 1600 );
    painter.drawImage( a, m_ir_img );
    QFont font("Arial", 50, QFont::Bold, false );
    painter.setFont(font);
    painter.drawText(10,120,s_interval_time);
    m_mutex.unlock();

}

void ImageWidget::img_trigged_slot( unsigned int puc_out_buf, int n_data_len )
{
//    qDebug()<<(void*)puc_out_buf<<n_data_len;
//    qDebug()<<(void*)mp_buffer<<(const void *)puc_out_buf;
//    memset( mp_buffer, 0, 640*480*4 );
    m_mutex.lock();
    qDebug()<<"show data: "<<n_data_len;
//    for( int i =0; i<640*480*4; i++ ) {
//        mp_buffer[i] = *( (char*)puc_out_buf+i );
//    }
//    memcpy( (void*)mp_buffer, (const void *)puc_out_buf, n_data_len );
    const uchar* mp = (const uchar*)(void*)mp_buffer;
    QImage m_mg;
    m_mg =  QImage( (const uchar *)mp, 640, 480, QImage::Format_RGB32 );
    m_mg = QImage( (const uchar*)rgb_to_rgb( m_mg,640, 480 ), 640, 480, QImage::Format_RGB32 );
    m_ir_img = m_mg.copy( 0, 0, 640, 480 );
    qint64 current = QDateTime::currentMSecsSinceEpoch();
    s_interval_time = QString("%1").arg( current - n_first_time );
    n_first_time = current;
    update();
    m_mutex.unlock();
//    update();

}

void ImageWidget::ir_img_trigged_slot( unsigned char* puc_out_buf, int n_data_len )
{
//    qDebug()<<(void*)puc_out_buf<<n_data_len;
//    qDebug()<<(void*)mp_buffer<<(const void *)puc_out_buf;
//    memset( mp_buffer, 0, 640*480*4 );
    m_mutex.lock();
    qDebug()<<"show data: "<<n_data_len;
    for( int i =0; i<640*480*4; i++ ) {
        mp_buffer[i] = *( puc_out_buf+i );
    }
//    memcpy( (void*)mp_buffer, (const void *)puc_out_buf, n_data_len );
//    const uchar* mp = (const uchar*)(void*)mp_buffer;
    QImage m_mg;
    m_mg =  QImage( (const uchar *)mp_buffer, 640, 480, QImage::Format_RGB32 );
    m_mg = QImage( (const uchar*)rgb_to_rgb(m_mg,640, 480 ), 640, 480, QImage::Format_RGB32 );
    m_ir_img = m_mg.copy(0,0,640,480);
    qint64 current = QDateTime::currentMSecsSinceEpoch();
    s_interval_time = QString("%1").arg( current - n_first_time );
    n_first_time = current;
    update();
    m_mutex.unlock();
//    update();

}

uchar* ImageWidget::rgb_to_rgb( QImage &image32, int n_width, int n_height )
{
    uchar* imagebits_32 = (uchar* )image32.constBits();         //获取图像首地址，32位图
    int n_line_num = 0;
    int w_32 = image32.bytesPerLine();
    for( int i=0; i<n_height; i++ ) {
        n_line_num = i*w_32;
        for( int j=0; j<n_width; j++ ) {
            int r_32 = imagebits_32[n_line_num+ j * 4 + 2];
            int g_32 = imagebits_32[n_line_num+ j * 4 + 1];
            int b_32 = imagebits_32[ n_line_num+ j * 4];
            imagebits_32[ n_line_num+ j * 4] = r_32;
            imagebits_32[n_line_num+ j * 4 + 2] = b_32;
        }
    }
    return imagebits_32;
//    return NULL;
}
