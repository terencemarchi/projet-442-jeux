# Intelligence artificielle

## Objectif

L'un des prolongements du projet consiste à faire jouer la carte automatiquement à partir de l'état courant du plateau.
Pour cela, une partie Python a été développée afin d'entraîner une politique de jeu, puis d'exporter le modèle vers le code embarqué.

## Environnement d'apprentissage

Le fichier `ia_python/env.py` définit un environnement `Gymnasium` adapté au jeu de dames.

Cet environnement :

- représente l'état du jeu sous forme numérique
- génère les actions valides
- applique un coup choisi par l'agent
- calcule une recompense
- gère un adversaire simple pour faire progresser l'apprentissage

L'espace d'observation utilise un vecteur de taille `51`.
L'espace d'action est discret et encode des couples départ-arrivée sur les cases jouables du plateau.

## Encodage des actions

Le plateau de dames `10 x 10` contient `50` cases jouables.
Le projet encode donc une action comme une paire :

- index de départ parmi les `50` cases jouables
- index d'arrivée parmi les `50` cases jouables

Le nombre total d'actions possibles est donc :

`50 x 50 = 2500`

Cette représentation reste simple à manipuler dans un algorithme de renforcement.

## Apprentissage du modèle

Le fichier `ia_python/train.py` entraîne une politique avec `MaskablePPO`.

L'usage de `MaskablePPO` est important, car il permet de masquer les actions invalides pendant l'apprentissage.
Le modèle n'explore alors que des coups légalement jouables, ce qui rend l'entraînement plus pertinent sur un jeu à fortes contraintes de règles.

Dans la version actuelle :

- l'apprentissage se fait sur `100000` itérations
- l'agent peut alterner automatiquement entre les blancs et les noirs
- le modèle sauvegardé est placé dans `ia_python/modeles_sb3/`

## Export vers STM32

Une fois le modèle entraîné, le script `ia_python/export_sb3_stm32.py` extrait les poids de la politique et génère deux fichiers :

- `projetDemo2026/Core/Inc/modele_policy_sb3_stm.h`
- `projetDemo2026/Core/Src/modele_policy_sb3_stm.c`

Les poids sont exportés sous forme de tableaux `float` directement exploitables en `C`.

Ce choix permet d'intégrer le modèle sans dépendance externe de type framework de deep learning embarqué.

## Intégration embarquée

Côté STM32, plusieurs fichiers prennent le relais :

- `dames_ia.c`
- `regles_ia_stm.c`
- `reseau_ia_stm.c`
- `modele_policy_sb3_stm.c`

L'objectif est de :

- convertir l'état du plateau dans une représentation compatible avec le modèle
- évaluer les actions candidates
- sélectionner un coup
- appliquer ce coup dans le moteur de jeu embarqué

## Limites actuelles

Cette intégration reste une première étape.
Elle se heurte à plusieurs contraintes :

- ressources mémoire limitées sur microcontrôleur
- coût de calcul d'une inférence embarquée
- écart entre l'environnement Python et l'exécution temps réel sur la carte
- qualité encore perfectible du niveau de jeu obtenu

Malgré cela, cette architecture constitue une base intéressante pour faire dialoguer apprentissage en Python et exécution embarquée sur `STM32`.
