// Ce define est crucial !
// Il dit à Catch2 de créer sa propre fonction main() dans ce fichier.
// Ne le mettez que dans UN SEUL fichier .cpp de test.
#include <cstdint>
#define CATCH_CONFIG_MAIN

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

// Incluez les en-têtes de votre bibliothèque que vous voulez tester
#include <genetic/genetic_config.hpp>
#include <genetic/genetic_utils.hpp>

// --- Tests de genetic_utils.hpp ---

TEST_CASE("Conversion Binaire <-> Réel", "[utils]") {
    // Test simple de la fonction bin_to_real
    // (Utilise des valeurs arbitraires, à adapter à votre implémentation)
    using Config = genetic::Config<double, uint32_t>;
    Config cfg;
    cfg.min_real = -10.0;   // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    cfg.max_real = 10.0;    // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    cfg.integer_bits = 16;  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    // (2^16 - 1)
    const uint32_t MAX_INT = 65535;

    SECTION("Test des bornes") {
        auto min_val = genetic::utils::bin_to_real<double, uint32_t>(0, cfg.min_real, cfg.max_real, cfg.integer_bits);
        auto max_val =
            genetic::utils::bin_to_real<double, uint32_t>(MAX_INT, cfg.min_real, cfg.max_real, cfg.integer_bits);

        // Utilise Catch2 pour vérifier les valeurs
        REQUIRE(min_val == -10.0);  // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        REQUIRE(max_val == 10.0);   // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    }

    SECTION("Test du milieu") {
        // (MAX_INT / 2)
        auto bin_to_real = [&cfg](uint32_t bin_value) -> double {
            return genetic::utils::bin_to_real<double, uint32_t>(bin_value, cfg.min_real, cfg.max_real,
                                                                 cfg.integer_bits);
        };

        auto mid_val = bin_to_real(cfg.get_integer_max() / 2);

        // On utilise Approx() pour gérer les imprécisions de notre représentation en binaire
        const double error_bit = 2 * (bin_to_real(1) - bin_to_real(0));
        // NOLINTNEXTLINE (cppcoreguidelines-avoid-magic-numbers)
        REQUIRE(mid_val == Catch::Approx((cfg.max_real + cfg.min_real) / 2).margin(error_bit));
    }
}

// --- Tests de genetic_config.hpp ---

// NOLINTNEXTLINE (readability-function-cognitive-complexity)
TEST_CASE("Validation de la Config", "[config]") {
    using Config = genetic::Config<double, uint32_t>;
    Config cfg;

    SECTION("Validation de base (doit passer)") {
        REQUIRE_NOTHROW(cfg.validate());  // NOLINT (cppcoreguidelines-avoid-do-while)
    }

    SECTION("Plage de réels invalide (doit échouer)") {
        cfg.min_real = 10.0;                                       // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        cfg.max_real = -10.0;                                      // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        REQUIRE_THROWS_AS(cfg.validate(), std::invalid_argument);  // NOLINT (cppcoreguidelines-avoid-do-while)
    }

    SECTION("Taille de population impaire (doit échouer)") {
        cfg.population_size = 101;                                 // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        REQUIRE_THROWS_AS(cfg.validate(), std::invalid_argument);  // NOLINT (cppcoreguidelines-avoid-do-while)
    }
}