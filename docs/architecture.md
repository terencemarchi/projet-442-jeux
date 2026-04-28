# Architecture

## Vue d'ensemble

Le projet est organisé autour de deux blocs principaux :

- une partie embarquée sur `STM32`
- une partie Python pour l'IA

Cette séparation permet de conserver une logique de jeu commune, tout en utilisant des outils différents selon les besoins :

- `C` pour l'exécution embarquée sur la carte
- `Python` pour les tests, l'entraînement et l'export du modèle

## Partie embarquée STM32

Le dossier principal du projet embarqué est `projetDemo2026/`.

Les fichiers importants sont répartis de la façon suivante :

| Fichier ou dossier | Role |
| --- | --- |
| `Core/Src/main.c` | boucle principale de l'application, navigation entre les écrans |
| `Core/Src/menu.c` | affichage des menus et gestion des choix utilisateur |
| `Core/Src/dames.c` | logique du jeu de dames, coups, captures, tour de jeu |
| `Core/Src/test_uart.c` | envoi et réception des messages `UART` |
| `Core/Src/dames_ia.c` | intégration des modes de jeu avec IA |
| `Core/Src/reseau_ia_stm.c` | traitement lié au réseau côté embarqué |
| `Core/Src/regles_ia_stm.c` | règles et génération de coups pour l'IA |
| `Core/Src/modele_policy_sb3_stm.c` | poids du modèle exporté vers STM32 |

## Logique applicative

L'application suit une organisation simple :

1. `main.c` gère l'écran courant et la boucle principale
2. `menu.c` gère les menus et le choix des modes
3. `dames.c` gère l'état du plateau et l'application des règles
4. `test_uart.c` assure l'échange de coups entre deux cartes
5. `dames_ia.c` et les fichiers associés gèrent les coups automatiques de l'IA

Cette décomposition rend le code plus lisible et évite de concentrer toute la logique dans un seul fichier.

## Partie Python

Le dossier `ia_python/` contient les scripts de travail autour de l'IA.

| Fichier | Role |
| --- | --- |
| `env.py` | environnement `Gymnasium` utilisé pour l'apprentissage |
| `train.py` | entraînement du modèle `MaskablePPO` |
| `rules.py` | représentation du jeu et génération des coups pour l'IA |
| `test.py` | tests et vérifications autour de l'environnement |
| `export_sb3_stm32.py` | export des poids du modèle vers des fichiers C |

## Flux global

Le projet peut être vu comme une chaîne en quatre parties :

1. interaction utilisateur sur l'écran tactile
2. application des règles de dames côté STM32
3. éventuelle transmission du coup vers une autre carte en mode `UART`
4. éventuelle sélection automatique d'un coup par l'IA en mode dédié

L'objectif de cette architecture est de garder une base de jeu solide, puis d'ajouter les fonctionnalités avancées autour de cette base sans casser le fonctionnement principal.
