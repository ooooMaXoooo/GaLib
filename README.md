# GaLib (Genetic Algorithm Library)

[![Langage: C++20](https://img.shields.io/badge/Langage-C++20-blue.svg)](https://isocpp.org/)
[![Licence](https://img.shields.io/github/license/ooooMaXoooo/GaLib)](LICENSE)
[![Build Status](https://github.com/ooooMaXoooo/GaLib/actions/workflows/ci.yml/badge.svg)](https://github.com/ooooMaXoooo/GaLib/actions/workflows/ci.yml)
[![GitHub Pages](https://img.shields.io/github/deployments/ooooMaXoooo/GaLib/github-pages?label=docs)](https://ooooMaXoooo.github.io/GaLib/)

**GaLib** est une bibliothèque C++20 *header-only*, moderne et flexible, conçue pour implémenter des algorithmes génétiques. Elle est typée statiquement, hautement configurable et pensée pour la performance et la facilité d'intégration.

## 🚀 Fonctionnalités Clés

* **Header-Only** : Intégrez-la simplement en ajoutant le dossier `include/` à vos chemins.
* **Moderne (C++20)** : Utilise les `concepts` C++20 (ex: `ConfigConcept` ) pour garantir la validité de la configuration à la compilation.
* **Hautement Configurable** :
    * Types personnalisables pour les gènes (ex: `double`, `float` pour les réels ; `uint32_t`, `uint64_t` pour les entiers).
    * Dimensions statiques maximales (`MaxVectors`, `MaxDimension`) pour optimiser l'utilisation de la stack (via `std::array`).
    * Dimensions dynamiques (taille de la population, nombre de gènes) définies à l'exécution.
* **Mécanismes Génétiques Avancés** :
    * **Auto-adaptation** : Activez (`enable_auto_adaptation` ) pour que les taux de mutation évoluent eux-mêmes.
    * **Élitisme** : Préservez le meilleur individu à chaque génération (`enable_elitism` ).
    * **Crossover** : Plusieurs méthodes de croisement au niveau du bit (`SINGLE_POINT_BIT_LEVEL`, `UNIFORM_BIT_LEVEL`).
    * **Sélection** : Sélection par tournoi (`TOURNAMENT`).

## 🛠️ Prérequis

* Un compilateur C++20 (GCC, Clang, MSVC).
* CMake (version 3.21+ ).
* [Vcpkg](https://github.com/microsoft/vcpkg) (recommandé pour gérer les dépendances).

La seule dépendance est `boost-type-index` (utilisée pour l'affichage des types dans le débogage ).

## ⚙️ Compilation

Ce projet est conçu pour être utilisé avec `vcpkg.json` (mode manifeste) et `CMakePresets.json`.

1.  Clonez le dépôt.
2.  Assurez-vous que Vcpkg est configuré pour s'intégrer à CMake (via le `toolchainfile`). Les presets de ce projet le localisent à `C:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake`.
3.  Listez les presets disponibles :
    ```bash
    cmake --list-presets
    ```
4.  Configurez en utilisant un preset (par exemple, MSVC Release) :
    ```bash
    cmake --preset windows-msvc-release
    ```
5.  Compilez :
    ```bash
    cmake --build --preset windows-msvc-release
    ```

## ⚡ Démarrage Rapide (Quick Start)

Voici comment trouver le vecteur 2D `(x, y)` dans l'intervalle `[-10, 10]` qui maximise la fonction $f(x, y) = x + y$.

```cpp
#include <iostream>
#include <vector>
#include "genetic/genetic.hpp"

int main() {
    // 1. Configurer l'algorithme 
    // <double (réels), uint32_t (entiers), 1 vecteur, 2 gènes par vecteur>
    using ConfigType = genetic::Config<double, uint32_t, 1, 2>;
    ConfigType config;

    config.population_size = 100;
    config.max_generations = 200;
    config.number_of_vectors = 1;     // Un seul vecteur (chromosome)
    config.dimension = 2;             // 2D: (x, y)
    config.min_real = -10.0;
    config.max_real = 10.0;
    config.integer_bits = 32;
    config.enable_auto_adaptation = true;
    config.enable_elitism = true;
    config.print_interval = 50;

    // 2. Définir la fonction de fitness (à maximiser) 
    using Real = ConfigType::real_type;
    auto fitness_function = [](const std::vector<std::vector<Real>>& vecs) -> Real {
        double x = vecs[0][0];
        double y = vecs[0][1];
        return x + y; // Maximiser la somme
    };

    try {
        // 3. Lancer l'algorithme 
        genetic::GeneticAlgorithm<ConfigType> ga(config, fitness_function);
        ga.run();

        // 4. Récupérer le résultat 
        auto best_ind = ga.get_best_individual();
        auto best_vecs = best_ind.to_real_vectors();
        
        std::cout << "\n================================\n";
        std::cout << "Meilleure solution trouvee: (" 
                  << best_vecs[0][0] << ", " << best_vecs[0][1] << ")\n";
        std::cout << "Fitness (objectif ~20.0): " << best_ind.get_fitness() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
```

## 📖 Exemple Avancé

Pour un exemple plus complexe de minimisation (maximisation de l'opposé) de la [fonction de Rosenbrock](https://fr.wikipedia.org/wiki/Fonction_de_Rosenbrock), consultez `examples/example_Rosenbrock.cpp`. Ce fichier teste également de multiples combinaisons de paramètres (élitisme, auto-adaptation, méthodes de crossover).

## 📚 Documentation

La documentation complète de l'API (générée via Doxygen) est disponible [ici](https://ooooMaXoooo.github.io/GaLib/).

## 📜 Licence

Ce projet est distribué sous la licence MIT. Voir le fichier `LICENSE` pour plus de détails.
