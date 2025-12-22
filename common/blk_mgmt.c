#include <stdlib.h>
#include <string.h>
#include "blk_mgmt.h"
#include "mem_ctrl.h"

struct blk* blk_init(const char *name)
{
	struct blk *blk = (struct blk *) calloc(1, sizeof(struct blk));
	if(blk == NULL)
		return NULL;

	blk->name = strdup(name);
	return blk;
}

void blk_deinit(struct blk *blk)
{
	memmap_deinit(blk);
	free(blk);
}
