#include "object_tree.h"
#include <m_libs/m_mem.h>

vcs_object_t *create_object() {
  vcs_object_t *object;

  object = (vcs_object_t *)m_mem_malloc(sizeof(vcs_object_t));
  object->name = NULL;
  object->parent = NULL;
  object->children = m_map_create(10);

  return object;
}

void delete_object(vcs_object_t *object) {}

#if 0
int create_tree_object(const char *root_path, const git_tree_entry *entry,
                       void *payload) {
  vcs_object_t *root_object = (vcs_object_t *)payload;
  vcs_object_t *current_object = NULL;
  char *filename = strdup(git_tree_entry_name(entry));
  char *local_root_path = strdup(root_path);
  char *tmp = local_root_path;
  char *root_path_token_pointers[32] = {NULL};
  uint8_t root_path_token_count = 0;
  m_com_sized_data_t key, value;

  if (*tmp != '\0') {
    root_path_token_pointers[root_path_token_count] = tmp;
    root_path_token_count++;
  }

  while (*tmp != '\0') {
    if (*tmp == '/') {
      *tmp = '\0';
      if (*(tmp + 1) != '\0') {
        root_path_token_pointers[root_path_token_count] = tmp + 1;
        root_path_token_count++;
      }
    }

    tmp++;
  }

  vcs_object_t *element = root_object;
  int i = 0;
  if (root_path_token_count > 0) {
    vcs_object_t *new_element;
    m_com_sized_data_t *result;

    for (i = 0; i < root_path_token_count; i++) {
      char *path_token = root_path_token_pointers[i];

      key.size = strlen(path_token);
      key.data = strdup(path_token);

      result = m_map_get(element->children, &key);
      if (result == NULL) {
        value.size = sizeof(vcs_object_t);
        new_element = create_object();
        new_element->parent = element;
        new_element->name = key.data;

        m_map_set(element->children, &key, &value);
        element = m_map_get(element->children, &key)->data;
        printf("Creating folder %s in depth %d\n", path_token, i + 1);
      } else {
        element = result->data;
        printf("Found folder %s in depth %d\n", path_token, i + 1);
      }
    }
  }

  key.size = strlen(filename);
  key.data = strdup(filename);

  value.size = sizeof(vcs_object_t);
  value.data = create_object();

  m_map_set(element->children, &key, &value);
  printf("Creating node %s in depth %d\n", filename, i + 1);

  printf("%s -> %s%s ", git_oid_tostr_s(git_tree_entry_id(entry)), root_path,
         filename);
  for (int i = 0; i < root_path_token_count; i++) {
    printf("%s ", root_path_token_pointers[i]);
  }
  putchar('\n');
  free(filename);
  free(local_root_path);
  return 0;
}
#endif
