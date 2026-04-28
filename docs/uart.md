# Communication UART

## Objectif

Le mode deux cartes permet à deux joueurs de s'affronter chacun sur sa propre carte `STM32`.
Chaque carte affiche donc le plateau, connaît son joueur local et doit rester synchronisée avec l'autre système.

Plutôt que d'envoyer en permanence tout l'état du plateau, le projet transmet uniquement les coups joués.

## Câblage retenu

La liaison entre cartes repose sur trois fils :

- `TX` de la première carte vers `RX` de la seconde
- `RX` de la première carte vers `TX` de la seconde
- `GND` commun entre les deux cartes

Ce choix permet de mettre en place une communication série simple sans matériel supplémentaire.

## Principe de synchronisation

Chaque carte exécute localement la logique complète du jeu.
Lorsqu'un joueur termine un coup sur sa carte :

1. le coup est stocké dans une structure locale
2. cette structure est convertie en texte
3. le texte est envoyé par `UART`
4. l'autre carte reçoit le message
5. elle reconstruit le coup
6. elle applique ce coup sur son propre plateau

Les deux cartes restent ainsi synchronisées à partir des actions de jeu, et non à partir d'un transfert complet du plateau.

## Serialisation des coups

Un coup n'est pas réduit à une seule case de départ et une seule case d'arrivée.
Dans le cas d'une prise multiple, un même coup peut comporter plusieurs étapes.

Le projet décrit donc un coup par :

- un numéro de coup
- un nombre d'étapes
- une suite de positions `(ligne, colonne)`

Le format texte utilise est le suivant :

`COUP;numero;nbEtapes;ligne0,colonne0;ligne1,colonne1;...`

Exemples :

- coup simple : `COUP;12;2;2,3;3,4;`
- coup multi-étapes : `COUP;18;4;5,2;3,4;1,6;0,7;`

Un caractère de fin de ligne est ajouté à la fin du message pour délimiter clairement une trame complète.

## Réception en interruption

La réception `UART` est réalisée octet par octet en interruption.

Le mécanisme général est le suivant :

- la carte arme la réception d'un octet
- lorsqu'un octet arrive, une interruption est déclenchée
- l'octet est ajouté à un buffer
- si le caractère reçu est `\\n` ou `\\r`, le message est considéré comme complet
- le buffer reçu devient alors le dernier message prêt à être traité

Cette approche permet de recevoir des messages de longueur variable tout en gardant un protocole simple.

## Reconstruction du coup

Une fois un message complet reçu, la carte distante le décode :

- vérification du mot-clé `COUP`
- lecture du numéro de coup
- lecture du nombre d'étapes
- lecture de chaque couple `ligne,colonne`
- reconstruction d'une structure `CoupDames`

Des vérifications supplémentaires sont faites pour refuser les messages incohérents :

- nombre d'étapes invalide
- coordonnées hors du plateau
- numéro de coup inattendu

## Application locale du coup reçu

Une fois désérialisé, le coup n'est pas appliqué aveuglément.
La carte reparcourt les étapes reçues et réutilise la logique locale du jeu pour :

- reconnaître un déplacement simple ou une prise
- mettre à jour le plateau
- gérer les captures intermédiaires
- finaliser le tour
- changer le joueur courant
- rafraîchir l'affichage

Ce choix garantit une meilleure cohérence, car les deux cartes s'appuient sur la même logique de jeu.

## Intérêt de cette approche

Le protocole retenu est volontairement simple, mais il présente plusieurs avantages :

- faible quantité de données transmises
- débogage plus facile grâce à un format texte lisible
- gestion naturelle des prises multiples
- vérification locale de la validité du coup
- synchronisation robuste des deux plateaux

Le mode deux cartes repose donc davantage sur un échange d'actions de jeu que sur un partage permanent d'état complet.
