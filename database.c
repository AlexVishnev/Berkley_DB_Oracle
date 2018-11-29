#include <db.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_database
{
    DB          *db_ptr;
    int         error;   
    u_int32_t   flag;
    char        *path;
}               t_database;

typedef struct  s_handler
{
    DBC *cursor;
    DBT key;
    DBT data;
}                  t_handler;

static void     data_base_write(t_database *_d, int key, char *_data)
{
    t_handler *ptr = (t_handler *)malloc(sizeof(t_handler));

    memset(&ptr->key, 0, sizeof(DBT));
    memset(&ptr->data, 0, sizeof(DBT));
    

    ptr->key.data = &key;
    ptr->key.size = sizeof(int);
    ptr->key.flags = DB_DBT_USERMEM;

    ptr->data.data = _data;
    ptr->data.size = strlen(_data) + 1;

    _d->error = _d->db_ptr->put(_d->db_ptr, NULL, &ptr->key, &ptr->data, DB_NOOVERWRITE);
    if (_d->error == DB_KEYEXIST)
        printf("exits: %s\n", db_strerror(_d->error));
    free(ptr);
}

static void    data_base_read(t_database *_data)
{
    t_handler *ptr = (t_handler *)malloc(sizeof(t_handler));

    memset(&ptr->key, 0, sizeof(DBT));
    memset(&ptr->data, 0, sizeof(DBT));
    
    _data->db_ptr->cursor(_data->db_ptr, NULL, &ptr->cursor, 0);
    while((_data->error = ptr->cursor->get(ptr->cursor, &ptr->key, &ptr->data, DB_NEXT)) == 0)
        printf("DATA == %s\n",(char *)ptr->data.data);

    if (ptr->cursor != NULL)
        ptr->cursor->close(ptr->cursor);
    if (_data->error)
        printf("%s\n", db_strerror(_data->error));
    free(ptr);
}

void     data_base_update(t_database *_data, int keys[], void *_addr)
{
    t_handler *ptr = (t_handler *)malloc(sizeof(t_handler));

    memset(&ptr->key, 0, sizeof(DBT));
    memset(&ptr->data, 0, sizeof(DBT));
    
    _data->db_ptr->cursor(_data->db_ptr, NULL, &ptr->cursor, 0);
    int cnt = 0;
    while((_data->error = ptr->cursor->get(ptr->cursor, &ptr->key, &ptr->data, DB_NEXT)) == 0)
    {
        ptr->data.data = _addr;
        ptr->key.data = &keys[cnt++];
    }

    if (ptr->cursor != NULL)
        ptr->cursor->close(ptr->cursor);
    if (_data->error)
        printf("update: %s\n", db_strerror(_data->error));
    free(ptr);
}
void     data_base_delete(t_database *_data)
{
    t_handler *ptr = (t_handler *)malloc(sizeof(t_handler));

    memset(&ptr->key, 0, sizeof(DBT));
    memset(&ptr->data, 0, sizeof(DBT));

    _data->db_ptr->del(_data->db_ptr, NULL, &ptr->key, 0);
    free(ptr); 
}

int main(int ac, char *av[])
{
    t_database *_data = (t_database *)malloc(sizeof(t_database));
    _data->flag = DB_CREATE;
    _data->error = db_create(&_data->db_ptr, NULL, 0);
    _data->path = "test.db";

    if (_data->error)
        printf("create: %s\n", db_strerror(_data->error));
   
    if (ac > 1)
        _data->path = strdup(av[1]);
    _data->error = _data->db_ptr->open(_data->db_ptr, NULL, _data->path, 
                                            NULL, DB_BTREE, _data->flag, 0);
    if (_data->error){
        printf("open: %s\n", db_strerror(_data->error));
        exit(-1);
    }

    data_base_delete(_data);
    data_base_write(_data, 1 , "Hellow");
    data_base_write(_data, 2 , "Testing");
    data_base_write(_data, 2 , "still Testing");
    data_base_read(_data);

    int test[5] = {1,2,3,4,5};
    data_base_update(_data, test , NULL);
    data_base_delete(_data);
    if (_data->db_ptr != NULL)
        _data->db_ptr->close(_data->db_ptr, 0);
    if (ac > 1)
        free(_data->path);
    free(_data);
    return 0;
}
