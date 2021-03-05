/*
 * =====================================================================================
 *
 *      Filename:   PHP7HashTable.h
 *
 *      Description:  PHP7HashTable 公共头文件
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
 *        Created:  2021年03月03日 23时50分50秒 
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  blogdaren
 *   Organization:  http://www.phpcreeper.com
 *
 * =====================================================================================
 */

typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef unsigned char       boolean; 
typedef unsigned long       ulong;
typedef unsigned short      ushort;
typedef char *              String;
typedef struct HashTable    HashTable;
typedef struct Bucket       Bucket;
typedef struct BucketValue  BucketValue;

#define IS_UNDEF    (1<<0)
#define IS_STRING   (1<<1)
#define IS_LONG     (1<<2)
#define IS_DOUBLE   (1<<3)
#define IS_ARRAY    (1<<4)

#define DEBUG                   1
#define USER_API
#define SUCCESS                 1
#define FAILURE                 0
#define HT_MIN_SIZE             8
#define HT_INVALID_IDX          ((uint32_t)-1)   


#define HT_HASH_SIZE(mask)  \
        (((size_t)(uint32_t)-(int32_t)(mask)) * sizeof(uint32_t))
#define HT_DATA_SIZE(size)  \
        ((size_t)(size) * sizeof(Bucket))
#define HT_SIZE_TOTAL(mask, size)   \
        (HT_HASH_SIZE((mask)) + HT_DATA_SIZE((size)))
#define HT_SIZE(ht) \
        HT_SIZE_TOTAL((ht)->tableMask, (ht)->tableSize)
#define HT_GET_HASH_START_ADDRESS(ht)  \
        ((uint32_t*)((char *)ht->arData - HT_HASH_SIZE(ht->tableMask)))
#define HT_OFFSET(ht, h)    \
        (h | ht->tableMask)
#define HT_IDX(ht, h)   \
        (*((uint32_t*)ht->arData + HT_OFFSET(ht, h)))
#define CHECK_WHETHER_NEED_TO_DO_RESIZE(ht) do{ \
            if((ht)->positionNumberUsed >= (ht)->tableSize){    \
                resizeHashTable(ht);    \
            }   \
        }while(0)   \



struct HashTable
{
    size_t tableSize;
    size_t tableMask;
    size_t positionNumberUsed;
    size_t validElementNumber;
    Bucket *arData;
};

struct Bucket
{
    ulong       h;
    String      key;
    BucketValue *val;
    size_t      next;
};

struct BucketValue
{
    ushort  type;
    ushort  flag;
    union {
        long        lval;
        double      dval;
        char        *str;
        HashTable   *arr;
    } v;
};

static HashTable *initHashTable(size_t size);
USER_API HashTable *createHashTable(size_t size);
USER_API BucketValue *setHashTableData(HashTable *ht, String key, BucketValue *pbv);
USER_API BucketValue *updateHashTableData(HashTable *ht, String key, BucketValue *pbv);
USER_API BucketValue *findHashTableDataByKey(HashTable *ht, String key);
static BucketValue *copyBucketValue(BucketValue *pbv);
USER_API boolean deleteHashTableDataByKey(HashTable *ht, String key);
USER_API boolean deleteHashTableDataByBucket(HashTable *ht, Bucket *pb);
static void resizeHashTable(HashTable *ht);
static boolean rehashHashTable(HashTable *ht);
USER_API void freeHashTable(HashTable *ht);
static void freeBucketKey(Bucket *pb);
static void freeBucketVal(BucketValue *pbv);
USER_API ulong getStringHash(char* str, uint32_t len);
USER_API size_t getHashTableSize(size_t size);


