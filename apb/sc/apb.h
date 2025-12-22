#ifndef __apb_h__
#define __apb_h__

#if SVSIM
#include "svdpi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum apb_os_e {
    IDLE,
    SETUP,
    ACCESS
};

uint64_t apb_get_addr(struct port *port, uint64_t addr);
void apb_process_pready(struct port *port, uint64_t addr, uint8_t *data,
		uint16_t size, uint8_t rw, uint8_t slverr);
#if SVSIM
void sv_apb_process_pready(struct port *port, uint64_t addr,
		const svOpenArrayHandle data, uint16_t size, uint8_t rw,
		uint8_t slverr);
void sv_apb_process_slave_pready(struct port *port, const svOpenArrayHandle data,
	uint16_t size, uint8_t *wait_for_response, uint8_t *pready);
void sv_apb_slave_process_access(struct port *port, uint8_t *state, uint8_t pwrite,
		uint64_t addr, const svOpenArrayHandle data, uint16_t size, uint8_t *pready,
		uint8_t *pslverr, uint8_t *wait_for_response);
#endif
uint8_t apb_process_slave_pready(struct port *port, uint8_t *data, uint16_t size,
		uint8_t *wait_for_response, uint8_t *pready);
void apb_slave_process_access(struct port *port, uint8_t *state, uint8_t pwrite,
		uint64_t addr, uint8_t *data, uint16_t size, uint8_t *pready,
		uint8_t *pslverr, uint8_t *wait_for_response);

#ifdef __cplusplus
}
#endif

#endif // __apb_h__
