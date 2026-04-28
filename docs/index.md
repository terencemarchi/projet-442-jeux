# Jeu de dames sur STM32

## Présentation

Ce projet s'inscrit dans le cadre d'un mini-projet réalisé sur carte `STM32F746` avec écran tactile.
L'objectif est de développer une application embarquée interactive permettant de jouer au jeu de dames directement sur la carte, puis d'étendre progressivement cette base vers un mode de jeu distribué et une première intelligence artificielle.

Le projet est développé principalement avec `STM32CubeIDE` pour la partie embarquée et avec `Python` pour les travaux liés à l'IA.

## Objectifs du projet

Le travail a été mené en plusieurs étapes :

- réaliser un jeu de dames jouable localement sur une seule carte
- implémenter les principales règles du jeu
- ajouter un mode sur deux cartes avec communication `UART`
- préparer puis intégrer une première IA capable de choisir un coup

## Fonctionnalites principales

La version actuelle prend en charge :

- l'affichage d'un plateau `10 x 10`
- la sélection tactile des pièces
- les déplacements simples
- les prises simples et multiples
- la prise obligatoire
- la prise au maximum
- la promotion en dame
- la détection de fin de partie
- un mode deux joueurs sur une carte
- un mode deux cartes via `UART`
- une première intégration d'IA

## Organisation de la documentation

Cette documentation est volontairement séparée en plusieurs pages :

- [Fonctionnement du jeu](fonctionnement.md) : vue d'ensemble de l'application et des modes disponibles
- [Architecture](architecture.md) : organisation du code embarqué et de la partie Python
- [Communication UART](uart.md) : protocole d'échange entre deux cartes
- [Intelligence artificielle](ia.md) : entraînement, représentation des actions et export vers STM32
- [Limites et perspectives](perspectives.md) : bilan critique et pistes d'amélioration

## Structure du dépôt

| Dossier | Role |
| --- | --- |
| `projetDemo2026/` | projet embarqué STM32 |
| `projetDemo2026/Core/Src/` | fichiers source C |
| `projetDemo2026/Core/Inc/` | fichiers d'en-tête |
| `ia_python/` | environnement Python pour l'IA, entraînement et export |

## Résumé du travail réalisé

Le projet a d'abord abouti à une version stable du jeu jouable localement sur une seule carte.
Cette base a ensuite été étendue avec une communication entre deux cartes afin de permettre une partie distribuée, chaque carte représentant l'état du jeu et transmettant les coups joués.

Enfin, une branche de travail dédiée à l'IA a ajouté un environnement Python, un apprentissage basé sur `MaskablePPO`, un script d'export du modèle, puis une intégration de ce modèle dans le code embarqué.
