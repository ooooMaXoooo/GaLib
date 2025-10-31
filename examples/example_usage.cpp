/**
 * @file example_usage.cpp
 * @brief Examples of how to use the genetic algorithm library
 */

#include <cmath>
#include <iostream>

#include "genetic/genetic.hpp"

// ========== Exemple 1: Maximiser la somme des coordonnées au carré ==========

void example1_simple() {
    std::cout << "=== Example 1: Simple optimization ===\n";

    // Configuration
    using ConfigType = genetic::Config<double, uint32_t>;
    ConfigType config;
    config.population_size = 100;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.max_generations = 100;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.number_of_vectors = 2;
    config.dimension = 3;
    config.min_real = -1000.0;                  // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.max_real = 1000.0;                   // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.integer_bits = 32;                   // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.initial_mutation_probability = 0.9;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)

    // Fonction fitness: maximiser -(somme)²
    // (on veut minimiser (somme)², donc maximiser son opposé)
    auto fitness = [](const std::vector<std::vector<double>>& vecs) -> double {
        double sum = 0.0;
        for (const auto& vec : vecs) {
            for (double val : vec) {
                sum += val;
            }
        }
        return -(sum * sum);
    };

    // Créer et exécuter l'algorithme
    genetic::GeneticAlgorithm<ConfigType> ga(config, fitness);
    ga.run(true);

    std::cout << "\n";
}

// ========== Exemple 2: Fonction de Rosenbrock (optimisation classique) ==========

void example2_rosenbrock() {
    std::cout << "=== Example 2: Rosenbrock function | no elitism - auto adaptation ===\n";

    // Configuration pour Rosenbrock (minimum global en (1, 1))
    using ConfigType = genetic::Config<double, uint32_t, 1, 2>;
    ConfigType config;
    config.population_size = 60;    // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.max_generations = 2000;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.number_of_vectors = 1;   // Un seul vecteur
    config.dimension = 2;           // 2D: (x, y)
    config.min_real = -5.0;         // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.max_real = 5.0;          // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.integer_bits = 32;       // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    // config.initial_mutation_probability = 0.5;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)
    config.tournament_size = 2;
    config.enable_elitism = false;
    config.print_interval = 50;  // NOLINT (cppcoreguidlines-avoid-magic-numbers)

    // Fonction de Rosenbrock: f(x,y) = (1-x)² + 100(y-x²)²
    // On veut minimiser, donc maximiser -f
    using Real = ConfigType::real_type;
    auto rosenbrock = [](const std::vector<std::vector<Real>>& vecs) -> Real {
        double x = vecs[0][0];
        double y = vecs[0][1];

        double term1 = (1.0 - x) * (1.0 - x);
        double term2 = 100.0 * (y - x * x) * (y - x * x);

        return -(term1 + term2);  // Négatif pour maximisation
    };

    genetic::GeneticAlgorithm<ConfigType> ga(config, rosenbrock);
    ga.run(true);

    // Le minimum global devrait être proche de (1, 1)
    auto best_vecs = ga.get_best_individual().to_real_vectors();
    std::cout << "Best solution found: (" << best_vecs[0][0] << ", " << best_vecs[0][1] << ")\n";
    std::cout << "Expected: (1.0, 1.0)\n";
    std::cout << "\n";
}

// ========== Exemple 3: Utilisation avec différents types ==========

void example3_different_types() {
    std::cout << "=== Example 3: Using float and uint16_t ===\n";

    // Utiliser float et uint16_t (16 bits)
    using ConfigType = genetic::Config<float, uint16_t, 1, 3>;
    ConfigType config;
    config.population_size = 40;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.max_generations = 50;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.number_of_vectors = 1;
    config.dimension = 3;
    config.min_real = -10.0f;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.max_real = 10.0f;   // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.integer_bits = 16;  // Utiliser seulement 16 bits // NOLINT (cppcoreguidelines-avoid-magic-numbers)

    // Fonction simple: maximiser la somme des valeurs absolues
    auto fitness = [](const std::vector<std::vector<ConfigType::real_type>>& vecs) -> ConfigType::real_type {
        float sum = 0.0f;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        for (const auto& vec : vecs) {
            for (float val : vec) {
                sum += std::abs(val);
            }
        }
        return sum;
    };

    genetic::GeneticAlgorithm<ConfigType> ga(config, fitness);
    ga.run(false);  // Sans verbose

    std::cout << "Best fitness: " << ga.get_best_fitness() << '\n';
    std::cout << "Best individual:\n" << ga.get_best_individual() << '\n';
    std::cout << '\n';
}

// ========== Exemple 4: Callback personnalisé ==========

void example4_with_callback() {
    std::cout << "=== Example 4: Custom callback ===\n";

    using ConfigType = genetic::Config<double, uint32_t, 1, 2>;
    ConfigType config;
    config.population_size = 100;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.max_generations = 50;   // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.number_of_vectors = 1;
    config.dimension = 2;
    config.min_real = -10.0;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.max_real = 10.0;   // NOLINT (cppcoreguidelines-avoid-magic-numbers)

    // Fonction sphère: f(x,y) = x² + y²
    using Real = ConfigType::real_type;
    auto sphere = [](const std::vector<std::vector<Real>>& vecs) -> Real {
        double sum = 0.0;
        for (const auto& vec : vecs) {
            for (double val : vec) {
                sum += val * val;
            }
        }
        return -sum;  // Minimiser
    };

    genetic::GeneticAlgorithm<ConfigType> ga(config, sphere);

    config.print_interval = 10;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    // Callback appelé à chaque génération
    int callback_count = 0;
    auto callback = [&callback_count, &config](size_t gen, double fitness, const genetic::Individu<ConfigType>& best) {
        // Afficher seulement toutes les 10 générations
        if (gen % config.print_interval == 0) {
            std::cout << "  [Callback] Gen " << gen << " - Fitness: " << fitness << '\n';
            callback_count++;
        }
    };

    ga.run(false, callback);  // Pas de verbose, mais callback actif

    std::cout << "Callback called " << callback_count << " times\n";
    std::cout << "\n";
}

// ========== Exemple 5: Seed fixe pour reproductibilité ==========

void example5_reproducible() {
    std::cout << "=== Example 5: Reproducible results with fixed seed ===\n";

    using ConfigType = genetic::Config<double, uint32_t, 1, 2>;
    ConfigType config;
    config.population_size = 60;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    config.max_generations = 30;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
                                  //   config.number_of_vectors = 1;
                                  //   config.dimension = 2;

    auto simple_fitness =
        [](const std::vector<std::vector<double>>& vecs) -> genetic::Config<double, uint32_t>::real_type {
        return vecs[0][0] + vecs[0][1];
    };

    // Première exécution avec seed = 12345
    constexpr size_t seed = 12345;
    genetic::GeneticAlgorithm<ConfigType> ga1(config, simple_fitness, seed);
    ga1.run(false);
    double fitness1 = ga1.get_best_fitness();

    // Deuxième exécution avec le même seed
    genetic::GeneticAlgorithm<ConfigType> ga2(config, simple_fitness, seed);
    ga2.run(false);
    double fitness2 = ga2.get_best_fitness();

    std::cout << "Run 1 best fitness: " << fitness1 << '\n';
    std::cout << "Run 2 best fitness: " << fitness2 << '\n';
    std::cout << "Results identical: " << (fitness1 == fitness2 ? "YES" : "NO") << '\n';
    std::cout << "\n";
}

// ========== Main ==========

int main() {
    genetic::print_info();
    std::cout << "\n";

    try {
        example1_simple();
        example2_rosenbrock();
        example3_different_types();
        example4_with_callback();
        example5_reproducible();

        std::cout << "All examples completed successfully!" << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    std::cout << "Appuyez sur une touche pour continuer ...";
    std::cin.get();
    return 0;
}
