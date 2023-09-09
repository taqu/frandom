#include "frandom.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

class Stopwatch
{
public:
    void start()
    {
        start_ = std::chrono::high_resolution_clock::now();
    }
    void stop()
    {
        end_ = std::chrono::high_resolution_clock::now();
    }

    std::chrono::high_resolution_clock::duration::rep milliseconds() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
};

template<class T>
void measure(const char* name, T func)
{
    static constexpr uint32_t Samples = 100'000'000;
    std::vector<float> floats;
    floats.resize(Samples);
    frandom::PCG32 random(12345);
    Stopwatch stopwatch;
    stopwatch.start();
    for(uint32_t i=0; i<Samples; ++i){
        floats[i] = func(random);
    }
    stopwatch.stop();
    std::cout << name << ": " << stopwatch.milliseconds() << std::endl;
}

int main(void)
{
    using namespace frandom;

    RandomAliasSelect aliasSelect;
    aliasSelect.build(32);

    for(uint32_t i=0; i<aliasSelect.size(); ++i){
        std::cout << "| " << i << " | " << aliasSelect.alias(i) << " | " << aliasSelect.weight(i) << " |" << std::endl;
        //std::cout << "| " << i << " | " << (32-aliasSelect.alias(i)) << " | " << aliasSelect.weight(i) << " |" << std::endl;
    }

#if 1
    measure("frandom_table", frandom::frandom_table);
    measure("frandom_downey", frandom::frandom_downey);
    measure("frandom_downey_opt", frandom::frandom_downey_opt);
#endif
#if 0
    std::random_device device;
    for(uint32_t j = 0; j < 100; ++j) {
        PCG32 random(device());
        std::cout << "[" << j << "]" << std::endl;
        for(uint32_t i = 0; i < 100'000'000; ++i) {
            float f = frandom_table(random);
            if(f<0.0f || 1.0f<=f){
                std::cout << "error:" << f << std::endl;
                return 0;
            }
        }
    }
#endif
    #if 0
    PCG32 random(12345);
    std::vector<uint64_t> histgram(aliasSelect.size(), 0);
    uint64_t samples = 1ULL<<24;
    for(uint64_t i=0; i<samples; ++i){
        uint32_t index = random_exponent(random.rand());
        histgram[index] += 1;
    }

    std::ofstream file("exp_result.csv", std::ios::binary);
    for(uint32_t i=0; i<aliasSelect.size(); ++i){
        std::cout << '[' << i << "] " << histgram[i] << std::endl;
        file << histgram[i] << std::endl;
    }
    #endif

#if 0 //Write tables
    std::ofstream aliases("aliases.txt", std::ios::binary);
    std::ofstream weights("weights.txt", std::ios::binary);
    for(uint32_t i=0; i<aliasSelect.size(); ++i){
        aliases << aliasSelect.alias(i) << ",\n";
        weights << aliasSelect.weight(i) << ",\n";
    }
    aliases.close();
    weights.close();
#endif

#if 0 //Pow2 distribution test
    PCG32 random(12345);
    std::vector<uint64_t> histgram(aliasSelect.size(), 0);
    uint64_t samples = 1ULL<<24;//1ULL<<40;
    for(uint64_t i=0; i<samples; ++i){
        uint32_t index = aliasSelect.select(random);
        histgram[index] += 1;
    }

    for(uint32_t i=0; i<aliasSelect.size(); ++i){
        std::cout << '[' << i << "] " << histgram[i] << std::endl;
    }

    std::ofstream file("result.csv", std::ios::binary);
    for(uint32_t i=0; i<aliasSelect.size(); ++i){
        std::cout << '[' << i << "] " << histgram[i] << std::endl;
        file << histgram[i] << std::endl;
    }
#endif
    return 0;
}
