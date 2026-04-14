# Jeu de dames 

Ce projet a pour objectif de realiser un jeu de dames sur une carte STM32.

## Etat actuel du projet

La version actuelle permet de jouer a deux joueurs sur une seule carte.

Fonctionnalites deja disponibles :
- affichage du plateau 10x10
- affichage des pions et des dames
- selection tactile des pieces
- deplacements simples
- prises simples et prises multiples
- prise obligatoire
- prise au maximum
- promotion en dame
- detection de fin de partie
- menu de lancement du jeu

## Principe de jeu

Le jeu se joue directement sur l'ecran tactile de la carte :
- un joueur touche une piece pour la selectionner
- puis touche une case valide pour jouer son coup
- les joueurs jouent chacun leur tour sur la meme carte

## Mode deux joueurs

Si on flash le jeu sur deux cartes disctinctes et qu'on utilise 3 cables pour faire une laison UART (TX <-> RX, RX <-> TX, et GND <-> GND) on peut jouer à deux sur deux cartes disctinctes.

## Suite...

Faire un réseau de neurone pour jouer "contre" la carte