#include "nginx_memo_pool.h"
#include <stdlib.h> // malloc的头文件

void* nginx_memo_pool::ngx_create_pool(size_t size)
{
    ngx_pool_s *p;
    p = (ngx_pool_s*)malloc(size); // 不考虑跨平台，直接使用malloc，C++指针类型转换
    if (p == nullptr) {
        return NULL;
    }

    p->d.last = (u_char*)p + sizeof(ngx_pool_s); // 内存池头信息以外的头地址
    p->d.end = (u_char*)p + size;  // 内存池末尾
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_s); // 实际可用大小
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL; // 小块内存，最大用一个页面的大小

    p->current = p; // 指向当前块的起始地址
    p->large = NULL;
    p->cleanup = NULL;

    pool_ = p;
    return p;
}

void nginx_memo_pool::ngx_destroy_pool()
{
    ngx_pool_s          *p, * n;
    ngx_pool_large_s    *l;
    ngx_pool_cleanup_s  *c;

    for (c = pool_->cleanup; c; c = c->next) {
        if (c->handler) { // 执行回调函数，释放用户的外部资源
            c->handler(c->data);
        }
    }

    for (l = pool_->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = pool_, n = pool_->d.next; /* void */; p = n, n = n->d.next) {
        free(p);

        if (n == nullptr) {
            break;
        }
    }
}

void nginx_memo_pool::ngx_reset_pool()
{
    ngx_pool_s* p;
    ngx_pool_large_s* l;

    for (l = pool_->large; l; l = l->next) { // 重置大块内存
        if (l->alloc) {
            free(l->alloc);
        }
    }

    // 重置小块内存

    p = pool_;
    if (p) { // 第一块（有头信息）指向除头信息和块信息之外的内存的首地址
        p->d.last = (u_char*)p + sizeof(ngx_pool_s);
        p->d.failed = 0;

        // 第一块之后的每个块（无头信息）都指向除块信息之外的内存的首地址
        for (p = p->d.next; p; p = p->d.next) {
            p->d.last = (u_char*)p + sizeof(ngx_pool_data_t);
            p->d.failed = 0;
        }
    }

    pool_->current = pool_; // 重置头信息
    pool_->large = NULL;
}

void* nginx_memo_pool::ngx_palloc(size_t size)
{
    if (size <= pool_->max) { // max <= 4095   
        return ngx_palloc_small(size, 1); // 所需内存小于本块内存池中最大可用内存
    }

    return ngx_palloc_large(size);
}

void* nginx_memo_pool::ngx_pnalloc(size_t size)
{
    if (size <= pool_->max) { // max <= 4095   
        return ngx_palloc_small(size, 0); // 所需内存小于本块内存池中最大可用内存
    }

    return ngx_palloc_large(size);
}

void* nginx_memo_pool::ngx_pcalloc(size_t size)
{
    void* p;

    p = ngx_palloc(size);
    if (p) {
        ngx_memzero(p, size); // 初始化清零
    }

    return p;
}

void nginx_memo_pool::ngx_pfree(void* p)
{
    ngx_pool_large_s* l;

    for (l = pool_->large; l; l = l->next) {
        if (p == l->alloc) {
            
            free(l->alloc);
            l->alloc = nullptr;

            return ;
        }
    }
}

ngx_pool_cleanup_s* nginx_memo_pool::ngx_pool_cleanup_add(size_t size)
{
    ngx_pool_cleanup_s* c;

    c = (ngx_pool_cleanup_s*)ngx_palloc(sizeof(ngx_pool_cleanup_s)); // 清理信息放到小块内存池中
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = ngx_palloc(size);
        if (c->data == NULL) {
            return NULL;
        }
    }
    else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = pool_->cleanup; // 新的清除信息节点，头插法            串到cleanup节点上

    pool_->cleanup = c;

    return c;
}

void* nginx_memo_pool::ngx_palloc_small(size_t size, ngx_uint_t align)
{
    u_char* m;
    ngx_pool_s* p;

    p = pool_->current; // 从p分配内存

    do {
        m = p->d.last; // 可分配内存的起始地址

        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT); // 将m调整到 平台相关的ulong 的整数倍
        }

        if ((size_t)(p->d.end - m) >= size) { // 内存池空闲内存空间 >= 申请的内存空间
            p->d.last = m + size; // 向下偏移到新的可分配起始地址

            return m;
        }

        p = p->d.next; // 本块剩余空闲内存不够，则向后寻找

    } while (p);

    return ngx_palloc_block(size); // 找到末尾发现当前内存块链表的内存不够用，新分配一个内存块
}

void* nginx_memo_pool::ngx_palloc_large(size_t size)
{
    void* p;
    ngx_uint_t         n;
    ngx_pool_large_s* large; // 结构体中有下块地址和本块内存地址

    p = malloc(size); // malloc 开辟大块内存 
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool_->large; large; large = large->next) {
        if (large->alloc == NULL) { // 复用大块内存头信息体链表中，现有的空头信息体。 alloc为空是pfree函数做的
            large->alloc = p;
            return p;
        }

        if (n++ > 3) { // 防止找的次数太多浪费时间
            break;
        }
    }

    large = (ngx_pool_large_s *)ngx_palloc_small(sizeof(ngx_pool_large_s), 1); // 在小块内存池中，存放大块内存的头信息体
    if (large == nullptr) { // 没有开辟成功
        free(p); // 把大块内存free掉
        return nullptr;
    }

    large->alloc = p; // 头结点记录大块内存的地址信息
    large->next = pool_->large; // 大块内存头信息体 头插法进链表
    pool_->large = large;

    return p;
}

void* nginx_memo_pool::ngx_palloc_block(size_t size)
{
    u_char       *m;
    size_t        psize;
    ngx_pool_s   *p, *_new;

    psize = (size_t)(pool_->d.end - (u_char*)pool_); // 计算pool的带头信息的块大小

    m = (u_char*)malloc(psize); // 开辟与pool等大的内存块
    if (m == nullptr) {
        return NULL;
    }

    _new = (ngx_pool_s*)m; // new指向新开辟块的起始地址

    _new->d.end = m + psize; // 指向新块的末尾地址
    _new->d.next = nullptr;
    _new->d.failed = 0;

    m += sizeof(ngx_pool_data_t); // m指向头信息之后的可用起始地址
    m = ngx_align_ptr(m, NGX_ALIGNMENT); // 调整对齐倍数
    _new->d.last = m + size; // 分配出size，指向新空闲地址

    // 在各块成链后，从current块开始分配内存失败多于4次
    // 说明current块的剩余可用内存很小了，认为很难再分配出去
    // 就将下一个块作为current块
    for (p = pool_->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool_->current = p->d.next;
        }
    }

    p->d.next = _new; // 将内存块成链

    return m;
}
