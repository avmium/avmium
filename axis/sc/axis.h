#ifndef __axis_h__
#define __axis_h__

#if SVSIM
#include "svdpi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void axis_process_resp(void *opaque, uint16_t trid);
#if SVSIM
int sv_axis_process_tvalid(void *opaque, uint64_t addr,
		const svOpenArrayHandle data, uint16_t size, uint8_t tlast);
#endif
int axis_process_tvalid(void *opaque, uint64_t addr, uint8_t *data,
		uint16_t size, uint8_t tlast);

#ifdef __cplusplus
}
#endif

#endif // __axis_h__
