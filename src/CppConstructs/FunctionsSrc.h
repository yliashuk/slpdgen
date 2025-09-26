#ifndef FUNCTIONSSRC_H
#define FUNCTIONSSRC_H

#include <string>

const std::string BitMaskFun = R"(
static uint32_t bit_mask(uint8_t numOfBits, uint8_t startPos)
{
    return ((1ul << numOfBits) - 1) << startPos;
}
)";

const std::string MinFun = R"(
static uint32_t min(uint32_t a, uint32_t b, uint32_t c)
{
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}
)";

const std::string BitCpyFun = R"(
static size_t bitcpy(void* dst, size_t dst_off, const void* src, size_t src_off, size_t n)
{
    if(dst_off % 8 == 0 && src_off % 8 == 0 && n % 8 == 0)
    {
        memcpy((char*)dst + dst_off / 8, (char*)src + src_off / 8, n / 8);
        return n;
    }

    for(size_t bit = 0, num_bits = 0; bit < n; bit += num_bits)
    {
        size_t s_off = src_off / 8;
        size_t s_bit_off = src_off % 8;
        uint8_t s_rest = 8 - s_bit_off;

        size_t d_off = dst_off / 8;
        size_t d_bit_off = dst_off % 8;
        uint8_t d_rest = 8 - d_bit_off;

        void* s_pos = (uint8_t*)src + s_off;
        void* d_pos = (uint8_t*)dst + d_off;

        uint8_t s_tr = (n - bit) > s_rest;
        uint8_t d_tr = (n - bit) > d_rest;
        num_bits = min(s_rest + 8 * s_tr, d_rest + 8 * d_tr, n - bit);

        uint16_t val = (s_tr ? *(uint16_t*)s_pos : *(uint8_t*)s_pos) >> s_bit_off;
        if(d_tr)
        {
            *(uint16_t*)d_pos &= ~bit_mask(num_bits, d_bit_off);
            *(uint16_t*)d_pos |= (val & bit_mask(num_bits, 0)) << d_bit_off;
        } else{
            *(uint8_t*)d_pos &= ~bit_mask(num_bits, d_bit_off);
            *(uint8_t*)d_pos |= (val & bit_mask(num_bits, 0)) << d_bit_off;
        }

        src_off += num_bits;
        dst_off += num_bits;
    }
    return n;
}
)";

const std::string BitFieldTemplate = R"(
#ifndef SLPD_BIT_FIELD
#define SLPD_BIT_FIELD
template<typename T, int size>
struct slpd_bit_field
{
    T value : size;
    bool operator==(slpd_bit_field<T, size> obj) const {return value == obj.value;}
};
#endif // SLPD_BIT_FIELD
)";

const std::string BitFieldTemplateC = R"(
#ifndef SLPD_C_BIT_FIELD
#define SLPD_C_BIT_FIELD(name, type, size)  \
typedef struct {                            \
    type value : size;                      \
}name;
#endif // SLPD_C_BIT_FIELD)";

const std::string BitFieldCFmt = R"(
#ifndef %s
#define %s
SLPD_C_BIT_FIELD(%s, %s, %s)
#endif // %s)";

const std::string MemoryManager = R"(
static char* buf_p;
static char* allocate(size_t size)
{
    char* tmp = buf_p;
    buf_p += size;
    return tmp;
}
)";

#endif // FUNCTIONSSRC_H
