/*
 * =====================================================================================
 *
 *      Filename:   PHP7HashTable.c
 *
 *      Description:  实战PHP7内核之哈希表官方实现
 *
 *  ------------------------------
 * / HashTable Data Layout Start /
 * ------------------------------
 *
 *                 +=============================+
 *                 | HT_IDX(ht, hN)              |
 *                 | ...                         |
 *                 | HT_IDX(ht, h2)              |
 *                 | HT_IDX(ht, h1)              |
 *                 +-----------------------------+
 * ht->arData ---> | Bucket[0]                   |
 *                 | Bucket[1]                   |
 *                 | ...                         |
 *                 | Bucket[ht->tableSize - 1]   |
 *                 +=============================+
 *
 *  -----------------------------
 * / HashTable Data Layout End  /
 * -----------------------------
 *
 *        Version:  1.0
 *        Created:  2021年03月03日 23时50分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  blogdaren
 *   Organization:  http://www.phpcreeper.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PHP7HashTable.h"

/**
 * @brief   init hashtable
 *
 * @return  HashTable *
 */
static HashTable *initHashTable(size_t size)
{
    HashTable *ht = NULL;
    Bucket *pdata = NULL;
    uint32_t *hash_start_address;
    int32_t i;

    if(NULL == ht) 
    {
        ht = (HashTable *)malloc(sizeof(HashTable));
    }

    if(NULL == ht) return;

    ht->tableSize = getHashTableSize(size);
    ht->tableMask = -(ht->tableSize);
    ht->positionNumberUsed = 0;
    ht->validElementNumber = 0;

    pdata = (Bucket*)((char *)malloc(HT_SIZE(ht)) + HT_HASH_SIZE(ht->tableMask));
    if(NULL == pdata)
    {
        free(ht);
        return NULL;
    }

    ht->arData = pdata;
    hash_start_address = HT_GET_HASH_START_ADDRESS(ht);
    memset(hash_start_address, HT_INVALID_IDX, HT_HASH_SIZE(ht->tableMask));

    for(i = 0; i < ht->tableSize; i++)
    {
        (ht->arData + i)->h = -1;
        (ht->arData + i)->key = NULL;
        (ht->arData + i)->val = NULL;
        (ht->arData + i)->next = -1;
    }

    return ht;
}

/**
 * @brief   create one new hashtable
 *
 * @param   size_t  size
 *
 * @return  HashTable *
 */
USER_API HashTable *createHashTable(size_t size)
{
    if(size <= 0) return NULL;

    HashTable *ht = NULL;
    ht = initHashTable(size);

    return ht;
}

USER_API size_t getHashTableSize(size_t size)
{
    if(size < HT_MIN_SIZE) size = HT_MIN_SIZE;

    size -= 1;
    size |= (size >> 1);
    size |= (size >> 2);
    size |= (size >> 4);
    size |= (size >> 8);
    size |= (size >> 16);
                                                                                                                                            
    return size + 1;
}

/**
 * @brief   get the hash value of string
 *
 * @param   char *      str
 * @param   uint32_t    len
 *
 * @return  ulong
 */
USER_API ulong getStringHash(char* str, uint32_t len)
{
   uint32_t i = 0;
   ulong hash = 5381;

   for(i = 0; i < len; str++, i++)
   {   
      hash = ((hash << 5) + hash) + (*str);
   }   

   return hash;
}

/**
 * @brief   set hashtable data: including add or update action
 *
 * @param   HashTable   *ht
 * @param   string      key
 * @param   BucketValue *pbv
 *
 * @return  BucketValue *
 */
USER_API BucketValue *setHashTableData(HashTable *ht, String key, BucketValue *pbv)
{
    uint32_t idx;
    ulong h;
    String mk, mv;
    Bucket *p;
    BucketValue *bval;

    if(NULL == key)  return NULL;

    bval = findHashTableDataByKey(ht, key);
    if(NULL != bval)  
    {
        return updateHashTableData(ht, key, pbv);
    }

    CHECK_WHETHER_NEED_TO_DO_RESIZE(ht);

    mk = (String)malloc(sizeof(char) * (strlen(key) + 1));
    if(NULL == mk)  return NULL;
    strcpy(mk, key);

    bval = copyBucketValue(pbv);
    if(NULL == bval)  
    {
        free(mk);
        return NULL;
    }

    p = ht->arData + ht->positionNumberUsed;
    p->h = h;
    p->key = mk;
    p->val = bval;
    h = getStringHash(key, strlen(key));
    p->next = HT_IDX(ht, h);
    HT_IDX(ht, h) = ht->positionNumberUsed;
    ht->positionNumberUsed++;
    ht->validElementNumber++;

    if(DEBUG) printf("[新 增] 成功添加元素: %s\n", p->key);

    return p->val;
}

/**
 * @brief   delete hashtable data by key
 *
 * @param   HashTable   *ht
 * @param   string      key
 *
 * @return  boolean
 */
USER_API boolean deleteHashTableDataByKey(HashTable *ht, String key)
{
    ulong h;
    Bucket *p = NULL, *prev = NULL;
    BucketValue *bval = NULL;
    uint32_t idx;

    bval = findHashTableDataByKey(ht, key);

    if(NULL == bval)  return FAILURE;

    h = getStringHash(key, strlen(key));
    idx = HT_IDX(ht, h);
    if(idx == HT_INVALID_IDX) return FAILURE;

    while(idx != HT_INVALID_IDX)
    {
        p = ht->arData + idx;

        if(strcmp(p->key, key) == 0) break;

        prev = p;
        idx = p->next;
    }

    if(prev != NULL)
    {
        prev->next = p->next;
    }
    else
    {
        HT_IDX(ht, h) = p->next;
    }

    ht->validElementNumber--;

    if(DEBUG) printf("[删 除] 成功删除元素：%s\n", p->key);

    freeBucketKey(p);

    return SUCCESS;
}

/**
 * @brief   free hashtable
 *
 * @param   HashTable *ht
 *
 * @return  void
 */
USER_API void freeHashTable(HashTable *ht)
{
    int32_t i;

    for(i = 0; i < ht->positionNumberUsed; i++)
    {
        freeBucketKey(ht->arData + i);
    }

    free(HT_GET_HASH_START_ADDRESS(ht));
    free(ht);

    ht = NULL;
}

/**
 * @brief   free bucket key 
 *
 * @param   Bucket *pb
 *
 * @return  void
 */
static void freeBucketKey(Bucket *pb)
{
    if(NULL == pb)  return;

    free(pb->key);

    pb->h = -1;
    pb->key = NULL;
    pb->next = -1;
    pb->val->flag = IS_UNDEF;

    return;
}

/**
 * @brief   free bucket value
 *
 * @param   BucketValue *pbv
 *
 * @return  void
 */
static void freeBucketVal(BucketValue *pbv)
{
    switch(pbv->type)
    {
        case IS_STRING:
            free(pbv->v.str);
            break;
        case IS_LONG:
            break;
        case IS_DOUBLE:
            break;
        case IS_ARRAY:
            freeHashTable(pbv->v.arr);
            break;
        default:
            printf("invalid bucket value type\n");
            break;
    }
}

/**
 * @brief   copy bucket value
 *
 * @param   BucketValue *pbv
 *
 * @return  BucketValue *
 */
static BucketValue *copyBucketValue(BucketValue *pbv)
{
    String mv;
    BucketValue *mbv;

    mbv = (BucketValue *)malloc(sizeof(BucketValue));
    if(NULL == mbv)  return NULL;

    switch(pbv->type)
    {
        case IS_STRING:
            mv = (String)malloc(sizeof(char) * (strlen(pbv->v.str) + 1));
            strcpy(mv, pbv->v.str);
            mbv->type = IS_STRING;
            mbv->v.str = mv;
            break;
        case IS_LONG:
            mbv->type = IS_LONG;
            mbv->v.lval = pbv->v.lval;
            break;
        case IS_DOUBLE:
            mbv->type = IS_DOUBLE;
            mbv->v.dval = pbv->v.dval;
            break;
        case IS_ARRAY:
            mbv->type = IS_ARRAY;
            mbv->v.arr = pbv->v.arr;
            break;
        default:
            printf("invalid bucket value type\n");
            return NULL;
            break;
    }

    return mbv;
}

/**
 * @brief   find hashtable data by key
 *
 * @param   HashTable   *ht
 * @param   string      key
 *
 * @return  BucketValue *
 */
USER_API BucketValue *findHashTableDataByKey(HashTable *ht, String key)
{
    ulong h;
    uint32_t hash, idx;
    Bucket *p = NULL;

    if(NULL == key)  return NULL;

    h = getStringHash(key, strlen(key));
    idx = HT_IDX(ht, h);

    while(idx != HT_INVALID_IDX)
    {
        p = ht->arData + idx;

        if(p == NULL || p->key == NULL) break;

        if((p->key == key) || (strlen(p->key) == strlen(key) && strcmp(p->key, key) == 0))
        {
            return p->val;
        }

        idx = p->next;
    }

    return NULL;
}


/**
 * @brief   updateHashTableData     
 *
 * @param   HashTable   *ht
 * @param   string      key
 * @param   BucketValue *pbv
 *
 * @return  BucketValue *
 */
USER_API BucketValue *updateHashTableData(HashTable *ht, String key, BucketValue *pbv)
{
    BucketValue *bval;

    bval = findHashTableDataByKey(ht, key);

    if(NULL == bval)  return NULL;

    switch(pbv->type)
    {
        case IS_STRING:
            bval->type = IS_STRING;
            bval->v.str = pbv->v.str;
            strcpy(bval->v.str, pbv->v.str);
            break;
        case IS_LONG:
            bval->type = IS_LONG;
            bval->v.lval = pbv->v.lval;
            break;
        case IS_DOUBLE:
            bval->type = IS_DOUBLE;
            bval->v.dval = pbv->v.dval;
            break;
        case IS_ARRAY:
            bval->type = IS_ARRAY;
            bval->v.arr = pbv->v.arr;
            break;
        default:
            printf("invalid bucket value type\n");
            return NULL;
            break;
    }

    return bval;
}

/**
 * @brief   resize hashtable
 *
 * @param   HashTable *ht
 *
 * @return  void
 */
static void resizeHashTable(HashTable *ht)
{
    Bucket *old_buckets = NULL, *new_buckets = NULL;
    uint32_t *old_hash_start_address = NULL, *new_hash_start_address;
    size_t old_table_size = ht->tableSize;

    old_buckets = ht->arData;
    old_hash_start_address = HT_GET_HASH_START_ADDRESS(ht);
    ht->tableSize = ht->tableSize + ht->tableSize;
    ht->tableMask = -(ht->tableSize);
    new_buckets = (Bucket*)((char *)malloc(HT_SIZE(ht)) + HT_HASH_SIZE(ht->tableMask));
    ht->arData = new_buckets;
    memcpy(ht->arData, old_buckets, sizeof(Bucket) * ht->positionNumberUsed);
    free(old_hash_start_address);
    new_hash_start_address = HT_GET_HASH_START_ADDRESS(ht);
    memset(new_hash_start_address, HT_INVALID_IDX, HT_HASH_SIZE(ht->tableMask));

    if(DEBUG) printf("[扩 容] 检测到哈希表数据量即将超出原始容量，触发自动扩容：%d --> %d\n", old_table_size, ht->tableSize);

    rehashHashTable(ht);
}


/**
 * @brief   rehash hashtable
 *
 * @param   HashTable   *ht
 *
 * @return  boolean
 */
static boolean rehashHashTable(HashTable *ht)
{
    String mk;
	Bucket *p, *b;
    uint32_t i, j;

	i = j = 0;
	p = ht->arData;

	if(ht->positionNumberUsed == ht->validElementNumber) 
    {
        while(i < ht->positionNumberUsed)
        {
            p->next = HT_IDX(ht, p->h);
            HT_IDX(ht, p->h) = i; 
			p++;
            i++;
		}

        return SUCCESS;
	} 

    while(i < ht->positionNumberUsed)
    {
        if(p->val->flag == IS_UNDEF) 
        {
            j = i;
            b = p;
            freeBucketVal(p->val);

            while(++i < ht->positionNumberUsed) 
            {
                p++;
                if(p->val->flag != IS_UNDEF) 
                {
                    b->h = p->h;
                    b->key = p->key;
                    b->next = HT_IDX(ht, b->h);
                    b->val = p->val;
                    HT_IDX(ht, b->h) = j; 
                    b++;
                    j++;
                }
                else
                {
                    freeBucketVal(p->val);
                }
            }

            ht->positionNumberUsed = j;
            break;
        }

        p->next = HT_IDX(ht, p->h);
        HT_IDX(ht, p->h) = i; 
        p++;
        i++;
    } 

    return SUCCESS;
}


