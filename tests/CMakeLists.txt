# Ce CMakeLists gère le téléchargement de Catch2 et la compilation des tests

# --- 1. Télécharger Catch2 ---
# Déclare le dépôt GitHub de Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.6.0 # Utilise une version récente et stable
)

# Rend Catch2 disponible (CMake le télécharge dans le dossier de build)
FetchContent_MakeAvailable(Catch2)

# --- 2. Créer l'exécutable de test ---
# Il sera nommé "run_tests" et compilé à partir de test_genetic.cpp
add_executable(run_tests test_genetic.cpp)

# --- 3. Lier les bibliothèques ---
target_link_libraries(run_tests PRIVATE
    # Lie notre bibliothèque GaLib (pour pouvoir la tester)
    genetic
    
    # Lie Catch2
    Catch2::Catch2WithMain
)

# --- 4. Enregistrer le test avec CTest ---
# Dit à CTest que l'exécutable "run_tests" est un test
# Le premier "run_tests" est le nom du test pour CTest
# Le second "run_tests" est le nom de la cible CMake (l'exécutable)
add_test(NAME run_tests COMMAND run_tests)