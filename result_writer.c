#include "result_writer.h"
#include <dlfcn.h>

void execute_write_result(object_tree_t *tree, char *write_result_lib, write_result_configuration_t *configuration)
{
    write_result_func_t result_writer_function = NULL;

    void *handle;

    if ((handle = dlopen(write_result_lib, RTLD_LAZY)) == NULL)
    {
        printf("%s cannot be loaded\n", write_result_lib);
        return;
    }

    result_writer_function = dlsym(handle, "write_result");
    if (dlerror() != NULL)
    {
        printf("%s cannot be loaded from %s\n", "write_result", write_result_lib);
        dlclose(handle);
        return;
    }

    result_writer_function(tree, configuration);

    dlclose(handle);
}
