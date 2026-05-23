/**
 * Fold 3: No Threads (Sequential)
 * Student: Yisak Demelash Metaferiya
 * ID: 1093891
 * CSC308 - Operating Systems, Lab 3
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE_LEN  4096
#define MAX_WORD_LEN  512
#define STUDENT_ID    "1093891"
#define STUDENT_NAME  "Yisak Demelash Metaferiya"


static inline int is_vowel(char c) {
    c = tolower(c);
    return c=='a'||c=='e'||c=='i'||c=='o'||c=='u';
}

int main() {
    const char *filename = "LargeFile/LargeFile.txt";

    printf("------------------------------------------------------\n");
    printf("  Fold 3: Sequential (No Threads)\n");
    printf("  Student:  %s |  ID: %s\n",STUDENT_NAME, STUDENT_ID);
    printf("------------------------------------------------------\n");

    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fail to open"); return 1;
    }

    long line_count = 0, word_count = 0, vowel_count = 0, upper_count = 0;
    char longest_word[MAX_LINE_LEN] = {0};
    int longest_len = 0;

    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), fp)) {
        line_count++;

        char *p = buffer;
        int in_word = 0;
        char word_buffer[MAX_LINE_LEN];
        int word_len = 0;

        while (*p) {
            if (is_vowel(*p)) {
                vowel_count++;
            }
            if (isupper((unsigned char) *p)) { upper_count++; }
            if (isspace((unsigned char) *p)) {
                if (in_word) {
                    word_buffer[word_len] = '\0';
                    word_count++;
                    if (word_len > longest_len) {
                        longest_len = word_len;
                        strncpy(longest_word, word_buffer, MAX_WORD_LEN-1);
                    }
                    in_word = 0;
                    word_len = 0;
                }
            } else {
                if (!in_word) { in_word = 1; }
                if (word_len < MAX_WORD_LEN - 1) {
                    word_buffer[word_len++] = *p;
                }
            }
            p++;
        }

        if (in_word) {
            word_buffer[word_len] = '\0';
            word_count++;
            if (word_len > longest_len) {
                longest_len = word_len;
                strncpy(longest_word, word_buffer, MAX_WORD_LEN-1);
            }
        }

        // progress print
        if (line_count % 200000 == 0)
            printf("[Sequential] Processed %ld lines so far...  [Student ID: %s]\n",
                  line_count, STUDENT_ID);
    }
    fclose(fp);

    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n============================================================\n");
    printf("  FINAL RESULTS  [Student ID: %s]\n", STUDENT_ID);
    printf("============================================================\n");
    printf("  Total lines          : %ld\n", line_count);
    printf("  Total words          : %ld\n", word_count);
    printf("  Total vowels         : %ld\n", vowel_count);
    printf("  Total uppercase      : %ld\n", upper_count);
    printf("  Longest word         : %s\n",  longest_word);
    printf("  Execution time       : %.4f seconds\n", elapsed);
    printf("============================================================\n");

    return 0;



}
