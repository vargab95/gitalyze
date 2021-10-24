#include "../analysis.h"
#include "../commit_list.h"
#include <m_libs/m_list.h>
#include <string.h>

int analyze(object_tree_t *tree, object_t *object)
{
    m_list_iterator_t *change_iterator = m_list_iterator_create(object->changes);
    m_com_sized_data_t *tmp, result, key;
    uint32_t number_of_changes = 0, number_of_changed_lines = 0;

    for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
         m_list_iterator_next(change_iterator))
    {
        change_t *change = tmp->data;

        number_of_changes++;
        number_of_changed_lines += change->added_lines + change->deleted_lines;
    }

    double average = 0.0;

    if (number_of_changes > 0)
    {
        average = (double)number_of_changed_lines / number_of_changes;
    }

    printf(" average=%lf lines", average);

    key.data = "average";
    key.size = strlen(key.data) + 1;
    result.size = sizeof(average);
    result.data = &average;
    m_map_store(object->analysis_results, &key, &result);

    return 0;
}
