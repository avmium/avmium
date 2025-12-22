`ifndef __common_svh__
`define __common_svh__

import "DPI-C" function chandle blk_init(
	input string name
);

import "DPI-C" function void blk_deinit(
	input chandle blk
);

import "DPI-C" function chandle port_init(
	input chandle blk, 
	input string name,
	input int qsize,
	input byte port_type,
	input byte fwrd_as_rcvd,
	input byte addr_normalize,
	input chandle user_function, 
	input chandle resp_function, 
);

import "DPI-C" function void get_port(
	input chandle blk, 
	input string name
);

import "DPI-C" function int is_que_empty(
	input chandle opaque
);

import "DPI-C" function int port_register_action(
	input chandle opaque,
	input byte unsigned action_type,
	input longint unsigned start_addr,
	input longint unsigned end_addr,
	input chandle callback
);

import "DPI-C" function int port_status(
	input chandle opaque
);

import "DPI-C" function void route(
	input chandle blk
);
import "DPI-C" function int memmap_init(
	input chandle opaque,
	input longint unsigned addr,
	input int unsigned size,
	input byte unsigned types, 
	input chandle port
);
	
import "DPI-C" function int sv_get_send_data(
	input chandle opaque,
	inout shortint unsigned trid,
	output longint unsigned addr,
	output byte unsigned rw,
	output byte data[], 
	inout shortint unsigned size
);

import "DPI-C" function int set_memmap(
	input chandle opaque,
	input longint unsigned addr,
	input byte data[], 
	input shortint unsigned size,
	input byte unsigned rw,
	input byte unsigned check_port
);

enum {
	FORWARD,
	DISCARD,
	FUNC
} port_action_e;

enum {
	MASTER,
	SLAVE
} port_type_e;

enum {
	MEM,
	PORT
} memap_type_e;

`endif /* __common_svh__ */
