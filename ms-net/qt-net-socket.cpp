#include "qt-net-socket.h"
#include <QNetworkConfigurationManager>
#include <QDateTime>


//“YFSTART”+data_len+data+“YFEND”
static char gs_start[] = "YFSTART";
static char gs_end[] = "YFEND";
//typedef struct {
//    unsigned char s_start[strlen(gs_start)];
//    unsigned long dw_data_len;
//} s_pack_head;

QtNetSocket::QtNetSocket(bool b_client, QTcpSocket *p_sock, QObject *parent) :
    QObject(parent)//, mb_client( b_client )
{
    mb_connected = false;

    mp_read_cb = NULL;
    mp_user_data = NULL;
    ms_ip = "";
    mn_port = 0;
    time_tmp = 0;

    mn_receive_buf_len = 0;
    mp_receive_buf = NULL;
    mn_receive_data_len = 0;
    mn_need_write_data_len = 0;
    if( b_client ) {
        if( p_sock ) {
            mp_socket = p_sock;
            qDebug() << "p_sock:" << mp_socket->peerAddress() << mp_socket->peerPort();
        } else{
            mp_socket = new QTcpSocket();
        }

        if( mp_socket ) {
//            mp_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, QVariant(100000));
            connect( mp_socket, SIGNAL(readyRead()), this, SLOT(receive_data()) );
            connect( mp_socket, SIGNAL(disconnected()), this, SLOT(disconnected_slot()) );
            connect( mp_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error_slot(QAbstractSocket::SocketError)) );
            connect( mp_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytes_written_slot(qint64) ) );
        }
        mp_server = NULL;
    } else {
        mp_socket = NULL;
        mp_server = new QTcpServer();
        connect( mp_server, SIGNAL(newConnection()), this, SLOT(new_connect_slot()) );
        connect( mp_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(server_error_slot(QAbstractSocket::SocketError)) );
    }
}

QtNetSocket::~QtNetSocket()
{
    free_recv_buf();
    if( mp_socket ) {
        mp_socket->close();
        delete mp_socket;
        mp_socket = NULL;
    }
    if( mp_server ) {
        mp_server->close();
        delete mp_server;
        mp_server = NULL;
    }
    if ( mp_receive_buf ) {
        delete mp_receive_buf;
        mp_receive_buf = NULL;
    }
}

bool QtNetSocket::stop_server()
{
    if( mp_server ) {
        qDebug() << "stop_server(); s";
        while( !m_client_list.isEmpty() ) {
            QtNetSocket *p_client = m_client_list.takeFirst();
            p_client->disconnect();
            delete p_client;
            p_client = NULL;
        }
        qDebug() << "stop_server(); e";
        mp_server->close();
        return true;
    }
    return false;
}

bool QtNetSocket::disconnect()
{
    if( mp_socket ) {
        qDebug() << "disconnect() s";
        mp_socket->disconnectFromHost();
        if ( mp_socket->state() == QAbstractSocket::UnconnectedState ||
                mp_socket->waitForDisconnected( 1000 ) ) {
            ms_ip = "";
            qDebug() << "disconnect() t";

            return true;
        }
        qDebug() << "disconnect() f";
    }
    return false;
}

bool QtNetSocket::send_data(QtNetSocket *p_net, YHANDLE h_data_buf, unsigned long u_buf_len)
{
    if( p_net->mp_socket ) {
        qint64 n_write_buf_len = p_net->mn_receive_buf_len >> 1;
        if( p_net->mn_need_write_data_len >= n_write_buf_len ) {
            qDebug() << "QtNetSocket::send_data buffer is full; mn_need_write_data_len=" << p_net->mn_need_write_data_len;
            //p_net->mn_need_write_data_len = 0;
            return false;
        }
        //qDebug() << "QtNetSocket::send_data sock state=" << p_net->mp_socket->state() << " ms_ip=" << p_net->ms_ip << "  write len=" << u_buf_len;
        if( p_net->mp_socket->state() == QAbstractSocket::UnconnectedState && !p_net->ms_ip.isEmpty() ) {
            qDebug() << "QtNetSocket::send_data connect server port=" << p_net->mn_port << " mp_read_cb=" <<p_net->mp_read_cb << "  mn_receive_buf_len=" << p_net->mn_receive_buf_len;
            if ( yf_net_socket_connect_server( p_net, p_net->ms_ip, p_net->mn_port, p_net->mp_read_cb, p_net->mp_user_data, p_net->mn_receive_buf_len ) == false) {
                qDebug() << "connect_server error";
                return false;
            }
        }

        if ( p_net->mp_socket->state() == QAbstractSocket::ConnectedState || p_net->mp_socket->waitForConnected( 1000 ) ) {
            //qint64 n_writed = 0, n_tmp;
            //char * p_data = (char *)h_data_buf;
//            s_pack_head *p_head = ( s_pack_head * )h_data_buf;
//            if( p_head->dw_data_len + sizeof( s_pack_head ) != u_buf_len ) {
//                qDebug() << "QtNetSocket::send_data p_head->dw_data_len=" << p_head->dw_data_len << " u_buf_len=" << u_buf_len;
//                p_head->dw_data_len = u_buf_len - sizeof( s_pack_head );
//            }
            int sendsize = strlen( gs_start ) + sizeof( u_buf_len )  + u_buf_len + strlen( gs_end );
            qDebug() << sendsize;
            p_net->mn_need_write_data_len += sendsize;
            char *p_data = new char[ sendsize ];
            memcpy ( p_data, gs_start , strlen( gs_start ) );
            //p_net->mp_socket->write( gs_start, strlen( gs_start ) );
            memcpy ( (void *)( p_data + strlen( gs_start ) ) , &u_buf_len , sizeof( u_buf_len ) );
            memcpy ( (void *)( p_data + strlen( gs_start ) + sizeof( u_buf_len ) ), h_data_buf , u_buf_len );
            memcpy ( (void *)( p_data + strlen( gs_start ) + sizeof( u_buf_len ) + u_buf_len ), gs_end, strlen( gs_end ));
            //p_net->mp_socket->write( (const char *)&u_buf_len, sizeof( u_buf_len ) );

            //p_net->mp_socket->write( gs_end, strlen(gs_end) );
            int i_write = 0;
            while( i_write != sendsize){
                 i_write += p_net->mp_socket->write( p_data + i_write, sendsize - i_write);
                 if ( p_net->mp_socket->state() == QAbstractSocket::ConnectedState ) {
                     while (p_net->mp_socket->waitForBytesWritten( 500 ) ) { }
                     //p_net->mp_socket->flush();
                 } else {
                     p_net->mp_socket->close();
                     if( p_data ){
                         delete []p_data;
                         p_data = NULL;
                     }
                     return false;
                 }
                 qDebug() << "waitForBytesWritten end" << i_write;
            }
            if( p_net->mp_socket->state() != QAbstractSocket::ConnectedState ){
                p_net->mp_socket->close();
                qDebug() << "close()";
            }
            if( p_data ){
                delete []p_data;
                p_data = NULL;
                qDebug() << "delete OK";
            }
            return true;
            /*while ( true ) {
                n_tmp = p_net->mp_socket->write( &p_data[n_writed], u_buf_len-n_writed );
                qDebug() << n_writed << u_buf_len ;
                qDebug() << "QtNetSocket::send_data write bytes=" << n_tmp ;
                if( n_tmp == -1 || n_tmp == 0 ) {
                    qDebug() << "QtNetSocket::send_data send data is zero";
                    break;
                } else {
                    n_writed += n_tmp;
                    p_net->mn_need_write_data_len += n_tmp;
                    qDebug() << "QtNetSocket::send_data mn_need_write_data_len=" << p_net->mn_need_write_data_len;
                    if( n_writed == u_buf_len ) {
                        if( n_tmp != strlen( gs_end ) ) {
                            qDebug() << "QtNetSocket::send_data send gs_end fail n_tmp=" << n_tmp;
                            return false;
                        }
                        qDebug() << "Socket start";
                        p_net->mn_need_write_data_len += n_tmp;
                        int i = 0;
                        while( p_net->mp_socket &&
                               p_net->mp_socket->state() == QAbstractSocket::ConnectedState
                               &&
                               ) {
                            qDebug() << i++;
                        }
                        qDebug() << "Socket ok";

                        return true;
                    }

                }
            }*/
        }
    }

    return false;
}

void QtNetSocket::free_recv_buf()
{
    if( mp_receive_buf ) {
        qDebug() << 2;
        delete []mp_receive_buf;
        mp_receive_buf = NULL;
    }
}

void QtNetSocket::receive_data()
{
    qint64 n_len, n_head_len = strlen( gs_start ) + sizeof(unsigned int);

    //qDebug() << "QtNetSocket::receive_data mp_receive_buf=" << (unsigned int)mp_receive_buf << " mn_receive_data_len=" << mn_receive_data_len << " mn_receive_buf_len=" << mn_receive_buf_len;
    if( mp_receive_buf && mn_receive_data_len < mn_receive_buf_len ) {
        mp_socket->setReadBufferSize(1000000);

        n_len = mp_socket->read( &mp_receive_buf[mn_receive_data_len], mn_receive_buf_len-mn_receive_data_len );
        //qDebug() << "QtNetSocket::receive_data read n_len=" << n_len << " mn_receive_data_len=" << mn_receive_data_len;
        if( n_len == -1 ) {
            return ;
        }
//        qDebug()<<n_len;
        mn_receive_data_len += n_len;
        if( mn_receive_data_len >= n_head_len ) {
            if( strncmp( (const char *)mp_receive_buf, gs_start, strlen(gs_start) ) != 0 ) {
                qWarning() << "QtNetSocket::receive_data not begin FSTART";
                mn_receive_data_len = 0;
                return ;
            }
            qint64 n_process_len = 0;
            unsigned long n_data_len;
            memcpy( &n_data_len, &mp_receive_buf[n_process_len + strlen(gs_start)], sizeof(unsigned long) );

            while( n_head_len <= mn_receive_data_len-n_process_len //assure the p_head is valid
                   && n_data_len + n_head_len + strlen(gs_end) <= mn_receive_data_len-n_process_len ) {

                //qDebug() << "QtNetSocket::receive_data mp_read_cb=" << (unsigned int)mp_read_cb << " mp_user_data=" << mp_user_data;
//                if( mn_receive_data_len == 795 ) {
//                    qDebug() << "main_cmd=" << p_head->by_main_cmd << " vice_cmd=" << p_head->by_vice_cmd << " len=" << p_head->dw_data_len << " seq=" << p_head->dw_seq;
//                }
                qint64 start = QDateTime::currentMSecsSinceEpoch();
                if( mp_read_cb ) {
                    //( char *p_buf, unsigned long u_buf_len, YHANDLE h_peer, void *p_user_data );
                    //qDebug() << "QtNetSocket::receive_data n_data_len=" << n_data_len;
                    mp_read_cb( &mp_receive_buf[n_process_len+n_head_len], n_data_len, //
                            this, mp_user_data );
                }
                qint64 end = QDateTime::currentMSecsSinceEpoch();
                qDebug()<<"interval_time: "<<( end-time_tmp );
                qDebug()<<mn_receive_data_len;
                time_tmp = end;
                n_process_len += n_head_len + n_data_len + strlen(gs_end);
                //p_head = ( s_pack_head * )&mp_receive_buf[n_process_len];
                memcpy( &n_data_len, &mp_receive_buf[n_process_len + strlen(gs_start)], sizeof(unsigned int) );
                //qDebug() << "n_process_len=" << n_process_len << " mn_receive_data_len=" << mn_receive_data_len << " n_data_len=" << n_data_len;
            }

            //qDebug() << "QtNetSocket::receive_data n_process_len=" << n_process_len << " mn_receive_data_len=" << mn_receive_data_len;
            if( n_process_len == mn_receive_data_len ) {
                mn_receive_data_len = 0;
            } else if( n_process_len < mn_receive_data_len ) {
                memmove( mp_receive_buf, &mp_receive_buf[n_process_len], mn_receive_data_len-n_process_len );
                mn_receive_data_len -= n_process_len;
            }
        }
    }
//    else {
//        qDebug() << "QtNetSocket::receive_data mn_receive_data_len=" << mn_receive_data_len << " mn_receive_buf_len=" << mn_receive_buf_len << " Error";
//    }
}

void QtNetSocket::disconnected_slot()
{
    mn_receive_data_len = 0;
}

void QtNetSocket::new_connect_slot()
{
    for( int i=0; i<m_client_list.size(); ++i ) {
        QtNetSocket *p_net = m_client_list.at( i );
        if( p_net->mp_socket && p_net->mp_socket->isValid() ) {

        } else {
            delete p_net;
            p_net = NULL;
            m_client_list.removeAt( i );
            qDebug() << "QtNetSocket::new_connect_slot delete the socket index=" << i;
            --i;
        }
    }

    QtNetSocket *p_net = new QtNetSocket( true, mp_server->nextPendingConnection() );
    p_net->mn_receive_buf_len = mn_receive_buf_len;
    p_net->mp_read_cb = mp_read_cb;
    if ( p_net->mp_receive_buf != NULL){
        delete p_net->mp_receive_buf;
        p_net->mp_receive_buf = NULL;
    }
    p_net->mp_receive_buf = new char[mn_receive_buf_len];
    p_net->mp_user_data = mp_user_data;
    m_client_list.clear();
    m_client_list.append( p_net );
    p_net->mb_connected = true;
    //if(  )
    qDebug() << "QtNetSocket::new_connect_slot clients=" << m_client_list.size() << " mn_receive_buf_len=" << mn_receive_buf_len;
    qDebug() << "QtNetSocket::new_connect_slot mp_read_cb=" << &mp_read_cb << " mp_receive_buf=" << p_net->mp_receive_buf << " mp_user_data=" << mp_user_data;
}


void QtNetSocket::error_slot(QAbstractSocket::SocketError err)
{
    if(err == QTcpSocket::RemoteHostClosedError)
            return;
    qDebug() << "QtNetSocket::error_slot err=" << mp_socket->errorString();
//    if( mp_socket ) {
//        this->mp_socket->
    //    }
}

void QtNetSocket::server_error_slot(QAbstractSocket::SocketError err)
{
    if(err == QTcpSocket::RemoteHostClosedError)
            return;
    qDebug() << "QtNetSocket::server_error_slot errString=" << mp_server->errorString();
}

void QtNetSocket::bytes_written_slot(qint64 n_size)
{
    mn_need_write_data_len -= n_size;
    qDebug() << "QtNetSocket::bytes_written_slot writed n_size=" << n_size << " mn_need_write_data_len=" << mn_need_write_data_len;
}


YFDLL_YFNETSOCKET_API yf_net_socket_get_last_error( YHANDLE handle, unsigned int &error ) {
    qDebug() << "yf_net_socket_get_last_error";
    if( handle ) {
        QtNetSocket *p_net = ( QtNetSocket * )handle;
        if( p_net->mp_server ) {
            //error = p_net->mp_server->errorString();
        } else if( p_net->mp_socket ){
            error = p_net->mp_socket->error();
            qDebug() << "yf_net_socket_get_last_error true" ;
            return true;
        }
    }
    qDebug() << "yf_net_socket_get_last_error false";
    return false;
}

//网络库服务端初始化
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_server_init( YHANDLE& h ) {
    qDebug() << "yf_net_socket_tcp_server_init";
    QtNetSocket *p_net = new QtNetSocket( false );
    if( p_net ) {
        h = p_net;
        qDebug() << "yf_net_socket_tcp_server_init return true";
        return true;
    } else {
        qDebug() << "yf_net_socket_tcp_server_init return false";
        return false;
    }
}

//网络库结束
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_server_un_init( YHANDLE& h ) {
    qDebug() << "yf_net_socket_tcp_server_un_init";
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        p_net->stop_server();
        delete p_net;
        p_net = NULL;
        h = NULL;
        qDebug() << "yf_net_socket_tcp_server_un_init true";
        return true;
    } else {
        return false;
        qDebug() << "yf_net_socket_tcp_server_un_init false" ;
    }
}

//启动网络库服务端(start_server_w:支持宽字符服务端ip,start_server_a:支持窄字符服务端ip)
//h:网络库句柄
//s_ip:服务端ip地址
//i_port:服务端监听端口
//p_read_cb_func:服务端收到客户端发送的完整数据时的回调函数
//p_user_data:用户自定义数据
//u_max_recv_packet_len:网络数据接收缓冲区最大长度
YFDLL_YFNETSOCKET_API yf_net_socket_start_server( YHANDLE h, const QString& s_ip, int i_port,
                                                    READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        if( p_net->mp_server ) {
            bool b_ret = p_net->mp_server->listen( QHostAddress( s_ip ), i_port );
            qDebug() << "yf_net_socket_start_server listen s_ip=" << s_ip << " i_port=" << i_port << " b_ret=" << b_ret;
            p_net->mp_read_cb = p_read_cb_func;
            p_net->mp_user_data = p_user_data;
            p_net->mn_receive_buf_len = u_max_recv_packet_len;
            //qDebug() << "yf_net_socket_start_server return b_ret=" << b_ret;
            return b_ret;
        }
    }

    return false;
}

YFDLL_YFNETSOCKET_API yf_net_socket_start_server_w( YHANDLE h, const std::wstring& s_ip, int i_port,
                                                        READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len ) {
    QString s_tip = QString::fromStdWString( s_ip );
    return yf_net_socket_start_server( h, s_tip, i_port, p_read_cb_func, p_user_data, u_max_recv_packet_len );
}

YFDLL_YFNETSOCKET_API yf_net_socket_start_server_a( YHANDLE h, const std::string& s_ip, int i_port,
                                                        READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len ) {
    QString s_tip = QString::fromStdString( s_ip );
    return yf_net_socket_start_server( h, s_tip, i_port, p_read_cb_func, p_user_data, u_max_recv_packet_len );
}

//获取已连接的客户端个数
//h:网络库句柄
//n_cnt:已连接的客户端个数
YFDLL_YFNETSOCKET_API yf_net_socket_get_client_count( YHANDLE h, int &n_cnt ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        if( p_net->mp_server ) {
            n_cnt = p_net->m_client_list.size();
            return true;
        }
    }

    return false;
}

//获取指定序号的已连接客户端的地址
//h:网络库句柄
//u_index:已连接的客户端序号(从0开始,如果有4个已连接客户端,则序号分别是0,1,2,3)
//h_peer:指定已连接客户端的地址
YFDLL_YFNETSOCKET_API yf_net_socket_get_peer( YHANDLE h, unsigned long u_index, YHANDLE &h_peer ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        if( p_net->mp_server && u_index < p_net->m_client_list.size() ) {
            h_peer = p_net->m_client_list.at( u_index );
            return true;
        }
    }

    return false;
}

//停止服务端
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_stop_server( YHANDLE h ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        return p_net->stop_server();
    }

    return false;
}

//向指定网络端发送数据
//h:网络库句柄
//h_peer:数据发送目的网络端地址
//h_data_buf:发送数据存放区
//u_buf_len:发送数据长度
YFDLL_YFNETSOCKET_API yf_net_socket_send_data( YHANDLE h, YHANDLE h_peer, YHANDLE h_data_buf, unsigned long u_buf_len ) {

    if( h && h_peer ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
            //return false;
        qDebug() << "yf_net_socket_send_data 1";
        return p_net->send_data( ( QtNetSocket * )h_peer, h_data_buf, u_buf_len );
    }
    return false;
}

//判断是否可以向指定网络端发送数据
//h:网络库句柄
//h_peer:发送数据的目的端地址
YFDLL_YFNETSOCKET_API yf_net_socket_is_peer_writable( YHANDLE h, YHANDLE h_peer ) {
    qDebug() << "yf_net_socket_is_peer_writable";
    if( h && h_peer ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        for( int i=0; i<p_net->m_client_list.size(); ++i ) {
            QtNetSocket *p_clie = p_net->m_client_list.at( i );
            if( (YHANDLE)p_clie == h_peer ) {
                if( p_clie->mp_socket && p_clie->mp_socket->isWritable() ) {
                    qDebug() << "yf_net_socket_is_peer_writable true";
                    return true;
                } else {
                    qDebug() << "yf_net_socket_is_peer_writable false";
                    return false;
                }
            }
        }

    }
    return false;
}

//网络库客户端初始化
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_client_init( YHANDLE& h ) {
    QtNetSocket *p_net = new QtNetSocket( true );
    if( p_net ) {
        h = p_net;
        return true;
    } else {
        return false;
    }
}

//网络库客户端结束
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_client_un_init( YHANDLE& h ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        p_net->disconnect();
        delete p_net;
        p_net = NULL;
        h = NULL;
        return true;
    } else {
        return false;
    }
}

YFDLL_YFNETSOCKET_API yf_net_socket_connect_server( YHANDLE h, QString& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len ) {

    qDebug() << "yf_net_socket_connect_server";
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        if( p_net->mp_socket ) {
            //p_net->mp_socket-
            p_net->mp_read_cb = p_read_cb;
            p_net->mp_user_data = p_user_data;
            if( p_net->mp_receive_buf && u_max_packet_len > p_net->mn_receive_buf_len ) {
                p_net->free_recv_buf();
            }
            if( !p_net->mp_receive_buf ) {
                p_net->mp_receive_buf = new char[ u_max_packet_len ];
                if( !p_net->mp_receive_buf ) {
                    qDebug() << "yf_net_socket_connect_server u_max_packet_len=" << u_max_packet_len << " memory shortage";
                    return false;
                }
                p_net->mn_receive_buf_len = u_max_packet_len;
            }
            p_net->mp_socket->connectToHost( s_server_ip, i_port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol );
            if ( p_net->mp_socket->waitForConnected( 1000 ) ) {
                p_net->ms_ip = s_server_ip;
                p_net->mn_port =i_port;
                qDebug() << "yf_net_socket_connect_server s_server_ip=" << s_server_ip << " i_port=" << i_port << " connected";
                return true;
            }
        }
    }

    return false;
}

//客户端连接服务端函数(connect_server_w:支持宽字符服务端ip,connect_server_a:支持窄字符服务端ip)
//h:网络库句柄
//s_server_ip:连接服务端的ip
//i_port:连接服务端的端口
//p_read_cb:客户端收到服务端完整数据时的回调函数
//p_user_data:用户自定义数据
//u_max_recv_packet_len:网络数据接收缓冲区最大长度
YFDLL_YFNETSOCKET_API yf_net_socket_connect_server_w( YHANDLE h, const std::wstring& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len ) {
    QString s_ip = QString::fromStdWString( s_server_ip );
    return yf_net_socket_connect_server( h, s_ip, i_port, p_read_cb, p_user_data, u_max_packet_len );
}

YFDLL_YFNETSOCKET_API yf_net_socket_connect_server_a( YHANDLE h, const std::string& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len ) {
    QString s_ip = QString::fromStdString( s_server_ip );
    return yf_net_socket_connect_server( h, s_ip, i_port, p_read_cb, p_user_data, u_max_packet_len );
}

//断开与服务端的连接
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_disconnect_server( YHANDLE h ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        return p_net->disconnect();
    }

    return false;
}

//往服务端发送数据
//h:网络库句柄
//H_data_buf:发送数据存放区
//u_buf_len:发送数据长度
YFDLL_YFNETSOCKET_API yf_net_socket_send_data_to_server( YHANDLE h, YHANDLE h_data_buf, unsigned long u_buf_len ) {
    if( h ) {
        QtNetSocket *p_net = ( QtNetSocket * )h;
        qDebug() << "yf_net_socket_send_data_to_server";
        return p_net->send_data( p_net, h_data_buf, u_buf_len );
    }

    return false;
}
