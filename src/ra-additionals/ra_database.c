/*
 * Database utilities implementation
 *
 * 	It implements the services needed by RA system components
 */
#include "ra_database.h"
#include <stdlib.h>
#include <time.h> // do not remove, needed by strptime()
#include <string.h>
#include <stdint.h>
#include "ra_print_wrapping.h"


const char *request_status_to_string(request_status_t status) {
    const char *request_status_t_to_string[] = {
            "PENDING",
            "SUCCESS",
            "FAILED",
            "EXPIRED_SUCCESS",
            "EXPIRED_FAILED",
            "EXPIRED_NONE"
    };

    return request_status_t_to_string[status];

}


RA_RESULT ra_db_get_application_id(
        /*input*/
        char *AID,
        /*output*/
        uint64_t *application_id) {
    char query[500];
    MYSQL_RES *result;
    int num_row;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_GET_APPLICATION_ID, AID);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = (int) mysql_num_rows(result);
                if (num_row != 0) {
                    row = mysql_fetch_row(result);
                    sscanf(row[0], "%"SCNu64, application_id);
                    //ret_v = RA_SUCCESS;
                } else {
                    ret_v = RA_ERROR_DB_QUERY;
                    ra_fprintf(stderr, "Error: application %s cannot be found \n", AID);
                }
                mysql_free_result(result);

            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "Query was: \"%s\"\n", query);
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;


}


RA_RESULT ra_db_get_attestator_id(char *AID, uint64_t attestatorNumber, uint64_t *attestator_id) {

    char query[500];
    MYSQL_RES *result;
    int num_row;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_ATTESTATOR_ID, AID, attestatorNumber);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row != 0) {
                    row = mysql_fetch_row(result);
                    sscanf(row[0], "%"SCNu64, attestator_id);
                    //ret_v = RA_SUCCESS;
                } else {
                    ret_v = RA_ERROR_DB_QUERY;
                    ra_fprintf(stderr, "Error: application %s-%"PRId64" doesn't exist\n", AID, attestatorNumber);
                }
                mysql_free_result(result);

            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "Query was: \"%s\"\n", query);
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;
}

RA_RESULT ra_db_get_attestation_parameters(char *AID, uint64_t attestator_no,
                                           /* output */
                                           uint64_t *application_id,
                                           uint64_t *attestator_id,
                                           uint32_t *sleep_avg,
                                           uint32_t *sleep_var) {

    char query[500];
    MYSQL_RES *result;
    uint64_t num_row;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_PRODUCT_PARAMETERS, AID, attestator_no);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row != 0) {
                    row = mysql_fetch_row(result);
                    sscanf(row[0], "%"SCNu64, application_id);
                    sscanf(row[1], "%"SCNu64, attestator_id);
                    sscanf(row[2], "%"SCNu32, sleep_avg);
                    sscanf(row[3], "%"SCNu32, sleep_var);
                    //ret_v = RA_SUCCESS;
                } else {
                    ret_v = RA_ERROR_DB_QUERY;
                    ra_fprintf(stderr, "Error: attestator %s-%"PRId64" doesn't exist\n", AID, attestator_no);
                }
                mysql_free_result(result);

            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "Query was: \"%s\"\n", query);
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;
}

RA_RESULT ra_db_finalize_request(uint64_t request_id, request_status_t status, uint32_t response_time) {

    char query[500];
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {

            //strftime(time_str,32,"%s", gmtime(&response_time));
            sprintf(query, DBMS_QUERY_FINALIZE_REQUEST, response_time, status, request_id);
            if (mysql_query(connection, query) != 0) {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;
}

/*
 * data_buffer allocated here,
 * IT MUST BE FREE'D ELSEWHERE
 */
RA_RESULT ra_db_get_data_to_verify(
        /*input*/
        uint64_t request_id,
        /*output*/
        uint8_t **nonce_buffer, uint32_t *nonce_buffer_size,
        uint8_t **data_buffer, uint32_t *data_buffer_size,
        time_t *send_time, request_status_t *status, uint16_t *validity_time_secs) {

    char query[500];
    MYSQL_RES *result;
    int num_row;
    MYSQL_ROW row;
    MYSQL *connection;
    struct tm tm_tmp;
    RA_RESULT ret_v = RA_SUCCESS;
    *nonce_buffer = NULL;
    *data_buffer = NULL;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_PREPARED_DATA_BY_REQUEST, request_id);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row != 0) {
                    row = mysql_fetch_row(result);
                    *nonce_buffer_size = mysql_fetch_lengths(result)[0];
                    if ((*nonce_buffer = (uint8_t *) malloc(*nonce_buffer_size)) != NULL) {
                        if (memcpy(*nonce_buffer, row[0], *nonce_buffer_size) != NULL) {

                            *data_buffer_size = mysql_fetch_lengths(result)[1];
                            if ((*data_buffer = (uint8_t *) malloc(*data_buffer_size)) != NULL) {
                                if (memcpy(*data_buffer, row[1], *data_buffer_size) != NULL) {
                                    //sscanf(row[2],"%"SCNd32	,send_time);
                                    strptime(row[2], "%s", &tm_tmp);
                                    *send_time = mktime(&tm_tmp);
                                    sscanf(row[3], "%d", status);
                                    sscanf(row[4], "%"SCNu16, validity_time_secs);
                                    //ret_v = RA_SUCCESS;
                                } else {
                                    ret_v = RA_ERROR_MEMORY;
                                }
                            } else {
                                ret_v = RA_ERROR_MEMORY;
                            }

                        } else {
                            ret_v = RA_ERROR_MEMORY;
                        }
                    } else {

                    }
                } else {
                    ra_fprintf(stderr, "Error: request %"PRId64" doesn't exist\n", request_id);
                    ret_v = RA_ERROR_DB_QUERY;
                }
                mysql_free_result(result);
            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    if (ret_v != RA_SUCCESS) {
        if (*data_buffer != NULL) free(data_buffer);
        if (*nonce_buffer != NULL) free(nonce_buffer);
    }
    return ret_v;
}

/*
 * valid_left includes the extracted one
 */
RA_RESULT ra_db_get_valid_nonce(uint64_t attestator_id, uint64_t *prepared_data_id, uint8_t **nonce_buffer, uint32_t *nonce_size,
                                uint32_t *valid_nonce_left, uint16_t *memory_area) {

    char query[500];
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;
    *nonce_buffer = NULL;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_NONCE, attestator_id);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                row = mysql_fetch_row(result);
                sscanf(row[3], "%"SCNd32, valid_nonce_left);
                if (*valid_nonce_left != 0) {

                    *nonce_size = mysql_fetch_lengths(result)[1];
                    sscanf(row[0], "%"SCNu64, prepared_data_id);

                    if ((*nonce_buffer = (uint8_t *) malloc(*nonce_size * sizeof(uint8_t))) != NULL) {
                        if (memcpy(*nonce_buffer, row[1], *nonce_size) != NULL) {
                            ret_v = RA_SUCCESS;
                        } else {
                            ret_v = RA_ERROR_MEMORY;
                        }
                    } else {
                        ret_v = RA_ERROR_MEMORY;
                    }
                    sscanf(row[2], "%"SCNu16, memory_area);
                    mysql_free_result(result);
                } else {
                    ret_v = RA_ERROR_DB_NO_DATA;
                }
            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }

        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    if (ret_v != RA_SUCCESS) {
        if (*nonce_buffer != NULL) free(*nonce_buffer);
    }
    return ret_v;
}


RA_RESULT ra_db_get_valid_nonce_by_area_label(
        /*input*/
        uint64_t attestator_id,
        uint16_t memory_area,
        /*output*/
        uint64_t *prepared_data_id,
        uint8_t **nonce_buffer,
        uint32_t *nonce_size) {

    char query[500];
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;
    my_ulonglong num_row;
    *nonce_buffer = NULL;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_NONCE_BY_MEMORY_LABEL, attestator_id, memory_area);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row != 0) {
                    row = mysql_fetch_row(result);

                    *nonce_size = mysql_fetch_lengths(result)[1];

                    sscanf(row[0], "%"SCNu64, prepared_data_id);

                    if ((*nonce_buffer = (uint8_t *) malloc(*nonce_size * sizeof(uint8_t))) != NULL) {
                        if (memcpy(*nonce_buffer, row[1], *nonce_size) != NULL) {
                            ret_v = RA_SUCCESS;
                        } else {
                            ret_v = RA_ERROR_MEMORY;
                        }
                    } else {
                        ret_v = RA_ERROR_MEMORY;
                    }

                    mysql_free_result(result);
                } else {
                    ret_v = RA_ERROR_DB_NO_DATA;
                }
            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }

        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    if (ret_v != RA_SUCCESS) {
        if (*nonce_buffer != NULL) free(*nonce_buffer);
    }
    return ret_v;
}

RA_RESULT ra_db_get_area_labels_to_attest_at_startup(
        /*input*/
        uint64_t attestator_id,
        /*output*/
        uint16_t **labels,
        uint16_t *labels_count
) {

    char query[500];
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;
    my_ulonglong num_row;
    uint16_t *tmp_labels;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_EXTRACT_AREAS_TO_ATTEST_AT_STARTUP, attestator_id);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row != 0) {
                    tmp_labels = (uint16_t *) malloc(num_row * sizeof(uint16_t));
                    if (tmp_labels != NULL) {
                        int i = 0;
                        while ((row = mysql_fetch_row(result)) != NULL) {
                            sscanf(row[0], "%"SCNu16, &(tmp_labels[i]));
                            i++;
                        }
                        *labels = tmp_labels;
                        *labels_count = (uint16_t) num_row;
                        ret_v = RA_SUCCESS;
                    } else {
                        ret_v = RA_ERROR_MEMORY;
                    }

                } else {
                    *labels_count = 0;
                    ret_v = RA_SUCCESS;
                }
            } else {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }

        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    if (ret_v != RA_SUCCESS) {
        if (*labels != NULL) free(*labels);
    }
    return ret_v;

}

RA_RESULT ra_db_set_reaction_status(
        /*input*/
        uint64_t application_id,
        /*output*/
        reaction_status_t status) {

    char query[500];
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {

            sprintf(query, DBMS_QUERY_SET_REACTION_STATUS, (uint64_t) status, application_id);
            if (mysql_query(connection, query) != 0) {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;

}

RA_RESULT ra_db_start_session(
        /*input*/
        uint64_t attestator_id,
        /* output */
        uint64_t *session_id
        ) {

    char query[500];
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {
            sprintf(query, DBMS_QUERY_CLOSE_ALL_ACTIVE_SESSIONS, attestator_id);
            if (mysql_query(connection, query) != 0) {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "Delete all active sessions MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }else {
                sprintf(query, DBMS_QUERY_INSERT_SESSION, attestator_id);
                if (mysql_query(connection, query) != 0) {
                    ret_v = RA_ERROR_DB_MYSQL;
                    ra_fprintf(stderr, "Insert session (after remove all active) MySql query failed\n");
                    ra_fprintf(stderr, "%s\n", mysql_error(connection));
                }else {
                    *session_id = mysql_insert_id(connection);
                }
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;

}

RA_RESULT ra_db_revoke_session(
        /*input*/
        uint64_t session_id
) {

    char query[500];
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {

            sprintf(query, DBMS_QUERY_DELETE_SESSION, session_id);
            if (mysql_query(connection, query) != 0) {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;

}

RA_RESULT ra_db_close_session(
        /*input*/
        uint64_t session_id
) {

    char query[500];
    MYSQL *connection;
    RA_RESULT ret_v = RA_SUCCESS;

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {

            sprintf(query, DBMS_QUERY_CLOSE_SESSION, session_id);
            if (mysql_query(connection, query) != 0) {
                ret_v = RA_ERROR_DB_MYSQL;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;

}

RA_RESULT ra_db_get_active_session(
        /*input*/
        uint64_t attestator_id,
        /* output*/
        uint64_t *session_id
) {

    char query[500];
    MYSQL_RES *result;
    MYSQL *connection;
    my_ulonglong num_row;
    RA_RESULT ret_v = RA_SUCCESS;
    MYSQL_ROW row;

    memset(query, 0, 500);

    if ((connection = mysql_init(NULL)) != NULL) {
        if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) != NULL) {

            sprintf(query, DBMS_QUERY_EXTRACT_ACTIVE_SESSION, attestator_id);
            if (mysql_query(connection, query) == 0) {
                result = mysql_store_result(connection);
                num_row = mysql_num_rows(result);
                if (num_row == 1) {

                    row = mysql_fetch_row(result);
                    sscanf(row[0], "%"SCNu64, session_id);

                }else{
                   ret_v = RA_ERROR_SESSION_TABLE_ANOMALY;
                }
            }else{
                ret_v = RA_ERROR_DB_QUERY;
                ra_fprintf(stderr, "MySql query failed\n");
                ra_fprintf(stderr, "%s\n", mysql_error(connection));
            }
        } else {
            ret_v = RA_ERROR_DB_MYSQL;
            ra_fprintf(stderr, "MySql connection failed\n");
            ra_fprintf(stderr, "%s\n", mysql_error(connection));
        }
        mysql_close(connection);
    } else {
        ret_v = RA_ERROR_DB_MYSQL;
        ra_fprintf(stderr, "MySql init failed\n");
    }

    return ret_v;

}
