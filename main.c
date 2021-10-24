#include <git2.h>
#include <m_libs/m_args.h>
#include <m_libs/m_common.h>
#include <m_libs/m_map.h>
#include <stdio.h>
#include <string.h>

#include "analysis.h"
#include "commit_list.h"
#include "object_tree.h"

enum
{
    ARG_REPOSITORY_PATH = 1,
    ARG_REPOSITORY_URL = 2,
    ARG_COMMIT_LIST_PATH = 3,
    ARG_OBJECT_TREE_PATH = 4,
    ARG_VERBOSE = 5,
    ARG_MAX_DEPTH = 6
} arg_types;

m_args_t *get_args(int argc, char **argv);
int process_commit_list(commit_list_t *commit_list, m_args_t *args);

int main(int argc, char **argv)
{
    commit_list_t *commit_list = commit_list_create();
    object_tree_t *object_tree = create_object_tree();
    m_args_t *args = get_args(argc, argv);
    m_args_entry_t *arg_entry;
    analysis_configuration_t analysis_configuration;

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

    build_object_tree(object_tree, commit_list);

    if ((arg_entry = m_args_get(args, ARG_MAX_DEPTH)) && arg_entry->flags.present)
    {
        analysis_configuration.max_depth = arg_entry->value.int_val;
    }
    else
    {
        analysis_configuration.max_depth = -1;
    }
    execute_analyzes(object_tree, analysis_libs, &analysis_configuration);

    commit_list_destroy(&commit_list);
    destroy_object_tree(&object_tree);
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
                                            .environment_variable = "GITALYZE_COMMIT_LIST_CACHE_PATH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_OBJECT_TREE_PATH,
                                            .short_switch = "-o",
                                            .long_switch = "--object-tree-cache",
                                            .environment_variable = "GITALYZE_OBJECT_TREE_CACHE_PATH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_VERBOSE,
                                            .short_switch = "-v",
                                            .long_switch = "--verbose",
                                            .environment_variable = "GITALYZE_VERBOSE",
                                            .flags = {.required = 0, .no_value = 1},
                                            .expected_type = ARG_TYPE_STRING,
                                            .preference = ARG_PREFER_SHORT});

    m_args_add_entry(args, (m_args_entry_t){.id = ARG_MAX_DEPTH,
                                            .short_switch = "-d",
                                            .long_switch = "--depth",
                                            .environment_variable = "GITALYZE_DEPTH",
                                            .flags = {.required = 0},
                                            .expected_type = ARG_TYPE_INT,
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
