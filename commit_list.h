#ifndef __COMMIT_LIST_H__
#define __COMMIT_LIST_H__

#include <stdio.h>
#include <stdint.h>

#include <m_libs/m_list.h>

typedef uint32_t change_counter_t;

typedef enum {
    OBJECT_CREATED = 1,
    OBJECT_MOVED = 2,
    OBJECT_COPIED = 3,
    OBJECT_DELETED = 4,
    OBJECT_MODIFIED = 5
} change_type_t;

typedef struct {
    char *old_file;
    char *new_file;
    change_type_t type;
    change_counter_t added_lines;
    change_counter_t deleted_lines;
} change_t;

typedef time_t commit_timestamp_t;

typedef struct {
    char *commit_id;
    commit_timestamp_t timestamp;
    m_list_t *change_list;
} commit_t;

typedef struct {
    char *last_commit_id;
    m_list_t *commit_list;
} commit_list_t;

commit_list_t* commit_list_create();
void commit_list_destroy(commit_list_t **commit_list);

commit_t* commit_create();
void commit_destroy(commit_t **commit);

void commit_list_dump(const commit_list_t * const commit_list, FILE *output_file);
void commit_list_load(commit_list_t *commit_list, FILE *input_file);

void commit_list_print(const commit_list_t * const commit_list);
void commit_list_fetch_local(commit_list_t *commit_list, const char *path);
void commit_list_fetch_remote(commit_list_t *commit_list, const char *url);

#endif
