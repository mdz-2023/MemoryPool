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
//volatile MyAllocator<T>::_S_free_list[_NFREELISTS] = { nullptr }; // ��ʼ��Ϊȫ��
//
//template <typename T>
//std::mutex MyAllocator<T>::mtx;

//template<typename T>
//inline T* MyAllocator<T>::allocate(size_t __n)
//{
//    void* __ret = 0;
//
//    if (__n > (size_t)_MAX_BYTES) { // ����128�ֽھͲ����ڴ�� ͨ��һ��������һ��
//        __ret = malloc_alloc::allocate(__n);
//    }
//    else { // С��128�ֽڣ�ʹ�ö���������
//        _Obj* volatile* __my_free_list  // ����ָ��, ָ��Ҫ����Ĵ�С���ʣ�n����ȡ8��������
//            = _S_free_list + _S_freelist_index(__n);
//
//        std::lock_guard<std::mutex> guard(mtx);
//        _Obj* __result = *__my_free_list; // resultָ������
//        if (__result == 0) // ����Ϊ��
//            __ret = _S_refill(_S_round_up(__n)); // ������СΪ n����ȡ8 ��һ������
//        else {
//            *__my_free_list = __result->_M_free_list_link; // �����е�ͷ���ָ������ĵڶ����ڵ�
//            __ret = __result; // �����ԭ��һ���ڵ�����ȥ
//        }
//        // ��������lock_guard�Զ�����
//    }
//
//    return __ret;
//}

//template<typename T>
//void MyAllocator<T>::deallocate(void* __p, size_t __n)
//{
//    if (__n > (size_t)_MAX_BYTES)// ����128�ֽڣ���ͨ��ʽ���ٺͻ����ڴ�
//        malloc_alloc::deallocate(__p, __n);
//    else {
//        _Obj* volatile* __my_free_list
//            = _S_free_list + _S_freelist_index(__n); // �ҵ������ж�Ӧ��ͷ���
//        _Obj* __q = (_Obj*)__p;
//
//        std::lock_guard<std::mutex> guard(mtx);
//        __q->_M_free_list_link = *__my_free_list; // Ҫ�黹�Ľڵ��ָ�룬ָ��ԭ�����׽ڵ�
//        *__my_free_list = __q; // �����е�ͷָ��ָ��Ҫ�黹�Ľڵ㣬���������ǰ����ӻ��սڵ�
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

    if (__old_sz > (size_t)_MAX_BYTES && __new_sz > (size_t)_MAX_BYTES) { // û���ڴ��
        return(realloc(__p, __new_sz));
    }
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p); // ����ͬ����С���������·���
    __result = allocate(__new_sz); // ���ڴ���л�ȡ���¿ռ�
    __copy_sz = __new_sz > __old_sz ? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz); // �Ӿ��ڴ濽�����ݵ����ڴ棬���ݳ��������ڴ�Ľ�С����
    deallocate(__p, __old_sz); // �ͷž��ڴ�
    return(__result);
}

//template<typename T>
//void* MyAllocator<T>::_S_refill(size_t __n)
//{
//    int __nobjs = 20;
//    char* __chunk = _S_chunk_alloc(__n, __nobjs); // ���������Ӧ��С��chunk���ڴ�أ���_chunkָ�����
//    _Obj* volatile* __my_free_list; // ָ�������¹ҵ�����
//    _Obj* __result;
//    _Obj* __current_obj;
//    _Obj* __next_obj;
//    int __i;
//
//    if (1 == __nobjs) return(__chunk); // _S_chunk_alloc()���ý���nobjs�����ֻ����һ��chunk��ֱ�ӷ���
//    __my_free_list = _S_free_list + _S_freelist_index(__n); // ӳ�������±꣬ȡ������ͷָ��
//
//    /* Build free list in chunk */
//    __result = (_Obj*)__chunk; // ��¼��һ���飬���Ϸ����ȥ
//    // �ֽ�ָ��+���֣�������n���ֽڡ�������������ͷָ��ָ�����һ���飨���У�
//    *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
//    for (__i = 1; ; __i++) {
//        __current_obj = __next_obj;
//        __next_obj = (_Obj*)((char*)__next_obj + __n); // nextָ��ƫ����������ֽ�n����ָ����һ����  
//        if (__nobjs - 1 == __i) { // ������ĩβ��
//            __current_obj->_M_free_list_link = 0;
//            break;
//        }
//        else {// ���������ֽڿ������γ�����ָ��浽ÿ����ڵ��ָ������
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
//    size_t __bytes_left = _S_end_free - _S_start_free; // ʣ���ֽ��������Է������ͬ��С��chunk��
//
//    if (__bytes_left >= __total_bytes) { // ʣ��������Ŀռ����㹻����20����СΪsize��chunk��
//        __result = _S_start_free;
//        _S_start_free += __total_bytes;
//        return(__result);
//    }
//    else if (__bytes_left >= __size) { // �����Ļ������㹻���伸���ģ����ķ������
//        __nobjs = (int)(__bytes_left / __size);
//        __total_bytes = __size * __nobjs;
//        __result = _S_start_free;
//        _S_start_free += __total_bytes;
//        return(__result);
//    }
//    else { // ʵ�ڲ����ã��ȴ�����ʣ��С�ڴ棬�ٷ����¿ռ�
//        size_t __bytes_to_get =
//            2 * __total_bytes + _S_round_up(_S_heap_size >> 4); // _S_heap_size����16 ������ȡ8�ı���
//        // �˴�ʣ�����������ε�һ��chunk�飬�ͽ�ʣ����ֽ���Ϊһ��chunk��Ÿ�������������ͷ�������������ÿһ��С�ڴ�
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
//            // ϵͳ�ռ䲻������鿴�Ҳ����chunk���������޿���chunk
//            // forѭ���ӵ�ǰ���䲻�˵Ŀ��С������Ϊ40�����ұ�������
//            // _p���ұߵ�һ���п��п����������Ϊ48�����б���
//            // Ȼ��ȡ��ͷ���飬��СΪ48����Ϊ40��start��end��ԭ48������ɾ���˽ڵ�
//            // ��start��end��Ϊ�п����ֽڣ����Լ����ݹ���ñ�����
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
//                // ���Ҳ��chunk��û�п��У�
//            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
//            // This should either throw an
//            // exception or remedy the situation.  Thus we assume it
//            // succeeded.
//        }
//        _S_heap_size += __bytes_to_get;
//        _S_end_free = _S_start_free + __bytes_to_get;
//        return(_S_chunk_alloc(__size, __nobjs)); // ��һ�ι����ڴ��ʱ/�޿սڵ�ʱ�ݹ����һ�Σ�׼����freeStart/End
//    }
//}
