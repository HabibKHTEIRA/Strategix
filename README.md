# Stratégix - Suite de Jeux de Stratégie

Stratégix est une application de bureau multiplateforme regroupant **7 jeux de réflexion et de stratégie combinatoire**, développée en **C++17** et **Qt (compatible Qt 6 et Qt 5)** avec une architecture MVC modulaire et un design minimaliste épuré.

![Menu Principal - Stratégix](im/capture.png)

---

## Configuration et Lancement

### Prérequis

* **Compilateur C++** : supportant la norme **C++17** (GCC >= 9, Clang >= 10 ou MSVC >= 2019)
* **CMake** : version 3.16 ou supérieure
* **Qt** : **Qt 6** ou **Qt 5** (modules `Core` et `Widgets`)

### 1. Compilation et Démarrage rapide

```bash
# 1. Génération des fichiers de configuration
cmake -B build

# 2. Compilation du projet
cmake --build build -j$(nproc)

# 3. Lancement de l'application
./build/strategix
```

### 2. Exécution des Tests Automatisés

L'ensemble des moteurs logiques est couvert par une suite de tests unitaires automatisés avec **CTest** :

```bash
ctest --test-dir build --output-on-failure
```

---

## 🎮 Fonctionnalités de l'Application

### 1. Les 7 Jeux Inclus

1. **Jeu de Dames (Checkers)** : Damier 8x8 avec déplacements diagonaux, prises simples, rafles enchaînées et couronnement en Dame.
2. **Jeu de Go (Weiqi / Baduk)** : Goban 9x9 avec calcul de libertés, captures de chaînes, règle du *Kō* et décompte territorial avec Komi.
3. **Puissance 4 (Connect Four)** : Grille verticale 7x6 avec simulation de gravité et détection d'alignement de 4 jetons.
4. **Reversi (Othello)** : Plateau 8x8 avec encadrement et retournement multidirectionnel des pions adverses.
5. **Quoridor** : Plateau 9x9 avec 10 barrières par joueur, déplacements, sauts et validation de chemin continu vers la ligne d'arrivée.
6. **Jeu du Moulin (Nine Men's Morris)** : 3 carrés concentriques (24 intersections) avec les 3 phases de jeu (Pose, Déplacement, Vol libre à 3 pions) et captures par alignement de 3.
7. **La Pipopipette (Dots and Boxes)** : Grille de 5x5 points formant 16 cases, tracé d'arêtes et fermeture stratégique des carrés.

### 2. Modes de Jeu

* **2 Joueurs (Local)** 
* **1 Joueur vs Ordinateur (Computer)**

### 3. Annulation de Coup (Undo)

Historique complet des états de jeu permettant d'annuler un ou plusieurs coups consécutifs sur l'ensemble des jeux.

### 4. Support Multilingue

* **3 Langues intégrées** :
  * 🇫🇷 **Français**
  * 🇬🇧 **English**
  * 🇸🇦 **العربية**

