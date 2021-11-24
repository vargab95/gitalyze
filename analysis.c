#include "analysis.h"
#include <dlfcn.h>

typedef struct
{
    analyze_func_t analysis_function;
    void *handle;
} analysis_function_composite_t;

void execute_analyzes(object_tree_t *tree, m_list_t *analysis_libs, analysis_configuration_t *configuration)
{
    m_com_sized_data_t *tmp;
    m_com_sized_data_t analysis_tmp;
    m_list_iterator_t *lib_name_iterator, *analysis_function_iterator;
    m_list_t *function_list = m_list_create();
    analyze_func_t analysis_function = NULL;
    object_tree_iterator_t *iterator = create_object_iterator(tree);
    object_t *object;
    analysis_function_composite_t comp_tmp, *comp_tmp_ptr;

    iterator->max_depth = configuration->max_depth;

    lib_name_iterator = m_list_iterator_create(analysis_libs);
    for (m_list_iterator_go_to_head(lib_name_iterator); (tmp = m_list_iterator_current(lib_name_iterator)) != NULL;
         m_list_iterator_next(lib_name_iterator))
    {
        char *analysis_lib = tmp->data;
        void *handle;

        if ((handle = dlopen(analysis_lib, RTLD_LAZY)) == NULL)
        {
            return;
        }

        analysis_function = dlsym(handle, "analyze");
        if (dlerror() != NULL)
        {
            printf("%s cannot be loaded\n", analysis_lib);
            dlclose(handle);
            return;
        }

        analysis_tmp.size = sizeof(comp_tmp);
        comp_tmp.analysis_function = analysis_function;
        comp_tmp.handle = handle;
        analysis_tmp.data = &comp_tmp;
        m_list_append_to_end_store(function_list, &analysis_tmp);
    }
    m_list_iterator_destroy(&lib_name_iterator);

    analysis_function_iterator = m_list_iterator_create(function_list);
    for (; (object = object_iterator_current(iterator)); object_iterator_next(iterator))
    {
        if (object->deleted)
        {
            continue;
        }

        for (m_list_iterator_go_to_head(analysis_function_iterator);
             (tmp = m_list_iterator_current(analysis_function_iterator)) != NULL;
             m_list_iterator_next(analysis_function_iterator))
        {
            comp_tmp_ptr = tmp->data;
            comp_tmp_ptr->analysis_function(tree, object);
        }
    }

    destroy_object_iterator(&iterator);

    for (m_list_iterator_go_to_head(analysis_function_iterator);
         (tmp = m_list_iterator_current(analysis_function_iterator)) != NULL;
         m_list_iterator_next(analysis_function_iterator))
    {
        comp_tmp_ptr = tmp->data;
        dlclose(comp_tmp_ptr->handle);
    }

    m_list_iterator_destroy(&analysis_function_iterator);
    m_list_destroy(&function_list);
}
