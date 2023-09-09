#ifndef INC_BLUEGEN_H_
#define INC_BLUEGEN_H_
/**
 */
#include <cassert>
#include <cstdint>
#include <vector>

namespace frandom
{
inline static constexpr uint64_t CPPRNG_DEFAULT_SEED64 = 12345ULL;

//--- SplitMix
//---------------------------------------------------------
/**
 * @brief A fast 64 bit PRNG
 *
 * | Feature |      |
 * | :------ | :--- |
 * | Bits    | 64   |
 * | Period  | 2^64 |
 * | Streams | 1    |
 */
class SplitMix
{
public:
    static uint64_t next(uint64_t& state);
};

//--- PCG32
//---------------------------------------------------------
/**
 * @brief A fast 32 bit PRNG
 *
 * | Feature |      |
 * | :------ | :--- |
 * | Bits    | 32   |
 * | Period  | 2^64 |
 * | Streams | 1    |
 */
class PCG32
{
public:
    /**
     * @brief Initialize with CPPRNG_DEFAULT_SEED64
     */
    PCG32();

    /**
     * @brief Initialize with a seed
     * @param [in] seed ... initialize states with
     */
    explicit PCG32(uint64_t seed);
    ~PCG32();

    /**
     * @brief Initialize states with a seed
     * @param [in] seed
     */
    void srand(uint64_t seed);

    /**
     * @brief Generate a 32bit unsigned value
     * @return
     */
    uint32_t rand();

    inline uint32_t operator()()
    {
        return rand();
    }
private:
    inline static constexpr uint64_t Increment = 1442695040888963407ULL;
    inline static constexpr uint64_t Multiplier = 6364136223846793005ULL;
    uint64_t state_;
};

//--- RandomAliasSelect
//---------------------------------------------------------
class RandomAliasSelect
{
public:
    RandomAliasSelect();
    ~RandomAliasSelect();
    uint32_t size() const;
    void build(uint32_t size);
    template<class T>
    uint32_t select(T& random) const;
    uint32_t alias(uint32_t i) const;
    uint32_t weight(uint32_t i) const;
private:
    RandomAliasSelect(const RandomAliasSelect&) = delete;
    RandomAliasSelect& operator=(const RandomAliasSelect&) = delete;
    uint32_t size_;
    uint32_t average_;
    std::vector<uint32_t> weights_;
    std::vector<uint32_t> aliases_;
};

template<class T>
uint32_t RandomAliasSelect::select(T& random) const
{
    uint32_t index = random.rand() & (size_-1);
    uint32_t x = random.rand() & (average_-1);
    return x<weights_[index]? index : aliases_[index];
}

float frandom_table(PCG32& random);
float frandom_downey(PCG32& random);
float frandom_downey_opt(PCG32& random);

} // namespace frandom
#endif // INC_BLUEGEN_H_
