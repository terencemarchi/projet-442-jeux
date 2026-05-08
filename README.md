# Jeu de dames sur STM32

## Contexte

Ce projet s'inscrit dans le cadre d'un mini-projet réalisé sur carte `STM32F746` avec écran tactile.  
L'objectif général est de développer une application embarquée interactive permettant de jouer au jeu de dames directement sur la carte.

Le projet a été développé avec `STM32CubeIDE` et `Visual Studio Code`.  
Une partie du travail a également porté sur l'ajout de fonctionnalités, notamment la communication entre deux cartes et l'intégration d'une intelligence artificielle.

## Objectif

L'objectif du projet est de mettre en œuvre une version embarquée du jeu de dames sur carte `STM32`, avec plusieurs niveaux de fonctionnalités :

- mise en place d'un jeu de dames jouable localement sur une seule carte
- gestion de l'affichage du plateau et des pièces
- interaction tactile avec l'utilisateur
- respect des principales règles du jeu
- ajout d'un mode de jeu sur deux cartes distinctes
- intégration d'une première intelligence artificielle capable de jouer contre l'utilisateur

## Fonctionnement général

L'application se lance directement sur la carte et propose une interface permettant d'accéder aux différents modes disponibles.

Le fonctionnement général repose sur les étapes suivantes :

- affichage d'un menu de navigation
- sélection d'un mode de jeu
- affichage du plateau de dames
- sélection tactile d'une pièce
- affichage et exécution d'un coup valide
- mise à jour du plateau
- changement de joueur
- détection d'une éventuelle fin de partie

L'ensemble de l'interaction utilisateur est réalisé à travers l'écran tactile de la carte.

## Règles implémentées

La version actuelle du projet prend en charge les principaux éléments du jeu de dames.  
Les fonctionnalités déjà implémentées sont les suivantes :

- affichage du plateau `10 x 10`
- affichage des pions et des dames
- sélection tactile des pièces
- déplacements simples
- prises simples
- prises multiples
- prise obligatoire
- prise au maximum
- promotion en dame
- détection de fin de partie

Les règles ont été implémentées directement dans le code embarqué afin de garantir un comportement cohérent du jeu sur la carte.

## Architecture du projet

Le projet est organisé en plusieurs parties complémentaires :

- une partie embarquée sur `STM32`, responsable de l'affichage, de la gestion tactile et de la logique du jeu
- une partie dédiée à l'interface utilisateur et aux menus
- une partie dédiée à la communication entre cartes
- une partie Python utilisée pour les travaux autour de l'intelligence artificielle

L'organisation du dépôt distingue notamment :

- `projetDemo2026/` : code embarqué principal du projet
- `projetDemo2026/Core/Src` : fichiers source en langage C
- `projetDemo2026/Core/Inc` : fichiers d'en-tête
- `ia_python/` : scripts Python pour l'entraînement, les tests et l'export du modèle

Cette séparation permet de distinguer clairement la logique embarquée et les outils utilisés pour préparer l'IA.

## Mode deux joueurs sur une carte

Une première version du projet permet de jouer à deux joueurs sur une seule carte.

Dans ce mode :

- les deux joueurs utilisent le même écran tactile
- les coups sont joués alternativement sur la même interface
- le plateau est mis à jour après chaque action
- les règles du jeu sont vérifiées à chaque tour

Ce mode constitue la base fonctionnelle principale du projet.

## Mode deux cartes

Le projet a ensuite été étendu afin de permettre une partie sur deux cartes distinctes.

Le principe retenu repose sur une communication `UART` entre les deux cartes.  
Chaque carte exécute le programme, affiche le plateau et échange les informations nécessaires pour synchroniser les coups.

La liaison repose sur le câblage suivant :

- `TX` de la première carte vers `RX` de la seconde
- `RX` de la première carte vers `TX` de la seconde
- `GND` commun entre les deux cartes

Dans ce mode :

- chaque joueur dispose de sa propre carte
- les actions effectuées sur une carte sont transmises à l'autre
- le plateau reste synchronisé entre les deux systèmes
- le tour du joueur est pris en compte pour autoriser ou interdire les actions

Ce travail a permis de valider une communication fonctionnelle entre deux cartes pour supporter une partie distribuée.

## Travail autour de l'intelligence artificielle

Une partie importante du projet a consisté à amorcer l'intégration d'une intelligence artificielle pour jouer au jeu de dames.

Cette partie repose sur deux volets :

### Partie Python

Des scripts Python ont été développés pour :

- modéliser l'environnement de jeu
- tester les règles
- entraîner un modèle
- évaluer son comportement
- exporter les paramètres utiles vers la version embarquée

Cette partie sert de base d'expérimentation avant l'intégration sur la carte.

### Partie embarquée

Une fois le travail Python préparé, une intégration sur `STM32` a été commencée.

Le projet contient notamment :

- des fichiers dédiés aux règles de l'IA
- des fichiers liés au réseau ou au modèle embarqué
- une interface permettant d'ajouter un mode de jeu contre la carte

L'objectif est de permettre à la carte de calculer un coup automatiquement à partir de l'état courant du plateau.

## Limites et difficultés

Plusieurs difficultés ont été rencontrées au cours du projet.

Tout d'abord, une piste de travail autour du `Bluetooth` a été explorée, mais elle n'a pas été retenue dans la version finale.  

Par ailleurs, l'intégration d'une intelligence artificielle sur microcontrôleur impose plusieurs contraintes :

- nécessité d'adapter et simplifier le modèle
- complexité de l'intégration entre l'entraînement Python et l'exécution embarquée

Enfin, certaines parties du projet peuvent encore être améliorées, notamment en termes de robustesse, d'ergonomie et de niveau de jeu de l'IA.

## Conclusion

Ce projet a permis de développer une application embarquée complète autour du jeu de dames sur carte `STM32F746`.

Une version jouable à deux sur une même carte a d'abord été mise en place, avec gestion de l'affichage, de l'écran tactile et des principales règles du jeu.  
Le projet a ensuite été étendu avec un mode de jeu sur deux cartes grâce à une communication `UART`.  
Enfin, un premier travail d'intégration d'intelligence artificielle a été ajouté, en combinant une phase de préparation en Python et une adaptation pour la carte embarquée.

L'ensemble constitue une base fonctionnelle solide, tout en ouvrant la voie à plusieurs prolongements techniques.

## Perspectives

Plusieurs pistes d'amélioration peuvent être envisagées pour la suite du projet :

- améliorer l'intelligence artificielle pour obtenir un niveau de jeu plus élevé
- optimiser l'inférence embarquée sur la carte
- enrichir l'interface graphique et les menus
- ajouter de nouveaux modes de jeu

## Références

- `STM32CubeIDE`
- bibliothèques `STM32 HAL`
