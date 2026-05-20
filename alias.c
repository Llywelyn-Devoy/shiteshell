#include "alias.h"
#include "io.h"
#include "shared.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define ALIAS_STORE_SIZE 20
#define ALIAS_STORE_FILENAME ".shiteshell_aliases"

char *aliasstore_get_filepath() {
    const char *home = (char *)getenv("HOME");

    const int buf_size = snprintf(NULL, 0, "%s/%s", home, ALIAS_STORE_FILENAME);
    char *buf = malloc((buf_size + 1) * sizeof(buf[0]));
    const int written =
        snprintf(buf, buf_size + 1, "%s/%s", home, ALIAS_STORE_FILENAME);
    assert(buf_size == written);
    return buf;
}

Alias *alias_create(char *name, char *cmd) {
    Alias *new = malloc(sizeof(Alias));
    assert(new);
    new->name = name;
    new->cmd = cmd;
    new->prev = NULL;
    new->next = NULL;
    return new;
}

void alias_free(Alias **alias_ptr) {
    shite_free((void **)&(*alias_ptr)->name);
    shite_free((void **)&(*alias_ptr)->cmd);
    shite_free((void **)alias_ptr);
}

Alias *_alias_parse_from_line(char *str) {
    // TODO: use strtok here I think...
    char *space = strchr(str, ' ');
    if (!space)
        return NULL;
    *space = '\0';

    const char *name = str;
    const char *cmd = space + 1;
    if (strlen(name) <= 0 || strlen(cmd) <= 0)
        return NULL;

    return alias_create(strdup(name), strdup(cmd));
}

void aliasstore_push(AliasStore *store, Alias *alias) {
    if (store->head == NULL) {
        assert(store->tail == NULL);
        store->head = alias;
        store->tail = alias;
    } else {
        alias->prev = store->tail;
        store->tail->next = alias;
        store->tail = alias;
    }
}

int aliasstore_count(AliasStore *store) {
    assert((store->head == NULL) == (store->tail == NULL));
    int count = 0;
    FOREACH_ALIAS(_, store) { count++; }
    return count;
}

void aliasstore_rm(AliasStore *store, Alias **alias_ptr) {
    if (0 == aliasstore_count(store))
        return;

    Alias *alias = *alias_ptr;
    if (!alias_ptr || !alias)
        return;

    const bool has_one_alias = 1 == aliasstore_count(store);
    if (alias == store->head)
        store->head = alias->next;
    if (alias == store->tail)
        store->tail = alias->prev;

    if (has_one_alias)
        assert(!store->head && !store->tail);

    if (alias->prev)
        alias->prev->next = alias->next;
    if (alias->next)
        alias->next->prev = alias->prev;

    alias_free(alias_ptr);
}

bool aliasstore_load(AliasStore *store) {
    bool success = true;
    int line_count = 0;
    char **lines = NULL;
    const char *filepath = aliasstore_get_filepath();
    DEFER(shite_free((void **)&filepath)) {
        read_lines(filepath, &lines, &line_count);
    }
    if (line_count > ALIAS_STORE_SIZE) {
        // TODO: This should probably be somewhere else
        fprintf(stderr, "ERROR: invalid history save file at %s.\n",
                ALIAS_STORE_FILENAME);
        exit(-1);
    }

    store->head = NULL;
    store->tail = NULL;
    for (int i = 0; i < line_count; i++) {
        char *line = lines[i];
        Alias *a = _alias_parse_from_line(line);
        if (!a) {
            success = false;
            break;
        }
        aliasstore_push(store, a);
    }

    for (int i = 0; i < line_count; i++) {
        shite_free((void **)&lines[i]);
    }
    shite_free((void **)&lines);

    return success;
}

bool aliasstore_save(AliasStore *store) {
    const char *filepath = aliasstore_get_filepath();
    FILE *fp = fopen(filepath, "w");
    shite_free((void **)&filepath);
    if (!fp) {
        return false;
    }

    FOREACH_ALIAS(alias, store) {
        fprintf(fp, "%s %s\n", alias->name, alias->cmd);
    }

    // TODO: error handling a bit here
    fclose(fp);
    return true;
}

void aliasstore_free(AliasStore *store) {
    Alias *alias = store->head;
    while (alias != NULL) {
        Alias *next = alias->next;
        alias_free(&alias);
        alias = next;
    }
    store->head = NULL;
    store->tail = NULL;
}

Alias *aliasstore_get_by_name(const AliasStore *store, const char *name) {
    FOREACH_ALIAS(alias, store) {
        if (STR_EQ(alias->name, name)) {
            return alias;
        }
    }
    return NULL;
}

void aliasstore_add_alias(AliasStore *store, char *name, char *cmd) {
    Alias *existing_alias = aliasstore_get_by_name(store, name);
    if (existing_alias) {
        shite_free((void **)&name);
        shite_free((void **)&existing_alias->cmd);
        existing_alias->cmd = cmd;

        printf("INFO: redefining alias \"%s\".\n", existing_alias->name);
        return;
    }

    aliasstore_push(store, alias_create(name, cmd));
}

bool mystrcat(char *dest, int dest_buf_size, int *dest_len, const char *src) {
    const int src_len = strlen(src);
    if (*dest_len + src_len + 1 > dest_buf_size)
        return false;

    strcat(dest, src);
    *dest_len += src_len;
    return true;
}

AliasResolveResult aliasstore_resolve_alias(const AliasStore *store,
                                            char **tokens, char *out_buf,
                                            int out_buf_size) {
    out_buf[0] = '\0';
    int out_len = 0;

#define resolve_concat(s)                                                      \
    do                                                                         \
        if (!mystrcat(out_buf, out_buf_size, &out_len, s))                     \
            return ARR_FAILURE;                                                \
    while (0);

    bool resolution_made = false;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (i != 0)
            resolve_concat(" ");

        char *tok = tokens[i];
        const Alias *alias = aliasstore_get_by_name(store, tok);
        if (alias) {
            resolve_concat(alias->cmd);
            resolution_made |= true;
        } else {
            resolve_concat(tok);
        }
    }

    return resolution_made ? ARR_SUCCESS : ARR_NO_CHANGE;
#undef resolve_concat
}

void aliasstore_print(const AliasStore *store) {
    FOREACH_ALIAS(alias, store) {
        printf("alias %s %s\n", alias->name, alias->cmd);
    }
}
