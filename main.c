#include <git2.h>
#include <m_libs/m_args.h>
#include <m_libs/m_common.h>
#include <m_libs/m_map.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "analysis.h"
#include "commit_list.h"
#include "object_tree/object_tree.h"
#include "result_writer.h"

enum
{
    ARG_REPOSITORY_PATH = 1,
    ARG_REPOSITORY_URL,
    ARG_COMMIT_LIST_PATH,
    ARG_VERBOSE,
    ARG_MAX_DEPTH,
    ARG_BUILD_TREE_FROM,
    ARG_BUILD_TREE_TO,
    ARG_BUILD_TREE_STEP,
    ARG_PRINT_LIST_INFO,
    ARG_OUTPUT_FORMAT,
    ARG_OUTPUT_FILE,
    ARG_INDENT_OUTPUT
} arg_types;

m_args_t *get_args(int argc, char **argv);
int process_commit_list(commit_list_t *commit_list, m_args_t *args);
void print_commit_list_info(commit_list_t *commit_list);

int main(int argc, char **argv)
{
    commit_list_t *commit_list = commit_list_create();
    object_tree_t *object_tree;
    m_args_t *args = get_args(argc, argv);
    m_args_entry_t *arg_entry;
    analysis_configuration_t analysis_configuration;
    write_result_configuration_t write_result_configuration;
    commit_timestamp_t from, to, step, stepped_from, stepped_to;

    if (!process_commit_list(commit_list, args))
    {
        puts("Cannot continue because commit list could not be loaded");
    }

    m_list_t *analysis_libs = m_list_create();
    m_com_sized_data_t tmp;
    tmp.data = "./analyses/libgitalyze_analysis_frequency.so";
    tmp.size = strlen(tmp.data);
    m_list_append_to_end_set(analysis_libs, &tmp);
    tmp.data = "./analyses/libgitalyze_analysis_average.so";
    tmp.size = strlen(tmp.data);
    m_list_append_to_end_set(analysis_libs, &tmp);

    if ((arg_entry = m_args_get(args, ARG_PRINT_LIST_INFO)) && arg_entry->flags.present)
    {
        print_commit_list_info(commit_list);
    }
    else
    {
        if ((arg_entry = m_args_get(args, ARG_BUILD_TREE_FROM)) && arg_entry->flags.present)
        {
            from = arg_entry->value.int_val;
        }
        else
        {
            from = 0;
        }

        if ((arg_entry = m_args_get(args, ARG_BUILD_TREE_TO)) && arg_entry->flags.present)
        {
            to = arg_entry->value.int_val;
        }
        else
        {
            to = time(0);
        }

        stepped_from = from;
        if ((arg_entry = m_args_get(args, ARG_BUILD_TREE_STEP)) && arg_entry->flags.present)
        {
            step = arg_entry->value.int_val;
            stepped_to = stepped_from + step;
        }
        else
        {
            step = time(0);
            stepped_to = to;
        }

        do
        {
            object_tree = create_object_tree();

            build_object_tree(object_tree, commit_list, stepped_from, stepped_to);

            if ((arg_entry = m_args_get(args, ARG_MAX_DEPTH)) && arg_entry->flags.present)
            {
                analysis_configuration.max_depth = arg_entry->value.int_val;
                write_result_configuration.max_depth = arg_entry->value.int_val;
            }
            else
            {
                analysis_configuration.max_depth = -1;
                write_result_configuration.max_depth = -1;
            }

            execute_analyzes(object_tree, analysis_libs, &analysis_configuration);

            if ((arg_entry = m_args_get(args, ARG_OUTPUT_FILE)) && arg_entry->flags.present)
            {
                write_result_configuration.file = fopen(arg_entry->value.string_val, "w");
            }
            else
            {
                write_result_configuration.file = stdout;
            }

            if ((arg_entry = m_args_get(args, ARG_INDENT_OUTPUT)) && arg_entry->flags.present)
            {
                write_result_configuration.indent = true;
            }
            else
            {
                write_result_configuration.indent = false;
            }

            execute_write_result(object_tree, "./result_writers/libgitalyze_json_result_writer.so",
                                 &write_result_configuration);

            destroy_object_tree(&object_tree);

            stepped_from += step;
            stepped_to += step;
        } while (stepped_from < to);
    }

    m_list_destroy(&analysis_libs);
    commit_list_destroy(&commit_list);
    m_args_destroy(&args);
}

m_args_t *get_args(int argc, char **argv)
{
    m_args_t *args = m_args_create();

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_REPOSITORY_PATH,
                                            .short_switch = "-p",
                                            .long_switch = "--path",
                                            .environment_variable = "GITALYZE_REPOSITORY_PATH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_REPOSITORY_URL,
                                            .short_switch = "-u",
                                            .long_switch = "--url",
                                            .environment_variable = "GITALYZE_REPOSITORY_URL",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_COMMIT_LIST_PATH,
                                            .short_switch = "-c",
                                            .long_switch = "--commit-list-cache",
                                            .environment_variable = "GITALYZE_COMMIT_LIST_LIST_PATH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_VERBOSE,
                                            .short_switch = "-v",
                                            .long_switch = "--verbose",
                                            .environment_variable = "GITALYZE_VERBOSE",
                                            .flags = {.required = 0, .no_value = 1},
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_MAX_DEPTH,
                                            .short_switch = "-d",
                                            .long_switch = "--depth",
                                            .environment_variable = "GITALYZE_DEPTH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_INT,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_BUILD_TREE_FROM,
                                            .short_switch = "-f",
                                            .long_switch = "--from",
                                            .environment_variable = "GITALIZE_TREE_FROM",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_INT,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_BUILD_TREE_TO,
                                            .short_switch = "-t",
                                            .long_switch = "--to",
                                            .environment_variable = "GITALYZE_TREE_TO",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_INT,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_BUILD_TREE_STEP,
                                            .short_switch = "-s",
                                            .long_switch = "--step",
                                            .environment_variable = "GITALYZE_TREE_STEP",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_INT,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_PRINT_LIST_INFO,
                                            .short_switch = "-i",
                                            .long_switch = "--cache-info",
                                            .environment_variable = "GITALYZE_LIST_INFO",
                                            .flags = {.required = 0, .no_value = 1},
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_OUTPUT_FORMAT,
                                            .long_switch = "--format",
                                            .environment_variable = "GITALYZE_OUTPUT_FORMAT",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_OUTPUT_FILE,
                                            .short_switch = "-o",
                                            .long_switch = "--output",
                                            .environment_variable = "GITALYZE_OUTPUT_FILE",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_INDENT_OUTPUT,
                                            .long_switch = "--indent",
                                            .environment_variable = "GITALYZE_INDENT_OUTPUT",
                                            .flags = {.required = 0, .no_value = 1},
                                            .preference = ARG_PREFER_SHORT});

    m_args_parse(args, argc, argv);

    return args;
}

int process_commit_list(commit_list_t *commit_list, m_args_t *args)
{
    m_args_entry_t *arg_entry;
    int result_code = 0;

    if ((arg_entry = m_args_get(args, ARG_COMMIT_LIST_PATH)) && arg_entry->flags.present)
    {
        FILE *fp = fopen(arg_entry->value.string_val, "rb");
        if (fp != NULL)
        {
            commit_list_load(commit_list, fp);
            fclose(fp);
            result_code = 1;
        }
    }

    if ((arg_entry = m_args_get(args, ARG_REPOSITORY_PATH)) && arg_entry->flags.present)
    {
        commit_list_fetch_local(commit_list, arg_entry->value.string_val);
        result_code = 2;
    }

    if ((arg_entry = m_args_get(args, ARG_VERBOSE)) && arg_entry->flags.present)
    {
        commit_list_print(commit_list);
    }

    if ((arg_entry = m_args_get(args, ARG_COMMIT_LIST_PATH)) && arg_entry->flags.present)
    {
        FILE *fp = fopen(arg_entry->value.string_val, "wb");
        if (fp != NULL)
        {
            commit_list_dump(commit_list, fp);
            fclose(fp);
        }
    }

    return result_code;
}

void print_commit_list_info(commit_list_t *commit_list)
{
    char buffer[32];
    m_com_sized_data_t *tmp;
    m_list_iterator_t *commit_iterator, *change_iterator;
    commit_timestamp_t first_commit_time = time(0), last_commit_time = 0;
    uint64_t commit_count = 0, change_count = 0;
    uint64_t biggest_commit = 0;
    uint32_t number_of_changes, max_number_of_changes = 0;

    commit_iterator = m_list_iterator_create(commit_list->commit_list);
    for (m_list_iterator_go_to_head(commit_iterator); (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_next(commit_iterator))
    {
        uint64_t changed_lines = 0;
        commit_t *commit = tmp->data;

        if (first_commit_time > commit->timestamp)
        {
            first_commit_time = commit->timestamp;
        }

        if (last_commit_time < commit->timestamp)
        {
            last_commit_time = commit->timestamp;
        }

        commit_count++;

        number_of_changes = 0;
        change_iterator = m_list_iterator_create(commit->change_list);
        for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
             m_list_iterator_next(change_iterator))
        {
            change_t *change = tmp->data;
            change_count++;
            number_of_changes++;

            changed_lines += change->added_lines + change->deleted_lines;
        }

        if (biggest_commit < changed_lines)
        {
            biggest_commit = changed_lines;
        }

        if (number_of_changes > max_number_of_changes)
        {
            max_number_of_changes = number_of_changes;
        }

        m_list_iterator_destroy(&change_iterator);
    }
    m_list_iterator_destroy(&commit_iterator);

    printf("Last commit id:                        %s\n", commit_list->last_commit_id);
    strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&last_commit_time));
    printf("Last commit timestamp:                 %s (%ld)\n", buffer, last_commit_time);
    strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&first_commit_time));
    printf("First commit timestamp:                %s (%ld)\n", buffer, first_commit_time);
    printf("Number of commits:                     %ld\n", commit_count);
    printf("Number of changes:                     %ld\n", change_count);
    printf("Biggest commit:                        %ld lines\n", biggest_commit);
    printf("Maximum number of changes in a commit: %d\n", max_number_of_changes);
}
