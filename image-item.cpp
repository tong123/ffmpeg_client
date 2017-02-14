#include "image-item.h"
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QImageReader>
#include <QDateTime>
#include <QApplication>
#include <QDesktopWidget>
#define ImageW 640
#define ImageH 480
//CImageItem::CImageItem( QQuickItem *parent ) : QQuickItem( parent )
CImageItem::CImageItem( QQuickItem *parent ) : QQuickPaintedItem( parent )
{
//    update();
//    QImage img( ":/wheat.jpg" );
//    m_ir_img = img;
    mp_buffer = new unsigned char[640*480*4];
    n_start_frame_count = 0;
    n_frame_rate = 0;
    n_width = QApplication::desktop()->width();
    n_height = QApplication::desktop()->height();
//    n_width = 480;
//    n_height = 640;
}

CImageItem::~CImageItem()
{
}

void CImageItem::paint( QPainter *painter )
{
    m_mutex.lock();
    QRect a(0,0,n_height,n_width);
    qDebug()<<"paint=====";
    m_ir_img = m_ir_img.scaled( n_height, n_width );
//    painter->drawImage( a, m_ir_img );
    painter->drawImage( a, m_ir_img );
    m_mutex.unlock();
}

//QSGNode *CImageItem::updatePaintNode(QSGNode *p_old_node, QQuickItem::UpdatePaintNodeData *)
//{
//    ImageNode *p_node = (ImageNode*)p_old_node;
//    if ( !p_node ) {
//        p_node = new ImageNode( window() );
//        p_node->removeAllChildNodes();
//        qDebug() << "p_node NULL";
//    }
//    int i_texture_id = p_node->get_material_texture_id();
//    glBindTexture( GL_TEXTURE_2D, i_texture_id );

//    QImage tem_img = QImage( m_ir_img );
//    if( m_ir_img.isNull() ) {
//        big_img = QImage(":/wheat.jpg").scaled( RIGHT_AREA_WIDTH, RIGHT_AREA_HIGHT );
//    } else {
//        big_img = QImage(  QSize( RIGHT_AREA_WIDTH, RIGHT_AREA_HIGHT ), QImage::Format_RGB32 );//.scaled( RIGHT_AREA_WIDTH, RIGHT_AREA_HIGHT );
//    }
//    QPainter painter( &big_img );
//    if( tem_img.isNull() ) {
//    } else {
//        painter.drawImage( QRect( 0, 0, tem_img.width(), tem_img.height() ), tem_img );
//    }
//    if( !big_img.isNull() ) {
//        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, RIGHT_AREA_WIDTH, RIGHT_AREA_HIGHT, GL_BGRA_EXT, GL_UNSIGNED_BYTE, big_img.constBits() );
//    }
//    p_node->setRect( boundingRect() );
//    return p_node;
////    return NULL;
//}

void CImageItem::img_trigged_slot( unsigned int puc_out_buf, int n_data_len )
{
    if( n_start_frame_count<=5 ) {
        n_start_frame_count++;
        return;
    }
    m_mutex.lock();
    qDebug()<<"img_trigged_slot====";
     if( n_frame_rate == 0 ) {
         n_prev_time = QDateTime::currentMSecsSinceEpoch();
         n_frame_rate++;
     } else {
         n_current_time = QDateTime::currentMSecsSinceEpoch();
         if( n_current_time - n_prev_time >= 1000 ) {
             emit show_frame_rate_info( QString("%1").arg( n_frame_rate ) );
             n_frame_rate = 0;
         } else {
             n_frame_rate++;
         }
     }
    qDebug()<<"n_frame_rate:  "<<n_frame_rate;
    for( int i=0; i<n_data_len; i++ ) {
        mp_buffer[i] = *( (unsigned char*)puc_out_buf + i );
    }
    m_ir_img =  QImage( (const uchar *)mp_buffer, 640, 480, QImage::Format_RGB32 );
    m_ir_img = QImage( (const uchar*)rgb_to_rgb(m_ir_img,640, 480 ), 640, 480, QImage::Format_RGB32 );
//    m_ir_img = m_ir_img.copy( 0, 0, m_ir_img.width(), m_ir_img.height() );
//    m_ir_img = m_ir_img.scaled( 640, 480 );
    update();
    m_mutex.unlock();
}

uchar* CImageItem::rgb_to_rgb( QImage &image32, int n_width, int n_height )
{
    uchar* imagebits_32 = (uchar* )image32.constBits();         //获取图像首地址，32位图
    int n_line_num = 0;
    int w_32 = image32.bytesPerLine();
    for( int i=0; i<n_height; i++ ) {
        n_line_num = i*w_32;
        for( int j=0; j<n_width; j++ ) {
            int r_32 = imagebits_32[ n_line_num+ j * 4 + 2 ];
            int g_32 = imagebits_32[ n_line_num+ j * 4 + 1 ];
            int b_32 = imagebits_32[ n_line_num+ j * 4 ];
            imagebits_32[ n_line_num+ j * 4 ] = r_32;
            imagebits_32[ n_line_num+ j * 4 + 2 ] = b_32;
        }
    }
    return imagebits_32;
//    return NULL;
}
