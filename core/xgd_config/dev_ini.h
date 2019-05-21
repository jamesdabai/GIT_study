#ifndef __DEV_INI_H__
#define __DEV_INI_H__


#define DEV_INI_NO_SECTION		-1
#define DEV_INI_NO_KEY			-2

#define DEV_DEF_COMMENT_NUM_LEN     65
#define DEV_DEF_PROFILE_NUM_LEN     129
#define DEV_DEF_KEY_VALUE_LEN       129
typedef struct RECORD
{
    char comments[DEV_DEF_COMMENT_NUM_LEN];
    char key[DEV_DEF_PROFILE_NUM_LEN];
    //char value[DEV_DEF_PROFILE_NUM_LEN];   
    char value[DEV_DEF_KEY_VALUE_LEN];    //key_value的值由之前的最大64byte，修改为最大128byte
    struct RECORD* next;
}T_RECORD;

typedef struct SECTION
{
    T_RECORD* first;
    T_RECORD* last;
    int sizeRecord;
    char comments[DEV_DEF_COMMENT_NUM_LEN];
    char name[DEV_DEF_PROFILE_NUM_LEN];
    struct SECTION* next;
}T_SECTION;

struct CONTENT
{
    int sizeSection;
    T_SECTION* first;
    T_SECTION* last;
    int ischg;
};

typedef void ini_ctrl_t;



ini_ctrl_t* dev_ini_load(const char* file);
int dev_ini_get_key(ini_ctrl_t* handler, const char* section, const char* key, char* buffer);
int dev_ini_set_key(ini_ctrl_t* handler, const char* section, const char* key, const char* value, const char* comment);
int dev_ini_del_key(ini_ctrl_t *handler, const char* section, const char* key);
int dev_ini_del_section(ini_ctrl_t * handler, const char* section);
int dev_ini_save(ini_ctrl_t* handler);
int dev_ini_free(ini_ctrl_t* handler);


#endif /* __cplusplus */


