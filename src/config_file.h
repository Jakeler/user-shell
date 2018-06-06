#ifndef _CONFIG_FILE_H
#define _CONFIG_FILE_H

typedef struct {
    char *key;
    char *value;
} config_file_entry_t;

typedef struct {
    int nr_entries;
    config_file_entry_t entries[];
} config_file_t;

/**
 * Opens a config file that is expected at the location
 * indicated by filename.
 *
 * The method will return a pointer to a valid cofig_file_t
 * structure on success and NULL on error.
 *
 * NOTE: The structure returned by this method must be freed by
 * the caller using the release_config method below.
 */
config_file_t *read_config_file(const char *filename);

/**
 * This method will release the config file
 * pointed to by config.
 *
 * This method will return 1 on success, and 0
 * on error.
 */
int release_config(config_file_t *config);

#endif  // _CONFIG_FILE_H
