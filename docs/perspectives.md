# Limites et perspectives

## Difficultés rencontrées

Le projet a mis en évidence plusieurs contraintes techniques.

### Communication sans fil

Une piste basée sur le `Bluetooth` a été envisagée, mais elle n'a pas été retenue dans la version finale.
Dans le cadre du matériel disponible, cette solution n'était pas suffisamment exploitable sans ajout de module externe.

### Contraintes embarquées

Le développement sur microcontrôleur impose plusieurs limites :

- mémoire disponible réduite
- puissance de calcul plus faible qu'un environnement PC
- contraintes de temps de réponse pour conserver une interface fluide
- complexité de mise au point lorsqu'il faut combiner interface, logique du jeu et communication

### IA encore perfectible

La partie intelligence artificielle reste expérimentale :

- le modèle doit encore progresser en qualité de jeu
- l'intégration embarquée peut encore être optimisée
- l'évaluation du modèle sur des parties plus longues reste à approfondir

## Bilan

Malgré ces limites, le projet a permis de mettre en place une base fonctionnelle solide :

- une version locale jouable sur une seule carte
- un mode distribué sur deux cartes grâce à `UART`
- une première chaîne complète entre entraînement Python et intégration sur STM32

## Pistes d'amélioration

Plusieurs prolongements peuvent être envisagés :

- améliorer le niveau de jeu de l'IA
- optimiser le calcul embarqué du modèle
- enrichir l'interface graphique
- renforcer la robustesse du mode deux cartes
- ajouter des outils de débogage ou de visualisation
- étudier à nouveau des solutions de communication sans fil adaptées

## Conclusion

Ce projet montre qu'il est possible de combiner interface tactile, logique de jeu, communication entre cartes et début d'intelligence artificielle sur une plateforme embarquée `STM32F746`.

La suite du travail peut maintenant s'orienter vers deux axes principaux :

- fiabiliser et documenter davantage la base existante
- approfondir les mécanismes les plus ambitieux, en particulier l'IA et les communications avancées
