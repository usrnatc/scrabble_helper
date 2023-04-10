// scrabble_helper.h
// Nathan Corcoran, 2023
//

#if !defined(__SCRABBLE_HELPER_H__)
#define __SCRABBLE_HELPER_H__

#include <stdint.h>
#include <getopt.h>
#include <errno.h>

#include "words.h"

#define ALPHABET_LENGTH 26
#define SORT_LEXICOGRAPHIC 1
#define SORT_LENGTH 2
#define SORT_LONGEST 3
#define CTX_INCLUDE 1
#define STD_DICTIONARY "sample_dictionary.txt"

typedef struct CharCount {
    int letters[ALPHABET_LENGTH];
} char_count_t;

typedef struct ScrabbleHelperCtx {
    uint8_t          sort;
    uint8_t          include : 1;
    char            *jumbled_letters;
    char_count_t    *jumbled_letter_frequency;
    char             included_letter;
    char            *dictionary_filepath;
    words_t         *matching_words;
} scrabble_ctx_t;

struct option longopts[] = {
    {"alpha", no_argument, NULL, 'a'},
    {"len", no_argument, NULL, 'l'},
    {"longest", no_argument, NULL, 'o'},
    {"include", required_argument, NULL, 'i'},
    {NULL, no_argument, NULL, 0}
};

int word_allowed_length(char *);
int word_only_alphabetical(char *);
char_count_t *count_letter_frequency(char *);
void free_context(scrabble_ctx_t *);
scrabble_ctx_t *generate_context(int, char **);
void keep_included_word(words_t *, char);
void keep_longest_words(words_t *);
int compare_word_lexicographical(const void *, const void *);
int compare_word_length(const void *, const void *);
int file_accessible(char *);
char *read_line(FILE *);
void find_matching_words(scrabble_ctx_t *);
void display_matching_words(scrabble_ctx_t *);

#endif /* __SCRABBLE_HELPER_H__ */
