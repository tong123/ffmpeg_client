#include "wifi-client.h"
#include "NetDiscover.h"
#include <QDebug>
#include "event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QAbstractSocket>
#include <QNetworkInterface>
#include <QDateTime>
#define LINUXUSEDNET "wlan0"

#ifdef __cplusplus
extern "C"{
#endif

#include  <libavutil/avstring.h>
#include  <libavutil/opt.h>
#include  <libavutil/parseutils.h>
#include  <libavutil/pixdesc.h>
#include  <libavutil/frame.h>
#include  <libavutil/imgutils.h>
#include  <libavutil/samplefmt.h>
#include  <libavformat/avformat.h>
#include  <libavcodec/avcodec.h>
#include  <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

static char gs_start[] = "YFSTART";
static char gs_end[] = "YFEND";

WifiClient::WifiClient(QObject *parent) : QObject(parent)
{
    mn_width = QApplication::desktop()->width();
    mn_height = QApplication::desktop()->height();
    qDebug()<<mn_width<<mn_width;
    s_mac_ip_list.clear();
    m_discover_timer = NULL;
//    m_find_timer = new QTimer( this );
//    connect( m_find_timer, SIGNAL(timeout()), SLOT( ip_find_time_slot()) );
//    m_find_timer->start(1000);
//    av_register_all();
    server_callback_init( get_handle(), net_connect , this );
    m_discover_timer = new QTimer( this );
    connect( m_discover_timer, SIGNAL(timeout()), this, SLOT(find_online_device()) );
    m_discover_timer->start( 5000 );
    mp_read_thread = new ReadThread( this );
    mp_socket = new QTcpSocket( this );
    connect( mp_socket, SIGNAL(connected()), this, SLOT(connect_slot()) );
    connect( mp_socket, SIGNAL(disconnected()), this, SLOT(disconnect_slot()) );
    connect( mp_socket, SIGNAL(readyRead()),this, SLOT(data_receive()) );
    connect( mp_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(display_error(QAbstractSocket::SocketError)) );

    mn_receive_buf_len = BUFFER_SIZE;
    mn_receive_data_len = 0;
    mn_need_write_data_len = 0;
    mp_receive_buf = new char[ BUFFER_SIZE  ];
    mp_decode = new Decode_H264( this );
    mb_update_status = false;
//    mp_socket->connectToHost( "192.168.0.106", DATA_PORT );
}

void WifiClient::ip_find_time_slot()
{
    qDebug()<< "ip find~~~~";
    get_network_interface();
//    if( !m_network_ip.isEmpty() && mb_discover_status ){
    if( !m_network_ip.isEmpty() ) {
        QString str = QString("watch_dog[%1]").arg( m_network_mac );
        qDebug()<<m_network_ip<<m_network_mac;
        Client_init( mh_find_handle, str.toStdString() );
        m_find_timer->stop();
    }
}
void WifiClient::get_network_interface()
{
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    m_network_ip = "";
    QList < QNetworkInterface >  NetList; //网卡链表
    foreach(QHostAddress address,info.addresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            m_network_ip = address.toString();
        }
    }
    int NetCount = 0;   //网卡个数
    int Neti = 0;
    QNetworkInterface thisNet;  //所要使用的网卡
    NetList = QNetworkInterface::allInterfaces();//获取所有网卡信息
    NetCount = NetList.count();   //统计网卡个数
    for ( Neti = 0;Neti < NetCount; Neti++){ //遍历所有网卡
#if defined Q_OS_WIN32
        if( NetList[Neti].isValid() ){ //判断该网卡是否是合法
            thisNet = NetList[Neti]; //将该网卡置为当前网卡
            m_network_mac = NetList[Neti].hardwareAddress();
            break;
        }
#elif defined Q_OS_LINUX
        if( NetList[Neti].name().compare( LINUXUSEDNET ) == 0 ){
            thisNet = NetList[ Neti ];
            QList < QNetworkAddressEntry > entryList =thisNet.addressEntries();
            m_network_mac = NetList[ Neti ].hardwareAddress();
            foreach( QNetworkAddressEntry entry,entryList )
            {//遍历每一个IP地址条目
                if( entry.ip().protocol() == QAbstractSocket::IPv4Protocol ){
                    m_network_ip = entry.ip().toString();
                    break;
                }
            }
            break;
        }
#endif
    }
}

void WifiClient::data_receive( )
{
//    qDebug()<<"data_recieve";
    qint64 n_len;
    qint64 n_head_len = strlen( gs_start ) + sizeof(unsigned int);

    //qDebug() << "QtNetSocket::receive_data mp_receive_buf=" << (unsigned int)mp_receive_buf << " mn_receive_data_len=" << mn_receive_data_len << " mn_receive_buf_len=" << mn_receive_buf_len;
    if( mp_receive_buf && mn_receive_data_len < BUFFER_SIZE ) {
        n_len = mp_socket->read( &mp_receive_buf[mn_receive_data_len], BUFFER_SIZE - mn_receive_data_len );
        if( n_len == -1 ) {
            qDebug()<<"read error!";
            return;
        }
//        qDebug()<<n_len;
        mn_receive_data_len += n_len;
        if( mn_receive_data_len >= n_head_len ) {
            if( strncmp( (const char *)mp_receive_buf, gs_start, strlen(gs_start) ) != 0 ) {
                qDebug() << "Socket::receive_data not begin FSTART";
                mn_receive_data_len = 0;
                return;
            }
            qint64 n_process_len = 0;
            unsigned int n_data_len;
            memcpy( &n_data_len, &mp_receive_buf[n_process_len + strlen(gs_start)], sizeof(unsigned int) );

            while( n_head_len <= mn_receive_data_len-n_process_len
                   && ( n_data_len + n_head_len + strlen(gs_end) ) <= ( mn_receive_data_len-n_process_len ) ) {
                qint64 start = QDateTime::currentMSecsSinceEpoch();
//                if( mp_read_cb ) {
                    //qDebug() << "receive_data n_data_len=" << n_data_len;
//                    mp_read_cb( &mp_receive_buf[n_process_len+n_head_len], n_data_len, this, mp_user_data );
//                }
                qDebug()<<"mn_receive_data_len:=="<<mn_receive_data_len;
                read_cb_fuc_slot( (unsigned char*)&mp_receive_buf[n_process_len+n_head_len], n_data_len );
                qint64 end = QDateTime::currentMSecsSinceEpoch();
                qDebug()<<"interval_time: "<<( end-time_tmp );
                qDebug()<<mn_receive_data_len;
                time_tmp = end;
                n_process_len += n_head_len + n_data_len + strlen(gs_end);
                memcpy( &n_data_len, &mp_receive_buf[n_process_len + strlen(gs_start)], sizeof(unsigned int) );
            }

            //qDebug() << "receive_data n_process_len=" << n_process_len << " mn_receive_data_len=" << mn_receive_data_len;
            if( n_process_len == mn_receive_data_len ) {
                mn_receive_data_len = 0;
            } else if( n_process_len < mn_receive_data_len ) {
                memmove( mp_receive_buf, &mp_receive_buf[n_process_len], mn_receive_data_len-n_process_len );
                mn_receive_data_len -= n_process_len;
            }
        }
    }
}

void WifiClient::read_cb_fuc_slot( unsigned char *p_buf, unsigned long u_buf_len )
{
    if( !m_head.parse( (char*)p_buf, u_buf_len ) ) {
        return ;
    }
    if ( m_head.get_pag_type() == DataPackage ) {
        switch( m_head.get_main_cmd() ) {
        case	Y_AD_DATA:
            switch( m_head.get_vice_cmd()) {
                case    Y_AD_DATA_ZIP_COR:
//                  QString s = QString((char*)m_head.get_data_ptr());
                    mp_decode->on_decode_h264_packet( m_head.get_data_ptr(), m_head.get_data_len() );
                }
                break;
            default:
                break;
            }
    }
}

void WifiClient::set_ir_img_data( unsigned char *puc_out_buf, unsigned int ui_data_len )
{
//    qDebug()<<"wifi_client: "<<puc_out_buf<<ui_data_len;
//    emit set_ir_img_show( puc_out_buf, ui_data_len );
//    QString s = QString( QLatin1String((const char *)puc_out_buf) );
    emit image_changed( (unsigned int)puc_out_buf, ui_data_len );
//    emit ir_image_changed( puc_out_buf, ui_data_len );
}

unsigned int WifiClient::get_current_font_size( int n_index )
{
    if( n_index <0 || n_index > s_mac_ip_list.count() ) {
        return false;
    }
    int n_min_size = mn_width <mn_height ? mn_width:mn_height;
    int n_font_size = n_min_size/( s_mac_ip_list.at(n_index).count() );
    return n_font_size;
}

unsigned int WifiClient::get_desktop_width()
{
    return mn_width;
}

unsigned int WifiClient::get_desktop_height()
{
    return mn_height;
}

bool WifiClient::connect_to_current_online_device( int n_index )
{
    if( n_index <0 || n_index > s_mac_ip_list.count() ) {
        return false;
    }
    QString s_mac_ip = s_mac_ip_list.at( n_index );
    QString s_ip = s_mac_ip.mid( s_mac_ip.lastIndexOf(" ")+1, s_mac_ip.count()-1 );
    mp_host_address.setAddress(s_ip);
    mp_socket->connectToHost( mp_host_address, DATA_PORT );
    return true;
}

bool WifiClient::net_connect( const std::string& s_name, const std::string& s_ip, void *p_user_data )
{
    WifiClient *p_this = (WifiClient*)p_user_data;
    p_this->net_connect_data_slot(s_name, s_ip, p_user_data );
    qDebug()<<"net_connect=====";
    return true;
}

void WifiClient::net_connect_data_slot( const std::string& s, const std::string& ip, void * )
{
    QString s_name = QString::fromStdString( s );
    QString s_ip = QString::fromStdString( ip );
    qDebug()<<s_name;
    if( !s_name.startsWith("C600G") || s_name.size() < 23  ) {
        if( s_mac_ip_list.isEmpty() && !mb_update_status ) {
            emit update_discover_model();
            mb_update_status = true;
        }
        return;
    }
    QString s_mac = s_name.mid( 6, 22 );
    QString s_mac_ip = QString("%1 %2").arg( s_mac ).arg( s_ip );
    if( !s_mac_ip_list.contains( s_mac_ip ) ) {
        s_mac_ip_list<<s_mac_ip;
        emit update_discover_model();
    }
}

void WifiClient::find_online_device()
{
    s_mac_ip_list.clear();
    mb_update_status = false;
    server_find( get_handle() );
    qDebug()<<"find_online_device====";
//    emit update_discover_model();
//    if( s_mac_ip_list.isEmpty() ) {
//    emit update_discover_model();
//    }
}

void WifiClient::search_online_device()
{
    if( m_discover_timer != NULL ) {
        m_discover_timer->stop();
        find_online_device();
        qDebug()<<"search_online_device...";
        m_discover_timer->start(6000);
    }
}

int WifiClient::get_mac_ip_list_count()
{
    return s_mac_ip_list.count();
}

QString WifiClient::get_current_mac_ip_content( int n_index )
{
    if( n_index <0 || n_index > s_mac_ip_list.count() ) {
        return QString( "" );
    }
    return s_mac_ip_list.at( n_index );
}

bool WifiClient::connect_to_server( QString s_ip )
{
    mp_read_thread->s_server_ip = s_ip;
    mp_read_thread->start();
    qDebug()<<"finished";
    return true;
}

void WifiClient::server_msg_cb( struct bufferevent *bev, void* arg )
{
    char msg[1024];
    size_t n_len = bufferevent_read( bev, msg, sizeof(msg) );
    msg[n_len] = '\0';
}

void WifiClient::event_cb( struct bufferevent *bev, short event, void *arg )
{
    WifiClient *mp_this = ( WifiClient* )arg;
    if( event & BEV_EVENT_EOF ) {
        qDebug()<<"connection closed";
    } else if( event & BEV_EVENT_ERROR ) {
        emit mp_this->connect_to_server_result( false );
        qDebug()<<"some other error";
    } else if( event & BEV_EVENT_CONNECTED ) {
        emit mp_this->connect_to_server_result( true );
        qDebug()<<"the client has connected to server";
        return;
    }
    bufferevent_free( bev );
    struct event *ev = ( struct event *)arg;
    event_free( ev );
}

ReadThread::ReadThread( WifiClient *mp )
{
    mp_wifi_client = mp;
    base = NULL;
}

void ReadThread::run()
{
    base = event_base_new();
    bev = bufferevent_socket_new( base, -1, BEV_OPT_CLOSE_ON_FREE );
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( DATA_PORT );
    server_addr.sin_addr.s_addr = inet_addr( (const char *)s_server_ip.toLocal8Bit().data() );
    int n_connect_status = bufferevent_socket_connect( bev, (struct sockaddr *)&server_addr, sizeof(server_addr) );
    if( n_connect_status != 0 ) {
        qDebug()<<"connect server error!";
    }
    bufferevent_setcb( bev, mp_wifi_client->server_msg_cb, NULL, mp_wifi_client->event_cb, mp_wifi_client );
    bufferevent_enable( bev, EV_READ | EV_PERSIST );
    event_base_dispatch( base );
    qDebug()<<"disconnect...";
}

void WifiClient::connect_slot( )
{
    qDebug()<<"connect to server!";
    emit connect_to_server_result( true );
}

void WifiClient::disconnect_slot( )
{
    qDebug()<<"disconnect";
    emit disconnect_net_status();
}


void WifiClient::display_error(QAbstractSocket::SocketError)
{
    qDebug()<<"socket error!";
    emit connect_to_server_result( false );
}

void WifiClient::disconnect_current_server()
{
    mp_socket->disconnectFromHost();
}
