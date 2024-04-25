#include "MyAllocator.h"
#include <iostream>
//template <typename T>
//char* MyAllocator<T>::_S_start_free = 0;
//
//template <typename T>
//char* MyAllocator<T>::_S_end_free = 0;
//
//template <typename T>
//size_t MyAllocator<T>::_S_heap_size = 0;
//
//template <typename T>
//typename MyAllocator<T>::_Obj* 
//volatile MyAllocator<T>::_S_free_list[_NFREELISTS] = { nullptr }; // 初始化为全空
//
//template <typename T>
//std::mutex MyAllocator<T>::mtx;

//template<typename T>
//inline T* MyAllocator<T>::allocate(size_t __n)
//{
//    void* __ret = 0;
//
//    if (__n > (size_t)_MAX_BYTES) { // 大于128字节就不用内存池 通过一级配置器一样
//        __ret = malloc_alloc::allocate(__n);
//    }
//    else { // 小于128字节，使用二级配置器
//        _Obj* volatile* __my_free_list  // 二级指针, 指向要分配的大小合适（n向上取8）的链表
//            = _S_free_list + _S_freelist_index(__n);
//
//        std::lock_guard<std::mutex> guard(mtx);
//        _Obj* __result = *__my_free_list; // result指向链表
//        if (__result == 0) // 链表为空
//            __ret = _S_refill(_S_round_up(__n)); // 分配块大小为 n向上取8 的一个链表
//        else {
//            *__my_free_list = __result->_M_free_list_link; // 数组中的头结点指向链表的第二个节点
//            __ret = __result; // 链表的原第一个节点分配出去
//        }
//        // 出作用域，lock_guard自动解锁
//    }
//
//    return __ret;
//}

//template<typename T>
//void MyAllocator<T>::deallocate(void* __p, size_t __n)
//{
//    if (__n > (size_t)_MAX_BYTES)// 大于128字节，普通方式开辟和回收内存
//        malloc_alloc::deallocate(__p, __n);
//    else {
//        _Obj* volatile* __my_free_list
//            = _S_free_list + _S_freelist_index(__n); // 找到数组中对应的头结点
//        _Obj* __q = (_Obj*)__p;
//
//        std::lock_guard<std::mutex> guard(mtx);
//        __q->_M_free_list_link = *__my_free_list; // 要归还的节点的指针，指向原链表首节点
//        *__my_free_list = __q; // 数组中的头指针指向要归还的节点，完成向链表前部添加回收节点
//        // lock guard is released here
//    };
//}

template<typename T>
T* MyAllocator<T>::allocate(size_t __n)
{
    return nullptr;
}

template<typename T>
void* MyAllocator<T>::reallocate(void* __p, size_t __old_sz, size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;
    std::cout << "_________";

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

//template<typename T>
//void* MyAllocator<T>::_S_refill(size_t __n)
//{
//    int __nobjs = 20;
//    char* __chunk = _S_chunk_alloc(__n, __nobjs); // 负责分配相应大小的chunk块内存池，由_chunk指针接收
//    _Obj* volatile* __my_free_list; // 指向数组下挂的链表
//    _Obj* __result;
//    _Obj* __current_obj;
//    _Obj* __next_obj;
//    int __i;
//
//    if (1 == __nobjs) return(__chunk); // _S_chunk_alloc()引用接受nobjs，如果只生成一个chunk，直接返回
//    __my_free_list = _S_free_list + _S_freelist_index(__n); // 映射数组下标，取得链表头指针
//
//    /* Build free list in chunk */
//    __result = (_Obj*)__chunk; // 记录第一个块，马上分配出去
//    // 字节指针+数字，往后走n个字节。即数组中链表头指针指向的下一个块（空闲）
//    *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
//    for (__i = 1; ; __i++) {
//        __current_obj = __next_obj;
//        __next_obj = (_Obj*)((char*)__next_obj + __n); // next指针偏移整个块的字节n，即指向下一个块  
//        if (__nobjs - 1 == __i) { // 到链表末尾了
//            __current_obj->_M_free_list_link = 0;
//            break;
//        }
//        else {// 将连续的字节块真正形成链表，指针存到每个块节点的指针域里
//            __current_obj->_M_free_list_link = __next_obj;
//        }
//    }
//    return(__result);
//}

//template<typename T>
//char* MyAllocator<T>::_S_chunk_alloc(size_t __size, int& __nobjs)
//{
//    char* __result;
//    size_t __total_bytes = __size * __nobjs;
//    size_t __bytes_left = _S_end_free - _S_start_free; // 剩余字节数，可以分配给不同大小的chunk块
//
//    if (__bytes_left >= __total_bytes) { // 剩余已申请的空间中足够分配20个大小为size的chunk块
//        __result = _S_start_free;
//        _S_start_free += __total_bytes;
//        return(__result);
//    }
//    else if (__bytes_left >= __size) { // 不够的话，计算够分配几个的，更改分配个数
//        __nobjs = (int)(__bytes_left / __size);
//        __total_bytes = __size * __nobjs;
//        __result = _S_start_free;
//        _S_start_free += __total_bytes;
//        return(__result);
//    }
//    else { // 实在不够用，先处理完剩余小内存，再分配新空间
//        size_t __bytes_to_get =
//            2 * __total_bytes + _S_round_up(_S_heap_size >> 4); // _S_heap_size除以16 再向上取8的倍数
//        // 此处剩余量不够本次的一个chunk块，就将剩余的字节作为一个chunk块放给合适他的链表头部，即充分利用每一块小内存
//        if (__bytes_left > 0) {
//            _Obj* volatile* __my_free_list =
//                _S_free_list + _S_freelist_index(__bytes_left);
//
//            ((_Obj*)_S_start_free)->_M_free_list_link = *__my_free_list;
//            *__my_free_list = (_Obj*)_S_start_free;
//        }
//        _S_start_free = (char*)malloc(__bytes_to_get);
//        if (0 == _S_start_free) {
//            size_t __i;
//            _Obj* volatile* __my_free_list;
//            _Obj* __p;
//            // 系统空间不够，则查看右侧更大chunk链表中有无空闲chunk
//            // for循环从当前分配不了的块大小（假设为40）往右遍历数组
//            // _p对右边第一个有空闲块的链表（假设为48）进行遍历
//            // 然后取其头部块，大小为48，作为40的start和end，原48的链表删除此节点
//            // 有start和end即为有空闲字节，可以继续递归调用本函数
//            for (__i = __size;
//                __i <= (size_t)_MAX_BYTES;
//                __i += (size_t)_ALIGN) {
//                __my_free_list = _S_free_list + _S_freelist_index(__i);
//                __p = *__my_free_list;
//                if (0 != __p) {
//                    *__my_free_list = __p->_M_free_list_link;
//                    _S_start_free = (char*)__p;
//                    _S_end_free = _S_start_free + __i;
//                    return(_S_chunk_alloc(__size, __nobjs));
//                    // Any leftover piece will eventually make it to the
//                    // right free list.
//                }
//            }
//            _S_end_free = 0;	// In case of exception.
//                // 若右侧大chunk都没有空闲：
//            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
//            // This should either throw an
//            // exception or remedy the situation.  Thus we assume it
//            // succeeded.
//        }
//        _S_heap_size += __bytes_to_get;
//        _S_end_free = _S_start_free + __bytes_to_get;
//        return(_S_chunk_alloc(__size, __nobjs)); // 第一次构造内存池时/无空节点时递归调用一次，准备好freeStart/End
//    }
//}
