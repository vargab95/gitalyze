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
} object_t;

typedef struct
{
    object_t *root;
} object_tree_t;

object_tree_t *create_object_tree();
void destroy_object_tree(object_tree_t **tree);
void build_object_tree(object_tree_t *tree,
                       const commit_list_t *const commit_list);

object_t *create_object();
void delete_object(object_t **object);

#endif
