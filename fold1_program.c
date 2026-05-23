/**
 * Fold 1: Implicit + Explicit Threading
 * Student: Yisak Demelash Metaferiya
 * ID: 1093891
 * CSC308 - Operating Systems, Lab 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <omp.h>
#include <time.h>

#define MAX_LINE_LEN   4096
#define MAX_WORD_LEN   512
#define NUM_THREADS    4
#define STUDENT_ID     "1093891"
#define STUDENT_NAME  "Yisak Demelash Metaferiya"

/* ───── Implicit Threading Results ───── */
typedef struct {
    long total_lines;
    long total_words;
    long total_vowels;
    int  num_threads_used;
} ImplicitResult;

/* ───── Explicit Thread Argument/Result ───── */
typedef struct {
    int    thread_id;
    char **lines;
    long   start;
    long   end;
    char   longest_word[MAX_WORD_LEN];
    long   uppercase_count;
} ThreadArg;

/* ─── Helper: is vowel ─── */
static inline int is_vowel(char c) {
    c = tolower(c);
    return c=='a'||c=='e'||c=='i'||c=='o'||c=='u';
}

/* ─── Implicit threading method ─── */
ImplicitResult perform_implicit_threading(char **lines, long total_lines) {
    ImplicitResult res = {0, 0, 0, 0};

    long line_count = 0, word_count = 0, vowel_count = 0;
    int  threads_used = 0;

    #pragma omp parallel reduction(+:line_count, word_count, vowel_count)
    {
        #pragma omp single
        {
            threads_used = omp_get_num_threads();
        }

        #pragma omp for schedule(dynamic, 1000)
        for (long i = 0; i < total_lines; i++) {
            line_count++;
            char *p = lines[i];//[*/*/*/*]
            int in_word = 0;
            while (*p) {
                if (is_vowel(*p)) vowel_count++;
                if (isspace((unsigned char)*p)) {
                    if (in_word) { word_count++; in_word = 0; }
                } else {
                    in_word = 1;
                }
                p++;
            }
            if (in_word) word_count++;
        }
    }

    res.total_lines  = line_count;
    res.total_words  = word_count;
    res.total_vowels = vowel_count;
    res.num_threads_used = threads_used;
    return res;
}

/* ─── Explicit thread worker ─── */
void *thread_worker(void *arg) {
    ThreadArg *ta = (ThreadArg *)arg;
    ta->uppercase_count = 0;
    ta->longest_word[0] = '\0';
    int longest_len = 0;

    printf("[Thread %lu] (arg_id=%d) processing lines %ld – %ld  [Student ID: %s]\n",
           (unsigned long)pthread_self(), ta->thread_id, ta->start, ta->end - 1, STUDENT_ID);

    for (long i = ta->start; i < ta->end; i++) {
        char buf[MAX_LINE_LEN];
        strncpy(buf, ta->lines[i], MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';

        char *tok = strtok(buf, " \t\n\r");
        while (tok) {
            /* uppercase count */
            for (char *c = tok; *c; c++) {
                if (isupper((unsigned char)*c)) ta->uppercase_count++;
            }
            /* longest word */
            int len = (int)strlen(tok);
            if (len > longest_len) {
                longest_len = len;
                strncpy(ta->longest_word, tok, MAX_WORD_LEN - 1);
                ta->longest_word[MAX_WORD_LEN - 1] = '\0';
            }
            tok = strtok(NULL, " \t\n\r");
        }
    }

    printf("[Thread %lu] (arg_id=%d) done. Longest='%s' Uppercase=%ld  [Student ID: %s]\n",
           (unsigned long)pthread_self(), ta->thread_id,
           ta->longest_word, ta->uppercase_count, STUDENT_ID);
    return NULL;
}

/* ─── Explicit threading method ─── */
void perform_explicit_threading(char **lines, long total_lines,
                                 char *out_longest, long *out_uppercase) {
    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];
    long workload = total_lines / NUM_THREADS;

    printf("\n[Main Thread PID-based ID] Launching %d explicit threads  [Student ID: %s]\n",
           NUM_THREADS, STUDENT_ID);

    for (int t = 0; t < NUM_THREADS; t++) {
        args[t].thread_id       = t;
        args[t].lines           = lines;
        args[t].start           = t * workload;
        args[t].end             = (t == NUM_THREADS - 1) ? total_lines : (t + 1) * workload;
        args[t].uppercase_count = 0;
        args[t].longest_word[0] = '\0';
        pthread_create(&threads[t], NULL, thread_worker, &args[t]);
    }

    long   total_upper = 0;
    int    longest_len = 0;
    out_longest[0] = '\0';

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
        total_upper += args[t].uppercase_count;
        int len = (int)strlen(args[t].longest_word);
        if (len > longest_len) {
            longest_len = len;
            strncpy(out_longest, args[t].longest_word, MAX_WORD_LEN - 1);
        }
    }

    *out_uppercase = total_upper;
}

/* ───────────────── main ───────────────── */
int main(int argc, char *argv[]) {
    const char *filename = "LargeFile/LargeFile.txt";

    printf("============================================================\n");
    printf("  Fold 1: Implicit + Explicit Threading\n");
    printf("  Student: %s |  ID: %s\n", STUDENT_NAME, STUDENT_ID);
    printf("============================================================\n\n");

    /* ── Read file ── */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("failed to open"); return 1; }

    /* Count lines first */
    long total_lines = 0;
    char buf[MAX_LINE_LEN];
    while (fgets(buf, sizeof(buf), fp)) total_lines++;
    rewind(fp);

    /* Allocate line array */
    char **lines = (char **)malloc(total_lines * sizeof(char *));
    if (!lines) { fprintf(stderr, "malloc failed\n"); fclose(fp); return 1; }

    for (long i = 0; i < total_lines; i++) {
        if (!fgets(buf, sizeof(buf), fp)) break;
        lines[i] = strdup(buf);
    }
    fclose(fp);

    printf("[Main Thread] File loaded: %ld lines  [Student ID: %s]\n\n", total_lines, STUDENT_ID);

    /* ── Method 1: Implicit Threading ── */
    printf("--- Method 1: Implicit Threading (OpenMP) ---\n");
    omp_set_num_threads(4);
    ImplicitResult imp = perform_implicit_threading(lines, total_lines);
    printf("[Implicit] Threads used    : %d\n",  imp.num_threads_used);
    printf("[Implicit] Total lines     : %ld\n", imp.total_lines);
    printf("[Implicit] Total words     : %ld\n", imp.total_words);
    printf("[Implicit] Total vowels    : %ld\n", imp.total_vowels);

    /* ── Method 2: Explicit Threading ── */
    printf("\n--- Method 2: Explicit Threading (pthreads) ---\n");
    char longest_word[MAX_WORD_LEN] = {0};
    long total_uppercase = 0;
    perform_explicit_threading(lines, total_lines, longest_word, &total_uppercase);

    /* ── Final Results ── */
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    printf("\n============================================================\n");
    printf("  FINAL RESULTS  [Student ID: %s]\n", STUDENT_ID);
    printf("============================================================\n");
    printf("  Total lines          : %ld\n",  imp.total_lines);
    printf("  Total words          : %ld\n",  imp.total_words);
    printf("  Total vowels         : %ld\n",  imp.total_vowels);
    printf("  Longest word         : %s\n",   longest_word);
    printf("  Total uppercase      : %ld\n",  total_uppercase);
    printf("  Execution time       : %.4f seconds\n", elapsed);
    printf("============================================================\n");

    /* cleanup */
    for (long i = 0; i < total_lines; i++) free(lines[i]);
    free(lines);
    return 0;
}
