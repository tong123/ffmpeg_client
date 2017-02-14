#ifndef _IRPROTOCOL_H_
#define _IRPROTOCOL_H_

#include "ir_protocol_def.h"
#pragma pack(push)
#pragma pack(1)
typedef struct tagNetPackageHead{
	unsigned char by_package_type;
    unsigned char by_main_cmd;			///< NetMainCmd
    unsigned char by_vice_cmd;			///< NetViceCmd
	unsigned char by_response_code;
	unsigned int dw_seq;
	unsigned int dw_data_len;
}NetPackageHead, *PNetPackageHead;

typedef struct tagNetPackage{
	NetPackageHead s_pag_head;
	unsigned char by_data[1];
}NetPackage, *PNetPackage;

#pragma pack(pop)

class CIRProtocol
{
public:
	CIRProtocol(void);
	~CIRProtocol(void);

    bool parse( char* p_data, unsigned int n_len, bool b_net = true );
	/*PNetPackage get_package(){
	return mp_netpag;
	}*/
    //void set_data( void * p_data ) { memcpy( this->get_data_ptr(), p_data, this->get_data_len() ); }
	unsigned char get_pag_type(){
		return mp_netpag->s_pag_head.by_package_type;
	}
	void set_pag_type( unsigned char by_val ){
		mp_netpag->s_pag_head.by_package_type = by_val;
	}

	unsigned char get_main_cmd(){
		return mp_netpag->s_pag_head.by_main_cmd;
	}
	void set_main_cmd( unsigned char by_val ){
		mp_netpag->s_pag_head.by_main_cmd = by_val;
	}

	unsigned char get_vice_cmd(){
		return mp_netpag->s_pag_head.by_vice_cmd;
	}
	void set_vice_cmd( unsigned char by_val ){
		mp_netpag->s_pag_head.by_vice_cmd = by_val;
	}

	unsigned char get_response_code(){
		return mp_netpag->s_pag_head.by_response_code;
	}
	void set_response_code( unsigned char by_val ){
		mp_netpag->s_pag_head.by_response_code = by_val;
	}

	unsigned int get_seq(){
		return mp_netpag->s_pag_head.dw_seq;
	}
    void set_seq( unsigned int dw_val ){
		mp_netpag->s_pag_head.dw_seq = dw_val;
	}

    unsigned long get_data_len(){
		return mp_netpag->s_pag_head.dw_data_len;
	}
    void set_data_len( unsigned int dw_val );

	unsigned char* get_data_ptr(){
		return mp_netpag->by_data;
	}
	unsigned char* get_pag_data_ptr(){
		return ( unsigned char* )mp_netpag;
	}
    unsigned long get_pag_len(){
		return mn_pag_len;
    }

	void pag_head_to_net();
	float s_to_f( unsigned short s_val );
	unsigned short f_to_s( float f_val );

private:
    void allot_buf( unsigned int n_len );
	void delete_buf();

private:
	PNetPackage mp_netpag;
    unsigned long mn_buf_len;
    unsigned long mn_pag_len;
};

#endif
