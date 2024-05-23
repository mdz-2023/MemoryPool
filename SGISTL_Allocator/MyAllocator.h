#pragma once
/*
* 移植SGISTL二级空间配置器源码  通过模板实现
* 空间配置器是容器使用的，容器对象的产生很有可能在多个线程操作
* 所以不同于nginx，本次要用多线程和模板
*/
#include <mutex>
#include <string.h>

// 封装了malloc和free，可以设置oom释放内存的回调函数
template <int __inst>
class __malloc_alloc_template {

private:

    static void* _S_oom_malloc(size_t);
    static void* _S_oom_realloc(void*, size_t);
    
    static void (*__malloc_alloc_oom_handler)(); // 回调函数

public:

    static void* allocate(size_t __n)
    {
        void* __result = malloc(__n);
        if (0 == __result) __result = _S_oom_malloc(__n);
        return __result;
    }

    static void deallocate(void* __p, size_t /* __n */)
    {
        free(__p);
    }

    static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
    {
        void* __result = realloc(__p, __new_sz);
        if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
        return __result;
    }

    static void (*__set_malloc_handler(void (*__f)()))()
    {
        void (*__old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = __f;
        return(__old);
    }

};
template <int __inst>
void (*__malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = nullptr; // 函数指针类型


template <int __inst>
void*
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n) // out of mamery
{
    void (*__my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler; // 回调函数
        if (0 == __my_malloc_handler) { throw std::bad_alloc(); } // 没有用户回调函数
        (*__my_malloc_handler)(); // 有用户回调函数
        __result = malloc(__n);
        if (__result) return(__result); // 死循环，直到调用成功，内存分配成功
    }
}

template <int __inst>
void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (*__my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { throw std::bad_alloc(); }
        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result) return(__result);
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;

// 内存池的粒度信息，最大字节按8字节对齐，共16个数组元素，每个元素挂一个链表
enum { _ALIGN = 8 }; // 对齐
enum { _MAX_BYTES = 128 }; // 最大字节
enum { _NFREELISTS = 16 }; // 自由链表个数：_MAX_BYTES/_ALIGN


// 静态表示所有变量都用同一个内存池
// 对于模板类，需要将实现（即定义）放在头文件中
template <typename T>
class MyAllocator
{
public:
    using value_type = T;

    // 正式的allocator中有的构造函数，这里也要加进去，不然报错
    /*
    * 1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(701,27): message : 无构造函数可以接受源类型，或构造函数重载决策不明确
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(698): message : 在编译 类 模板 成员函数“std::vector<int,MyAllocator<int>>::~vector(void) noexcept”时
1>C:\Users\15092\source\repos\Nginx\SGISTL_Allocator\testMyAllocator.cpp(7): message : 查看对正在编译的函数 模板 实例化“std::vector<int,MyAllocator<int>>::~vector(void) noexcept”的引用
1>C:\Users\15092\source\repos\Nginx\SGISTL_Allocator\testMyAllocator.cpp(7): message : 查看对正在编译的 类 模板 实例化“std::vector<int,MyAllocator<int>>”的引用
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(701,25): error C2530: “_Alproxy”: 必须初始化引用
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(702,1): error C3536: “_Alproxy”: 初始化之前无法使用
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(702,9): error C2672: “_Delete_plain_internal”: 未找到匹配的重载函数
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(702,1): error C2893: 未能使函数模板“void std::_Delete_plain_internal(_Alloc &,_Alloc::value_type *const ) noexcept”专用化
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\xmemory(945): message : 参见“std::_Delete_plain_internal”的声明
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(702,1): message : 用下列模板参数:
1>C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vector(702,1): message : “_Alloc=int”
    */
    constexpr MyAllocator() noexcept {}

    constexpr MyAllocator(const MyAllocator&) noexcept = default;
    template <class _Other>
    constexpr MyAllocator(const MyAllocator<_Other>&) noexcept {}


    // 负责内存开辟的函数
    T* allocate(size_t __n) {
        __n *= sizeof(T); // 容器调用的时候__n传的是元素个数
        void* __ret = 0;

        if (__n > (size_t)_MAX_BYTES) { // 大于128字节就不用内存池 通过一级配置器一样
            __ret = malloc_alloc::allocate(__n);
        }
        else { // 小于128字节，使用二级配置器
            _Obj* volatile* __my_free_list  // 二级指针, 指向要分配的大小合适（n向上取8）的链表
                = _S_free_list + _S_freelist_index(__n);

            std::lock_guard<std::mutex> guard(mtx);
            _Obj* __result = *__my_free_list; // result指向链表
            if (__result == 0) // 链表为空
                __ret = _S_refill(_S_round_up(__n)); // 分配块大小为 n向上取8 的一个链表
            else {
                *__my_free_list = __result->_M_free_list_link; // 数组中的头结点指向链表的第二个节点
                __ret = __result; // 链表的原第一个节点分配出去
            }
            // 出作用域，lock_guard自动解锁
        }

        return (T*)__ret;
    }

    // __p指向要回收的内存起始地址，__n表示其大小
    static void deallocate(void* __p, size_t __n) {
        if (__n > (size_t)_MAX_BYTES)// 大于128字节，普通方式开辟和回收内存
            malloc_alloc::deallocate(__p, __n);
        else {
            _Obj* volatile* __my_free_list
                = _S_free_list + _S_freelist_index(__n); // 找到数组中对应的头结点
            _Obj* __q = (_Obj*)__p;

            std::lock_guard<std::mutex> guard(mtx);
            __q->_M_free_list_link = *__my_free_list; // 要归还的节点的指针，指向原链表首节点
            *__my_free_list = __q; // 数组中的头指针指向要归还的节点，完成向链表前部添加回收节点
            // lock guard is released here
        };
    }

    static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz) {
        void* __result;
        size_t __copy_sz;

        if (__old_sz > (size_t)_MAX_BYTES && __new_sz > (size_t)_MAX_BYTES) { // 没用内存池
            return(realloc(__p, __new_sz));
        }
        if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p); // 几乎同样大小，无需重新分配
        __result = allocate(__new_sz); // 从内存池中获取到新空间
        __copy_sz = __new_sz > __old_sz ? __old_sz : __new_sz;
        memcpy(__result, __p, __copy_sz); // 从旧内存拷贝数据到新内存，数据长度是两内存的较小长度
        deallocate(__p, __old_sz); // 释放旧内存
        return(__result);
    }

    // 将 __byte 上调至最临近的 _ALIGN 的倍数
    static size_t _S_round_up(size_t __bytes)
    {
        return (((__bytes)+(size_t)_ALIGN - 1) & ~((size_t)_ALIGN - 1));
    }

    // 对象构造
    void construct(T* __p, const T& val) {
        new (__p) T(val);
    }

    // 对象析构
    void destroy(T* __p) {
        __p->~T();
    }

private:
    // 每一个内存chunk块的头信息
    union _Obj {
        union _Obj* _M_free_list_link; // 静态链表的指针域
        char _M_client_data[1];    /* The client sees this.        */
    };
    // 静态数组，组织所有的静态链表。每个数组元素是_obj*
    // volatile 防止多线程对数组进行缓存，导致读不到最新数组
    static _Obj* volatile _S_free_list[_NFREELISTS];

    // 内存池基于freelist实现，需要考虑线程安全
    static std::mutex mtx;

    // 返回 _bytes 大小的chunk块 位于 free-list 中的编号
    static size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes)+(size_t)_ALIGN - 1) / (size_t)_ALIGN - 1);
    }
     
    // 把分配的chunk块进行连接
    static void* _S_refill(size_t __n) {
        int __nobjs = 20;
        char* __chunk = _S_chunk_alloc(__n, __nobjs); // 负责分配相应大小的chunk块内存池，由_chunk指针接收
        _Obj* volatile* __my_free_list; // 指向数组下挂的链表
        _Obj* __result;
        _Obj* __current_obj;
        _Obj* __next_obj;
        int __i;

        if (1 == __nobjs) return(__chunk); // _S_chunk_alloc()引用接受nobjs，如果只生成一个chunk，直接返回
        __my_free_list = _S_free_list + _S_freelist_index(__n); // 映射数组下标，取得链表头指针

        /* Build free list in chunk */
        __result = (_Obj*)__chunk; // 记录第一个块，马上分配出去
        // 字节指针+数字，往后走n个字节。即数组中链表头指针指向的下一个块（空闲）
        *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
        for (__i = 1; ; __i++) {
            __current_obj = __next_obj;
            __next_obj = (_Obj*)((char*)__next_obj + __n); // next指针偏移整个块的字节n，即指向下一个块  
            if (__nobjs - 1 == __i) { // 到链表末尾了
                __current_obj->_M_free_list_link = 0;
                break;
            }
            else {// 将连续的字节块真正形成链表，指针存到每个块节点的指针域里
                __current_obj->_M_free_list_link = __next_obj;
            }
        }
        return(__result);
    }

    // 主要负责分配chunk块
    static char* _S_chunk_alloc(size_t __size, int& __nobjs) {
        char* __result;
        size_t __total_bytes = __size * __nobjs;
        size_t __bytes_left = _S_end_free - _S_start_free; // 剩余字节数，可以分配给不同大小的chunk块

        if (__bytes_left >= __total_bytes) { // 剩余已申请的空间中足够分配20个大小为size的chunk块
            __result = _S_start_free;
            _S_start_free += __total_bytes;
            return(__result);
        }
        else if (__bytes_left >= __size) { // 不够的话，计算够分配几个的，更改分配个数
            __nobjs = (int)(__bytes_left / __size);
            __total_bytes = __size * __nobjs;
            __result = _S_start_free;
            _S_start_free += __total_bytes;
            return(__result);
        }
        else { // 实在不够用，先处理完剩余小内存，再分配新空间
            size_t __bytes_to_get =
                2 * __total_bytes + _S_round_up(_S_heap_size >> 4); // _S_heap_size除以16 再向上取8的倍数
            // 此处剩余量不够本次的一个chunk块，就将剩余的字节作为一个chunk块放给合适他的链表头部，即充分利用每一块小内存
            if (__bytes_left > 0) {
                _Obj* volatile* __my_free_list =
                    _S_free_list + _S_freelist_index(__bytes_left);

                ((_Obj*)_S_start_free)->_M_free_list_link = *__my_free_list;
                *__my_free_list = (_Obj*)_S_start_free;
            }
            _S_start_free = (char*)malloc(__bytes_to_get);
            if (0 == _S_start_free) {
                size_t __i;
                _Obj* volatile* __my_free_list;
                _Obj* __p;
                // 系统空间不够，则查看右侧更大chunk链表中有无空闲chunk
                // for循环从当前分配不了的块大小（假设为40）往右遍历数组
                // _p对右边第一个有空闲块的链表（假设为48）进行遍历
                // 然后取其头部块，大小为48，作为40的start和end，原48的链表删除此节点
                // 有start和end即为有空闲字节，可以继续递归调用本函数
                for (__i = __size;
                    __i <= (size_t)_MAX_BYTES;
                    __i += (size_t)_ALIGN) {
                    __my_free_list = _S_free_list + _S_freelist_index(__i);
                    __p = *__my_free_list;
                    if (0 != __p) {
                        *__my_free_list = __p->_M_free_list_link;
                        _S_start_free = (char*)__p;
                        _S_end_free = _S_start_free + __i;
                        return(_S_chunk_alloc(__size, __nobjs));
                        // Any leftover piece will eventually make it to the
                        // right free list.
                    }
                }
                _S_end_free = 0;	// In case of exception.
                    // 若右侧大chunk都没有空闲：
                _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
                // This should either throw an
                // exception or remedy the situation.  Thus we assume it
                // succeeded.
            }
            _S_heap_size += __bytes_to_get;
            _S_end_free = _S_start_free + __bytes_to_get;
            return(_S_chunk_alloc(__size, __nobjs)); // 第一次构造内存池时/无空节点时递归调用一次，准备好freeStart/End
        }
    }


    // Chunk allocation state. 已分配的Chunk的三个状态
    static char* _S_start_free;
    static char* _S_end_free;
    static size_t _S_heap_size;
};

template <typename T>
char* MyAllocator<T>::_S_start_free = 0;

template <typename T>
char* MyAllocator<T>::_S_end_free = 0;

template <typename T>
size_t MyAllocator<T>::_S_heap_size = 0;

template <typename T>
typename MyAllocator<T>::_Obj*
volatile MyAllocator<T>::_S_free_list[_NFREELISTS] = { nullptr }; // 初始化为全空

template <typename T>
std::mutex MyAllocator<T>::mtx;
