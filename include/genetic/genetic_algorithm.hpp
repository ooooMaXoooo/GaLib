#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "genetic_config.hpp"
#include "genetic_individu.hpp"
#include "genetic_utils.hpp"

namespace genetic {

/**
 * @brief Main genetic algorithm class
 *
 * @tparam ConfigType the specific type of a config
 */
template <ConfigConcept ConfigType>
class GeneticAlgorithm {
public:
    using Real = typename ConfigType::real_type;
    using Integer = typename ConfigType::integer_type;

    using Individual = Individu<ConfigType>;
    using Population = std::vector<Individual>;

    /**
     * @brief Fitness function type
     * Takes vectors of real values and returns a fitness score (higher is better)
     */
    using FitnessFunction = std::function<Real(const std::vector<std::vector<Real>>&)>;

private:
    ConfigType m_config;
    FitnessFunction m_fitness_func;
    std::mt19937_64 m_rng;
    Population m_population;

    size_t m_current_generation = 0;
    Real m_best_fitness;
    Individual m_best_individual;

    // section pour la fonction selection
    std::uniform_int_distribution<size_t> m_index_dist;
    Population m_selected;

    // section commune aux fonctions suivantes : crossover, bit_level_crossover, bit_level_crossover_probas
    std::uniform_int_distribution<size_t> m_cut_dist;
    std::uniform_int_distribution<size_t> m_cut_dist_proba;

public:
    /**
     * @brief Constructs the genetic algorithm
     *
     * @param config Configuration parameters
     * @param fitness_func Fitness function to maximize
     * @param seed Random seed (0 for random seed)
     */
    GeneticAlgorithm(const ConfigType& config, FitnessFunction fitness_func, uint64_t seed = 0)
        : m_config(config),
          m_fitness_func(fitness_func),
          m_rng(seed == 0 ? std::random_device{}() : seed),
          m_best_fitness(std::numeric_limits<Real>::lowest()),
          m_index_dist(0, config.population_size - 1),
          m_cut_dist(0, (config.dimension * config.integer_bits) - 1),
          m_cut_dist_proba(0, ((config.number_of_vectors + 1) * config.integer_bits) - 1) {
        m_config.validate();
        initialize_population();
        m_selected.resize(m_config.get_half_population_size());
    }

    /**
     * @brief Runs the complete genetic algorithm
     *
     * @param verbose Print progress information
     * @param callback Optional callback called after each generation
     */
    void run(bool verbose = true, std::function<void(size_t, Real, const Individual&)> callback = nullptr) {
        if (verbose) {
            std::cout << "Starting genetic algorithm..." << '\n';
            std::cout << m_config;
            std::cout << '\n';
        }

        for (size_t gen = 0; gen < m_config.max_generations; gen++) {
            step();

            if (verbose && (gen % m_config.print_interval == 0 || gen == m_config.max_generations - 1)) {
                std::cout << "Generation " << gen + 1 << "/" << m_config.max_generations
                          << " - Best fitness: " << m_best_fitness;
                if (m_config.enable_auto_adaptation) {
                    std::cout << "~ Proba Array: [";
                    auto probas = m_best_individual.get_mutation_probas();
                    for (int i = 0; i < probas.size(); i++) {
                        std::cout << genetic::utils::bin_to_proba<Real>(probas[i], m_config.integer_bits);
                        if (i < probas.size()) {
                            std::cout << ", ";
                        }
                    }
                    std::cout << ']';
                }
                std::cout << '\n';
            }

            if (callback) {
                callback(gen, m_best_fitness, m_best_individual);
            }
        }

        if (verbose) {
            std::cout << "\nFinal best fitness: " << m_best_fitness << std::endl;
            std::cout << "Best individual:\n" << m_best_individual << std::endl;
        }
    }

    void reset(const ConfigType& config) {
        config.validate();

        m_current_generation = 0;

        m_config = config;
        m_best_fitness = std::numeric_limits<Real>::lowest();
        m_index_dist = std::uniform_int_distribution<size_t>(0, config.population_size - 1);
        m_cut_dist = std::uniform_int_distribution<size_t>(0, (config.dimension * config.integer_bits) - 1);
        m_cut_dist_proba =
            std::uniform_int_distribution<size_t>(0, ((config.number_of_vectors + 1) * config.integer_bits) - 1);

        m_best_individual = Individual(m_config, m_rng);
        initialize_population();

        m_selected.clear();
        m_selected.resize(m_config.get_half_population_size());
    }

    // ========== Getters ==========

    [[nodiscard]] Real get_best_fitness() const { return m_best_fitness; }
    [[nodiscard]] const Individual& get_best_individual() const { return m_best_individual; }
    [[nodiscard]] size_t get_current_generation() const { return m_current_generation; }
    [[nodiscard]] const Population& get_population() const { return m_population; }
    [[nodiscard]] const Config<Real, Integer>& get_config() const { return m_config; }

private:
    /**
     * @brief Initializes the population with random individuals
     */
    void initialize_population() {
        m_population.clear();
        m_population.reserve(m_config.population_size);

        for (size_t i = 0; i < m_config.population_size; i++) {
            m_population.emplace_back(m_config, m_rng);
        }

        update_best();
    }

    /**
     * @brief Evaluates the fitness of an individual
     */
    Real evaluate(Individual& ind) {
        if (ind.have_been_evaluated()) {
            return ind.get_fitness();
        }

        auto real_vecs = ind.to_real_vectors();
        Real eval = m_fitness_func(real_vecs);
        ind.set_fitness(eval);
        return eval;
    }

    /**
     * @brief Tournament selection
     */
    void selection() {
        for (size_t i = 0; i < m_config.get_half_population_size(); i++) {
            // Sélectionner tournament_size individus
            // TODO : optimiser en évitant de stocker les indices
            std::vector<size_t> tournament_indices;
            tournament_indices.reserve(m_config.tournament_size);
            for (size_t t = 0; t < m_config.tournament_size; t++) {
                tournament_indices.push_back(m_index_dist(m_rng));
            }

            // Trouver le meilleur
            size_t best_idx = tournament_indices[0];
            Real best_eval = evaluate(m_population[best_idx]);

            for (size_t t = 1; t < m_config.tournament_size; t++) {
                size_t idx = tournament_indices[t];
                Real eval = evaluate(m_population[idx]);
                if (eval > best_eval) {
                    best_eval = eval;
                    best_idx = idx;
                }
            }

            m_selected[i] = m_population[best_idx];
        }
    }

    /**
     * @brief Crossover between two parents at bit level
     */
    std::pair<Individual, Individual> crossover(const Individual& p1, const Individual& p2) {
        Individual child1 = p1;
        Individual child2 = p2;

        const size_t num_vecs = m_config.number_of_vectors;

        // Crossover des chromosomes de données
        for (size_t chromo = 0; chromo < num_vecs; chromo++) {
            bit_level_crossover(p1, p2, child1, child2, chromo);
        }

        // Crossover des probabilités de mutation
        if (m_config.enable_auto_adaptation) {
            bit_level_crossover_probas(p1, p2, child1, child2);
        }

        child1.invalidate_fitness();
        child2.invalidate_fitness();

        return {child1, child2};
    }

    /**
     * @brief Bit-level crossover for a chromosome
     */
    void bit_level_crossover(const Individual& p1, const Individual& p2, Individual& child1, Individual& child2,
                             size_t chromo) {
        switch (m_config.crossover_method) {
            case CrossoverType::SINGLE_POINT_BIT_LEVEL:
                bit_level_crossover_single_point_bit_level(p1, p2, child1, child2, chromo);
                break;
            case CrossoverType::UNIFORM_BIT_LEVEL:
                bit_level_crossover_uniform_bit_level(p1, p2, child1, child2, chromo);
                break;
        }
    }

    /**
     * @brief Bit-level crossover for a chromosome using single point bit level
     */
    void bit_level_crossover_single_point_bit_level(const Individual& p1, const Individual& p2, Individual& child1,
                                                    Individual& child2, size_t chromo) {
        size_t cut_point = m_cut_dist(m_rng);

        size_t k = cut_point / m_config.integer_bits;
        size_t k_prime = cut_point % m_config.integer_bits;

        // Copier avant le cut
        for (size_t i = 0; i < k; i++) {
            child1.set_gene(chromo, i, p1.get_gene(chromo, i));
            child2.set_gene(chromo, i, p2.get_gene(chromo, i));
        }

        // Gérer le gène coupé
        if (k < m_config.dimension) {
            Integer g1 = p1.get_gene(chromo, k);
            Integer g2 = p2.get_gene(chromo, k);

            Integer mask1 = (Integer(1) << k_prime) - Integer(1);
            Integer mask2 = ~mask1;

            child1.set_gene(chromo, k, (g1 & mask1) | (g2 & mask2));
            child2.set_gene(chromo, k, (g2 & mask1) | (g1 & mask2));
        }

        // Copier après le cut (de l'autre parent)
        for (size_t i = k + 1; i < m_config.dimension; i++) {
            child1.set_gene(chromo, i, p2.get_gene(chromo, i));
            child2.set_gene(chromo, i, p1.get_gene(chromo, i));
        }
    }

    /**
     * @brief Bit-level crossover for a chromosome using uniform bit level
     */
    void bit_level_crossover_uniform_bit_level(const Individual& p1, const Individual& p2, Individual& child1,
                                               Individual& child2, size_t chromo) {
        // on itère sur chaque gène
        for (size_t k = 0; k < m_config.dimension; k++) {
            Integer g1 = p1.get_gene(chromo, k);
            Integer g2 = p2.get_gene(chromo, k);

            // on itère sur chaque bit
            Integer gene_child_1 = 0;
            Integer gene_child_2 = 0;
            for (size_t i = 0; i < m_config.integer_bits; i++) {
                Integer mask = Integer(1) << i;
                if (ConfigType::proba_distribution(m_rng) <= m_config.uniform_crossover_probability) {
                    // on met le bit du parent 1 pour l'enfant 1
                    gene_child_1 |= g1 & mask;
                    gene_child_2 |= g2 & mask;
                } else {
                    // on met le bit du parent 1 pour l'enfant 2
                    gene_child_1 |= g2 & mask;
                    gene_child_2 |= g1 & mask;
                }
            }

            child1.set_gene(chromo, k, gene_child_1);
            child2.set_gene(chromo, k, gene_child_2);
        }
    }

    /**
     * @brief Bit-level crossover for mutation probabilities
     */
    void bit_level_crossover_probas(const Individual& p1, const Individual& p2, Individual& child1,
                                    Individual& child2) {
        switch (m_config.crossover_method) {
            case CrossoverType::SINGLE_POINT_BIT_LEVEL:
                bit_level_crossover_single_point_bit_level_probas(p1, p2, child1, child2);
                break;
            case CrossoverType::UNIFORM_BIT_LEVEL:
                bit_level_crossover_uniform_bit_level_probas(p1, p2, child1, child2);
                break;
        }
    }

    /**
     * @brief Bit-level crossover for mutation probabilities using single point bit level
     */
    void bit_level_crossover_single_point_bit_level_probas(const Individual& p1, const Individual& p2,
                                                           Individual& child1, Individual& child2) {
        size_t cut_point = m_cut_dist_proba(m_rng);

        size_t k = cut_point / m_config.integer_bits;
        size_t k_prime = cut_point % m_config.integer_bits;

        for (size_t i = 0; i < k; i++) {
            child1.set_mutation_proba(i, p1.get_mutation_proba(i));
            child2.set_mutation_proba(i, p2.get_mutation_proba(i));
        }

        if (k < m_config.number_of_vectors + 1) {
            Integer pr1 = p1.get_mutation_proba(k);
            Integer pr2 = p2.get_mutation_proba(k);

            Integer mask1 = (Integer(1) << k_prime) - Integer(1);
            Integer mask2 = ~mask1;

            child1.set_mutation_proba(k, (pr1 & mask1) | (pr2 & mask2));
            child2.set_mutation_proba(k, (pr2 & mask1) | (pr1 & mask2));
        }

        for (size_t i = k + 1; i < m_config.number_of_vectors + 1; i++) {
            child1.set_mutation_proba(i, p2.get_mutation_proba(i));
            child2.set_mutation_proba(i, p1.get_mutation_proba(i));
        }
    }

    /**
     * @brief Bit-level crossover for mutation probabilities using uniform bit level
     */
    void bit_level_crossover_uniform_bit_level_probas(const Individual& p1, const Individual& p2, Individual& child1,
                                                      Individual& child2) {
        // on itère sur chaque probabilités
        for (size_t k = 0; k < m_config.number_of_vectors + 1; k++) {
            Integer g1 = p1.get_mutation_proba(k);
            Integer g2 = p2.get_mutation_proba(k);

            // on itère sur chaque bit
            Integer gene_child_1 = 0;
            Integer gene_child_2 = 0;
            for (size_t i = 0; i < m_config.integer_bits; i++) {
                Integer mask = Integer(1) << i;
                if (ConfigType::proba_distribution(m_rng) <= m_config.uniform_crossover_probability) {
                    // on met le bit du parent 1 pour l'enfant 1
                    gene_child_1 |= g1 & mask;
                    gene_child_2 |= g2 & mask;
                } else {
                    // on met le bit du parent 1 pour l'enfant 2
                    gene_child_1 |= g2 & mask;
                    gene_child_2 |= g1 & mask;
                }
            }

            child1.set_mutation_proba(k, gene_child_1);
            child2.set_mutation_proba(k, gene_child_2);
        }
    }

    /**
     * @brief Creates offspring from selected parents
     */
    void create_offspring() {
        // Mélanger les parents
        std::shuffle(m_selected.begin(), m_selected.end(), m_rng);

        // Créer des paires et faire crossover
        for (size_t i = 0; i < m_selected.size(); i += 2) {
            // Ajouter les enfants du crossover
            auto [child1, child2] = crossover(m_selected[i], m_selected[i + 1]);
            m_population[i] = child1;
            m_population[i + 1] = child2;
        }

        // on le fait une deuxième fois pour avoir un population complète :
        // Mélanger les parents
        std::shuffle(m_selected.begin(), m_selected.end(), m_rng);

        // Créer des paires et faire crossover
        for (size_t i = 0; i < m_selected.size(); i += 2) {
            // Ajouter les enfants du crossover
            auto [child1, child2] = crossover(m_selected[i], m_selected[i + 1]);
            m_population[m_selected.size() + i] = child1;
            m_population[(m_selected.size() + i + 1)] = child2;
        }
    }

    /**
     * @brief Applies mutation to entire population
     */
    void mutate_population() {
        for (auto& ind : m_population) {
            ind.mutate(m_rng);
        }
    }

    /**
     * @brief Runs one generation
     */
    void step() {
        // Sélection
        selection();

        // sauvegarder le meilleur pour plus tard si il y a élitisme
        if (m_config.enable_elitism) {
            update_best();
        }

        // Crossover
        create_offspring();

        // Mutation
        mutate_population();

        // mettre le meilleur quelque part dans la nouvelle population si il y a élitisme
        if (m_config.enable_elitism) {
            add_best();
        }

        // Mise à jour du meilleur
        update_best();

        m_current_generation++;
    }

    /**
     * @brief Updates the best individual found so far
     * @pre Selection function need to have been executed
     */
    void update_best() {
        for (auto& ind : m_population) {
            // 'evaluate' utilise le cache (have_been_evaluated) ou le recalcule.
            // Cela garantit que la fitness de CHAQUE individu est mise à jour et stockée.
            Real fitness = evaluate(ind);

            // 3. Comparer avec le meilleur trouvé jusqu'à présent
            if (fitness > m_best_fitness) {
                m_best_fitness = fitness;
                m_best_individual = ind;
            }
        }
    }

    void add_best() {
        // on sélectionne aléatoirement un individu, qu'on remplace par le meilleur actuel pré-sauvegardé
        size_t indice = m_index_dist(m_rng);
        m_population[indice] = m_best_individual;
    }
};
}  // namespace genetic