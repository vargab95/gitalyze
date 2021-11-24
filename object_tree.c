#include "object_tree.h"
#include <limits.h>
#include <m_libs/m_mem.h>
#include <string.h>

static void tokenize_path(char *path_ptr, char **path_token_pointers, uint8_t *path_token_count);
static object_t *get_or_create_object(object_tree_t *tree, char **path_token_pointers, uint8_t path_token_count);
static void sign_deleted_folders(object_tree_t *tree);
static void sign_deleted_if_all_children_of_folder_deleted(object_t *object);

object_tree_t *create_object_tree()
{
    object_tree_t *tree;

    tree = (object_tree_t *)m_mem_malloc(sizeof(object_tree_t));
    tree->root = create_object();
    tree->first_commit_date = LONG_MAX;
    tree->last_commit_date = 0;

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

    object = (object_t *)m_mem_calloc(1, sizeof(object_t));
    object->children = m_map_create(10);
    object->child_iterator = m_map_iterator_create(object->children);
    object->analysis_results = m_map_create(5);
    object->changes = m_list_create();

    return object;
}

void delete_object(object_t **object)
{
    m_com_sized_data_t *tmp;
    object_t *child;

    m_map_iterator_reset((*object)->child_iterator);
    for (; (tmp = m_map_iterator_value((*object)->child_iterator)); m_map_iterator_next((*object)->child_iterator))
    {
        child = tmp->data;
        delete_object(&child);
    }

    free((*object)->name);
    m_list_destroy(&(*object)->changes);
    m_map_iterator_destroy(&(*object)->child_iterator);
    m_map_destroy(&(*object)->children);
    m_map_destroy(&(*object)->analysis_results);

    if (!(*object)->parent)
    {
        free(*object);
    }

    *object = NULL;
}

void build_object_tree(object_tree_t *tree, const commit_list_t *const commit_list, time_t from, time_t to)
{
    m_com_sized_data_t *tmp, value;
    m_list_iterator_t *commit_iterator, *change_iterator;
    commit_t *commit;

    commit_iterator = m_list_iterator_create(commit_list->commit_list);

    m_list_iterator_go_to_tail(commit_iterator);
    tmp = m_list_iterator_current(commit_iterator);
    commit = tmp->data;
    tree->first_commit_date = commit->timestamp;

    m_list_iterator_go_to_head(commit_iterator);
    tmp = m_list_iterator_current(commit_iterator);
    commit = tmp->data;
    tree->last_commit_date = commit->timestamp;

    for (m_list_iterator_go_to_tail(commit_iterator); (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_previous(commit_iterator))
    {
        bool register_change = 0;
        commit = tmp->data;

        if (commit->timestamp > to)
        {
            break;
        }

        if (commit->timestamp >= from)
        {
            register_change = 1;
        }

        change_iterator = m_list_iterator_create(commit->change_list);
        for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
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
                object = get_or_create_object(tree, path_token_pointers, path_token_count);
            }

            if (object)
            {
                // TODO Test readd
                object->deleted = (change->type == OBJECT_DELETED);

                if (change->type == OBJECT_MOVED || change->type == OBJECT_COPIED)
                {
                    char *old_path_token_pointers[32] = {NULL};
                    uint8_t old_path_token_count = 0;
                    char *local_old_path = strdup(change->old_file);
                    object_t *old_object = NULL;

                    tokenize_path(local_old_path, old_path_token_pointers, &old_path_token_count);

                    if (old_path_token_count > 0)
                    {
                        old_object = get_or_create_object(tree, old_path_token_pointers, old_path_token_count);
                    }

                    if (old_object)
                    {
                        m_list_iterator_t *old_change_iterator;

                        object->added_lines = old_object->added_lines;
                        object->removed_lines = old_object->removed_lines;

                        old_change_iterator = m_list_iterator_create(commit->change_list);
                        for (m_list_iterator_go_to_head(old_change_iterator);
                             (tmp = m_list_iterator_current(old_change_iterator)) != NULL;
                             m_list_iterator_next(old_change_iterator))
                        {
                            m_list_append_to_end_set(object->changes, tmp);
                        }
                        m_list_iterator_destroy(&old_change_iterator);

                        if (change->type == OBJECT_MOVED)
                        {
                            old_object->deleted = 1;
                        }
                    }

                    free(local_old_path);
                }

                // printf("%s %p %ld %ld %ld\n", commit->commit_id, object, object->added_lines, object->removed_lines,
                //        object->added_lines - object->removed_lines);

                if (register_change)
                {
                    value.size = sizeof(change);
                    value.data = change;
                    m_list_append_to_end_set(object->changes, &value);

                    object->added_lines += change->added_lines;
                    object->removed_lines += change->deleted_lines;
                    parent = object->parent;

                    while (parent)
                    {
                        parent->added_lines += change->added_lines;
                        parent->removed_lines += change->deleted_lines;

                        // Is this the best solution?
                        m_list_append_to_end_set(parent->changes, &value);

                        parent = parent->parent;
                    }
                }
            }

            free(local_path);
        }
        m_list_iterator_destroy(&change_iterator);
    }
    m_list_iterator_destroy(&commit_iterator);

    sign_deleted_folders(tree);
}

static void tokenize_path(char *path_ptr, char **path_token_pointers, uint8_t *path_token_count)
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

static object_t *get_or_create_object(object_tree_t *tree, char **path_token_pointers, uint8_t path_token_count)
{
    object_t *element = tree->root;
    object_t *new_element;
    m_com_sized_data_t *result;
    m_com_sized_data_t key, value;
    int i;

    for (i = 0; i < path_token_count; i++)
    {
        char *path_token = path_token_pointers[i];

        key.size = strlen(path_token) + 1;
        key.data = strdup(path_token);

        result = m_map_get(element->children, &key);
        if (result == NULL)
        {
            new_element = create_object();
            new_element->parent = element;
            new_element->name = strdup(key.data);
            value.size = sizeof(object_t);
            value.data = new_element;

            m_map_store(element->children, &key, &value);
            element = m_map_get(element->children, &key)->data;
            // printf("Creating folder %s in depth %d\n", path_token, i + 1);

            free(new_element);
        }
        else
        {
            element = result->data;
            // printf("Found folder %s in depth %d\n", path_token, i + 1);
        }

        free(key.data);
    }

    return element;
}

object_tree_iterator_t *create_object_iterator(object_tree_t *tree)
{
    object_tree_iterator_t *iterator = (object_tree_iterator_t *)m_mem_malloc(sizeof(object_tree_iterator_t));

    iterator->object = tree->root;
    iterator->tree = tree;
    iterator->max_depth = -1;
    iterator->__depth = 0;
    m_map_iterator_reset(iterator->object->child_iterator);
    tree->reference_count++;

    return iterator;
}

void destroy_object_iterator(object_tree_iterator_t **iterator)
{
    (*iterator)->tree->reference_count--;
    free(*iterator);
    *iterator = NULL;
}

void object_iterator_next(object_tree_iterator_t *iterator)
{
    m_com_sized_data_t *value;

    // If there is no object, cannot be continued
    if (iterator->object == NULL)
    {
        return;
    }

    if (iterator->max_depth > 0 && iterator->__depth < iterator->max_depth)
    {
        // Check if there is a child
        value = m_map_iterator_value(iterator->object->child_iterator);
        if (value)
        {
            // Set child as current object
            iterator->object = value->data;
            iterator->__depth++;

            // Reset child's iterator
            m_map_iterator_reset(iterator->object->child_iterator);

            // Step parent's child iterator to the next child
            m_map_iterator_next(iterator->object->parent->child_iterator);

            return;
        }
    }

    // If no other child is present, go up to the parent
    iterator->object = iterator->object->parent;
    iterator->__depth--;

    // Call iterator for parent
    object_iterator_next(iterator);
}

object_t *object_iterator_current(object_tree_iterator_t *iterator)
{
    return iterator->object;
}

static void sign_deleted_folders(object_tree_t *tree)
{
    object_t *object;
    object_tree_iterator_t *iterator = create_object_iterator(tree);

    for (; (object = object_iterator_current(iterator)); object_iterator_next(iterator))
    {
        sign_deleted_if_all_children_of_folder_deleted(object);
    }

    destroy_object_iterator(&iterator);
}

static void sign_deleted_if_all_children_of_folder_deleted(object_t *object)
{
    m_com_sized_data_t *tmp;
    object_t *child;
    boolean all_children_deleted = true, has_child = false;
    m_map_iterator_t *local_child_iterator = m_map_iterator_create(object->children);

    for (; (tmp = m_map_iterator_value(local_child_iterator)); m_map_iterator_next(local_child_iterator))
    {
        child = tmp->data;

        has_child = true;

        if (!child->deleted)
        {
            all_children_deleted = false;
            break;
        }
    }

    if (has_child && all_children_deleted)
    {
        object->deleted = true;
        if (object->parent)
        {
            sign_deleted_if_all_children_of_folder_deleted(object->parent);
        }
    }

    m_map_iterator_destroy(&local_child_iterator);
}
