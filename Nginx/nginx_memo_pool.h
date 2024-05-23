#pragma once
#include "struct.h"
#include <memory.h>
#include <cstdint>

/*
	移植nginx内存池源码，使用面向对象实现
*/

// d向上取临近a的倍数
#define ngx_align(d, a) (((d) + (a - 1)) & ~(a - 1))
// 把指针p向上取临近a的倍数
#define ngx_align_ptr(p, a) \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
// 小块内存分配考虑字节对齐时的单位
#define NGX_ALIGNMENT   sizeof(unsigned long) 
// 内存清零
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
//#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)

//nginx默认一个屋里页面的大小
const int ngx_pagesize = 4096;
// nginx小块内存池可分配的最大空间
const int NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize - 1; // 能分配的最大块内存：4k, 一页
// 默认的内存池开辟大小
const int NGX_DEFAULT_POOL_SIZE = (16 * 1024); // 默认池子大小
// 对齐大小
const int NGX_POOL_ALIGNMENT = 16; // 池子对齐大小
// 最小池子大小
const int NGX_MIN_POOL_SIZE = ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)),
	NGX_POOL_ALIGNMENT); // 最小大小，向上取16的倍数

class nginx_memo_pool
{
public:
	void* ngx_create_pool(size_t size);// 创建size大小的内存池，小块内存大小不超过一个页面大小
	void ngx_destroy_pool();// 内存池的销毁函数
	void ngx_reset_pool(); // 内存池的重置函数

	void* ngx_palloc(size_t size); // 考虑内存对齐，从内存池申请size大小的内存
	void* ngx_pnalloc(size_t size); // 不考虑内存对齐, ...
	void* ngx_pcalloc(size_t size); // 调用ngx_palloc，但是初始化为0
	//void* ngx_pmemalign(size_t size, size_t alignment); // 
	void ngx_pfree(void* p); // 释放大块内存


	// 内存清理相关
	ngx_pool_cleanup_s* ngx_pool_cleanup_add(size_t size); // 添加回调清理函数操作函数

private:
	ngx_pool_s* pool_; // 指向内存池的入口指针

	void* ngx_palloc_small(size_t size, ngx_uint_t align); // 小块内存分配
	void* ngx_palloc_large(size_t size); // 大块内存分配
	void* ngx_palloc_block(size_t size); // 分配新的小块内存池
};

