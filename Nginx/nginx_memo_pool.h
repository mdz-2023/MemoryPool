#pragma once
#include "struct.h"
#include <memory.h>

/*
	��ֲnginx�ڴ��Դ�룬ʹ���������ʵ��
*/

// d����ȡ�ٽ�a�ı���
#define ngx_align(d, a) (((d) + (a - 1)) & ~(a - 1))
// ��ָ��p����ȡ�ٽ�a�ı���
#define ngx_align_ptr(p, a) \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
// С���ڴ���俼���ֽڶ���ʱ�ĵ�λ
#define NGX_ALIGNMENT   sizeof(unsigned long) 
// �ڴ�����
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
//#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)

//nginxĬ��һ������ҳ��Ĵ�С
const int ngx_pagesize = 4096;
// nginxС���ڴ�ؿɷ�������ռ�
const int NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize - 1; // �ܷ���������ڴ棺4k, һҳ
// Ĭ�ϵ��ڴ�ؿ��ٴ�С
const int NGX_DEFAULT_POOL_SIZE = (16 * 1024); // Ĭ�ϳ��Ӵ�С
// �����С
const int NGX_POOL_ALIGNMENT = 16; // ���Ӷ����С
// ��С���Ӵ�С
const int NGX_MIN_POOL_SIZE = ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)),
	NGX_POOL_ALIGNMENT); // ��С��С������ȡ16�ı���

class nginx_memo_pool
{
public:
	void* ngx_create_pool(size_t size);// ����size��С���ڴ�أ�С���ڴ��С������һ��ҳ���С
	void ngx_destroy_pool();// �ڴ�ص����ٺ���
	void ngx_reset_pool(); // �ڴ�ص����ú���

	void* ngx_palloc(size_t size); // �����ڴ���룬���ڴ������size��С���ڴ�
	void* ngx_pnalloc(size_t size); // �������ڴ����, ...
	void* ngx_pcalloc(size_t size); // ����ngx_palloc�����ǳ�ʼ��Ϊ0
	//void* ngx_pmemalign(size_t size, size_t alignment); // 
	void ngx_pfree(void* p); // �ͷŴ���ڴ�


	// �ڴ��������
	ngx_pool_cleanup_s* ngx_pool_cleanup_add(size_t size); // ��ӻص���������������

private:
	ngx_pool_s* pool_; // ָ���ڴ�ص����ָ��

	void* ngx_palloc_small(size_t size, ngx_uint_t align); // С���ڴ����
	void* ngx_palloc_large(size_t size); // ����ڴ����
	void* ngx_palloc_block(size_t size); // �����µ�С���ڴ��
};

