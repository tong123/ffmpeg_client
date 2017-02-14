#include "IRProtocol.h"
#include "string.h"
#include "msendian.h"
CIRProtocol::CIRProtocol(void):mp_netpag( NULL )
    ,mn_buf_len( 1228820 )
    ,mn_pag_len( sizeof( NetPackageHead ) )
{
    allot_buf( mn_buf_len );
}


CIRProtocol::~CIRProtocol(void)
{
    delete_buf();
}


bool CIRProtocol::parse( char* p_data, unsigned int n_len, bool b_net )
{
    if ( mn_buf_len < n_len )	{
        allot_buf( n_len );
        mn_buf_len = n_len;
    }
    memcpy( reinterpret_cast<void*>( mp_netpag ), reinterpret_cast<void*>( p_data ), n_len );
    if ( b_net ){
        mp_netpag->s_pag_head.dw_seq = net_to_host_l( mp_netpag->s_pag_head.dw_seq );
        mp_netpag->s_pag_head.dw_data_len = net_to_host_l( mp_netpag->s_pag_head.dw_data_len );
    }

    return true;
}

void CIRProtocol::pag_head_to_net()
{
    mp_netpag->s_pag_head.dw_seq = host_to_net_l( mp_netpag->s_pag_head.dw_seq );
    mp_netpag->s_pag_head.dw_data_len = host_to_net_l( mp_netpag->s_pag_head.dw_data_len );
}

float CIRProtocol::s_to_f( unsigned short s_val )
{
    return float ( ( float )s_val / 100 - 100 );
}


unsigned short CIRProtocol::f_to_s( float f_val )
{
    return (unsigned short) ( ( f_val + 100 ) * 100 );
}


void CIRProtocol::set_data_len( unsigned int dw_val )
{
    if ( mn_buf_len < ( dw_val + sizeof( NetPackageHead ) ) ){
        allot_buf( dw_val + sizeof( NetPackageHead ) );
    }
    mp_netpag->s_pag_head.dw_data_len = dw_val;
    mn_buf_len = dw_val;
    mn_pag_len = dw_val + sizeof( NetPackageHead );
}


void CIRProtocol::allot_buf( unsigned int n_len )
{
    PNetPackage p_old = mp_netpag;
    mp_netpag = reinterpret_cast <PNetPackage>( new char[n_len] );
    mn_buf_len = n_len;
    if ( p_old != NULL ){
        mp_netpag->s_pag_head = p_old->s_pag_head;
        delete []reinterpret_cast <char*>( p_old );
    }
}

void CIRProtocol::delete_buf()
{
    if ( mp_netpag != NULL ){
        delete []reinterpret_cast <char*>( mp_netpag );
        mp_netpag = NULL;
    }
}
