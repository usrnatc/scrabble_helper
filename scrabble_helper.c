// scrabble_helper.c
// Nathan Corcoran, 2023
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <getopt.h>

#include "scrabble_helper.h"
#include "errors.h"
#include "words.h"

/**
 * Reads a line from a given input stream and returns it as a null-terminated
 * string. The function allocates memory for the string dynamically, so the
 * caller is responsible for freeing the memory when it is no longer needed.
 *
 * @param in_stream A pointer to a FILE object representing the input stream
 *                  to read from.
 * @return          A pointer to the null-terminated string read from the input
 *                  stream, or NULL if an error occurred or the end of file was
 *                  reached without reading any characters.
 */
char *
read_line(FILE *in_stream)
{
    int      buffer_size;
    int      num_read;
    int      next;
    char    *buffer;

    buffer_size = BUFFER;
    buffer = malloc(buffer_size * sizeof(*buffer));
    if (!buffer)
        goto exit;

    num_read = 0;
    while (1) {

        next = fgetc(in_stream);
        if (next == EOF && num_read == 0) {

            free(buffer);
            buffer = NULL;
            goto exit;
        }

        if (next == '\n' || next == EOF) {

            buffer[num_read] = '\0';
            goto exit;
        }

        buffer[num_read++] = next;
        if (num_read == buffer_size) {

            char *temp_buffer;

            buffer_size++;
            temp_buffer = realloc(buffer, buffer_size * sizeof(*buffer));
            if (!temp_buffer) {

                free(buffer);
                buffer = NULL;
                goto exit;
            }
            buffer = temp_buffer;
        }
    }

exit:
    return (buffer);
}

/**
 * Checks if a file is accessible for reading. The function attempts to open
 * the file specified by the given file path using "r" mode and then closes it
 * immediately. If the file is accessible, the function returns 1. Otherwise,
 * it returns 0.
 *
 * @param filepath A pointer to a null-terminated string representing the file
 *                 path to check.
 * @return         1 if the file is accessible, 0 otherwise.
 */
int
file_accessible(char *filepath)
{
    FILE *file;

    if (!filepath)
        return (0);

    file = fopen(filepath, "r");
    if (!file)
        return (0);

    fclose(file);
    return (1);
}

/**
 * Compares two strings (words) by their length and then by their lexical order.
 * The function takes two pointers to constant void as input. These are expected
 * to be pointers to two null-terminated strings that are to be compared.
 *
 * @param word_a A pointer to a constant void representing the first word to
 *               compare.
 * @param word_b A pointer to a constant void representing the second word to
 *               compare.
 *
 * @return      An integer less than, equal to, or greater than 0 if the first
 *              string is shorter than, equal to, or longer than the second
 *              string, respectively. If the strings are of the same length,
 *              the function returns 0 if they are lexicographically equal or
 *              a negative or positive integer if the first string is
 *              lexicographically less than or greater than the second string,
 *              respectively.
 */
int
compare_word_length(
    const void *word_a,
    const void *word_b
) {
    const char *comparable_word_a;
    const char *comparable_word_b;
    size_t length_a;
    size_t length_b;

    comparable_word_a = *(char * const *) word_a;
    comparable_word_b = *(char * const *) word_b;
    length_a = strlen(comparable_word_a);
    length_b = strlen(comparable_word_b);

    if (length_a == length_b) {

        int lexical_comp;

        lexical_comp = strcasecmp(comparable_word_a, comparable_word_b);
        if (!lexical_comp)
            return (strcmp(comparable_word_a, comparable_word_b));
        return (lexical_comp);
    }

    if (length_a > length_b)
        return (-1);

    return (1);
}

/**
 * Compares two strings (words) lexicographically in a case-insensitive manner.
 * The function takes two pointers to constant void as input. These are expected
 * to be pointers to two null-terminated strings that are to be compared.
 *
 * @param word_a A pointer to a constant void representing the first word to
 *               compare.
 * @param word_b A pointer to a constant void representing the second word to
 *               compare.
 *
 * @return      An integer less than, equal to, or greater than 0 if the first
 *              string is lexicographically less than, equal to, or greater than
 *              the second string, respectively. The comparison is
 *              case-insensitive.
 */
int
compare_word_lexicographical(
    const void *word_a,
    const void *word_b
) {
    int         compare;
    const char *comparable_word_a;
    const char *comparable_word_b;

    comparable_word_a = *(char * const *) word_a;
    comparable_word_b = *(char * const *) word_b;

    compare = strcasecmp(comparable_word_a, comparable_word_b);
    if (!compare)
        return (strcmp(comparable_word_a, comparable_word_b));

    return (compare);
}

/**
 * Generates and initializes a scrabble context based on the command
 * line arguments provided by the user.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of pointers to the command line arguments.
 * @return A pointer to the generated scrabble context.
 */
scrabble_ctx_t *
generate_context(
    int     argc,
    char  **argv
) {
    scrabble_ctx_t *context;
    int             option;

    context = malloc(sizeof(*context));
    if (!context)
        err_and_exit(ERR_OOM_CODE, ERR_OOM_MSG, __LINE__);

    context->sort = 0;
    context->include = 0;

    opterr = 0;
    do {

        option = getopt_long_only(argc, argv, ":", longopts, NULL);

        switch (option) {
            case 'a':
                if (!context->sort) {
                    
                    context->sort = SORT_LEXICOGRAPHIC;
                    break;
                }
                free(context);
                err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);

            case 'l':
                if (!context->sort) {

                    context->sort = SORT_LENGTH;
                    break;
                }
                free(context);
                err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);

            case 'o':
                if (!context->sort) {
                    
                    context->sort = SORT_LONGEST;
                    break;
                }
                free(context);
                err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);

            case 'i':
                if (!context->include) {

                    context->include = CTX_INCLUDE;
                    if (strlen(optarg) == 1 && isalpha(*optarg)) {

                        context->included_letter = *optarg;
                        break;
                    }
                }
                free(context);
                err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);

            case '?':
                free(context);
                err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);
        }
    } while (option != -1);

    argc -= optind;
    argv += optind;


    /* at this point could have just the jumbled letters or both the jumbled
     * letters and the dictionary path provided                             */
    if (argc < 1 || argc > 2) {

        free(context);
        err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);
    }

    context->jumbled_letters = strdup(argv[0]);
    if (!word_only_alphabetical(context->jumbled_letters) ||
            !word_allowed_length(context->jumbled_letters)) {

        free(context->jumbled_letters);
        free(context);
        err_and_exit(ERR_USAGE_CODE, ERR_USAGE_MSG, __LINE__);
    }
    context->jumbled_letter_frequency = count_letter_frequency(context->jumbled_letters);

    if (argc == 2) {
        context->dictionary_filepath = strdup(argv[1]);
        if (!file_accessible(context->dictionary_filepath)) {

            free(context->jumbled_letters);
            free(context->jumbled_letter_frequency);
            free(context->dictionary_filepath);
            free(context);
            err_and_exit(ERR_FILE_CODE, ERR_FILE_MSG, __LINE__);
        }
    } else {

        context->dictionary_filepath = strdup(STD_DICTIONARY);
        if (!file_accessible(context->dictionary_filepath)) {

            free(context->jumbled_letters);
            free(context->jumbled_letter_frequency);
            free(context->dictionary_filepath);
            free(context);
            err_and_exit(ERR_FILE_CODE, ERR_FILE_MSG, __LINE__);
        }
    }

    return (context);
}

/**
 * Frees the memory allocated to a scrabble context.
 *
 * @param context A pointer to the scrabble context to be freed.
 */
void
free_context(scrabble_ctx_t *context)
{
    if (!context)
        return;

    free(context->jumbled_letters);
    free(context->jumbled_letter_frequency);
    free(context->dictionary_filepath);
    free_words(context->matching_words);
    free(context);
}

/**
 * Counts the frequency of each letter in a given word.
 *
 * @param word Pointer to the string (word) to be counted.
 * @return A pointer to a char_count_t structure containing the letter frequency.
 */
char_count_t *
count_letter_frequency(char *word)
{
    char_count_t *letter_frequency;

    letter_frequency = calloc(1, sizeof(*letter_frequency));

    if (!word || !strlen(word))
        goto exit;

    for (int i = 0; i < strlen(word); i++) {

        int index;

        if (islower(word[i]))
            index = word[i] - 'a';
        if (isupper(word[i]))
            index = word[i] - 'A';

        letter_frequency->letters[index]++;
    }

exit:
    return (letter_frequency);
}

/**
 * Checks if a given word contains only alphabetical characters.
 *
 * @param word Pointer to the string (word) to be checked.
 * @return 1 if the word contains only alphabetical characters, 0 otherwise.
 */
int
word_only_alphabetical(char *word)
{
    int only_alphabetical;

    if (!word)
        return (0);

    only_alphabetical = 1;

    for (int i = 0; i < strlen(word); i++) {

        if (!isalpha(word[i])) {

            only_alphabetical = 0;
            goto exit;
        }
    }

exit:
    return (only_alphabetical);
}

/**
 * Checks if a given word is of an allowed length (greater than or equal to 3).
 *
 * @param word Pointer to the string (word) to be checked.
 * @return 1 if the word is of an allowed length, 0 otherwise.
 */
int
word_allowed_length(char *word)
{
    int length;

    if (!word)
        return (0);

    length = strlen(word);

    return (length >= 3);
}

/**
 * This function takes a pointer to a scrabble_ctx_t struct and searches the
 * dictionary file for all words that can be formed from the jumbled letters
 * provided. It then populates the matching_words array of the
 * context with these words.
 *
 * @param context A pointer to the scrabble context.
 */
void
find_matching_words(
    scrabble_ctx_t  *context
) {
    char *next;
    FILE *dictionary;

    dictionary = fopen(context->dictionary_filepath, "r");
    
    /* sanity check */
    if (!dictionary) {
        free_context(context);
        err_and_exit(ERR_FILE_CODE, ERR_FILE_MSG, __LINE__);
    }

    context->matching_words = new_words(0);

    while (next = read_line(dictionary), next) {

        char_count_t *dictionary_word_letter_frequency;

        if (!word_only_alphabetical(next) || !word_allowed_length(next)) {

            free(next);
            continue;
        }

        dictionary_word_letter_frequency = count_letter_frequency(next);

        for (int i = 0; i < ALPHABET_LENGTH; i++) {

            if (dictionary_word_letter_frequency->letters[i] > 
                    context->jumbled_letter_frequency->letters[i])
                break;

            if (i == 25)
                insert_word(context->matching_words, strdup(next));
        }

        free(next);
        free(dictionary_word_letter_frequency);
    }
    fclose(dictionary);
}

/**
 * This function takes a pointer to a scrabble_ctx_t struct, then sorts (if
 * necessary) and displays the matching words found in the dictionary file.
 *
 * @param context A pointer to the scrabble context.
 */
void
display_matching_words(
    scrabble_ctx_t  *context
) {
    switch (context->sort) {
        case SORT_LEXICOGRAPHIC:
            qsort(context->matching_words->words, context->matching_words->used,
                    sizeof(*context->matching_words->words), 
                    compare_word_lexicographical);
            break;

        case SORT_LENGTH:
            qsort(context->matching_words->words, context->matching_words->used,
                    sizeof(*context->matching_words->words), compare_word_length);
            break;

        case SORT_LONGEST:
            keep_longest_words(context->matching_words);
            break;
    }

    if (context->include)
        keep_included_word(context->matching_words, context->included_letter);

    if (!context->matching_words->used) {

        free_context(context);
        err_and_exit(ERR_NOTHING_CODE, ERR_NOTHING_MSG, __LINE__);
    }

    for (int i = 0; i < context->matching_words->used; i++) {

        printf("%s\n", context->matching_words->words[i]);
    }
}

int
main(
    int argc,
    char **argv
) {
    scrabble_ctx_t *context;

    context = generate_context(argc, argv);

    find_matching_words(context);
    display_matching_words(context);

    free_context(context);

    err_and_exit(ERR_SUCCESS_CODE, ERR_SUCCESS_MSG, __LINE__);

    /* should be unreachable */
    return (0);
}

