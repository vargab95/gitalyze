#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <m_libs/m_map.h>

typedef struct __vcs_object_t {
  char *name;
  struct __vcs_object_t *parent;
  m_map_t *children;
} vcs_object_t;

vcs_object_t* create_object();
void delete_object(vcs_object_t *object);

#endif
