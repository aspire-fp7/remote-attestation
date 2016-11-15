/*
 * DO_HASH BLOCK INTERFACE
 *
 * HASH{ nonce || prepared_data_buffer || AID || attestator number }
 *
 */

#ifndef DO_HASH_H_
#define DO_HASH_H_

#include <inttypes.h>

#include <ra_results.h>

#include "ra_print_wrapping.h"

/*!
 * @brief ra_do_hash_NAYjDD3l2s* packages the attestation data
 *
 * ra_do_hash_NAYjDD3l2s generates the hashed buffer to
 * be used for attestation response and verification.
 * The output of ra_do_hash_NAYjDD3l2s if computed as:
 * @f[
 *    \mathit{HASH_{\left[ key \right]}} \left(  \mathit{nonce\_buffer} \: \| \:
 *    \mathit{nonce\_size} \: \| \:
 *    \mathit{data\_buffer} \: \| \:
 *    \mathit{data\_size} \: \| \:
 *    \mathit{AID} \: \| \:
 *    \mathit{attestator\_number} \right)
 * @f]
 *
 *
 *
 * @param[in] nonce_buffer
 *      The pointer to the buffer containing the nonce for @f$ \mathit{HASH} @f$ function.
 *
 * @param[in] nonce_size
 *      The number of bytes of the given nonce buffer.
 *
 * @param[in] data_buffer
 *      The pointer to the buffer containing the data for @f$ \mathit{HASH} @f$ function.
 *
 * @param[in] data_size
 *      The number of bytes of the given data buffer.
 *
 * @param[in] AID
 *      The pointer to the buffer containing the application ID (in binary form, NO string) for @f$ \mathit{HASH} @f$ function.
 *      The size is assumed to be #AID_SIZE.
 *
 * @param[in] attestator_number
 *      The number of the attestator to include in @f$ \mathit{HASH} @f$ function.
 *
 * @param[in] key
 *      Key to be used in case of keyed @f$ \mathit{HASH} @f$ function.

 * @param[in] key_size
 *      The number of bytes of the passed key buffer.
 *
 * @param[out] packed_data
 *      The result of the computed @f$ \mathit{HASH} @f$ function.
 *
 * @param[out] packed_data_size
 *      Size of the result. It depends on the @f$ \mathit{HASH} @f$ function actually used.
 *
 * @return
 *      A #RA_RESULT value reporting the outcome of the elaboration
 *
 *
 */
RA_RESULT ra_do_hash_NAYjDD3l2s(uint8_t *nonce_buffer,
                                uint32_t nonce_size,
                                uint8_t *data_buffer,
                                uint32_t data_size,
                                uint8_t *AID,
                                uint64_t attestator_number,
                                void *key,
                                uint8_t key_size,
                                uint8_t **packed_data,
                                uint32_t *packed_data_size);


#endif /* DO_HASH_H_ */
