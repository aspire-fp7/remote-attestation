/*
 * verifier_forwarder.c
 *
 */

#include <stdlib.h>
#include <stdint.h>

#include "ra_print_wrapping.h"

#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>

#include <ra_defines.h>
#include <linux/limits.h>

struct active_verifier {
    uint64_t application_id;
    pid_t process_id;
};

FILE *log_file;

int main(int argc, char **argv) {
    uint8_t *in_buf;
    int in_buf_len, read_bytes = 0;
    log_file = fopen(VERIFIER_FORWARDER_LOG_PATH, "a");

    // parameters check
    if (argc < 3) {
        fprintf(log_file, "parameters error\n");
        fclose(log_file);
        return 1;
    }

    // payload length
    in_buf_len = atoi(argv[2]);

    if (in_buf_len <= 0) {
        fprintf(log_file, "buffer length error\n");
        fclose(log_file);
        return 1;
    }

    // allocate buffer for incoming payload
    in_buf = (uint8_t *) malloc(in_buf_len * sizeof(char));

    while (read_bytes < in_buf_len) {

        int n = (int) read(STDIN_FILENO,
                             in_buf + read_bytes * sizeof(char),
                           (size_t) (in_buf_len - read_bytes));

        if (n == -1) {
            fprintf(log_file, "buffer reading error\n");
            fclose(log_file);
            free(in_buf);
            return 1;
        } else {
            read_bytes += n;
        }

        if (n == 0 && read_bytes < in_buf_len) {
            fprintf(log_file, "buffer reading, total bytes less than declared, error\n");
            fclose(log_file);
            free(in_buf);
            return 1;
        }
    }

    fprintf(log_file, "Received %d buffer:\n", in_buf_len);fflush(log_file);
    for (int j = 0; j < in_buf_len; ++j) {
        fprintf(log_file, "%02"PRIX8, in_buf[j]);fflush(log_file);
        if ((j + 1) % 16 == 0) {fprintf(log_file, "\n");fflush(log_file);}
    }
    fprintf(log_file, "\n");fflush(log_file);

    fprintf(log_file, "Reading AID\n");fflush(log_file);
    time_t received_response_time = time(NULL);
    char received_app_SN_string[AID_SIZE * 2 + 1];
    for (int i = 0; i < AID_SIZE; i++) {
        fprintf(log_file, "%02"PRIX8, in_buf[i]);
        fflush(log_file);
        if (snprintf(&received_app_SN_string[i * 2], 3, "%02"PRIX8, in_buf[i]) != 2) {
            fprintf(log_file, "Memory error\n");
            fflush(log_file);
            fclose(log_file);
            free(in_buf);
            return 1;
        }
    }

    uint64_t attestator_number = (uint64_t)*(in_buf+AID_SIZE);
    fprintf(log_file,"Attestator number: %20"PRIu64"\n", attestator_number);

    time_t n = time(NULL);
    fprintf(log_file, "\n%s Forwarding buffer:\n", ctime(&n));
    for (int c = 0; c < in_buf_len; c++) {
        fprintf(log_file, "%02X", (unsigned char) (*((int8_t *) (in_buf + c))));
        if (c % 16 == 15) fprintf(log_file, "\n");
    }
    fprintf(log_file, "\n");
    fflush(log_file);
    //fclose(log);
    int fd[2];
    pipe(fd);
    pid_t childpid;

    if ((childpid = fork()) == -1) {
        fprintf(log_file, "fork() error\n");
        fclose(log_file);
        return 1;
    }

    if (childpid == 0) {
        fprintf(log_file, "Child\n");
        char verifier_path[PATH_MAX];
        close(fd[1]);
        dup2(fd[0], 0);
//		if(read(STDIN_FILENO, tty, in_buf_len) != in_buf_len)
//			fprintf(log, "error READING\n");
//		for (int c = 0; c < in_buf_len; c++) {
//			fprintf(log, "%02X", (unsigned char) (*((int8_t*) (tty + c))));
//			if (c % 16 == 15) fprintf(log, "\n");
//		}
//		fprintf(log, "\n");
//		tty[in_buf_len] = '\0';
//		fprintf(log, "Child: %s\n", tty);
        sprintf(verifier_path, VERIFIER_COMMAND_PATH_STRING_FORMAT, received_app_SN_string,attestator_number);
        fprintf(log_file, "Child: exec'ing\n%s\n", verifier_path);
        fclose(log_file);
        execl(verifier_path, VERIFIER_COMMAND, NULL);
        log_file = fopen(VERIFIER_FORWARDER_LOG_PATH, "a");
        fprintf(log_file, "Error exec\n");
        return 1;

    } else {
        fprintf(log_file, "Father\n");
        close(fd[0]);

        if (write(fd[1], &received_response_time, sizeof(received_response_time)) != sizeof(received_response_time))
            fprintf(log_file, "error WRITING time\n");
        if (write(fd[1], in_buf, (size_t) in_buf_len) != in_buf_len)
            fprintf(log_file, "error WRITING buffer\n");
    }

    fclose(log_file);
    free(in_buf);

    return 0;

}
