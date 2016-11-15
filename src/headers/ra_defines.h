/*
 * ra_defines.h
 *
 *  Created on: Aug 7, 2015
 *      Author: alessio
 */

#ifndef RA_DEFINES_H_
#define RA_DEFINES_H_


#include <inttypes.h>


/**
 *  Generic definitions
 */
#define ASPIRE_STD_RA_BASE_PATH                "/opt/online_backends"

#define DEFAULT_NONCE_LENGTH                    "32"
#define DEFAULT_NONCE_TOTAL                     "100"
#define NONCE_CRITICAL_THRESHOLD                10


#define AID_SIZE 		        				16

#define EXTRACTOR_COMMAND_PATH_STRING_FORMAT    ASPIRE_STD_RA_BASE_PATH"/%s/remote_attestation/%020"PRIu64"/extractor"
#define EXTRACTOR_COMMAND                       "extractor"
#define VERIFIER_COMMAND_PATH_STRING_FORMAT    	ASPIRE_STD_RA_BASE_PATH"/%s/remote_attestation/%020"PRIu64"/verifier"
#define VERIFIER_COMMAND                        "verifier"
#define NOT_SERVED_RE_SCHEDULING_STANDARD_TIME  5.0
#define MAX_ATTESTATORS_ALLOWED 				10
#define EXTRACTOR_MAX_ERROR_ALLOWED           	20
#define ATTEST_AT_STARTUP_MAX_ERRORS	    	5
#define ADS_PATH_STRING_FORMAT                	ASPIRE_STD_RA_BASE_PATH"/%s/remote_attestation/%020"PRIu64"/binaries/ads"
#define EXE_PATH_STRING_FORMAT                	ASPIRE_STD_RA_BASE_PATH"/%s/remote_attestation/%020"PRIu64"/binaries/exe"

#define VERIFIER_LOG_PATH                       ASPIRE_STD_RA_BASE_PATH"/remote_attestation/verifier.log"
#define VERIFIER_FORWARDER_LOG_PATH             ASPIRE_STD_RA_BASE_PATH"/remote_attestation/verifier_forwarder.log"

//#define VERIFIER_ERROR_LOG_PATH                 ASPIRE_STD_RA_BASE_PATH"/verifier_error.log"

/**
 * Protocol sizes definitions
 */

/*
 * Message format verifier_forwarder -> verifier:
 *
 * |  8 Bytes	|   AID_SIZE Bytes	|    8 Bytes	|   8 Bytes	 |    4 Bytes   |  HASH_LENGTH Bytes |
 * |  RESP_TIME	|        AID		|  ATTESTER NO	| REQUEST_ID |  HASH_LENGTH |    HASH BUFFER 	 |
 *
 */
#define RESP_TIME_SIZE				sizeof(time_t)
#define RESP_REQUEST_ID_SIZE		sizeof(uint64_t)
#define RESP_ATTESTATOR_NO_SIZE     sizeof(uint64_t)
#define RESP_HASH_LENGTH_SIZE		sizeof(uint32_t)
#define VERIFIER_RESP_HEADER_SIZE   RESP_TIME_SIZE+ \
                                    AID_SIZE+ \
                                    RESP_ATTESTATOR_NO_SIZE+ \
                                    RESP_REQUEST_ID_SIZE+ \
									RESP_HASH_LENGTH_SIZE

/*
 *  Attestation response format from the attestator:
 *
 * |   AID_SIZE Bytes	|    8 Bytes	|   8 Bytes	 |    4 Bytes   |  HASH_LENGTH Bytes |
 * |        AID		    |  ATTESTER NO	| REQUEST_ID |  HASH_LENGTH |    HASH BUFFER 	 |
 *
 */
#define ATTESTATOR_RESP_HEADER_SIZE AID_SIZE+ \
                                    RESP_ATTESTATOR_NO_SIZE+ \
									RESP_REQUEST_ID_SIZE+ \
									RESP_HASH_LENGTH_SIZE

/*
 * message format:
 *
 * |   8 Bytes	|    4 Bytes   | NONCE_LENGTH Bytes |
 * | REQUEST_ID | NONCE_LENGTH |    NONCE BUFFER 	|
 *
 */
#define REQUEST_ID_SIZE                         sizeof(uint64_t)
#define REQUEST_NONCE_LENGTH_SIZE               sizeof(uint32_t)
#define REQUEST_HEADER_SIZE                     REQUEST_ID_SIZE + REQUEST_NONCE_LENGTH_SIZE



#endif /* RA_DEFINES_H_ */
