#ifndef _READCFG_H
#define _READCFG_H

extern const char *config_file;

struct _var {
    const char *name;
    void *var;
    int (*getvar)(struct _var *key, const char* str);
    int  initialized;
};

typedef struct _var var_t;
extern var_t vars[];

int getvarstr( var_t *key , const char *str);
int getvarbool( var_t *key , const char *str);

int read_config();

#endif /* _READCFG_H */
