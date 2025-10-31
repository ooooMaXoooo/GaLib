# Ce CMakeLists gère le téléchargement de Catch2 et la compilation des tests

# --- 1. Télécharger Catch2 ---
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.6.0 # Utilise une version récente et stable
)

# --- Début du HACK pour Clangd (v4 - Ordre corrigé) ---
#
# Problème : clangd devine les commandes pour nos en-têtes en
#          choisissant les commandes de la cible Catch2, qui ne
#          connaissent pas Vcpkg/Boost.
#
# Solution : On ajoute le chemin de Boost (via Vcpkg) aux
#          répertoires d'inclusion globaux AVANT de créer la
#          cible Catch2, pour qu'elle hérite de ce chemin.
#
get_target_property(BOOST_INCLUDES Boost::type_index INTERFACE_INCLUDE_DIRECTORIES)

if(BOOST_INCLUDES)
    message(STATUS "Clangd HACK: Ajout global des includes Vcpkg/Boost")

    # Ajoute le chemin de Boost à TOUTES les cibles
    # qui seront définies après cette ligne (y compris Catch2).
    include_directories(SYSTEM ${BOOST_INCLUDES})
else()
    message(WARNING "Clangd HACK: Impossible de trouver les includes de Boost::type_index")
endif()
# --- Fin du HACK pour Clangd (v4) ---


# Rend Catch2 disponible (CMake le télécharge dans le dossier de build)
# Cette cible va maintenant hériter du include_directories ci-dessus
FetchContent_MakeAvailable(Catch2)


# --- 2. Créer l'exécutable de test ---
add_executable(run_tests test_genetic.cpp)

# --- 3. Lier les bibliothèques ---
target_link_libraries(run_tests PRIVATE
    # Lie notre bibliothèque GaLib (pour pouvoir la tester)
    genetic

    # Lie Catch2
    Catch2::Catch2WithMain
)

# --- 4. Enregistrer le test avec CTest ---
add_test(NAME run_tests COMMAND run_tests)