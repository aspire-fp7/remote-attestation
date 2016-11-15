/*
 * EXTRACTOR IMPLEMENTATION
 *
 * It computes and stores attestation data for verification phases
 */

#include "bfd.h"

#include "ra_print_wrapping.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>

#include <ra_nonce_generation.h>
#include <ra_nonce_interpretation.h>
#include <ra_data_preparation.h>
#include <ra_database.h>
#include <ra_defines.h> // do not remove

void getTextSectionOffsetAndSize(const char *filename, int fd, size_t *offset, size_t *size) {

    bfd *tmp_bfd = NULL;
    asection *sec = NULL;
    bfd_init();

    /* Open the file for BFD, using the file descriptor we already have */
    tmp_bfd = bfd_fdopenr(filename, NULL, fd);
    if (tmp_bfd == NULL)
        printf("(Extractor) Didn't successfully manage to open the executable for BFD!");

    /* Determine the format of the file */
    if (bfd_check_format(tmp_bfd, bfd_object) == false) {
        fprintf(stderr, "(Extractor) Error, exe file format not recognized\n");
        exit(1);
    } else {
        fprintf(stdout, "(Extractor) exe file format successfully recognized\n");
    }

    /* Get the .text section and the information we need from it */
    if ((sec = bfd_get_section_by_name(tmp_bfd, ".text")) == NULL) {
        fprintf(stderr, "(Extractor) Error getting .text section\n");
        exit(1);
    }
    *size = bfd_section_size(tmp_bfd, sec);
    *offset = (size_t) sec->filepos;
}

/*
 * Extractor main
 *
 * args:
 *	argv[1])	Application Serial Number
 *	argv[2])	Attestator number
 * 	argv[3])	Nonce length
 * 	argv[4])	Total nonce-prepared_data to produce
 *
 */
int main(int argc, char *argv[]) {

    RA_table ads_struct;

    int ads_binary_fd;
    int exe_binary_fd;
    uint32_t nonce_length;
    uint64_t attestator_number;
    int total_data_to_exctract = atoi(argv[4]);
    int total_data_exctracted = 0;
    uint8_t *current_nonce;
    int error_counter;
    MYSQL *connection;
    char query[500];
    char *dyn_query;
    struct stat file_stat;
    uint8_t app_AID[AID_SIZE];
    char *app_AID_string_from_cli;
    char app_AID_string_from_ADS[AID_SIZE * 2 + 1];
    uint64_t attestator_id;
    char *nonce_to_store;
    char *prepared_data_to_store;
    uint32_t prepared_data_size;
    MYSQL_ROW row;
    char ads_bin_path[PATH_MAX];
    char exe_bin_path[PATH_MAX];
    size_t offset, size;



    /*********  parse args ***********/
    if (strlen(argv[1]) != AID_SIZE * 2) {
        fprintf(stderr, "(Extractor) Error: AID string wrong length, exiting\n");
        exit(1);
    }
    app_AID_string_from_cli = argv[1];
    for (int j = 0; j < AID_SIZE; ++j) {
        sscanf(&(app_AID_string_from_cli[j * 2]), "%02"SCNu8, &(app_AID[j]));
    }

    sscanf(argv[2], "%"SCNd64, &attestator_number);

    sscanf(argv[3], "%"SCNd32, &nonce_length);

    sprintf(ads_bin_path, ADS_PATH_STRING_FORMAT, app_AID_string_from_cli, attestator_number);
    sprintf(exe_bin_path, EXE_PATH_STRING_FORMAT, app_AID_string_from_cli, attestator_number);
    fprintf(stdout, "(Extractor) ads path: %s | exe path: %s\n", ads_bin_path, exe_bin_path);
    /********* args parsed ***********/

    /*****  Load data structures *****/
    fprintf(stdout, "(Extractor) Opening ADS file\n");
    if ((ads_binary_fd = open(ads_bin_path, O_RDONLY)) <= 0) {
        fprintf(stderr, "(Extractor) Error: cannot open ADS file, exiting\n");
        exit(1);
    }
    fprintf(stdout, "(Extractor) Reading ADS file info\n");
    if (fstat(ads_binary_fd, &file_stat) != 0) {
        fprintf(stderr, "(Extractor) Error: cannot read ADS file information, exiting\n");
        exit(1);
    }
    fprintf(stdout, "(Extractor) Alloc'ing ADS struct memory\n");
    if ((void *) (data_structure_blob_NAYjDD3l2s = (uint64_t) malloc((size_t) file_stat.st_size)) == NULL) {
        fprintf(stderr, "(Extractor) Error: memory allocation, exiting\n");
        exit(1);
    }
    fprintf(stdout, "(Extractor) Copying ADS\n");
    if (read(ads_binary_fd, (uint8_t *) data_structure_blob_NAYjDD3l2s, (size_t) file_stat.st_size) != file_stat.st_size) {
        fprintf(stderr, "(Extractor) Error: cannot read ADS file content correctly, exiting\n");
        free((void *) data_structure_blob_NAYjDD3l2s);
        exit(1);
    }
    fprintf(stdout, "(Extractor) Parsing ADS\n");
    if ((ads_struct = ra_parse_binary_ads_NAYjDD3l2s()) == NULL) {
        fprintf(stderr, "(Extractor) Error: parsing ADS, exiting\n");
        free((void *) data_structure_blob_NAYjDD3l2s);
        exit(1);
    }
    ra_print_table_info_NAYjDD3l2s(ads_struct);
    ra_get_AID_as_string_NAYjDD3l2s(ads_struct, (char *) &app_AID_string_from_ADS);
    if (strcmp(app_AID_string_from_cli, app_AID_string_from_ADS) != 0) {
        fprintf(stderr, "(Extractor) Error: AID from ADS is different from the specified one, exiting\n");
        exit(1);
    }

    close(ads_binary_fd);
    free((void *) data_structure_blob_NAYjDD3l2s);

    fprintf(stdout, "(Extractor) Opening EXE file\n");
    if ((exe_binary_fd = open(exe_bin_path, O_RDONLY)) <= 0) {
        fprintf(stderr, "(Extractor) Error: cannot open program binary file, exiting\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        exit(1);
    }
    fprintf(stdout, "(Extractor) Getting text section\n");
    getTextSectionOffsetAndSize(exe_bin_path, exe_binary_fd, &offset, &size);

    fprintf(stdout, "(Extractor) Offset = %zu\n", offset);
    fprintf(stdout, "(Extractor) Alloc'ing memory for EXE\n");
    if ((void *) (base_address_NAYjDD3l2s = (uint64_t) malloc(size)) == NULL) {
        fprintf(stderr, "(Extractor) Error: memory allocation, exiting\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        exit(1);
    }
    fprintf(stdout, "(Extractor) Copying EXE file\n");
    if (pread(exe_binary_fd, (uint8_t *) base_address_NAYjDD3l2s, size, offset) != size) {
        fprintf(stderr, "(Extractor) Error: cannot read program binary file content correctly, exiting\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        exit(1);
    }

    close(exe_binary_fd);
    /****** Data structures loaded *********/


    fprintf(stdout, "(Extractor) Start extraction\n");
    current_nonce = (uint8_t *) malloc((size_t) nonce_length);
    if (current_nonce == NULL) {
        fprintf(stderr, "(Extractor) Error: memory allocation, exiting\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        free((void *) base_address_NAYjDD3l2s);
        exit(1);
    }
    nonce_to_store = (char *) malloc((size_t) (nonce_length * 2 + 1));
    if (nonce_to_store == NULL) {
        fprintf(stderr, "(Extractor) Error: memory allocation, exiting\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        free((void *) base_address_NAYjDD3l2s);
        exit(1);
    }


    /************** MySQL initialization *************/
    connection = mysql_init(NULL);
    if (connection == NULL) {
        fprintf(stderr, "(Extractor) mysql_init() failed\n");
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        free((void *) base_address_NAYjDD3l2s);
        exit(1);
    }
    if (mysql_real_connect(connection, DBMS_HOST, DBMS_USER, DBMS_PWD, DBMS_DATABASE, 0, NULL, 0) == NULL) {
        fprintf(stderr, "(Extractor) %s\n", mysql_error(connection));
        mysql_close(connection);
        ra_destroy_table_NAYjDD3l2s(ads_struct);
        free((void *) base_address_NAYjDD3l2s);
        exit(1);
    }
    /************** MySQL initialized ****************/


    /*********** Application existence check **************/
    if (ra_db_get_attestator_id(app_AID_string_from_ADS, attestator_number, &attestator_id) != RA_SUCCESS) {
        fprintf(stderr, "(Extractor) Error while checking application existence\n");
        exit(1);
    }
    /*************** Application exists *******************/


#ifdef DEBUG
    printf("(Extractor) Printing first 8 bytes starting from base_address\n");
    for (int i = 0; i < 8; i++) {
        fprintf(stdout, "%02X", (unsigned char) (*((uint8_t *) (base_address_NAYjDD3l2s + i))));
    }
    fprintf(stdout, "-------------------------------------------------------------\n");
#endif


    error_counter = 0;
    for (total_data_exctracted = 0;
         total_data_exctracted < total_data_to_exctract;
         total_data_exctracted++) {

        /************ Generate & set nonce ************/
        if (random_generate_nonce_NAYjDD3l2s(current_nonce, nonce_length) != RA_SUCCESS) {
            printf("(Extractor) Error nr. %d: encountered generating random nonce\n", error_counter + 1);
            total_data_exctracted--;
            if (++error_counter >= EXTRACTOR_MAX_ERROR_ALLOWED) {
                break;
            }
            continue;
        }

        if (ra_set_current_nonce_NAYjDD3l2s(ads_struct, current_nonce, nonce_length) != RA_SUCCESS) {
            fprintf(stderr, "(Extractor) Error while setting nonce in RA table\n");
            if (++error_counter >= EXTRACTOR_MAX_ERROR_ALLOWED) {
                break;
            }
            continue;
        }
        /*********** Nonce genearated & set ***********/

#ifdef DEBUG
        print_encoded_info_NAYjDD3l2s(ads_struct);
        fprintf(stdout, "(Extractor) Printing prepared DATA\n");
        for (int i = 0; i < ra_get_prepared_data_size_NAYjDD3l2s(ads_struct); i++) {
            printf("%02X", (unsigned char) (*((uint8_t *) (ra_get_prepared_data_reference_NAYjDD3l2s(ads_struct) + i))));
        }
        fprintf(stdout, "\n---------------------------------\n");
#endif

        /********** Prepare attestation data ***********/
        ra_prepare_data_NAYjDD3l2s(ads_struct);

        prepared_data_size = ra_get_prepared_data_size_NAYjDD3l2s(ads_struct);

        if ((prepared_data_to_store = (char *) malloc(prepared_data_size * 2 + 1)) == NULL) {
            printf("(Extractor) Error nr. %d: encountered allocating buffer for data to store\n", error_counter + 1);
            total_data_exctracted--;
            if (++error_counter >= EXTRACTOR_MAX_ERROR_ALLOWED) {
                break;
            }
            continue;
        }
        /********** Attestation data prepared **********/



        /************ Format attestation data for DB *************/
        mysql_real_escape_string(connection,
                                 nonce_to_store,
                                 (char *) current_nonce, nonce_length);

        mysql_real_escape_string(connection,
                                 prepared_data_to_store,
                                 (char *) ra_get_prepared_data_reference_NAYjDD3l2s(ads_struct),
                                 ra_get_prepared_data_size_NAYjDD3l2s(ads_struct));

        dyn_query = (char *) malloc((strlen(DBMS_QUERY_INSERT_DATA)
                                     + strlen(prepared_data_to_store)
                                     + strlen(nonce_to_store) +
                                     MAX_BIGINT_WIDTH) * sizeof(char));

        decoded_info nonce_info;
        decode_nonce_NAYjDD3l2s(ads_struct, &nonce_info);

        sprintf(dyn_query,
                DBMS_QUERY_INSERT_DATA,
                attestator_id, nonce_to_store,
                prepared_data_to_store, nonce_info.area_label);
        /********** Attestation data for DB formatted ************/


        /********** Insert formatted attestation data in DB ***********/
        if (mysql_real_query(connection, dyn_query, strlen(dyn_query))) {
            printf("(Extractor) Error nr. %d: encountered inserting data into DB (mysql error:%s)\n", error_counter + 1, mysql_error(connection));
            total_data_exctracted--;
            if (++error_counter >= EXTRACTOR_MAX_ERROR_ALLOWED) {
                break;
            }
            continue;
        }
        /********** Formatted attestation data inserted in DB ***********/

    }

    if (error_counter == EXTRACTOR_MAX_ERROR_ALLOWED) {
        fprintf(stdout, "(Extractor) Max error allowed reached, extraction process halted\n");
    } else {
        fprintf(stdout, "(Extractor) Extraction successfully terminated\n");
    }

    fprintf(stdout, "(Extractor) Records extracted: %d/%d\n", total_data_exctracted, total_data_to_exctract);
    fprintf(stdout, "(Extractor) Total error encountered: %d\n", error_counter);

    if (error_counter == EXTRACTOR_MAX_ERROR_ALLOWED) {
        return 1;
    } else {
        return 0;
    }
}
