#include <windows.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"

#define DICTIONARY_WORD_COUNT 352253
#define MAX_WORD_LIST_SIZE 10000
#define MAX_NUM_THREADS 32

struct ctx {
    char* dictionary_file_path;
    char* jumbled_letters;
    uint8_t* jumbled_letters_freq;
    uint32_t jumbled_letter_mask;
    uint8_t included_letter;
    uint8_t sort_lexicographically;
    uint8_t sort_length;
    uint8_t allow_repeated;
};

struct word_t {
    char* word;
    int word_length;
};

struct work_order {
    char* fileContents;
    ctx* context;
    uint32_t startOffset;
    uint32_t endOffset;
};

struct work_queue {
    word_t* word_list;
    volatile uint64_t word_count;
    uint32_t WorkOrderCount;
    work_order* WorkOrders;
    volatile uint64_t NextWorkOrderIndex;
    volatile uint64_t TotalWordsFound;
    volatile uint64_t Retired;
};

int
compare_lexicographically(const void* a, const void* b)
{
    word_t* word_a = (word_t*) a;
    word_t* word_b = (word_t*) b;

    return strncmp(word_a->word, word_b->word, (word_a->word_length < word_b->word_length) ? word_a->word_length : word_b->word_length);
}

int
compare_word_length(const void* a, const void* b)
{
    word_t* word_a = (word_t*) a;
    word_t* word_b = (word_t*) b;

    if (word_a->word_length < word_b->word_length)
        return -1;

    if (word_a->word_length > word_b->word_length)
        return 1;

    return 0;
}

uint64_t
locked_add_and_return_previous_value(uint64_t volatile* Value, uint64_t Delta)
{
    uint64_t Result = InterlockedExchangeAdd64((volatile LONG64*) Value, Delta);

    return Result;
}

static void
add_word_to_list(word_t* wordl, uint64_t volatile* word_count, char* location, int size)
{
    uint64_t current_index = locked_add_and_return_previous_value(word_count, 1);

    if (current_index < MAX_WORD_LIST_SIZE) {
        wordl[current_index].word = location;
        wordl[current_index].word_length = size;
    } else {
        fprintf(stderr, "list max reached\n");
    }
}

static uint8_t
is_word_delim(int c)
{
    return c == '\n' || c == '\r' || c == ' ';
}

static uint32_t
get_cpu_count(void)
{
    SYSTEM_INFO Info;
    GetSystemInfo(&Info);

    return Info.dwNumberOfProcessors;
}

static uint32_t
get_word_mask(char* word)
{
    char* ptr = word;
    uint32_t result = 0;

    while (*ptr++) {
        result |= (1 << (*ptr - 'a'));
    }

    return result;
}

static uint32_t
process_words(work_queue* Queue)
{
    uint64_t WorkOrderIndex = locked_add_and_return_previous_value(&Queue->NextWorkOrderIndex, 1);

    if (WorkOrderIndex >= Queue->WorkOrderCount)
        return FALSE;

    work_order* Order = Queue->WorkOrders + WorkOrderIndex;
    word_t* word_list = Queue->word_list;
    char* fileContents = Order->fileContents;
    uint32_t startOffset = Order->startOffset;
    uint32_t endOffset = Order->endOffset;
    ctx* context = Order->context;
    uint64_t words_found = 0;

    char* ptr =  fileContents + startOffset;
    char* wordstart = ptr;
    uint8_t word_freq[26] = {};

    while (ptr < fileContents + endOffset) {
        if (is_word_delim(*ptr)) {
            if (!context->allow_repeated) {
                if (ptr > wordstart) {
                    memset(word_freq, 0, sizeof(word_freq));

                    char* w = wordstart;

                    while (w != ptr) {
                        word_freq[(*w - 'a')]++;
                        ++w;
                    }
                }

                if (context->included_letter)
                    if (!word_freq[(context->included_letter - 'a')])
                        goto NEXT_WORD;

                uint8_t found = 1;

                for (int i = 0; i < 26; ++i) {
                    if (word_freq[i] > context->jumbled_letters_freq[i]) {
                        found = 0;
                        break;
                    }
                }

                if (found) {
                    ++words_found;
                    add_word_to_list(word_list, &Queue->word_count, wordstart, (int) (ptr - wordstart));
                }
            } else {
                if (ptr > wordstart) {
                    uint32_t word_mask = 0;

                    for (char* w = wordstart; w != ptr; ++w) {
                        word_mask |= (1 << (*w - 'a'));
                    }

                    if (context->included_letter)
                        if (!(word_mask & (1 << (context->included_letter - 'a'))))
                            goto NEXT_WORD;

                    if ((word_mask & context->jumbled_letter_mask) == word_mask) {
                        ++words_found;
                        add_word_to_list(word_list, &Queue->word_count, wordstart, (int) (ptr - wordstart));
                    }
                }
            }

NEXT_WORD:
            while (is_word_delim(*ptr))
                ++ptr;

            wordstart = ptr;
        } else {
            ++ptr;
        }
    }

    locked_add_and_return_previous_value(&Queue->TotalWordsFound, words_found);
    locked_add_and_return_previous_value(&Queue->Retired, 1);

    return TRUE;
}

DWORD WINAPI
thread_func(LPVOID lpParam)
{
    work_queue* Queue = (work_queue*) lpParam;
    while (process_words(Queue)) {}

    return 0;
}

static void
create_worker_thread(void* parameter)
{
    DWORD thread_id;
    HANDLE wt = CreateThread(NULL, 0, thread_func, parameter, 0, &thread_id);
    CloseHandle(wt);
}

static void
usage(void)
{
    printf(
        "Usage: ./sch jumbled_letters [-i c] [-s | -a] [-d dictionary_file_path] [-h] [-r]\n"
        "Generate spellable words from jumbled letters.\n"
        "Example: ./sch \"aeuild\" -i f -s -d \"./dictionary.txt\" -r\n\n"
        "Spellable word selection:\n"
        "    -r                         allow characters within jumbled_letters to repeatedly be used\n"
        "    -i c                       all found words must include letter 'c'\n"
        "    -d dictionary_file_path    use wordlist found in dictionary_file_path\n"
        "                               NOTE: words need to be line separated and lowercase\n\n"
        "Output control:\n"
        "    -s    sort found spellable words by word size\n"
        "    -a    sort found spellable words lexicographically\n\n"
        "Miscellaneous:\n"
        "    -h    display this help message\n"
    );

    exit(-1);
}

int
main(int argc, char** argv)
{
    int opt;
    ctx context = {};
    uint8_t jumbled_letters_freq[26] = {};

    if (argc < 2)
        usage();

    context.jumbled_letters = argv[1];

    while (opt = getopt(argc, argv, "i:sad:hr"), opt != -1) {
        switch (opt) {
            case 'i':
                context.included_letter = optarg[0];
                break;

            case 's': {
                if (context.sort_lexicographically)
                    usage();

                context.sort_length = 1;
            } break;

            case 'a': {
                if (context.sort_length)
                    usage();

                context.sort_lexicographically = 1;
            } break;

            case 'd':
                context.dictionary_file_path = optarg;
                break;

            case 'h':
                usage();
                break;

            case 'r':
                context.allow_repeated = 1;
                break;

            case '?':
                usage();
                break;

            default:
                break;
        }
    }

    char* ptr = context.jumbled_letters;

    if (!context.allow_repeated) {
        while (*ptr) {
            jumbled_letters_freq[(*ptr - 'a')]++;
            ++ptr;
        }

        if (context.included_letter)
            ++jumbled_letters_freq[(context.included_letter - 'a')];

        context.jumbled_letters_freq = jumbled_letters_freq;
    } else {
        context.jumbled_letter_mask = get_word_mask(ptr);

        if (context.included_letter)
            context.jumbled_letter_mask |= (1 << (context.included_letter - 'a'));
    }

    if (!context.dictionary_file_path)
        context.dictionary_file_path = (char*) "dictionary.txt";

    char* filename = context.dictionary_file_path;
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        printf("Error opening file \"%s\": %lu\n", filename, dwError);
        return -2;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);

    if (fileSize == INVALID_FILE_SIZE) {
        DWORD dwError = GetLastError();
        printf("Error getting file size \"%s\": %lu\n", filename, dwError);
        return -3;
    }

    char* fileContents = (char*) VirtualAlloc(NULL, fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!fileContents) {
        printf("Memory allocation failed\n");
        CloseHandle(hFile);
        return -4;
    }

    DWORD bytesRead;

    if (!ReadFile(hFile, fileContents, fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        printf("Error reading file\n");
        VirtualFree(fileContents, 0, MEM_RELEASE);
        CloseHandle(hFile);
        return -5;
    }

    CloseHandle(hFile);

    word_t* word_list = (word_t*) VirtualAlloc(NULL, DICTIONARY_WORD_COUNT * sizeof(word_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    work_queue Queue = {};
    uint32_t core_count = get_cpu_count();
    uint32_t chunk_size = (fileSize + core_count - 1) / core_count;
    uint32_t last_line_end = 0;
    uint32_t lastEndOffset = 0;

    Queue.WorkOrders = (work_order*) VirtualAlloc(NULL, core_count * sizeof(work_order), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    Queue.word_list = word_list;
    Queue.word_count = 0;

    for (uint32_t i = 0; i < core_count; ++i) {
        uint32_t startOffset = lastEndOffset + 1;
        uint32_t endOffset = (i == (core_count - 1)) ? fileSize : (i + 1) * chunk_size;

        if (i < MAX_NUM_THREADS) {
            while (endOffset < fileSize && fileContents[endOffset] != '\n')
                ++endOffset;
        }

        work_order* order = Queue.WorkOrders + Queue.WorkOrderCount++;
        assert(Queue.WorkOrderCount <= core_count);

        lastEndOffset = endOffset;
        order->endOffset = endOffset;
        order->startOffset = startOffset;
        order->fileContents = fileContents;
        order->context = &context;
    }

    assert(Queue.WorkOrderCount == core_count);

    locked_add_and_return_previous_value(&Queue.NextWorkOrderIndex, 0);

    clock_t start_time = clock();

    for (uint32_t thread_index = 0; thread_index < core_count; ++thread_index) {
        create_worker_thread(&Queue);
    }

    while (Queue.Retired < core_count) {
        if (process_words(&Queue)) {
            uint32_t progress = 100 * (uint32_t) Queue.Retired / core_count;
            printf("\r%3d%% complete", progress);
            fflush(stdout);
        }
    }

    clock_t end_time = clock();
    clock_t total_time = end_time - start_time;
    printf("\r100%% complete\n\n");

    uint64_t word_count = Queue.word_count;

    if (context.sort_length || context.sort_lexicographically) {
        size_t final_word_count = (size_t) word_count;
        if (context.sort_length) {
            qsort(word_list, final_word_count, sizeof(*word_list), compare_word_length);
        }

        if (context.sort_lexicographically) {
            qsort(word_list, final_word_count, sizeof(*word_list), compare_lexicographically);
        }
    }

    for (uint64_t i = 0, i_max = word_count; i < i_max; ++i) {
        word_t current_word = word_list[i];
        printf("%.*s\n", current_word.word_length, current_word.word);
    }

    printf("\n**********************************************************\n");
    printf("** STATISTICS\n");
    printf("**********************************************************\n");
    printf("** TotalCores      :  %u\n", core_count);
    printf("** TotalTime       : ~%ld ms\n", total_time);
    printf("** TotalWords      :  %u words\n", DICTIONARY_WORD_COUNT);
    printf("** WordsFound      :  %llu words\n", Queue.TotalWordsFound);
    printf("** TimePerWord     : ~%f ms\n", (float) total_time / (float) DICTIONARY_WORD_COUNT);
    printf("**********************************************************\n\n");

    // VirtualFree(fileContents, 0, MEM_RELEASE);

    return 0;
}
