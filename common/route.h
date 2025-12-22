#ifndef __route_h__
#define __route_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "blk_mgmt.h"

void route(struct blk *blk);
void master_route_resp(struct port *p);

#ifdef __cplusplus
}
#endif

#endif // __route_h__
