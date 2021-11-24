#ifndef __RESULT_WRITER_H__
#define __RESULT_WRITER_H__

#include <stdio.h>

#include "object_tree/object_tree.h"
#include <m_libs/m_list.h>

typedef struct
{
    int max_depth;
    FILE *file;
    bool indent;
} write_result_configuration_t;

typedef int (*write_result_func_t)(object_tree_t *tree, const write_result_configuration_t *const configuration);
void execute_write_result(object_tree_t *tree, char *write_result_lib, write_result_configuration_t *configuration);

#endif
