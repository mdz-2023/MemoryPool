 #pragma once
/*
* ��ԭstruct�ṹ�� ����Ϊ�����
*/

// �����ض���
using u_char = unsigned char;
using ngx_uint_t = unsigned int;
// ǰ����������
struct ngx_pool_s;

// ���������ص�������������
typedef void (*ngx_pool_cleanup_pt)(void* data); // �ص����� ����ָ��
struct ngx_pool_cleanup_s { // ��������Ϣ��Ҳ�������С���ڴ���ڴ����
    ngx_pool_cleanup_pt   handler; // �ص����� ����ָ��  ����Ԥ�����õĻص�����
    void*                 data; // ��Դ��ַ
    ngx_pool_cleanup_s*   next; // �ܶ��ͷ���Դ�Ķ�������������
};

//����ڴ�ͷ��Ϣ
struct ngx_pool_large_s {
    ngx_pool_large_s*   next; // ���еĴ���ڴ����Ҳ������һ��������
    void*               alloc; // ��������ȥ�Ĵ���ڴ����ʼ��ַ
};

// С���ڴ�ͷ��Ϣ���ڴ�ص�ÿ���鶼��
struct ngx_pool_data_t {
    u_char*     last; // С���ڴ�ؿ����ڴ����ʼ��ַ
    u_char*     end; // �����ڴ��ĩβ��ַ
    ngx_pool_s* next; // ����С���ڴ�ض���������һ��������
    ngx_uint_t  failed; // ��¼��ǰ����Ϊcurrent�飬�����ڴ�ʧ�ܵĴ���
};

// �ڴ��ͷ��Ϣ�͹�����Ϣ��ֻ���ڴ�صĵ�һ������
struct ngx_pool_s {
    ngx_pool_data_t       d; // poolͷ��Ϣ��С���ڴ�ص�ʹ�����
    size_t                max; // ��С��ķֽ���
    ngx_pool_s*           current; // ��һ���ṩС���ڴ�����С���ڴ��
    //ngx_chain_t* chain; // ���������ڴ��
    ngx_pool_large_s*     large; // ����ڴ�����ָ��
    ngx_pool_cleanup_s*   cleanup; // �ڴ�ص�������ػص�������������������������
    //ngx_log_t* log; // ��־
};
