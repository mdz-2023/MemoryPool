 #pragma once
/*
* 将原struct结构体 声明为多个类
*/

// 类型重定义
using u_char = unsigned char;
using ngx_uint_t = unsigned int;
// 前置类型声明
struct ngx_pool_s;

// 清理函数（回调函数）的类型
typedef void (*ngx_pool_cleanup_pt)(void* data); // 回调函数 函数指针
struct ngx_pool_cleanup_s { // 本清理信息，也会分配在小块内存的内存池上
    ngx_pool_cleanup_pt   handler; // 回调函数 函数指针  保存预先设置的回调函数
    void*                 data; // 资源地址
    ngx_pool_cleanup_s*   next; // 很多释放资源的动作，做成链表
};

//大块内存头信息
struct ngx_pool_large_s {
    ngx_pool_large_s*   next; // 所有的大块内存分配也被串在一条链表上
    void*               alloc; // 保存分配出去的大块内存的起始地址
};

// 小块内存头信息，内存池的每个块都有
struct ngx_pool_data_t {
    u_char*     last; // 小块内存池可用内存的起始地址
    u_char*     end; // 可用内存的末尾地址
    ngx_pool_s* next; // 所有小块内存池都被穿在了一条链表上
    ngx_uint_t  failed; // 记录当前块作为current块，分配内存失败的次数
};

// 内存池头信息和管理信息，只有内存池的第一个块有
struct ngx_pool_s {
    ngx_pool_data_t       d; // pool头信息，小块内存池的使用情况
    size_t                max; // 大小块的分界线
    ngx_pool_s*           current; // 第一个提供小块内存分配的小块内存池
    //ngx_chain_t* chain; // 链接所有内存池
    ngx_pool_large_s*     large; // 大块内存的入口指针
    ngx_pool_cleanup_s*   cleanup; // 内存池的清理相关回调操作（类似析构函数）链表
    //ngx_log_t* log; // 日志
};
