#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PHP7HashTable.h"

void showHashTableData(HashTable *ht);

int main()
{
    HashTable *ht;
    String k;
    BucketValue bv, *pbv;

    ht = createHashTable(5);

    k = "author";
    bv.type  = IS_STRING;
    bv.v.str = "blogdaren";
    pbv = setHashTableData(ht, k, &bv);

    k = "city";
    bv.type  = IS_STRING;
    bv.v.str = "yuncheng";
    pbv = setHashTableData(ht, k, &bv);

    k = "nickname";
    bv.type  = IS_STRING;
    bv.v.str = "phpcreeper";
    pbv = setHashTableData(ht, k, &bv);

    k = "site";
    bv.type  = IS_STRING;
    bv.v.str = "http://www.phpcreeer.com";
    pbv = setHashTableData(ht, k, &bv);

    showHashTableData(ht);
    deleteHashTableDataByKey(ht, "city");
    showHashTableData(ht);

    k = "score1";
    bv.type  = IS_LONG;
    bv.v.lval = 10;
    pbv = setHashTableData(ht, k, &bv);

    k = "score2";
    bv.type  = IS_LONG;
    bv.v.lval = 20;
    pbv = setHashTableData(ht, k, &bv);

    k = "score3";
    bv.type  = IS_LONG;
    bv.v.lval = 30;
    pbv = setHashTableData(ht, k, &bv);

    k = "price1";
    bv.type  = IS_DOUBLE;
    bv.v.dval = 40.56;
    pbv = setHashTableData(ht, k, &bv);

    k = "price2";
    bv.type  = IS_DOUBLE;
    bv.v.dval = 50.78;
    pbv = setHashTableData(ht, k, &bv);

    showHashTableData(ht);
    freeHashTable(ht);

    return 0;
}

void showHashTableData(HashTable *ht)
{
    puts("====================================================================");
    ulong h;
    Bucket *p;
    int32_t i = 0;

    if(NULL == ht)
    {
        printf("[通 知] 哈希表没有任何数据记录\n");
    }

    while(i < ht->positionNumberUsed)
    {
        p = ht->arData + i;

        if(p->val->flag == IS_UNDEF)
        {
            printf("[序号%d] 当前位置元素已经被置为无效元素 (即:当前元素已被伪移除)\n", i + 1);
        }
        else
        {
            switch(p->val->type)
            {
                case IS_STRING:
                    printf("[序号%d] %s => %s\n", i + 1, p->key, p->val->v.str);
                    break;
                case IS_LONG:
                    printf("[序号%d] %s => %ld\n", i + 1, p->key, p->val->v.lval);
                    break;
                case IS_DOUBLE:
                    printf("[序号%d] %s => %f\n", i + 1, p->key, p->val->v.dval);
                    break;
                case IS_ARRAY:
                    printf("[序号%d] %s => %p\n", i + 1, p->key, p->val->v.arr);
                    break;
                default:
                    break;
            }
        }
        i++;
    }

    printf("[合 计] 哈希表大小：%d | 当前总共使用槽位：%d | 当前总共有效元素: %d\n", 
            ht->tableSize, 
            ht->positionNumberUsed, 
            ht->validElementNumber
    );
    puts("====================================================================");
}



