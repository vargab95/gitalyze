#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "commit_list.h"
#include <m_libs/m_list.h>
#include <m_libs/m_map.h>

typedef struct __vcs_object_t
{
    char *name;
    uint64_t added_lines;
    uint64_t removed_lines;

    uint8_t deleted : 1;
    uint8_t __reserved : 7;

    m_list_t *changes;

    struct __vcs_object_t *parent;
    m_map_t *children;
    m_map_iterator_t *child_iterator;
    m_map_t *analysis_results;
} object_t;

typedef struct
{
    object_t *root;
    uint32_t reference_count;

    time_t first_commit_date;
    time_t last_commit_date;
} object_tree_t;

typedef struct
{
    object_tree_t *tree;
    object_t *object;
    uint8_t max_depth;
    uint8_t __depth;
} object_tree_iterator_t;

object_tree_t *create_object_tree();
void destroy_object_tree(object_tree_t **tree);
void build_object_tree(object_tree_t *tree, const commit_list_t *const commit_list);

object_t *create_object();
void delete_object(object_t **object);

object_tree_iterator_t *create_object_iterator(object_tree_t *tree);
void destroy_object_iterator(object_tree_iterator_t **iterator);
void object_iterator_next(object_tree_iterator_t *iterator);
object_t *object_iterator_current(object_tree_iterator_t *iterator);

#endif
