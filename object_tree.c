#include "object_tree.h"
#include <m_libs/m_mem.h>
#include <string.h>

static void tokenize_path(char *path_ptr, char **path_token_pointers,
                          uint8_t *path_token_count);
static object_t *get_or_create_object(object_tree_t *tree,
                                      char **path_token_pointers,
                                      uint8_t path_token_count);

object_tree_t *create_object_tree()
{
    object_tree_t *tree;

    tree = (object_tree_t *)m_mem_malloc(sizeof(object_tree_t));
    tree->root = create_object();

    return tree;
}

void destroy_object_tree(object_tree_t **tree)
{
    delete_object(&(*tree)->root);
    free(*tree);
    *tree = NULL;
}

object_t *create_object()
{
    object_t *object;

    object = (object_t *)m_mem_malloc(sizeof(object_t));
    object->name = NULL;
    object->parent = NULL;
    object->added_lines = 0;
    object->removed_lines = 0;
    object->children = m_map_create(10);
    object->changes = m_list_create();

    return object;
}

void delete_object(object_t **object) {}

void build_object_tree(object_tree_t *tree,
                       const commit_list_t *const commit_list)
{
    m_com_sized_data_t *tmp, value;
    m_list_iterator_t *commit_iterator, *change_iterator;

    commit_iterator = m_list_iterator_create(commit_list->commit_list);
    for (m_list_iterator_go_to_tail(commit_iterator);
         (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_previous(commit_iterator))
    {
        commit_t *commit = tmp->data;

        change_iterator = m_list_iterator_create(commit->change_list);
        for (m_list_iterator_go_to_head(change_iterator);
             (tmp = m_list_iterator_current(change_iterator)) != NULL;
             m_list_iterator_next(change_iterator))
        {
            change_t *change = tmp->data;
            char *local_path = strdup(change->new_file);
            char *path_token_pointers[32] = {NULL};
            uint8_t path_token_count = 0;
            object_t *object = NULL;
            object_t *parent = NULL;

            tokenize_path(local_path, path_token_pointers, &path_token_count);

            if (path_token_count > 0)
            {
                object = get_or_create_object(tree, path_token_pointers,
                                              path_token_count);
            }

            if (object)
            {
                value.size = sizeof(change);
                value.data = change;
                m_list_append_to_end_set(object->changes, &value);

                object->added_lines += change->added_lines;
                object->removed_lines += change->deleted_lines;

                printf("%s %p %ld %ld %ld\n", commit->commit_id, object,
                       object->added_lines, object->removed_lines,
                       object->added_lines - object->removed_lines);

                parent = object->parent;
                while (parent)
                {
                    parent->added_lines += change->added_lines;
                    parent->removed_lines += change->deleted_lines;
                    parent = parent->parent;
                }
            }

            free(local_path);
        }
        m_list_iterator_destroy(&change_iterator);
    }
    m_list_iterator_destroy(&commit_iterator);
}

static void tokenize_path(char *path_ptr, char **path_token_pointers,
                          uint8_t *path_token_count)
{
    if (*path_ptr != '\0')
    {
        path_token_pointers[*path_token_count] = path_ptr;
        (*path_token_count)++;
    }

    while (*path_ptr != '\0')
    {
        if (*path_ptr == '/')
        {
            *path_ptr = '\0';
            if (*(path_ptr + 1) != '\0')
            {
                path_token_pointers[*path_token_count] = path_ptr + 1;
                (*path_token_count)++;
            }
        }

        path_ptr++;
    }
}

static object_t *get_or_create_object(object_tree_t *tree,
                                      char **path_token_pointers,
                                      uint8_t path_token_count)
{
    object_t *element = tree->root;
    object_t *new_element;
    m_com_sized_data_t *result;
    m_com_sized_data_t key, value;
    int i;

    for (i = 0; i < path_token_count; i++)
    {
        char *path_token = path_token_pointers[i];

        key.size = strlen(path_token);
        key.data = strdup(path_token);

        result = m_map_get(element->children, &key);
        if (result == NULL)
        {
            new_element = create_object();
            new_element->parent = element;
            new_element->name = key.data;
            value.size = sizeof(object_t);
            value.data = new_element;

            m_map_set(element->children, &key, &value);
            element = m_map_get(element->children, &key)->data;
            printf("Creating folder %s in depth %d\n", path_token, i + 1);
        }
        else
        {
            element = result->data;
            printf("Found folder %s in depth %d\n", path_token, i + 1);
        }
    }

    return element;
}
