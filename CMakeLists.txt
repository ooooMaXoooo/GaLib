cmake_minimum_required(VERSION 3.21)

# Fixe le warning Vcpkg
cmake_policy(SET CMP0167 NEW)

project(GeneticLibrary VERSION 1.0.0)

# Trouver Boost (via Vcpkg)
find_package(Boost REQUIRED COMPONENTS type_index)

# Configuration du C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Génération du fichier compile_commands.json pour Clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- CONFIGURATION DES MODES DE BUILD ET ASAN ---

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Aucun CMAKE_BUILD_TYPE défini, utilisation de 'Debug' par défaut.")
endif()

# Détection du nom de l'exécutable du compilateur (maintenu pour les messages)
get_filename_component(CXX_COMPILER_NAME "${CMAKE_CXX_COMPILER}" NAME)
message(STATUS "Nom du compilateur CXX détecté : ${CXX_COMPILER_NAME}")

# CAS 1: MSVC (Visual Studio) - La configuration la plus stable.
if(MSVC)
    message(STATUS "Configuration pour MSVC (ASan ON) - Utilisation du support natif.")
    
    # MSVC gère nativement le flag /fsanitize=address pour la compilation ET la liaison.
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /fsanitize=address")
    
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG")

    # MSVC gère le runtime /MD (Release DLL) automatiquement avec /fsanitize=address
    # dans les versions récentes, mais la variable reste pour le contrôle:
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")


elseif(WIN32 AND CXX_COMPILER_NAME MATCHES "clang|g\\+\\+")
    # CAS 2: Windows MinGW ou Clang non-MSVC (ASan OFF)
    message(STATUS "Configuration pour MinGW/Clang non-MSVC (Windows) (ASan désactivé car instable)")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g") 
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    
else()
    # CAS 3: Linux/macOS (GCC/Clang) - ASan ON
    message(STATUS "Configuration pour GCC / Clang (Linux/macOS) (ASan activé)")
    
    # GCC/Clang gère nativement le flag pour la compilation ET la liaison.
    set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -fno-omit-frame-pointer") 
    
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    
endif()
# --- FIN DU BLOC DE CONFIGURATION ---



# --- Configuration des Tests Unitaires (Ajout) ---
include(FetchContent)  # 1. Active le module pour télécharger Catch2
enable_testing()       # 2. Active CTest pour ce projet
# --- Fin de l'ajout ---


# Header-only library
add_library(genetic INTERFACE)
target_include_directories(genetic INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(genetic INTERFACE Boost::type_index)

# Examples
add_executable(example_usage examples/example_usage.cpp)
target_link_libraries(example_usage PRIVATE genetic) # Liaison simple et standard

add_executable(example_simple examples/example_simple.cpp)
target_link_libraries(example_simple PRIVATE genetic) # Liaison simple et standard

add_executable(example_rosenbrock examples/example_Rosenbrock.cpp)
target_link_libraries(example_rosenbrock PRIVATE genetic) # Liaison simple et standard


# --- Ajout du sous-répertoire de tests ---
add_subdirectory(tests) # 3. Demande à CMake d'aller lire le CMakeLists.txt dans /tests
# --- Fin de l'ajout ---


# --- Doxygen Documentation (Ajout) ---
# Localiser Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
    message(STATUS "Doxygen trouvé: ${DOXYGEN_EXECUTABLE}")

    # Définir les chemins pour le Doxyfile
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Configurer le fichier Doxyfile en remplaçant les variables @...@
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    # Ajouter la cible "doc"
    # Cette cible exécutera Doxygen dans le répertoire de build
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Génération de la documentation Doxygen..."
        VERBATIM
    )
    message(STATUS "Cible 'doc' ajoutée. Exécutez 'cmake --build . --target doc' pour générer la documentation.")

else()
    message(WARNING "Doxygen n'a pas été trouvé. La cible 'doc' ne sera pas créée.")
endif()
# --- Fin de l'ajout Doxygen ---


# --- Installation (version moderne) ---
include(GNUInstallDirs)

install(DIRECTORY include/genetic
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS genetic EXPORT genetic-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/genetic
)

install(EXPORT genetic-targets
    FILE genetic-config.cmake
    NAMESPACE genetic::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/genetic
)