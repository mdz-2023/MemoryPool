#pragma once
// 内存池的粒度信息，最大字节按8字节对齐，共16个数组元素，每个元素挂一个链表
enum { _ALIGN = 8 }; // 对齐
enum { _MAX_BYTES = 128 }; // 最大字节
enum { _NFREELISTS = 16 }; // 自由链表个数：_MAX_BYTES/_ALIGN

// 将 __byte 上调至最临近的 8 的倍数
// (size_t)强转为4字节无符号整数，通过对右边取反再相与
// 1-8 =》 8, 9-16 =》16, ...
static size_t
_S_round_up(size_t __bytes)
{
    return (((__bytes)+(size_t)_ALIGN - 1) & ~((size_t)_ALIGN - 1));
}


