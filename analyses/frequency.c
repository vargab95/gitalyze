#include "../analysis.h"
#include "../commit_list.h"
#include <m_libs/m_list.h>
#include <string.h>

int analyze(object_tree_t *tree, object_t *object)
{
    m_list_iterator_t *change_iterator = m_list_iterator_create(object->changes);
    m_com_sized_data_t *tmp, result, key;
    uint64_t number_of_changes = 0;
    commit_timestamp_t oldest_timestamp = 200000000000000, newest_timestamp = 0;

    for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
         m_list_iterator_next(change_iterator))
    {
        change_t *change = tmp->data;

        number_of_changes++;

        if (change->commit->timestamp > newest_timestamp)
        {
            newest_timestamp = change->commit->timestamp;
        }

        if (change->commit->timestamp < oldest_timestamp)
        {
            oldest_timestamp = change->commit->timestamp;
        }
    }

    m_list_iterator_destroy(&change_iterator);

    double frequency = 0.0;

    if (newest_timestamp - oldest_timestamp > 0)
    {
        // frequency = number_of_changes / ((double)(newest_timestamp - oldest_timestamp) / (3600 * 24));
        frequency = number_of_changes / ((double)(tree->last_commit_date - tree->first_commit_date) / (3600 * 24));
    }

    printf(" frequency=%lf 1/day", frequency);

    key.data = "frequency";
    key.size = strlen(key.data) + 1;
    result.size = sizeof(frequency);
    result.data = &frequency;
    m_map_store(object->analysis_results, &key, &result);

    return 0;
}
