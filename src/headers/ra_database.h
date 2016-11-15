/*
 *
 * Database utilities definitions
 *
 * 	It contains macro and defines useful for database interaction
 * 	and
 * 	declares "services" (in the DAO/ORM world sense) for database queries*
 *
 */

#ifndef RA_DATABASE_H_
#define RA_DATABASE_H_

#include <inttypes.h>
#include <time.h>
#include <mysql/mysql.h>
#include <ra_results.h>
//#include "ra_data_table.h"
#include <ra_defines.h>

#include "ra_print_wrapping.h"


#define DBMS_HOST                   "localhost"
#define DBMS_USER                   "ra_user"
#define DBMS_PWD                    NULL
#define DBMS_DATABASE               "RA_development"

#define DBMS_TAB_APPLICATION        "ra_application"
#define DBMS_TAB_PREPARED_DATA      "ra_prepared_data"
#define DBMS_TAB_ATTESTATOR         "ra_attestator"
#define DBMS_TAB_REQUEST            "ra_request"
#define DBMS_TAB_REACTION           "ra_reaction"
#define DBMS_TAB_ATTEST_AT_STARTUP  "ra_attest_at_startup_area"
#define DBMS_TAB_SESSION            "ra_session"

/*
 * prepared_data_store
 */
#define DBMS_QUERY_GET_APPLICATION_ID    "\
SELECT id \
FROM "DBMS_TAB_APPLICATION" \
WHERE AID = '%s';"

#define DBMS_QUERY_INSERT_DATA    "\
INSERT INTO "DBMS_TAB_PREPARED_DATA"(attestator_id, nonce, data, memory_area) \
VALUES ('%"PRIu64"','%s','%s',%"PRIu16");"


/*
 * manager
 */
#define DBMS_QUERY_EXTRACT_ATTESTATOR_ID    "\
SELECT att.id \
FROM "DBMS_TAB_ATTESTATOR" AS att," DBMS_TAB_APPLICATION" AS a \
WHERE att.application_id = a.id AND \
a.AID = '%s' AND \
att.attestator_no = %"PRIu64";"


#define DBMS_QUERY_EXTRACT_PRODUCT_PARAMETERS    "\
SELECT app.id, att.id, att.sleep_avg, att.sleep_var \
FROM "DBMS_TAB_ATTESTATOR" AS att, " DBMS_TAB_APPLICATION" AS app \
WHERE att.application_id = app.id AND \
app.AID = '%s' AND \
att.attestator_no = %"PRIu64";"


#define DBMS_QUERY_EXTRACT_NONCE        "\
SELECT pd.id , pd.nonce , pd.memory_area , COUNT(*) \
FROM "DBMS_TAB_PREPARED_DATA" AS pd , "DBMS_TAB_ATTESTATOR" AS a \
WHERE \
a.id = %"PRIu64" AND \
a.id = pd.attestator_id AND \
pd.id NOT IN \
(SELECT DISTINCT req.prepared_data_id \
FROM "DBMS_TAB_REQUEST" AS req);"

#define DBMS_QUERY_EXTRACT_NONCE_BY_MEMORY_LABEL   "\
SELECT pd.id , pd.nonce \
FROM "DBMS_TAB_PREPARED_DATA" AS pd , "DBMS_TAB_ATTESTATOR" AS a \
WHERE \
a.id = %"PRIu64" AND \
a.id = pd.attestator_id AND \
pd.memory_area = %"PRIu16" AND \
pd.id NOT IN \
(SELECT DISTINCT req.prepared_data_id \
FROM "DBMS_TAB_REQUEST" AS req);"

#define DBMS_QUERY_EXTRACT_AREAS_TO_ATTEST_AT_STARTUP   "\
SELECT memory_area \
FROM "DBMS_TAB_ATTEST_AT_STARTUP" \
WHERE attestator_id = %"PRIu64


#define DBMS_QUERY_INSERT_REQUEST        "\
INSERT INTO "DBMS_TAB_REQUEST"(prepared_data_id, session_id, validity_time, is_startup) \
VALUES ('%"PRIu64"', '%"PRIu64"','%"PRIu16"', %d);"


#define DBMS_QUERY_INSERT_SESSION "\
INSERT INTO "DBMS_TAB_SESSION"(attestator_id, start, is_active) \
VALUES (%"PRIu64", NOW(), TRUE);"


#define DBMS_QUERY_DELETE_SESSION "\
DELETE FROM "DBMS_TAB_SESSION" WHERE id = %"PRIu64";"

#define DBMS_QUERY_DELETE_ALL_ACTIVE_SESSIONS "\
DELETE FROM "DBMS_TAB_SESSION" WHERE id = %"PRIu64" AND is_active=TRUE;"

#define DBMS_QUERY_CLOSE_ALL_ACTIVE_SESSIONS "\
UPDATE "DBMS_TAB_SESSION" \
SET is_active=FALSE \
WHERE attestator_id = %"PRIu64";"

#define DBMS_QUERY_CLOSE_SESSION "\
UPDATE "DBMS_TAB_SESSION" \
SET finish=NOW(), is_active=FALSE \
WHERE id = %"PRIu64";"


#define DBMS_QUERY_EXTRACT_ACTIVE_SESSION   "\
SELECT id \
FROM "DBMS_TAB_SESSION" \
WHERE attestator_id = %"PRIu64" AND is_active = TRUE;"


#define DBMS_QUERY_EXTRACT_PREPARED_DATA_BY_REQUEST "\
SELECT nonce, data, UNIX_TIMESTAMP(send_time), status, validity_time \
FROM "DBMS_TAB_PREPARED_DATA" AS pd, "DBMS_TAB_REQUEST" AS r \
WHERE pd.id = r.prepared_data_id AND \
r.id = %"PRIu64";"

#define DBMS_QUERY_FINALIZE_REQUEST        "\
UPDATE "DBMS_TAB_REQUEST" \
SET response_time = %"PRIu32", \
status = %u \
WHERE id = %"PRIu64";"

//#define DBMS_QUERY_INITIALIZE_REACTION_STATUS  "\
//"INSERT INTO "DBMS_TAB_REACTION" (id, application_id, reaction_status_id) VALUES (NULL, %"PRIu64", 0)"

#define DBMS_QUERY_SET_REACTION_STATUS  "\
UPDATE "DBMS_TAB_REACTION" \
SET reaction_status_id = %"PRIu64" \
WHERE application_id = %"PRIu64";"


typedef enum {
    PENDING,
    SUCCESS,
    FAILED,
    EXPIRED_SUCCESS,
    EXPIRED_FAILED,
    EXPIRED_NONE
} request_status_t;


typedef enum {
    REACTION_STATUS_NONE = 0,
    REACTION_STATUS_COMPROMISED = 1,
    REACTION_STATUS_UNKNOWN = 2

} reaction_status_t;

const char *request_status_to_string(request_status_t status);


RA_RESULT ra_db_get_application_id(
        /*input*/
        char *AID,
        /*output*/
        uint64_t *application_id);

RA_RESULT ra_db_get_attestator_id(
        /*input*/
        char *AID,
        uint64_t attestatorNumber,
        /*output*/
        uint64_t *attestator_id);

RA_RESULT ra_db_get_attestation_parameters(
        /*input*/
        char *AID,
        uint64_t attestator_no,
        /*output*/
        uint64_t *application_id,
        uint64_t *attestator_id,
        uint32_t *sleep_avg,
        uint32_t *sleep_var);

RA_RESULT ra_db_finalize_request(
        /*input*/
        uint64_t request_id,
        request_status_t status,
        uint32_t response_time);

RA_RESULT ra_db_get_data_to_verify(
        /*input*/
        uint64_t request_id,
        /*output*/
        uint8_t **nonce_buffer,
        uint32_t *nonce_buffer_size,
        uint8_t **data_buffer,
        uint32_t *data_buffer_size,
        time_t *send_time,
        request_status_t *status,
        uint16_t *validity_time_secs);

RA_RESULT ra_db_get_valid_nonce(
        /*input*/
        uint64_t attestator_id,
        /*output*/
        uint64_t *prepared_data_id,
        uint8_t **nonce_buffer,
        uint32_t *nonce_size,
        uint32_t *valid_nonce_left,
        uint16_t *memory_area);

RA_RESULT ra_db_get_valid_nonce_by_area_label(
        /*input*/
        uint64_t attestator_id,
        uint16_t memory_area,
        /*output*/
        uint64_t *prepared_data_id,
        uint8_t **nonce_buffer,
        uint32_t *nonce_size);

RA_RESULT ra_db_get_area_labels_to_attest_at_startup(
        /*input*/
        uint64_t attestator_id,
        /*output*/
        uint16_t **labels,
        uint16_t *labels_count
);

RA_RESULT ra_db_set_reaction_status(
        /*input*/
        uint64_t application_id,
        reaction_status_t status);

RA_RESULT ra_db_start_session(
        /*input*/
        uint64_t application_id,
        /* output */
        uint64_t *session_id);

RA_RESULT ra_db_revoke_session(
        /*input*/
        uint64_t session_id);

RA_RESULT ra_db_close_session(
        /*input*/
        uint64_t session_id);

RA_RESULT ra_db_get_active_session(
        /*input*/
        uint64_t attestator_id,
        /* output*/
        uint64_t *session_id);

#endif /* RA_DATABASE_H__ */
