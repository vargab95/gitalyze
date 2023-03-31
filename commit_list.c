#include <string.h>
#include <time.h>

#include <git2.h>
#include <m_libs/m_mem.h>

#include "commit_list.h"

commit_list_t *commit_list_create()
{
    commit_list_t *commit_list;

    commit_list = (commit_list_t *)m_mem_calloc(1, sizeof(commit_list_t));
    commit_list->commit_list = m_list_create();

    return commit_list;
}

void commit_list_destroy(commit_list_t **commit_list)
{
    m_com_sized_data_t *tmp;
    m_list_iterator_t *commit_iterator;

    commit_iterator = m_list_iterator_create((*commit_list)->commit_list);
    for (m_list_iterator_go_to_head(commit_iterator); (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_next(commit_iterator))
    {
        commit_destroy((commit_t **)&tmp->data);
    }

    m_list_iterator_destroy(&commit_iterator);
    m_list_destroy(&(*commit_list)->commit_list);
    free((*commit_list)->last_commit_id);
    free(*commit_list);
    *commit_list = NULL;
}

commit_t *commit_create()
{
    commit_t *commit;

    commit = (commit_t *)m_mem_calloc(1, sizeof(commit_t));
    commit->change_list = m_list_create();

    return commit;
}

void commit_destroy(commit_t **commit)
{
    m_com_sized_data_t *tmp;
    m_list_iterator_t *change_iterator;

    change_iterator = m_list_iterator_create((*commit)->change_list);
    for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
         m_list_iterator_next(change_iterator))
    {
        change_t *change = tmp->data;

        free(change->old_file);
        free(change->new_file);
        free(change);
    }

    m_list_iterator_destroy(&change_iterator);
    m_list_destroy(&(*commit)->change_list);

    free((*commit)->commit_id);
    free(*commit);
    *commit = NULL;
}

void commit_list_dump(const commit_list_t *const commit_list, FILE *output_file)
{
    m_com_sized_data_t *tmp;
    m_list_iterator_t *commit_iterator, *change_iterator;
    uint32_t tmp_str_len;
    size_t commit_list_size;

    commit_list_size = m_list_get_size(commit_list->commit_list);

    tmp_str_len = strlen(commit_list->last_commit_id);
    fwrite(&tmp_str_len, sizeof(tmp_str_len), 1, output_file);
    fwrite(commit_list->last_commit_id, sizeof(char), tmp_str_len, output_file);
    fwrite(&commit_list_size, sizeof(commit_list_size), 1, output_file);
    commit_iterator = m_list_iterator_create(commit_list->commit_list);
    for (m_list_iterator_go_to_head(commit_iterator); (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_next(commit_iterator))
    {
        commit_t *commit = tmp->data;
        size_t change_list_size = m_list_get_size(commit->change_list);

        tmp_str_len = strlen(commit_list->last_commit_id);
        fwrite(&tmp_str_len, sizeof(tmp_str_len), 1, output_file);
        fwrite(commit->commit_id, sizeof(char), tmp_str_len, output_file);
        fwrite(&commit->timestamp, sizeof(commit_timestamp_t), 1, output_file);

        fwrite(&change_list_size, sizeof(change_list_size), 1, output_file);
        change_iterator = m_list_iterator_create(commit->change_list);
        for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
             m_list_iterator_next(change_iterator))
        {
            change_t *change = tmp->data;

            tmp_str_len = strlen(change->old_file);
            fwrite(&tmp_str_len, sizeof(tmp_str_len), 1, output_file);
            fwrite(change->old_file, sizeof(char), tmp_str_len, output_file);

            tmp_str_len = strlen(change->new_file);
            fwrite(&tmp_str_len, sizeof(tmp_str_len), 1, output_file);
            fwrite(change->new_file, sizeof(char), tmp_str_len, output_file);

            fwrite(&change->type, sizeof(change->type), 1, output_file);
            fwrite(&change->added_lines, sizeof(change->added_lines), 1, output_file);
            fwrite(&change->deleted_lines, sizeof(change->deleted_lines), 1, output_file);
        }
        m_list_iterator_destroy(&change_iterator);
    }
    m_list_iterator_destroy(&commit_iterator);
}

void commit_list_load(commit_list_t *commit_list, FILE *input_file)
{
    m_com_sized_data_t tmp;
    uint32_t tmp_str_len, commit_list_size, change_list_size;

    fread(&tmp_str_len, sizeof(tmp_str_len), 1, input_file);
    commit_list->last_commit_id = (char *)m_mem_calloc(tmp_str_len + 1, sizeof(char));
    fread(commit_list->last_commit_id, sizeof(char), tmp_str_len, input_file);
    fread(&commit_list_size, sizeof(commit_list_size), 1, input_file);
    for (uint32_t i = 0; i < commit_list_size; i++)
    {
        commit_t *commit = commit_create();

        fread(&tmp_str_len, sizeof(tmp_str_len), 1, input_file);
        commit->commit_id = (char *)m_mem_calloc(tmp_str_len + 1, sizeof(char));
        fread(commit->commit_id, sizeof(char), tmp_str_len, input_file);
        fread(&commit->timestamp, sizeof(commit_timestamp_t), 1, input_file);
        fread(&change_list_size, sizeof(change_list_size), 1, input_file);
        for (uint32_t j = 0; j < change_list_size; j++)
        {
            change_t *change = (change_t *)m_mem_calloc(1, sizeof(change_t));

            change->commit = commit;

            fread(&tmp_str_len, sizeof(tmp_str_len), 1, input_file);
            change->old_file = (char *)m_mem_calloc(tmp_str_len + 1, sizeof(char));
            fread(change->old_file, sizeof(char), tmp_str_len, input_file);

            fread(&tmp_str_len, sizeof(tmp_str_len), 1, input_file);
            change->new_file = (char *)m_mem_calloc(tmp_str_len + 1, sizeof(char));
            fread(change->new_file, sizeof(char), tmp_str_len, input_file);

            fread(&change->type, sizeof(change->type), 1, input_file);
            fread(&change->added_lines, sizeof(change->added_lines), 1, input_file);
            fread(&change->deleted_lines, sizeof(change->deleted_lines), 1, input_file);

            tmp.size = sizeof(change);
            tmp.data = change;
            m_list_append_to_end_set(commit->change_list, &tmp);
        }

        tmp.size = sizeof(commit);
        tmp.data = commit;
        m_list_append_to_end_set(commit_list->commit_list, &tmp);
    }
}

void commit_list_print(const commit_list_t *const commit_list)
{
    m_com_sized_data_t *tmp;
    m_list_iterator_t *commit_iterator, *change_iterator;

    printf("Last commit id: %s\n", commit_list->last_commit_id);
    puts("Commit list");

    commit_iterator = m_list_iterator_create(commit_list->commit_list);
    for (m_list_iterator_go_to_head(commit_iterator); (tmp = m_list_iterator_current(commit_iterator)) != NULL;
         m_list_iterator_next(commit_iterator))
    {
        char buffer[32];
        commit_t *commit = tmp->data;
        strftime(buffer, sizeof(buffer), "%FT%TZ", gmtime(&commit->timestamp));

        printf("  %s %s\n", buffer, commit->commit_id);

        change_iterator = m_list_iterator_create(commit->change_list);
        for (m_list_iterator_go_to_head(change_iterator); (tmp = m_list_iterator_current(change_iterator)) != NULL;
             m_list_iterator_next(change_iterator))
        {
            change_t *change = tmp->data;
            char *type;

            switch (change->type)
            {
            case OBJECT_COPIED:
                type = "copied";
                break;
            case OBJECT_CREATED:
                type = "created";
                break;
            case OBJECT_MOVED:
                type = "moved";
                break;
            case OBJECT_DELETED:
                type = "deleted";
                break;
            case OBJECT_MODIFIED:
                type = "modified";
                break;
            }

            printf("    New file: %s, old file: %s, type: %s, added: %d, "
                   "deleted: %d\n",
                   change->new_file, change->old_file, type, change->added_lines, change->deleted_lines);
        }
        m_list_iterator_destroy(&change_iterator);
    }
    m_list_iterator_destroy(&commit_iterator);
}

int each_file_cb(const git_diff_delta *delta, float progress, void *payload)
{
    commit_t *commit = (commit_t *)payload;
    change_t *change = (change_t *)m_mem_calloc(1, sizeof(change_t));
    m_com_sized_data_t tmp;

    change->commit = commit;

    tmp.size = sizeof(change_t *);
    tmp.data = change;

    m_list_append_to_end_set(commit->change_list, &tmp);
    // printf("%s %f\n", commit->commit_id, progress);

    // puts("EACH FILE CB");
    switch (delta->status)
    {
    case GIT_DELTA_ADDED:
        change->type = OBJECT_CREATED;
        break;
    case GIT_DELTA_COPIED:
        change->type = OBJECT_COPIED;
        break;
    case GIT_DELTA_DELETED:
        change->type = OBJECT_DELETED;
        break;
    case GIT_DELTA_RENAMED:
        change->type = OBJECT_MOVED;
        break;
    case GIT_DELTA_MODIFIED:
    case GIT_DELTA_TYPECHANGE:
        change->type = OBJECT_MODIFIED;
        break;
    case GIT_DELTA_IGNORED:
    case GIT_DELTA_UNTRACKED:
    case GIT_DELTA_CONFLICTED:
    case GIT_DELTA_UNMODIFIED:
    case GIT_DELTA_UNREADABLE:
        puts("ERROR: Non handled file status");
        break;
    }

    change->old_file = strdup(delta->old_file.path);
    change->new_file = strdup(delta->new_file.path);

    // printf("%s %d %d\n", commit->commit_id, delta->status, delta->similarity);
    // printf("CID: %s OF: %s NF: %s T: %d\n", commit->commit_id, change->new_file, change->old_file, change->type);
    return 0;
}

int each_binary_cb(const git_diff_delta *delta, const git_diff_binary *binary, void *payload)
{
    // puts("EACH BINARY CB");
    return 0;
}

int each_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload)
{
    commit_t *commit = (commit_t *)payload;
    m_com_sized_data_t *last = m_list_get_last(commit->change_list);
    change_t *change = last->data;
    // // puts("EACH HUNK CB");
    change->added_lines += hunk->new_lines;
    change->deleted_lines += hunk->old_lines;

    // printf("HUNK %s %s %d %d %d %d\n", change->new_file, change->old_file, hunk->new_lines, hunk->old_lines,
    //       change->added_lines, change->deleted_lines);

    // printf("%s || - %d  + %d\n", commit->commit_id, hunk->old_lines,
    //        hunk->new_lines);
    return 0;
}

int each_line_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload)
{
    // puts("EACH LINE CB");
    return 0;
}

void commit_list_fetch_local(commit_list_t *commit_list, const char *path)
{
    git_repository *repository;
    git_object *obj = NULL;
    git_commit *git_commit = NULL, *parent = NULL;
    git_tree *commit_tree = NULL, *parent_tree = NULL;
    git_diff *diff = NULL;
    commit_t *commit;
    m_com_sized_data_t tmp;
    char *old_last_commit_id;
    int error;

    git_libgit2_init();

    error = git_repository_open(&repository, path);
    if (error < 0)
    {
        const git_error *e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        return;
    }

    error = git_revparse_single(&obj, repository, "HEAD");
    if (error < 0)
    {
        printf("1 %d\n", error);
    }

    error = git_commit_lookup(&git_commit, repository, git_object_id(obj));
    if (error < 0)
    {
        printf("2 %d\n", error);
    }

    old_last_commit_id = commit_list->last_commit_id;
    commit_list->last_commit_id = strdup(git_oid_tostr_s(git_commit_id(git_commit)));
    if (old_last_commit_id && strcmp(old_last_commit_id, commit_list->last_commit_id) == 0)
    {
        puts("Repo is up to date");
        git_commit_free(git_commit);
        git_object_free(obj);
        git_repository_free(repository);
        git_libgit2_shutdown();
        free(old_last_commit_id);
        return;
    }

    do
    {
        error = git_commit_parent(&parent, git_commit, 0);
        if (error == -3)
        {
            parent = NULL;
        }
        else if (error < 0)
        {
            printf("3 %d\n", error);
            break;
        }

        if (old_last_commit_id && strcmp(old_last_commit_id, git_oid_tostr_s(git_commit_id(parent))) == 0)
        {
            puts("Last commit found");
            break;
        }

        error = git_commit_tree(&commit_tree, git_commit);
        if (error < 0)
        {
            printf("4 %d\n", error);
            break;
        }

        if (parent != NULL)
        {
            error = git_commit_tree(&parent_tree, parent);
            if (error < 0)
            {
                printf("5 %d\n", error);
                break;
            }
        }
        else
        {
            parent_tree = NULL;
        }

        error = git_diff_tree_to_tree(&diff, repository, parent_tree, commit_tree, NULL);
        if (error < 0)
        {
            printf("6 %d\n", error);
            break;
        }

        git_diff_find_similar(diff, NULL);

        commit = commit_create();
        commit->commit_id = strdup(git_oid_tostr_s(git_commit_id(git_commit)));
        commit->timestamp = git_commit_time(git_commit);
        error = git_diff_foreach(diff, each_file_cb, each_binary_cb, each_hunk_cb, each_line_cb, commit);
        if (error < 0)
        {
            printf("7 %d\n", error);
            break;
        }

        tmp.size = sizeof(commit);
        tmp.data = commit;
        m_list_append_to_end_set(commit_list->commit_list, &tmp);

        git_tree_free(commit_tree);
        git_tree_free(parent_tree);
        git_diff_free(diff);
        git_commit_free(git_commit);
        git_commit = parent;
    } while (parent);

    free(old_last_commit_id);

    git_commit_free(parent);
    git_object_free(obj);
    git_repository_free(repository);
    git_libgit2_shutdown();
}

void commit_list_fetch_remote(commit_list_t *commit_list, const char *url) {}
