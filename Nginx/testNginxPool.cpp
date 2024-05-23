// Nginx.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "nginx_memo_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>


typedef struct Data stData;
struct Data
{
    char* ptr;
    FILE* pfile;
};

void func1(char* p)
{
    printf("free ptr mem!");
    free(p);
}
void func2(FILE* pf)
{
    printf("close file!");
    fclose(pf);
}
int main()
{
    // 512 - sizeof(ngx_pool_t) - 4095   =>   max
    nginx_memo_pool memPool;
    // 可实现在构造函数中
    if (memPool.ngx_create_pool(512) == nullptr)
    {
        printf("ngx_create_pool fail...");
        return 0;
    }

    void* p1 = memPool.ngx_palloc(128); // 从小块内存池分配的
    if (p1 == NULL)
    {
        printf("ngx_palloc 128 bytes fail...");
        return 0;
    }

    stData* p2 = (stData * )memPool.ngx_palloc(512); // 从大块内存池分配的
    if (p2 == NULL)
    {
        printf("ngx_palloc 512 bytes fail...");
        return 0;
    }
    p2->ptr = (char*)malloc(12);
    strcpy(p2->ptr, "hello world");
    p2->pfile = fopen("data.txt", "w");

    ngx_pool_cleanup_s* c1 = memPool.ngx_pool_cleanup_add(sizeof(char*));
    c1->handler = (ngx_pool_cleanup_pt)func1;
    c1->data = p2->ptr;

    ngx_pool_cleanup_s* c2 = memPool.ngx_pool_cleanup_add(sizeof(FILE*));
    c2->handler = (ngx_pool_cleanup_pt)func2;
    c2->data = p2->pfile;

    // 可实现在析构函数中
    memPool.ngx_destroy_pool(); // 1.调用所有的预置的清理函数 2.释放大块内存 3.释放小块内存池所有内存

    return 0;
}
