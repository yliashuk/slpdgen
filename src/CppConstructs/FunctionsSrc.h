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
static uint8_t min(uint8_t a, uint8_t b){return a < b ? a : b;}
)";

const std::string BitCpyFun = R"(
static size_t bitcpy(void* dst, size_t dst_off, const void * src, size_t src_off, size_t n)
{
    if(dst_off % 8 == 0 && src_off % 8 == 0)
    {
        size_t num_bytes = (n + 7) / 8;
        memcpy((char*)dst + dst_off / 8, (char*)src + src_off / 8, num_bytes);
        return dst_off + n;
    }

    for(size_t bit = 0, num_bits = 0; bit < n; bit += num_bits)
    {
        uint32_t src_byte_pos = src_off / 8;
        uint8_t src_bit_pos = src_off % 8;

        uint32_t dst_byte_pos = dst_off / 8;
        uint8_t dst_bit_pos = dst_off % 8;

        num_bits = min(16 - dst_bit_pos, n - bit);

        uint32_t* src_p = (uint32_t*)((uint8_t*)src + src_byte_pos);
        uint16_t* dst_p = (uint16_t*)((uint8_t*)dst + dst_byte_pos);

        *dst_p &= ~bit_mask(num_bits, dst_bit_pos);

        uint32_t val = *src_p >> src_bit_pos;

        *dst_p |= (val & bit_mask(num_bits, 0)) << dst_bit_pos;

        src_off += num_bits;
        dst_off += num_bits;
    }

    return dst_off;
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
