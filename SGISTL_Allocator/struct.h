#pragma once
// �ڴ�ص�������Ϣ������ֽڰ�8�ֽڶ��룬��16������Ԫ�أ�ÿ��Ԫ�ع�һ������
enum { _ALIGN = 8 }; // ����
enum { _MAX_BYTES = 128 }; // ����ֽ�
enum { _NFREELISTS = 16 }; // �������������_MAX_BYTES/_ALIGN

// �� __byte �ϵ������ٽ��� 8 �ı���
// (size_t)ǿתΪ4�ֽ��޷���������ͨ�����ұ�ȡ��������
// 1-8 =�� 8, 9-16 =��16, ...
static size_t
_S_round_up(size_t __bytes)
{
    return (((__bytes)+(size_t)_ALIGN - 1) & ~((size_t)_ALIGN - 1));
}


