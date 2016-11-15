/*
 * NONCE_GENERATION BLOCK IMPLENTATION
 */


#include "ra_nonce_generation.h"

#include <openssl/rand.h>
#include <string.h>
#include "ra_nonce_interpretation.h"


/************************************************************************/
/*				NONCE GENERATION IMPLEMENTATION 						*/
/************************************************************************/


/*
 * generated_nonce must be allocated elsewhere with required_size byte space
 */
RA_RESULT random_generate_nonce_NAYjDD3l2s(uint8_t *generated_nonce, uint32_t required_size)
{
	if(!RAND_bytes(generated_nonce,required_size))
		return RA_ERROR_RANDOM_GENERATOR;
	return RA_SUCCESS;
}

/************************************************************************/

