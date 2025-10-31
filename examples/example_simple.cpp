#include "genetic/genetic.hpp"

int main(int argc, char** argv) {
    using maConfigType = genetic::Config<double, uint32_t, 1, 1>;

    maConfigType config;

    genetic::GeneticAlgorithm<maConfigType> ga{
        config,
        [](const std::vector<std::vector<double>>& vecs) -> maConfigType::real_type {
            double sum = 0.0;
            for (const auto& vec : vecs) {
                for (double val : vec) {
                    sum += val;
                }
            }
            return (sum * sum);  // Negative because we want to minimize
        },
        0};

    ga.run();

    std::cin.get();
    return 0;
}