# Contribuer à GaLib

Merci de l'intérêt que vous portez à GaLib ! Nous sommes ravis de recevoir des contributions de la communauté.

## 💬 Poser des Questions

Si vous avez des questions sur l'utilisation de la bibliothèque, le meilleur endroit pour les poser est [l'onglet "Issues"](https://github.com/ooooMaXoooo/GaLib/issues) (pensez à utiliser le label "question").

## 🐛 Signaler des Bugs

Si vous pensez avoir trouvé un bug, veuillez [ouvrir une "Issue"](https://github.com/ooooMaXoooo/GaLib/issues/new/choose). Veuillez inclure autant d'informations que possible :
* La version du compilateur que vous utilisez (MSVC, GCC, Clang).
* Un **exemple de code minimal reproductible** qui démontre le problème.
* Ce que vous attendiez qu'il se passe.
* Ce qui s'est réellement passé (logs d'erreur, etc.).

## 💡 Proposer des Améliorations (Pull Requests)

Si vous souhaitez ajouter une nouvelle fonctionnalité ou corriger un bug, voici le processus :

### 1. Configuration de l'Environnement

Le projet utilise C++20, CMake et Vcpkg.

1.  Clonez le dépôt : `git clone https://github.com/ooooMaXoooo/GaLib.git`
2.  Assurez-vous d'avoir [Vcpkg](https://vcpkg.io/en/getting-started.html) installé et que votre `CMAKE_TOOLCHAIN_FILE` pointe vers celui-ci.
3.  Utilisez les `CMakePresets.json` fournis. Configurez le projet (ex: `cmake --preset windows-msvc-debug`).
4.  Compilez le projet (ex: `cmake --build --preset windows-msvc-debug`).

### 2. Standards de Code

* **Formatage** : Ce projet utilise `.clang-format`. Veuillez formater votre code avant de le "commiter". La CI vérifiera cela.
* **Style** : Suivez le style de code C++ moderne existant (smart pointers, `const`-correctness, etc.).

### 3. Lancer les Tests

**Toute contribution doit être accompagnée de tests.**

1.  Ajoutez vos tests unitaires dans le dossier `tests/` (en utilisant le framework Catch2).
2.  Lancez les tests localement en exécutant `ctest` depuis votre dossier de build (ex: `build/msvc-debug/`).
    ```bash
    cd build/msvc-debug
    ctest
    ```
3.  Tous les tests doivent passer avant que votre PR puisse être fusionnée.

### 4. Soumettre la Pull Request

1.  "Forkez" le dépôt.
2.  Créez une nouvelle branche pour votre fonctionnalité (`git checkout -b feature/ma-super-fonction`).
3.  "Commitez" vos changements.
4.  "Poussez" (push) votre branche vers votre fork.
5.  Ouvrez une Pull Request (PR) vers la branche `main` de GaLib.
6.  Assurez-vous que la CI (GitHub Actions) passe avec succès.