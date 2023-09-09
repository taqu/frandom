#include "frandom.h"
#include <cmath>
#include <bit>
#include <iostream>

namespace frandom
{
namespace
{
    /**
     * @brief 32 bit right rotation
     * @param [in] x ... input
     * @param [in] r ... count of rotation
     * @return rotated
     */
    inline uint32_t rotr32(uint32_t x, uint32_t r)
    {
        return (x >> r) | (x << ((~r + 1) & 31U));
    }

    /**
     * @brief 64 bit right rotation
     * @param [in] x ... input
     * @param [in] r ... count of rotation
     * @return rotated
     */
    inline uint64_t rotr64(uint64_t value, uint32_t rot)
    {
        return (value >> rot) | (value << ((~rot + 1) & 63U));
    }

    /**
     * @brief Convert to a [0 1) real number
     * @param [in] x
     * @return a [0 1) real number
     */
    inline float to_real32(uint32_t x)
    {
        return static_cast<float>((x >> 8) * (1.0 / 16777216.0));
    }

    /**
     * @brief Convert to a [0 1) real number
     * @param [in] x
     * @return a [0 1) real number
     */
    inline double to_real64(uint64_t x)
    {
        return (x >> 11) * (1.0 / 9007199254740992.0);
    }

    /**
     * @brief Scramble an input
     * @param [in] x
     * @return scrambled
     */
    uint32_t scramble(uint32_t x)
    {
        x += 0x7f4A7C15UL;
        uint32_t t = x;
        t = (t ^ (t >> 15)) * 0x1CE4E5B9UL;
        t = (t ^ (t >> 13)) * 0x133111EBUL;
        return t ^ (t >> 15);
    }

    /**
     * @brief Scramble an input
     * @param [in] x
     * @return scrambled
     */
    uint64_t scramble(uint64_t x)
    {
        x += 0x9E3779B97f4A7C15ULL;
        uint64_t t = x;
        t = (t ^ (t >> 30)) * 0xBF58476D1CE4E5B9ULL;
        t = (t ^ (t >> 27)) * 0x94D049BB133111EBULL;
        return t ^ (t >> 31);
    }
} // namespace

//--- SplitMix
//--------------------------------------------
uint64_t SplitMix::next(uint64_t& state)
{
    state += 0x9E3779B97f4A7C15ULL;
    uint64_t t = state;
    t = (t ^ (t >> 30)) * 0xBF58476D1CE4E5B9ULL;
    t = (t ^ (t >> 27)) * 0x94D049BB133111EBULL;
    return t ^ (t >> 31);
}

//--- PCG32
//------------------------------------------------------------
PCG32::PCG32()
    : state_{CPPRNG_DEFAULT_SEED64}
{
}

PCG32::PCG32(uint64_t seed)
{
    srand(seed);
}

PCG32::~PCG32()
{
}

void PCG32::srand(uint64_t seed)
{
    state_ = SplitMix::next(seed);
    while(0 == state_) {
        state_ = SplitMix::next(state_);
    }
}

uint32_t PCG32::rand()
{
    uint64_t x = state_;
    uint32_t c = static_cast<uint32_t>(x >> 59);
    state_ = x * Multiplier + Increment;
    x ^= x >> 18;
    return rotr32(static_cast<uint32_t>(x >> 27), c);
}

RandomAliasSelect::RandomAliasSelect()
    : size_(0)
    , average_(0)
{
}

RandomAliasSelect::~RandomAliasSelect()
{
}

uint32_t RandomAliasSelect::size() const
{
    return size_;
}

uint32_t RandomAliasSelect::alias(uint32_t i) const
{
    assert(i < size_);
    return aliases_[i];
}

uint32_t RandomAliasSelect::weight(uint32_t i) const
{
    assert(i < size_);
    return weights_[i];
}

void RandomAliasSelect::build(uint32_t size)
{
    assert(0 < size && size <= 32);

    size_ = size;

    std::vector<uint32_t> weights;
    weights.resize(size_);
    weights[0] = 1;
    for(uint32_t i = 1; i < size_; ++i) {
        weights[i] = weights[i - 1] << 1;
    }

    weights_.resize(size_);
    aliases_.resize(size_);

    std::vector<uint32_t> indices;
    indices.resize(size_);
    average_ = 1UL << (size_ - static_cast<uint32_t>(std::log2(size_)));
    int32_t underfull = -1;
    int32_t overfull = static_cast<int32_t>(size_);
    for(uint32_t i = 0; i < size_; ++i) {
        if(average_ <= weights[i]) {
            --overfull;
            indices[overfull] = i;
        } else {
            ++underfull;
            indices[underfull] = i;
        }
    }
    while(0 <= underfull && overfull < static_cast<int32_t>(size_)) {
        uint32_t under = indices[underfull];
        --underfull;
        uint32_t over = indices[overfull];
        ++overfull;
        aliases_[under] = over;
        weights_[under] = weights[under];
        weights[over] += weights[under] - average_;
        if(weights[over] < average_) {
            ++underfull;
            indices[underfull] = over;
        } else {
            --overfull;
            indices[overfull] = over;
        }
    }
    while(0 <= underfull) {
        std::cout << "underfull [" << indices[underfull] << "] " << weights_[indices[underfull]] << std::endl;
        weights_[indices[underfull]] = average_;
        --underfull;
    }
    while(overfull < static_cast<int32_t>(size_)) {
        std::cout << "overfull " << indices[overfull] << "] " << weights_[indices[overfull]] << std::endl;
        weights_[indices[overfull]] = average_;
        ++overfull;
    }
}

namespace
{
#if 0
    uint8_t alias_pow2_32bit[] = {
        28,
        29,
        29,
        29,
        30,
        30,
        30,
        30,
        30,
        30,
        30,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        31,
        0,
        27,
        28,
        29,
        30,
    };
#else
    uint8_t alias_pow2_32bit[] = {
        4,//28,
        3,//29,
        3,//29,
        3,//29,
        2,//30,
        2,//30,
        2,//30,
        2,//30,
        2,//30,
        2,//30,
        2,//30,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        1,//31,
        32,//0,
        5,//27,
        4,//28,
        3,//29,
        2,//30,
    };
#endif
    uint32_t weight_pow2_32bit[] = {
        1,
        2,
        4,
        8,
        16,
        32,
        64,
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768,
        65536,
        131072,
        262144,
        524288,
        1048576,
        2097152,
        4194304,
        8388608,
        16777216,
        33554432,
        67108864,
        134217728,
        134217727,
        134217726,
        134217712,
        134215680,
    };
    inline int32_t frandom_exponent(uint32_t x)
    {
        static constexpr uint32_t average_exp = 27;
        static constexpr uint32_t average_mask = (1UL << 27) - 1;
        uint32_t index = (x >> average_exp);
        uint32_t weight = x & average_mask;
        return weight < weight_pow2_32bit[index] ? 32-index : alias_pow2_32bit[index];
    }
} // namespace

float frandom_table(PCG32& random)
{
    int32_t exponent = 126;
    for(;;){
        uint32_t x;
        for(;;) {
            x = random();
            if(0 != x) /*[[likely]]*/ {
                exponent -= frandom_exponent(x);
                break;
            } else {
                exponent -= 32;
                if(exponent < 0) /*[[unlikely]]*/ {
                    exponent = 0;
                    break;
                }
            }
        }
        x = random();
        uint32_t fraction = x&0x7F'FFFFUL;
        if(0==fraction && (x&0x8000'0000UL)){
            if(126<=exponent)/*[[unlikely]]*/{
                continue;
            }
            ++exponent;
        }
        return std::bit_cast<float,uint32_t>((exponent<<23)|fraction);
    }
}

float frandom_downey(PCG32& random)
{
    constexpr int32_t lowExp = 0;
    constexpr int32_t highExp = 127;
    int32_t exponent = highExp - 1;
    while(true) {
        const uint32_t bits = random();
        if(0 == bits) {
            exponent -= 32;
            if(exponent < lowExp) {
                exponent = lowExp;
                break;
            }
        } else {
            int32_t c = std::countr_zero(bits);
            exponent -= c;
            break;
        }
    }
    const uint32_t u = random();
    const uint32_t mantissa = (u>>8)&0x7FFFFFUL;
    if(0==mantissa && (u>>31)){
        ++exponent;
    }
    return std::bit_cast<float,uint32_t>((exponent<<23)|mantissa);
}

float frandom_downey_opt(PCG32& random)
{
    constexpr int32_t lowExp = 0;
    constexpr int32_t highExp = 127;
    const uint32_t u = random();
    const uint32_t b = u & 0xFFU;
    int32_t exponent = highExp - 1;
    if(0 == b) {
        exponent -= 8;
        while(true) {
            const uint32_t bits = random();
            if(0 == bits) {
                exponent -= 32;
                if(exponent < lowExp) {
                    exponent = lowExp;
                    break;
                }
            } else {
                int32_t c = std::countr_zero(bits);
                exponent -= c;
                break;
            }
        }
    }else{
        int32_t c = std::countr_zero(b);
        exponent -= c;
    }
    const uint32_t mantissa = (u>>8)&0x7FFFFFUL;
    if(0==mantissa && (u>>31)){
        ++exponent;
    }
    return std::bit_cast<float,uint32_t>((exponent<<23)|mantissa);
}
} // namespace frandom

