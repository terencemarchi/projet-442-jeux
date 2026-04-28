# Fonctionnement du jeu

## Interface générale

L'application se lance directement sur la carte et affiche un menu de navigation.
Ce menu permet d'accéder aux différents modes du projet :

- jeu de dames sur une seule carte
- jeu de dames sur deux cartes via `UART`
- modes liés à l'intelligence artificielle

L'interaction se fait entièrement à l'aide de l'écran tactile.

## Déroulement d'un tour

Le fonctionnement général d'un tour de jeu repose sur les étapes suivantes :

1. le joueur sélectionne une pièce avec l'écran tactile
2. le programme vérifie si cette pièce peut être jouée
3. le joueur choisit une case d'arrivée
4. le programme valide le déplacement ou la prise
5. le plateau est mis à jour
6. le tour passe au joueur suivant

Si une prise multiple est possible, le joueur doit poursuivre son coup jusqu'à ce qu'aucune nouvelle prise valide ne soit disponible.

## Règles implémentées

Les règles prises en charge dans la version actuelle sont :

- déplacements simples des pions
- déplacements des dames
- prises simples
- prises multiples
- prise obligatoire
- prise au maximum
- promotion automatique en dame
- détection de fin de partie

La logique des règles est gérée directement dans le code embarqué, ce qui permet de vérifier la validité de chaque coup avant sa prise en compte.

## Mode une carte

Dans le mode local, les deux joueurs jouent sur le même écran :

- le plateau est unique
- les joueurs jouent à tour de rôle
- toutes les décisions sont prises localement

Ce mode a servi de base fonctionnelle pour la suite du projet.

## Mode deux cartes

Dans le mode `UART`, chaque joueur dispose de sa propre carte :

- une carte joue les blancs
- l'autre joue les noirs
- les deux cartes exécutent localement la logique du jeu
- seuls les coups joués sont transmis d'une carte à l'autre

Ce choix permet de synchroniser les deux plateaux tout en gardant une implémentation simple et légère.

## Modes IA

Le projet contient également des modes expérimentaux autour de l'intelligence artificielle, notamment :

- `IA vs IA`
- `Joueur contre IA`

Ces modes réutilisent la logique du jeu de dames, mais délèguent la sélection de certains coups au modèle embarqué.
