#include <time.h>

#include <m_libs/m_map.h>

#include "../analysis.h"
#include "../result_writer.h"

void print_indentation(const write_result_configuration_t *const configuration, int count)
{
    if (!configuration->indent)
    {
        return;
    }

    for (int i = 0; i < count; i++)
    {
        fprintf(configuration->file, "  ");
    }
}

void print_line_ending(const write_result_configuration_t *const configuration)
{
    if (!configuration->indent)
    {
        return;
    }

    fprintf(configuration->file, "\n");
}

void write_file_tree(const write_result_configuration_t *const configuration, object_t *object, int depth)
{
    m_map_iterator_t *result_iterator;
    m_map_iterator_t *child_iterator;
    m_com_sized_data_t *tmp;
    object_t *child;
    bool should_write_children = false, should_write_results;

    print_indentation(configuration, depth);
    result_iterator = m_map_iterator_create(object->analysis_results);

    fprintf(configuration->file, "\"%s\": {", object->name ? object->name : "root");
    print_line_ending(configuration);

    tmp = m_map_iterator_value(result_iterator);
    should_write_results = (tmp != NULL);

    if (should_write_results)
    {
        print_indentation(configuration, depth + 1);
        fprintf(configuration->file, "\"results\": {");
        print_line_ending(configuration);

        while (tmp)
        {
            analysis_result_t *analysis_result = tmp->data;

            print_indentation(configuration, depth + 2);
            fprintf(configuration->file, "\"%s\": %lf", analysis_result->name, analysis_result->result.d);

            m_map_iterator_next(result_iterator);
            tmp = m_map_iterator_value(result_iterator);

            if (tmp)
            {
                fprintf(configuration->file, ",");
                print_line_ending(configuration);
            }
            else
            {
                print_line_ending(configuration);
                print_indentation(configuration, depth + 1);
                fprintf(configuration->file, "}");
            }
        }
    }

    m_map_iterator_destroy(&result_iterator);

    child_iterator = m_map_iterator_create(object->children);
    tmp = m_map_iterator_value(result_iterator);
    should_write_children = (tmp != NULL) && ((configuration->max_depth < 0) || (depth < configuration->max_depth));

    if (should_write_children)
    {
        if (should_write_results)
        {
            fprintf(configuration->file, ",");
            print_line_ending(configuration);
        }

        print_indentation(configuration, depth + 1);
        fprintf(configuration->file, "\"children\": {");
        print_line_ending(configuration);

        while (tmp)
        {
            child = tmp->data;

            if (!child->deleted)
            {
                write_file_tree(configuration, child, depth + 2);
            }

            do
            {
                m_map_iterator_next(result_iterator);
                tmp = m_map_iterator_value(result_iterator);
            } while (tmp && ((object_t *)tmp->data)->deleted);

            if (!child->deleted)
            {
                if (tmp)
                {
                    fprintf(configuration->file, ",");
                }
                print_line_ending(configuration);
            }
        }

        print_indentation(configuration, depth + 1);
        fprintf(configuration->file, "}");
    }

    m_map_iterator_destroy(&child_iterator);

    print_line_ending(configuration);
    print_indentation(configuration, depth);
    fprintf(configuration->file, "}");
}

int write_result(object_tree_t *tree, const write_result_configuration_t *const configuration)
{
    m_map_iterator_t *result_iterator;
    m_com_sized_data_t *tmp;
    char from_buffer[32], to_buffer[32];

    object_t *object;

    strftime(from_buffer, sizeof(from_buffer), "%FT%TZ", gmtime(&tree->from));
    strftime(to_buffer, sizeof(to_buffer), "%FT%TZ", gmtime(&tree->to));
    fprintf(configuration->file, "{");

    print_line_ending(configuration);
    print_indentation(configuration, 1);
    fprintf(configuration->file, "\"from\": \"%s\",", from_buffer);

    print_line_ending(configuration);
    print_indentation(configuration, 1);
    fprintf(configuration->file, "\"to\": \"%s\",", to_buffer);
    print_line_ending(configuration);

    write_file_tree(configuration, tree->root, 1);

    print_line_ending(configuration);
    fprintf(configuration->file, "}");
    print_line_ending(configuration);

    return 0;
}
