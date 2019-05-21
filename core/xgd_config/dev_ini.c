#include <stdio.h>
#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "dev_ini.h"

#if 0
#define TRACE  printf
#else
#define TRACE(arg...)  
#endif



static struct RECORD* ini_get_record(struct SECTION* psection, const char* lpKey);
static int ini_add_section(ini_ctrl_t* handler, const char* section, const char* comment);
static struct RECORD* ini_get_record(struct SECTION* psection, const char* lpKey);
static int ini_add_key(ini_ctrl_t* handler, const char* section, const char* key, const char* comment);
static struct SECTION* ini_get_section(ini_ctrl_t* handler, const char* lpSection);

static char ini_filename[32] = {0};
static char* ini_fgets(char *s, int size, int fd)
{
    int len = 0;
    char buff = 0;
    char *pstr = NULL;

    if(NULL == s)
    {
        return pstr;
    }

    while(len < size)
    {
        if(read(fd,&buff, 1)>0)
        {
            if(buff == 0xa)
                break;
            else if(buff == 0xd)
                continue;
            else
            {
                *(s+len) = buff;
                len++;
            }
        }
        else
            break;
    }

    if(len > 0)
        pstr = s;
        
    return pstr;
}

static int ini_load(ini_ctrl_t* handler, const char* lpFile)
{
    int ini_fd;
    char buffer[256] = {0};
    char comments[512] = {0};
    char NowSection[128] = {0};
    char key[128] = {0};
    char value[128] = {0};
    char* pdest;
    int index, ret = 0;
    int len = 0;

    strcpy(comments, "");
    strcpy(NowSection, "");

    if ((ini_fd = open(lpFile, O_RDONLY, 0)) != -1)
    {
        while (ini_fgets(buffer, sizeof(buffer), ini_fd) != NULL)
        {
            /*=======BEGIN: fusuipu 2013.05.15  10:20 modify在linux里边，回车结束符为0x0d 0x0a===========*/
            len = strlen(buffer);

            if(buffer[len - 2] == '\r' && buffer[len - 1] == '\n')
            {
                buffer[len - 2] = 0X00;
                buffer[len - 1] = 0X00;
            }

            /*====================== END======================== */
            if (buffer[len - 1] == '\n')
            {
                buffer[len - 1] = '\0';
            }

            if (buffer[len - 1] == '\r')
            {
                buffer[len - 1] = '\0';
            }

            switch (buffer[0])
            {
                 case '[':
                    {
                        pdest = strrchr(buffer, ']');

                        if (pdest == NULL)
                        {
                            close(ini_fd);
                            TRACE("parse ini error!\n");
                            return -1;
                        }
                        index = pdest - buffer;
                        memcpy(NowSection, buffer + 1, index - 1);
                        NowSection[index - 1] = '\0';
                        ret = ini_add_section((struct CONTENT *)handler, NowSection, comments);
                        strcpy(comments, "");
                    }
                    break;

                 case '#':
                 case ';':
                    {
                        if (strlen(comments) > 0)
                        {
                            strcat(comments, "\n");
                        }
                        strcat(comments, buffer);
                    }
                    break;

                 default:
                    {
                        pdest = strrchr(buffer, '=');

                        if (pdest == NULL)
                        {
                            continue;
//                            close(ini_fd);
//                            printf("parse ini error\n");
//                            return false;
                        }
                        index = pdest - buffer;
                        memcpy(key, buffer, index);
                        key[index] = '\0';
                        memset(value, 0, sizeof(value)); //fusuipu modefied at 2013-5-15
                        memcpy(value, buffer + index + 1, strlen(buffer) - index - 1); //fusuipu modefied at 2013-5-15

                        if (strcmp(NowSection, "") == 0)
                        {
                            close(ini_fd);
                            TRACE("parse ini error\n");
                            return -1;
                        }
                        else
                        {
                            /*=======BEGIN: fusuipu 2013.05.14  15:58 modify===========*/
                            ret = ini_add_key(handler, NowSection, key, comments);  //先要添加key选项

                            if(ret < 0)
                            {
                                close(ini_fd);
                                return ret;     //key添加错误，直接返回
                            }
                            /*====================== END======================== */
                            ret = dev_ini_set_key((struct CONTENT *)handler, NowSection, key, value, comments);
                            strcpy(comments, "");
                        }
                    }
                    break;
            }

            memset(buffer, 0, sizeof(buffer)); //fusuipu modefied at 2013-5-15
        }

        close(ini_fd);
    }
    else
    {
//        TRACE("open file error !\n");
        return -1;   //fusuipu 2013.03.20 8:39
    }
    return ret;
}

/*****************************************************************************
** Descriptions:	增加ini的一个小节
** Parameters:          const char* section:小节名称
                               const char* comment:注释
                               SDK_INI_HAND *handler:SDK_INI_CONTENT指针
** Returned value:	内存错误返回:-1
                                传入参数为NULL则返回SDK_PARA_ERR
                                传入小节名称或者注释长度超限则返回SDK_PARA_ERR
                                正确返回:小节名长度
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
static int ini_add_section(ini_ctrl_t* handler, const char* section, const char* comment)
{
    struct SECTION* psection;

    if(NULL == section || NULL == handler) //|| NULL == comment 可以为空//shiweisong 2013.09.29 15:36
    {
        return -1;
    }

    if((sizeof(((struct SECTION*) 0)->name) <= strlen(section))  )
    {
        return -1;
    }

    if((comment != NULL) && (sizeof(((struct SECTION*) 0)->comments) <= strlen(comment))) //入参小于数组定义长度
    {
        return -1;
    }
    psection = ini_get_section((struct CONTENT *)handler, section);

    if (psection == NULL)
    {
        psection = (struct SECTION *) rt_malloc(sizeof(struct SECTION));

        if (psection == NULL)
        {
            TRACE("cannot dev_mem_malloc memory !\n");
            return -1;
        }
        memset(psection, 0, sizeof(struct SECTION));

        strcpy(psection->name, section);

        if(comment != NULL) //comment 如果不为空shiweisong 2013.09.29 15:39
        {
            if ((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
            {
                sprintf(psection->comments, "#%s", comment);
            }
            else
            {
                strcpy(psection->comments, comment);
            }
        }
        psection->first = NULL;
        psection->last = NULL;
        psection->next = NULL;
        psection->sizeRecord = 0;

        ((struct CONTENT *)handler)->sizeSection++;

        if (((struct CONTENT *)handler)->first == NULL)
        {
            ((struct CONTENT *)handler)->first = psection;
            ((struct CONTENT *)handler)->last = psection;
        }
        else
        {
            ((struct CONTENT *)handler)->last->next = psection;
            ((struct CONTENT *)handler)->last = psection;
        }
    }
    else
    {
        strcpy(psection->name, section);

        if(comment != NULL) //comment 如果不为空shiweisong 2013.09.29 15:39
        {
            if ((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
            {
                sprintf(psection->comments, "#%s", comment);
            }
            else
            {
                strcpy(psection->comments, comment);
            }
        }
    }
    return strlen(section);
}

static struct RECORD* ini_get_record(struct SECTION* psection, const char* lpKey)
{
    int found = 0;
    struct RECORD* pRecord;

    if (psection != NULL)
    {
        pRecord = psection->first;

        while (pRecord != NULL)
        {
            //TRACE("lpKey:%s, pRecord->key:%s\r\n",lpKey, pRecord->key);
            if (strcmp(lpKey, pRecord->key) == 0)
            {
                found = 1;
                break;
            }
            pRecord = pRecord->next;
        }

        if (found == 1)
        {
            return pRecord;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

/*****************************************************************************
** Descriptions:	增加关键字
** Parameters:          SDK_INI_HAND *handler:SDK_INI_HAND指针
                               const char* section:小节名称
                               const char* key:关键字
                               const char* comment:注释
** Returned value:	内存错误返回:-1
                                传入参数为NULL则返回SDK_PARA_ERR
                                传入小节名称、关键字或者注释长度超限则返回SDK_PARA_ERR
                                小节不存在返回SDK_INI_NO_SECTION
                                正确返回:关键字长度
** Created By:		shijianglong  2013.03.14
** Remarks:
*****************************************************************************/
static int ini_add_key(ini_ctrl_t* handler, const char* section, const char* key, const char* comment)
{
    struct SECTION* psection = NULL;
    struct RECORD* precord = NULL;


    if(NULL == section || NULL == key ||  handler == NULL)   //fusuipu 2013.05.15 17:13
    {
        return -1;
    }

    if((sizeof(((struct SECTION*) 0)->name) <= strlen(section)) ||
       (sizeof(((struct RECORD*) 0)->key) <= strlen(key)))       //入参小于数组定义长度
    {
        return -1;
    }

    if((comment != NULL && (sizeof(((struct RECORD*) 0)->comments) <= strlen(comment))))
    {
        return -1;
    }
    psection = ini_get_section((struct CONTENT *)handler, section);

    /*=======BEGIN: fusuipu 2013.05.15  15:58 modify===========*/
    if (psection == NULL)
    {
        ini_add_section(handler, section, comment);
        psection = ini_get_section(handler, section);
    }

    if (psection == NULL)
    {
        return -1;
    }
    /*====================== END======================== */
    precord = ini_get_record(psection, key);

    if (precord == NULL)
    {
//        return SDK_INI_NO_KEY;
        precord = (struct RECORD *) rt_malloc(sizeof(struct RECORD));

        if (precord == NULL)
        {
            TRACE("cannot dev_mem_malloc memory !\n");
            return -1;
        }
        memset(precord, 0, (sizeof(struct RECORD)));

        precord->next = NULL;

        psection->sizeRecord++;

        if (psection->first == NULL)
        {
            psection->first = precord;
            psection->last = precord;
        }
        else
        {
            psection->last->next = precord;
            psection->last = precord;
        }
    }

    if(comment != NULL) //shiweisong 2013.09.29 16:7
    {
        if ((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
        {
            sprintf(precord->comments, "#%s", comment);
        }
        else
        {
            strcpy(precord->comments, comment);
        }
    }
    strcpy(precord->key, key);
    {
        strcpy(precord->value, "");
    }
    return strlen(key);
}

static struct SECTION* ini_get_section(ini_ctrl_t* handler, const char* lpSection)
{
    int found = 0;

    struct SECTION* psection = ((struct CONTENT *)handler)->first;

    while (psection != NULL)
    {
        //TRACE("psection->name:%s, lpSection:%s\r\n",psection->name, lpSection);
        if (strcmp(psection->name, lpSection) == 0)
        {
            found = 1;
            break;
        }
        psection = psection->next;
    }

    if (found == 1)
    {
        return psection;
    }
    else
    {
        return NULL;
    }
}


/*****************************************************************************
** Descriptions:	删除ini文件中某个小节的数据
** Parameters:          const char* section:小节名称
                               ini_ctrl_t * *handler:SDK_INI_CONTENT指针
** Returned value:	无此小节返回:SDK_INI_NO_SECTION
                                传入参数为NULL则返回SDK_PARA_ERR
                                传入小节名称长度超限则返回SDK_PARA_ERR
                                成功返回:小节内关键字个数
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
static int ini_clr_section(ini_ctrl_t * handler, const char* section)
{
    struct SECTION* psection = NULL;
    struct RECORD*  precord;
    int remove = 0;

    if(NULL == section || NULL == handler)
    {
        return -1;
    }

    if(sizeof(((struct SECTION*) 0)->name) < strlen(section))       //入参小于数组定义长度
    {
        return -1;
    }
    psection = ini_get_section((struct CONTENT*)handler, section);

    if (psection == NULL)
    {
        return -1;
    }
    precord = psection->first;

    while (precord != NULL)
    {
        psection->first = precord->next;
        psection->sizeRecord--;
        rt_free(precord);
        remove++;
        precord = psection->first;
    }

    return remove;
}

/*****************************************************************************
** Descriptions:
** Parameters:          ini_ctrl_t * handler
                               const char* lsection
** Returned value:
** Created By:		fusuipu  2013.05.16
** Remarks:
*****************************************************************************/
static int ini_del_section(ini_ctrl_t * handler, const char* lsection)
{
    int found = 0;
    struct SECTION* psecpre = NULL;
    struct SECTION* psection = ((struct CONTENT *)handler)->first;

    while (psection != NULL)
    {
        TRACE("ini_del_section name:%s,section:%s\r\n",psection->name, lsection);
        if (strcmp(psection->name, lsection) == 0)
        {
            found = 1;
            break;
        }
        psecpre = psection;
        psection = psection->next;
    }

    if (found == 1)
    {
        if(NULL == psecpre)
            ((struct CONTENT *)handler)->first = psection->next;
        else
            psecpre->next = psection->next;
        ((struct CONTENT *)handler)->sizeSection--;
        rt_free(psection);
        return 0;
    }
    else
    {
        return -1;
    }
}


/*****************************************************************************
** Descriptions:        加载ini文件信息
** Parameters:          const char* pFile:文件路径名称
** Returned value:	成功返回SDK_INI_CONTENT指针
                                失败或者参数错返回NULL
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
ini_ctrl_t* dev_ini_load(const char* file)
{
    int inifd;
    struct CONTENT *handler = NULL;

    if(NULL == file)
    {
        return NULL;
    }

    inifd = open(file, O_RDONLY, 0);
    if(-1 == inifd)
    {
        TRACE("creat file fail!\n");
        return NULL;
    }
    close(inifd);
    
    handler = (struct CONTENT *) rt_malloc(sizeof(struct CONTENT));

    if (handler == NULL)
    {
        TRACE("cannot dev_mem_malloc memory !\n");
        return NULL;
    }
    memset(handler, 0, (sizeof(struct CONTENT)));

    handler->sizeSection = 0;
    handler->first = NULL;
    handler->last = NULL;

    if (ini_load(handler, file) == -1)
    {
    //    TRACE("initial parse file error !\n");
        rt_free(handler);
        return NULL;
    }

    memset(ini_filename,0,sizeof(ini_filename));
    snprintf(ini_filename,sizeof(ini_filename),"%s",file);
    handler->ischg = 0;    //ini_load会调用dev_ini_set_key函数
    return (ini_ctrl_t*)handler;
}

/*****************************************************************************
** Descriptions:	获取ini文件数据
** Parameters:          const char* section:小节名称
                               const char* key:关键字
                               char* buffer:读出来的数据
                               ini_ctrl_t* *handler:SDK_INI_CONTENT指针
** Returned value:	成功返回获取数据长度
                                传入参数为NULL则返回SDK_PARA_ERR
                                传入小节名称或者关键字名称长度超限则返回SDK_PARA_ERR
                                没有该小节返回SDK_INI_NO_SECTION
                                没有该关键字返回SDK_INI_NO_KEY
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
int dev_ini_get_key(ini_ctrl_t* handler, const char* section, const char* key, char* buffer)
{
    struct SECTION* psection = NULL;

    struct RECORD* precord = NULL;

    TRACE("dev_ini_get_key\r\n");
    if(NULL == section || NULL == key || NULL == buffer || NULL == handler)
    {
        return -1;
    }
    psection = ini_get_section((struct CONTENT *)handler, section);
    precord = ini_get_record(psection, key);

    if((sizeof(((struct SECTION*) 0)->name) <= strlen(section)) || 
       (sizeof(((struct RECORD*) 0)->key) <= strlen(key)))       //入参小于数组定义长度
    {
        return -1;
    }

    if (psection == NULL)
    {
        return DEV_INI_NO_SECTION;
    }

    if (precord == NULL)
    {
        return DEV_INI_NO_KEY;
    }
    else
    {
        strcpy(buffer, precord->value);
        return strlen(buffer);
    }
}

/*****************************************************************************
** Descriptions:	写入ini文件数据
** Parameters:          const char* section:小节名称
                    const char* key:关键字
                    const char* value:写入数据
                    const char* comment:写入的注释，如无则传入"".
                    ini_ctrl_t* *handler:SDK_INI_CONTENT指针
** Returned value:	内存错误返回:-1
                    传入参数为NULL则返回SDK_PARA_ERR
                    传入小节名称、关键字、数据或者注释长度超限则返回SDK_PARA_ERR
                    无此小节返回SDK_INI_NO_SECTION
                    无此关键字返回SDK_INI_NO_KEY
                    成功返回:写入数据长度
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
int dev_ini_set_key(ini_ctrl_t* handler, const char* section, const char* key, const char* value, const char* comment)
{
    struct SECTION* psection = NULL;
    struct RECORD* precord = NULL;
    struct CONTENT *phandler = (struct CONTENT *)handler;

    if(NULL == section || NULL == key || NULL == value || NULL == handler) //fusuipu 2013.05.15 17:17
    {
        return -1;
    }

    /*=======BEGIN: fusuipu 2013.05.15  17:27 modify===========*/
    if((sizeof(((struct SECTION*) 0)->name) <= strlen(section)) ||
       (sizeof(((struct RECORD*) 0)->key) <= strlen(key)) ||
       (sizeof(((struct RECORD*) 0)->value) <= strlen(value)) )       //入参小于数组定义长度
    {
        return -1;
    }

    if((comment != NULL) && (sizeof(((struct RECORD*) 0)->comments) <= strlen(comment))) //shiweisong 2013.09.29 16:9
    {
        return -1;
    }
    /*====================== END======================== */
    psection = ini_get_section((struct CONTENT *)handler, section);

    if (psection == NULL)
    {
        ini_add_section(handler, section, comment);
        psection = ini_get_section(handler, section);
    }

    if (psection == NULL)
    {
        return DEV_INI_NO_SECTION;
    }
    precord = ini_get_record(psection, key);

    /*=======BEGIN: fusuipu 2013.05.14  15:40 modify===========*/
    if (precord == NULL)
    {
        ini_add_key(handler, section, key, comment);
        precord = ini_get_record(psection, key);
    }

    if (precord == NULL)
    {
        return DEV_INI_NO_SECTION;
    }

    if(comment != NULL)
    {
        if ((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
        {
            sprintf(precord->comments, "#%s", comment);
        }
        else
        {
            strcpy(precord->comments, comment);
        }
    }
    strcpy(precord->key, key);
    {
        strcpy(precord->value, value);
    }

    phandler->ischg = 1;
    return strlen(value);
}

/*****************************************************************************
** Descriptions:	删除某小节内的一个关键字
** Parameters:          const char* pSection:小节名称
                               const char* pKey:关键字
                               SDK_INI_HAND *pHand:SDK_INI_CONTENT指针
** Returned value:  小节不存在返回SDK_INI_NO_SECTION
                                key不存在返回SDK_INI_NO_KEY
                                传入参数为NULL则返回SDK_PARA_ERR
                                传入小节名称、关键字长度超限则返回SDK_PARA_ERR
                                删除成功返回SDK_OK
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
int dev_ini_del_key(ini_ctrl_t *handler, const char* section, const char* key)
{
    struct SECTION* psection = NULL;
    struct RECORD* precord1, * precord2;
    int remove = 0;
    struct CONTENT *phandler = (struct CONTENT *)handler;

    if(NULL == section || NULL == key || NULL == handler)
    {
        return -1;
    }

    if((sizeof(((struct SECTION*) 0)->name) < strlen(section)) ||
       (sizeof(((struct RECORD*) 0)->key) < strlen(key)))       //入参小于数组定义长度
    {
        return -1;
    }
    psection = ini_get_section((struct CONTENT *)handler, section);

    if (psection == NULL)
    {
        return -1;
    }
    precord1 = psection->first;

    if (precord1 == NULL)
    {
        return -1;
    }

    if (strcmp(key, precord1->key) == 0)
    {
        psection->first = precord1->next;
        psection->sizeRecord--;
        rt_free(precord1);
        
        phandler->ischg = 1;
        return 0;
    }

    while (precord1 != NULL)
    {
        if (precord1->next != NULL)
        {
            if (strcmp(key, precord1->next->key) == 0)
            {
                precord2 = precord1->next;
                precord1->next = precord1->next->next;
                psection->sizeRecord--;
                rt_free(precord2);
                remove = 0;
                break;
            }
        }
        precord1 = precord1->next;
    }

    phandler->ischg = 1;

    return remove;
}

/*****************************************************************************
** Descriptions:
** Parameters:          ini_ctrl_t * handler
                               const char* section
** Returned value:
** Created By:		fusuipu  2013.05.16
** Remarks:
*****************************************************************************/
int dev_ini_del_section(ini_ctrl_t * handler, const char* section)
{
    int ret = 0;
    struct CONTENT *phandler = (struct CONTENT *)handler;

    if(NULL == section || NULL == handler)
    {
        return -1;
    }

    if(sizeof(((struct SECTION*) 0)->name) < strlen(section))       //入参小于数组定义长度
    {
        return -1;
    }
    ret = ini_clr_section(handler, section);

    if(ret < 0)
    {
        return ret;
    }

    ret = ini_del_section(handler, section);
    if(0 == ret)
        phandler->ischg = 1;
    
    return ret;
}

/*****************************************************************************
** Descriptions:	保存修改后的ini文件
** Parameters:          const char* pFile:文件路径名称
                               ini_ctrl_t* *handler:SDK_INI_CONTENT指针
** Returned value:	文件操作失败返回:-1
                                传入参数为NULL则返回SDK_PARA_ERR
                                成功返回:0
** Created By:		shijianglong  2013.03.13
** Remarks:
*****************************************************************************/
int dev_ini_save(ini_ctrl_t* handler)
{
    int ini_fd;
    struct SECTION* psection = NULL;
    struct RECORD* precord;
    char buff[DEV_DEF_KEY_VALUE_LEN+2] = {0};
    struct CONTENT *phandler = (struct CONTENT *)handler;

    if(NULL == handler)
    {
        return -1;
    }
    psection = ((struct CONTENT *)handler)->first;

    if (-1 == (ini_fd = open(ini_filename, O_WRONLY | O_CREAT | O_TRUNC, 0)))
    {
        TRACE("open file error\n");
        return -1;
    }

    while (psection != NULL)
    {
        if (strlen(psection->comments) != 0)
        {
            memset(buff,0,DEV_DEF_KEY_VALUE_LEN+2);
            snprintf(buff,DEV_DEF_KEY_VALUE_LEN+2,"%s\r\n",psection->comments);
            write(ini_fd,buff, strlen(buff));
        }
        memset(buff,0,DEV_DEF_KEY_VALUE_LEN+2);
        snprintf(buff,DEV_DEF_KEY_VALUE_LEN+2,"[%s]\r\n", psection->name);
        write(ini_fd, buff, strlen(buff));
        precord = psection->first;

        while (precord != NULL)
        {
            if (strlen(precord->comments) != 0)
            {
                memset(buff,0,DEV_DEF_KEY_VALUE_LEN+2);
                snprintf(buff,DEV_DEF_KEY_VALUE_LEN+2,"%s\r\n", precord->comments);
                write(ini_fd, buff, strlen(buff));
            }
            memset(buff,0,DEV_DEF_KEY_VALUE_LEN+2);
            snprintf(buff,DEV_DEF_KEY_VALUE_LEN+2,"%s=%s\r\n", precord->key, precord->value);
            write(ini_fd, buff, strlen(buff));

            precord = precord->next;
        }

        psection = psection->next;
    }

    close(ini_fd);
    phandler->ischg = 0;
    return 0;
}

/*****************************************************************************
** Descriptions:	关闭文件操作
** Parameters:          ini_ctrl_t* handler:文件数据指针
** Returned value:	参数错误返回SDK_PARA_ERR
                                成功返回SDK_OK
** Created By:		shijianglong  2013.03.14
** Remarks:
*****************************************************************************/
int dev_ini_free(ini_ctrl_t* handler)
{
    struct SECTION* psection = NULL;
    struct RECORD* precord;
    struct SECTION* ptempsection;
    struct RECORD* ptemprecord;

    if(NULL == handler)
    {
        return -1;
    }
    psection = ((struct CONTENT *)handler)->first; //fusuipu 2013.03.20 11:4

    while (psection != NULL)
    {
        precord = psection->first;

        while (precord != NULL)
        {
            ptemprecord = precord->next;
            rt_free(precord);
            precord = ptemprecord;
        }

        ptempsection = psection->next;
        rt_free(psection);
        psection = ptempsection;
    }

    rt_free((struct CONTENT *)handler);
    return 0;
}

int dev_ini_create(const char* file)
{
	int fd;
    
	fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		rt_kprintf("open file for write failed\n");
		return -1;
	}
    
    close(fd);

    return 0;
}

static char ini_path[64]="test.ini";
int ini_test(int argc,char *argv[])
{
    char *pcmd,*psection,*pkey,*pvalue;
    ini_ctrl_t *ini_handle = NULL;
    char buff[64];
#if 0
    ini_handle = dev_ini_load(ini_path);
    if(ini_handle == NULL)
    {
        printf("INI FILE:%s not exist, now create it\n",ini_path);
        dev_ini_create(ini_path);
        ini_handle = dev_ini_load(ini_path);
    }
#endif


    pcmd = argv[1];
    
    if(strcmp(pcmd,"file") == 0)
    {
        if(argc > 2)
        {
            memset(ini_path,0,sizeof(ini_path));
            strncpy(ini_path,argv[2],sizeof(ini_path)-1);
            rt_kprintf("set ini file path:%s\n",ini_path);
        }
        else
        {
            rt_kprintf("cur ini file path:%s\n",ini_path);
        }
    }
    else if(strcmp(pcmd,"create") == 0)
    {
        dev_ini_create(ini_path);
        rt_kprintf("create empty ini file:%s\n",ini_path);
    }
    else if(strcmp(pcmd,"dump") == 0)
    {
        ini_handle = dev_ini_load(ini_path);
        if(ini_handle == NULL)
        {
            printf("Load ini file:%s error\n",ini_path);
            return -1;
        }

        //dev_ini_set_key(ini_handle,"MAIN","baudrate","115200",NULL);
        //dev_ini_set_key(ini_handle,"MAIN","interface","uart",NULL);

        dev_ini_save(ini_handle);
        dev_ini_free(ini_handle);        
    }
    else if(strcmp(pcmd,"set") == 0)
    {
        if(argc < 5)
        {
            rt_kprintf("param number error\n");
            return -1;
        }

        psection = argv[2];
        pkey     = argv[3];
        pvalue   = argv[4];
        ini_handle = dev_ini_load(ini_path);
        if(ini_handle == NULL)
        {
            printf("Load ini file:%s error\n",ini_path);
            return -1;
        }

        dev_ini_set_key(ini_handle,psection,pkey,pvalue,NULL);
        
        dev_ini_save(ini_handle);
        dev_ini_free(ini_handle);
    }
    else if(strcmp(pcmd,"get") == 0)
    {
        if(argc < 4)
        {
            rt_kprintf("param number error\n");
            return -1;
        }
        
        psection = argv[2];
        pkey     = argv[3];

        ini_handle = dev_ini_load(ini_path);
        if(ini_handle == NULL)
        {
            printf("Load ini file:%s error\n",ini_path);
            return -1;
        }

        memset(buff,0,sizeof(buff));
        dev_ini_get_key(ini_handle,psection,pkey,buff);
        
        //dev_ini_save(ini_handle);
        dev_ini_free(ini_handle);

        rt_kprintf("%s\n",buff);
    }
    else
    {}
    

    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(ini_test, ini file test);
#endif

#if 0
int ini_test()
{
#define INI_TEST  "mtd0/test_ini.ini"

    ini_ctrl_t* ini_hand;
    u8 buff[255] = {0};
    char buff1[1024] = "";
    int fd1 = 0;   
    
    if(0 != dev_vfs_access(INI_TEST))
    {
        ini_hand = dev_ini_creat();
        if(NULL == ini_hand)
        {
            TRACE("NULL == ini_hand");
            return -2;
        }
        
        if(0 != dev_ini_save(ini_hand,INI_TEST))
        {
            TRACE("creat ini failed");
            return -3;
        }

        dev_ini_close(ini_hand);
    }

    ini_hand = dev_ini_load(INI_TEST);
    if(NULL == ini_hand)
    {
        TRACE("load ini failed");
        return -4;
    }

    dev_ini_set_key(ini_hand, "data", "val", "helloworld", "ini test");

    memset(buff,0,sizeof(buff));
    dev_ini_get_key(ini_hand, "data", "val", buff);
    TRACE("buff:%s",buff);

    dev_ini_save(ini_hand, INI_TEST);
    dev_ini_close(ini_hand);
 

    if(0 != ddi_vfs_access(INI_TEST))
    {
        TRACE("file 111not exist");
        return ;
    }

    fd1 = ddi_vfs_open(INI_TEST,"r");
    if(0 == fd1)
    {
        TRACE("open failed");
        return ;
    }

    memset(buff1,0,1024);
    TRACE("read len111:%d",ddi_vfs_read(buff1, 1024, fd1));
    PrintFormat(buff1, 357);
    ddi_vfs_close(fd1);
}
#endif
