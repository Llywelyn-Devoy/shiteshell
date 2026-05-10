typedef struct AliasT {
    char *name;
    char *cmd;
    struct AliasT *prev;
    struct AliasT *next;
} Alias;

typedef struct {
    Alias *head;
    Alias *tail;
} AliasStore;

char *aliasstore_get_filepath();

Alias *alias_create(char *name, char *cmd);

void alias_free(Alias **alias_ptr);

Alias *_alias_parse_from_line(char *str);

void aliasstore_push(AliasStore *store, Alias *alias);


int aliasstore_count(AliasStore *store);

void aliasstore_rm(AliasStore *store, Alias **alias_ptr);

bool aliasstore_load(AliasStore *store);

bool aliasstore_save(AliasStore *store);

void aliasstore_free(AliasStore *store);

Alias *aliasstore_get_by_name(const AliasStore *store, const char *name);

void aliasstore_add_alias(AliasStore *store, char *name, char *cmd);

typedef enum { Failure, Success, NoResolution } AliasResolveResult;

AliasResolveResult aliasstore_resolve_alias(const AliasStore *store,
                                            char **tokens, char *out,
                                            int out_buf_size);

void aliasstore_print(const AliasStore *store);
