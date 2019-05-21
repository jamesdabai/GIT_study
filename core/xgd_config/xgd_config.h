#ifndef __XGD_CONFIG_H__
#define __XGD_CONFIG_H__
#include <dfs_posix.h>

#define SECTION_NAEM "XGD_SYS_SET"
#define XGD_CFG_FILE  "xgd_config.ini"
#define NULL  0

typedef struct 
{
    char* key;
    char* value;
}DEFAULT_CFG_XGD;

int xgd_config_init(void);
int xgd_config_get(const char* section, const char* key, char* value,int len);
int xgd_config_set(const char* section, const char* key, char* value);
int xgd_config_del_section(const char* section);
int xgd_config_del_key(const char* section, const char* key);
int xgd_config_save(void);
int xgd_config_free(void);

#endif



