#include "config_file.h"

#define _CONFIG_DEFAULT_NUM_ENTRIES 100

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static void _free_all_entries(config_file_entry_t *ptr, size_t num);

static config_file_entry_t *_resize(config_file_entry_t *old, size_t old_size, size_t new_size) {
    if (old == NULL) {
        config_file_entry_t *entries =
                malloc(sizeof(config_file_entry_t) * _CONFIG_DEFAULT_NUM_ENTRIES);
        if (!entries) {
            perror("Cannot allocate memory for inital entries structures\n");
        }
        return entries;
    }
    config_file_entry_t *entries = malloc(sizeof(config_file_entry_t) * new_size);
    if (!entries) {
        perror("Cannot allocate memory to resize entry count");
        _free_all_entries(old, old_size);
        goto out_free;
    }
    memcpy(entries, old, sizeof(config_file_entry_t) * old_size);
out_free:
    free(old);
    return entries;
}

static void _free_all_entries(config_file_entry_t *ptr, size_t num) {
    if (!ptr) {
        return;
    }
    for (size_t i = 0; i < num; i++) {
        free(ptr[i].key);
        free(ptr[i].value);
    }
}

config_file_t *read_config_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file for reading");
        return NULL;
    }
    config_file_t *cf = NULL;
    config_file_entry_t *entries = _resize(NULL, 0, 0);
    if (entries == NULL) {
        goto out;
    }
    int count = 0;
    size_t position = 0;
    size_t cur_size = _CONFIG_DEFAULT_NUM_ENTRIES;
    do {
        if (position >= cur_size) {
            entries = _resize(entries, cur_size, cur_size + 10);
            if (entries == NULL) {
                goto out;  // if _resize fails, all entries have been freed
            }
            cur_size += 10;
        }
        errno = 0;
        count = fscanf(file, "%m[^=]=%ms ", &entries[position].key, &entries[position].value);
        if (count == EOF) {
            break;
        }
        position++;
        if (count < 2) {
            break;
        }
    } while (true);

    /*
     * When we left the loop, we must have count != 2.
     * It can either be
     *  EOF, which means we read everything (good, unless errno is
     *      != 0).
     *  or it can be between 0 and 2, which means we failed to parse
     *      something (bad).
     */
    if (count == EOF) {
        // position holds the number of entries successfuly read
        if (errno) {
            // so an error occurred while reading... we will clean up memory
            // and then return from this method with NULL
            perror("Error reading from config file");
            _free_all_entries(entries, position);
            goto out_entries;
        }
        cf = malloc(sizeof(config_file_t) + position * sizeof(config_file_entry_t));
        if (!cf) {
            perror("Cannot allocate memory for config file structure");
            _free_all_entries(entries, position);
            goto out_entries;
        }
        // now we copy all the entries that found into the structure that we will
        // return.
        (void)memcpy(cf->entries, entries, position * sizeof(config_file_entry_t));
        cf->nr_entries = position;
    } else {  // count == 0 || count == 1

        // position holds 1 + the number of successfuly read entries.
        // position - 1 might hold "half an entry" if parsing failed mid-way
        fprintf(stderr,
                "Error reading from file. Could not parse format. Failed on entry %lu (counting "
                "from 0)",
                position - 1);

        // be sure to free the "half entry" as well
        if (count == 1) {
            fprintf(stderr, "\n\tGot this far: %s\n", entries[position - 1].key);
            free(entries[position - 1].key);
        }
        // free whatever we read successfully
        _free_all_entries(entries, position - 1);
    }
out_entries:
    free(entries);
out:
    if (fclose(file)) {
        perror("Could not close file");
    }

    return cf;
}

int release_config(config_file_t *cf) {
    if (cf) {
        if (cf->nr_entries <= 0) {
            return 1;
        }
        _free_all_entries(cf->entries, (size_t)cf->nr_entries);
        free(cf);
    }
    return 1;
}
