#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__

#include "object_tree.h"
#include <m_libs/m_list.h>

typedef struct
{
    int max_depth;
} analysis_configuration_t;

int analyze(object_tree_t *tree, object_t *object);
typedef int (*analyze_func_t)(object_tree_t *tree, object_t *);
void execute_analyzes(object_tree_t *tree, m_list_t *analysis_libs, analysis_configuration_t *configuration);

#endif
