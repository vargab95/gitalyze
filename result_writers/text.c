#include <time.h>

#include <m_libs/m_map.h>

#include "../analysis.h"
#include "../result_writer.h"

void print_indentation(FILE *file, object_t *object)
{
    if (object->parent)
    {
        print_indentation(file, object->parent);
        fprintf(file, "  ");
    }
}

int write_result(object_tree_t *tree, const write_result_configuration_t *const configuration)
{
    object_tree_iterator_t *iterator = create_object_iterator(tree);
    m_map_iterator_t *result_iterator;
    m_com_sized_data_t *tmp;
    char from_buffer[32], to_buffer[32];

    object_t *object;

    iterator->max_depth = configuration->max_depth;

    strftime(from_buffer, sizeof(from_buffer), "%FT%TZ", gmtime(&tree->from));
    strftime(to_buffer, sizeof(to_buffer), "%FT%TZ", gmtime(&tree->to));
    printf("\nFrom %s to %s\n", from_buffer, to_buffer);

    for (; (object = object_iterator_current(iterator)); object_iterator_next(iterator))
    {
        if (object->deleted)
        {
            continue;
        }

        print_indentation(configuration->file, object);
        fprintf(configuration->file, "%s", object->name ? object->name : "root");

        result_iterator = m_map_iterator_create(object->analysis_results);
        for (; (tmp = m_map_iterator_value(result_iterator)); m_map_iterator_next(result_iterator))
        {
            analysis_result_t *analysis_result = tmp->data;

            fprintf(configuration->file, " %s=%lf %s", analysis_result->name, analysis_result->result.d,
                    analysis_result->unit);
        }
        m_map_iterator_destroy(&result_iterator);

        putchar('\n');
    }

    destroy_object_iterator(&iterator);

    return 0;
}
