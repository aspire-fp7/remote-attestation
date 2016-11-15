/*
 * 	ATTESTER INTERFACE
 */

#ifndef ATTESTATOR_H_
#define ATTESTATOR_H_

#include <ra_print_wrapping.h>

/* The constructor attribute indicates this function must be called as an initialization routine at startup */
int attestator_init_NAYjDD3l2s() __attribute__((constructor(#ATTESTATOR_PRIORITY#)));

void attestator_deinit_NAYjDD3l2s();// __attribute__((destructor(101)));


#endif /* ATTESTATOR_H_ */
