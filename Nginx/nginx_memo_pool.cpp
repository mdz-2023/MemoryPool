#include "nginx_memo_pool.h"
#include <stdlib.h> // malloc��ͷ�ļ�

void* nginx_memo_pool::ngx_create_pool(size_t size)
{
    ngx_pool_s *p;
    p = (ngx_pool_s*)malloc(size); // �����ǿ�ƽ̨��ֱ��ʹ��malloc��C++ָ������ת��
    if (p == nullptr) {
        return NULL;
    }

    p->d.last = (u_char*)p + sizeof(ngx_pool_s); // �ڴ��ͷ��Ϣ�����ͷ��ַ
    p->d.end = (u_char*)p + size;  // �ڴ��ĩβ
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_s); // ʵ�ʿ��ô�С
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL; // С���ڴ棬�����һ��ҳ��Ĵ�С

    p->current = p; // ָ��ǰ�����ʼ��ַ
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
        if (c->handler) { // ִ�лص��������ͷ��û����ⲿ��Դ
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

    for (l = pool_->large; l; l = l->next) { // ���ô���ڴ�
        if (l->alloc) {
            free(l->alloc);
        }
    }

    // ����С���ڴ�

    p = pool_;
    if (p) { // ��һ�飨��ͷ��Ϣ��ָ���ͷ��Ϣ�Ϳ���Ϣ֮����ڴ���׵�ַ
        p->d.last = (u_char*)p + sizeof(ngx_pool_s);
        p->d.failed = 0;

        // ��һ��֮���ÿ���飨��ͷ��Ϣ����ָ�������Ϣ֮����ڴ���׵�ַ
        for (p = p->d.next; p; p = p->d.next) {
            p->d.last = (u_char*)p + sizeof(ngx_pool_data_t);
            p->d.failed = 0;
        }
    }

    pool_->current = pool_; // ����ͷ��Ϣ
    pool_->large = NULL;
}

void* nginx_memo_pool::ngx_palloc(size_t size)
{
    if (size <= pool_->max) { // max <= 4095   
        return ngx_palloc_small(size, 1); // �����ڴ�С�ڱ����ڴ�����������ڴ�
    }

    return ngx_palloc_large(size);
}

void* nginx_memo_pool::ngx_pnalloc(size_t size)
{
    if (size <= pool_->max) { // max <= 4095   
        return ngx_palloc_small(size, 0); // �����ڴ�С�ڱ����ڴ�����������ڴ�
    }

    return ngx_palloc_large(size);
}

void* nginx_memo_pool::ngx_pcalloc(size_t size)
{
    void* p;

    p = ngx_palloc(size);
    if (p) {
        ngx_memzero(p, size); // ��ʼ������
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

    c = (ngx_pool_cleanup_s*)ngx_palloc(sizeof(ngx_pool_cleanup_s)); // ������Ϣ�ŵ�С���ڴ����
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
    c->next = pool_->cleanup; // �µ������Ϣ�ڵ㣬ͷ�巨            ����cleanup�ڵ���

    pool_->cleanup = c;

    return c;
}

void* nginx_memo_pool::ngx_palloc_small(size_t size, ngx_uint_t align)
{
    u_char* m;
    ngx_pool_s* p;

    p = pool_->current; // ��p�����ڴ�

    do {
        m = p->d.last; // �ɷ����ڴ����ʼ��ַ

        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT); // ��m������ ƽ̨��ص�ulong ��������
        }

        if ((size_t)(p->d.end - m) >= size) { // �ڴ�ؿ����ڴ�ռ� >= ������ڴ�ռ�
            p->d.last = m + size; // ����ƫ�Ƶ��µĿɷ�����ʼ��ַ

            return m;
        }

        p = p->d.next; // ����ʣ������ڴ治���������Ѱ��

    } while (p);

    return ngx_palloc_block(size); // �ҵ�ĩβ���ֵ�ǰ�ڴ��������ڴ治���ã��·���һ���ڴ��
}

void* nginx_memo_pool::ngx_palloc_large(size_t size)
{
    void* p;
    ngx_uint_t         n;
    ngx_pool_large_s* large; // �ṹ�������¿��ַ�ͱ����ڴ��ַ

    p = malloc(size); // malloc ���ٴ���ڴ� 
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool_->large; large; large = large->next) {
        if (large->alloc == NULL) { // ���ô���ڴ�ͷ��Ϣ�������У����еĿ�ͷ��Ϣ�塣 allocΪ����pfree��������
            large->alloc = p;
            return p;
        }

        if (n++ > 3) { // ��ֹ�ҵĴ���̫���˷�ʱ��
            break;
        }
    }

    large = (ngx_pool_large_s *)ngx_palloc_small(sizeof(ngx_pool_large_s), 1); // ��С���ڴ���У���Ŵ���ڴ��ͷ��Ϣ��
    if (large == nullptr) { // û�п��ٳɹ�
        free(p); // �Ѵ���ڴ�free��
        return nullptr;
    }

    large->alloc = p; // ͷ����¼����ڴ�ĵ�ַ��Ϣ
    large->next = pool_->large; // ����ڴ�ͷ��Ϣ�� ͷ�巨������
    pool_->large = large;

    return p;
}

void* nginx_memo_pool::ngx_palloc_block(size_t size)
{
    u_char       *m;
    size_t        psize;
    ngx_pool_s   *p, *_new;

    psize = (size_t)(pool_->d.end - (u_char*)pool_); // ����pool�Ĵ�ͷ��Ϣ�Ŀ��С

    m = (u_char*)malloc(psize); // ������pool�ȴ���ڴ��
    if (m == nullptr) {
        return NULL;
    }

    _new = (ngx_pool_s*)m; // newָ���¿��ٿ����ʼ��ַ

    _new->d.end = m + psize; // ָ���¿��ĩβ��ַ
    _new->d.next = nullptr;
    _new->d.failed = 0;

    m += sizeof(ngx_pool_data_t); // mָ��ͷ��Ϣ֮��Ŀ�����ʼ��ַ
    m = ngx_align_ptr(m, NGX_ALIGNMENT); // �������뱶��
    _new->d.last = m + size; // �����size��ָ���¿��е�ַ

    // �ڸ�������󣬴�current�鿪ʼ�����ڴ�ʧ�ܶ���4��
    // ˵��current���ʣ������ڴ��С�ˣ���Ϊ�����ٷ����ȥ
    // �ͽ���һ������Ϊcurrent��
    for (p = pool_->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool_->current = p->d.next;
        }
    }

    p->d.next = _new; // ���ڴ�����

    return m;
}
