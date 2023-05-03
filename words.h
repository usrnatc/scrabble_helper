// words.h
// Nathan Corcoran, 2023
//

#if !defined(__WORDS_H__)
#define __WORDS_H__

#include <stdlib.h>

#define BUFFER 1024

typedef struct Words {
    char **words;
    size_t used;
    size_t size;
} words_t;

void free_words(words_t *);
void remove_word(words_t *, int);

/**
 * Inserts a word into the words_t struct at the end of the array. If the
 * array is full, it will be resized to fit the new word.
 *
 * @param words A pointer to the words_t struct to insert the word into.
 * @param word A pointer to the word to insert into the words_t struct.
 */
void
insert_word(
    words_t *words, 
    char    *word
) {
    if (!words || !word)
        return;

    if (words->used == words->size) {
        
        char **tmp_words;

        words->size++;
        tmp_words = realloc(words->words, words->size * sizeof(*words->words));
        if (!tmp_words) {

            free_words(words);
            free(word);
            return;
        }

        words->words = tmp_words;
        words->words[words->used] = malloc(BUFFER * sizeof(**words->words));
        if (!words->words[words->used]) {

            remove_word(words, words->used);
            free(word);
            return;
        }
    }
    strcpy(words->words[words->used++], word);
    free(word);
}

/**
 * Creates a new words_t struct with the given initial size.
 *
 * @param initial_size The initial size of the words_t struct.
 * @return A pointer to the newly created words_t struct.
 */
words_t *
new_words(size_t initial_size)
{
    words_t *words;

    words = malloc(sizeof(*words));
    if (!words)
        goto exit;

    words->size = initial_size;
    words->used = 0;

    if (!words->size)
        goto exit;

    words->words = malloc(words->size * sizeof(*words->words));
    if (!words->words) {

        free(words);
        words = NULL;
        goto exit;
    }

    for (int i = 0; i < words->size; i++) {

        words->words[i] = malloc(BUFFER * sizeof(char));
        if (!words->words[i]) {

            free_words(words); /* NULLs words */
            goto exit;
        }
    }

exit:
    return (words);
}

/**
 * Frees the memory allocated to the words_t struct.
 *
 * @param words A pointer to the words_t struct to free.
 */
void
free_words(words_t *words)
{
    for (int i = 0; i < words->size; i++) {

        free(words->words[i]);
    }
    free(words->words);
    words->words = NULL;
    words->used = words->size = 0;
    free(words);
    words = NULL;
}

/**
 * Removes a word from the words_t struct at the given index.
 *
 * @param words A pointer to the words_t struct to remove the word from.
 * @param index The index of the word to remove.
 */
void
remove_word(
    words_t *words,
    int      index
) {
    if (index > words->used)
        return;

    free(words->words[index]);

    for (int i = index; i < words->used; i++) {

        words->words[i] = words->words[i + 1];
    }
    words->used--;
}

/**
 * Removes all words in the provided words_t structure except the longest
 * words. The function modifies the words_t structure in place.
 *
 * @param words A pointer to a words_t structure containing the words to be
 *              filtered.
 */
void
keep_longest_words(words_t *words)
{
    int max_length;
    int num_words;
    int word_length;

    max_length = strlen(words->words[0]);
    num_words = words->used;

    for (int i = 0; i < num_words; i++) {

        word_length = strlen(words->words[i]);
        max_length = (word_length > max_length) ? word_length : max_length;
    }

    for (int i = 0; i < num_words; i++) {

        word_length = strlen(words->words[i]);
        if (word_length < max_length) {
            remove_word(words, i);
            i--;
            num_words--;
        }
    }
}

/**
 * Searches a given words_t structure and removes all words that
 * do not contain a given letter. The function is case-insensitive,
 * so it considers both upper and lower case versions of the given letter.
 *
 * @param words A pointer to a words_t structure to be searched
 * @param letter Letter to search for in the words of the words_t structure
 */
void
keep_included_word(
    words_t *words,
    char     letter
) {
    int     num_words;
    char    letter_other_case;

    num_words = words->used;
    letter_other_case = (isupper(letter)) ? tolower(letter) : toupper(letter);

    for (int i = 0; i < num_words; i++) {

        if (!strchr(words->words[i], letter) &&
                !strchr(words->words[i], letter_other_case)) {

            remove_word(words, i);
            i--;
            num_words--;
        }
    }
}

#endif /* __WORDS_H__ */
