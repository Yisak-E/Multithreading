/*
 * Fold 2: Implicit Threading ONLY (OpenMP)
 * Student: Yisak Demelash Metaferiya
 * ID: 1093891
 * CSC308 - Operating Systems, Lab 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <omp.h>
#include <time.h>

#define MAX_LINE_LEN  4096
#define MAX_WORD_LEN  512
#define STUDENT_ID    "1093891"
#define STUDENT_NAME  "Yisak Demelash Metaferiya"

static inline int is_vowel(char c) {
    c = tolower(c);
    return c=='a'||c=='e'||c=='i'||c=='o'||c=='u';
}

int main(void) {
    const char *filename = "LargeFile/LargeFile.txt";

    printf("============================================================\n");
    printf("  Fold 2: Implicit Threading Only (OpenMP)\n");
    printf("  Student: %s |  ID: %s\n",STUDENT_NAME, STUDENT_ID);
    printf("============================================================\n\n");

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    /* Read file into memory */
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("fopen"); return 1; }

    long total_lines = 0;
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), fp)) total_lines++;
    rewind(fp);

    char **lines = (char **)malloc(total_lines * sizeof(char *));
    for (long i = 0; i < total_lines; i++) {
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            // handle error or EOF
        }

        lines[i] = strdup(buffer);
    }
    fclose(fp);

    printf("[Main] File loaded: %ld lines  [Student ID: %s]\n\n", total_lines, STUDENT_ID);

    long line_count = 0, word_count = 0, vowel_count = 0, upper_count = 0;
    int  threads_used = 0;

    /* Per-thread longest word tracking */
    int max_threads = omp_get_max_threads();
    char **local_longest = (char **)malloc(max_threads * sizeof(char *));
    int  *local_longest_len = (int *)calloc(max_threads, sizeof(int));
    for (int t = 0; t < max_threads; t++) {
        local_longest[t] = (char *)calloc(MAX_WORD_LEN, 1);
    }

    omp_set_num_threads(4);

    #pragma omp parallel reduction(+:line_count,word_count,vowel_count,upper_count)
    {
        int tid = omp_get_thread_num();

        #pragma omp single
        {
            threads_used = omp_get_num_threads();
            printf("[OpenMP] Number of threads used: %d  [Student ID: %s]\n",
                   threads_used, STUDENT_ID);
        }

        #pragma omp for schedule(dynamic, 500)
        for (long i = 0; i < total_lines; i++) {
            line_count++;
            char tmp[MAX_LINE_LEN];
            strncpy(tmp, lines[i], MAX_LINE_LEN - 1);
            tmp[MAX_LINE_LEN - 1] = '\0';

            char *p = tmp;
            int in_word = 0;
            char word_buffer[MAX_WORD_LEN];
            int  wlen = 0;

            while (*p) {
                if (is_vowel(*p)) vowel_count++;
                if (isupper((unsigned char)*p)) upper_count++;

                if (isspace((unsigned char)*p)) {
                    if (in_word) {
                        word_count++;
                        word_buffer[wlen] = '\0';
                        if (wlen > local_longest_len[tid]) {
                            local_longest_len[tid] = wlen;
                            strncpy(local_longest[tid], word_buffer, MAX_WORD_LEN - 1);
                        }
                        in_word = 0; wlen = 0;
                    }
                } else {
                    if (wlen < MAX_WORD_LEN - 1) word_buffer[wlen++] = *p;
                    in_word = 1;
                }
                p++;
            }
            if (in_word) {
                word_count++;
                word_buffer[wlen] = '\0';
                if (wlen > local_longest_len[tid]) {
                    local_longest_len[tid] = wlen;
                    strncpy(local_longest[tid], word_buffer, MAX_WORD_LEN - 1);
                }
            }
        }
    }

    /* Merge longest words */
    char global_longest[MAX_WORD_LEN] = {0};
    int  global_len = 0;
    for (int t = 0; t < max_threads; t++) {
        if (local_longest_len[t] > global_len) {
            global_len = local_longest_len[t];
            strncpy(global_longest, local_longest[t], MAX_WORD_LEN - 1);
        }
        free(local_longest[t]);
    }
    free(local_longest);
    free(local_longest_len);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    printf("\n============================================================\n");
    printf("  FINAL RESULTS  [Student ID: %s]\n", STUDENT_ID);
    printf("============================================================\n");
    printf("  Threads used         : %d\n",  threads_used);
    printf("  Total lines          : %ld\n", line_count);
    printf("  Total words          : %ld\n", word_count);
    printf("  Total vowels         : %ld\n", vowel_count);
    printf("  Total uppercase      : %ld\n", upper_count);
    printf("  Longest word         : %s\n",  global_longest);
    printf("  Execution time       : %.4f seconds\n", elapsed);
    printf("============================================================\n");

    for (long i = 0; i < total_lines; i++) free(lines[i]);
    free(lines);
    return 0;
}
