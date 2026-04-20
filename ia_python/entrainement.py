"""Boucle d'entrainement minimale pour l'IA.

Version V1 :
- une seule IA pour les blancs et les noirs
- auto-jeu avec choix de coups guides par le reseau
- cible = recompense immediate
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

from regles_ia import EtatJeuIA
from regles_ia import appliquer_coup
from regles_ia import calculer_recompense
from regles_ia import convertir_etat_en_entrees
from regles_ia import generer_coups_possibles
from regles_ia import initialiser_partie
from reseau_ia import ReseauIA


@dataclass
class StatistiquesEntrainement:
    nb_parties: int
    nb_coups_total: int
    recompense_moyenne: float
    perte_moyenne: float


def choisir_coup_aleatoire(
    etat: EtatJeuIA, generateur: np.random.Generator
):
    """Choisit un coup valide au hasard."""
    coups = generer_coups_possibles(etat)
    if not coups:
        return None

    index = int(generateur.integers(0, len(coups)))
    return coups[index]


def choisir_meilleur_coup(
    reseau: ReseauIA,
    etat: EtatJeuIA,
):
    """Choisit le meilleur coup selon l'evaluation du reseau.

    Les blancs cherchent a maximiser le score.
    Les noirs cherchent a minimiser le score.
    """
    coups = generer_coups_possibles(etat)
    if not coups:
        return None

    meilleur_coup = None
    meilleur_score = None

    for coup in coups:
        etat_simule = appliquer_coup(etat, coup)
        entrees_simulees = convertir_etat_en_entrees(etat_simule)
        score = reseau.predire(entrees_simulees)

        if meilleur_coup is None:
            meilleur_coup = coup
            meilleur_score = score
            continue

        if etat.joueur_courant == 1:
            if score > meilleur_score:
                meilleur_coup = coup
                meilleur_score = score
        else:
            if score < meilleur_score:
                meilleur_coup = coup
                meilleur_score = score

    return meilleur_coup


def jouer_un_coup_et_entrainer(
    reseau: ReseauIA,
    etat: EtatJeuIA,
    generateur: np.random.Generator,
    taux_apprentissage: float,
    epsilon: float,
) -> tuple[EtatJeuIA, float, float]:
    """Joue un coup, puis entraine le reseau sur la recompense.

    On garde un peu d'exploration avec epsilon :
    - avec probabilite epsilon : coup aleatoire
    - sinon : meilleur coup selon le reseau

    Retourne :
    - le nouvel etat
    - la recompense immediate
    - la perte d'apprentissage
    """
    if float(generateur.random()) < epsilon:
        coup = choisir_coup_aleatoire(etat, generateur)
    else:
        coup = choisir_meilleur_coup(reseau, etat)

    if coup is None:
        return etat, 0.0, 0.0

    etat_avant = etat
    entrees = convertir_etat_en_entrees(etat_avant)
    etat_apres = appliquer_coup(etat_avant, coup)
    recompense = calculer_recompense(etat_avant, etat_apres)

    informations_apprentissage = reseau.apprendre(
        entrees=entrees,
        cible=recompense,
        taux_apprentissage=taux_apprentissage,
    )

    return etat_apres, recompense, informations_apprentissage["perte"]


def jouer_une_partie_entrainement(
    reseau: ReseauIA,
    taux_apprentissage: float = 0.01,
    graine: int | None = None,
    nb_coups_max: int = 200,
    epsilon: float = 0.2,
) -> dict[str, float]:
    """Joue une partie complete d'auto-jeu et entraine le reseau."""
    etat = initialiser_partie()
    generateur = np.random.default_rng(graine)

    recompense_totale = 0.0
    perte_totale = 0.0
    nb_coups_effectifs = 0

    while (not etat.partie_terminee) and (nb_coups_effectifs < nb_coups_max):
        etat, recompense, perte = jouer_un_coup_et_entrainer(
            reseau=reseau,
            etat=etat,
            generateur=generateur,
            taux_apprentissage=taux_apprentissage,
            epsilon=epsilon,
        )
        recompense_totale += recompense
        perte_totale += perte
        nb_coups_effectifs += 1

    return {
        "nb_coups": float(nb_coups_effectifs),
        "recompense_totale": recompense_totale,
        "perte_totale": perte_totale,
        "gagnant": float(etat.gagnant),
    }


def lancer_entrainement(
    nb_parties: int = 100,
    taux_apprentissage: float = 0.01,
    graine: int | None = None,
    epsilon: float = 0.2,
) -> tuple[ReseauIA, StatistiquesEntrainement]:
    """Lance plusieurs parties d'entrainement et retourne le reseau appris."""
    reseau = ReseauIA(graine=graine)
    generateur = np.random.default_rng(graine)

    recompense_cumulee = 0.0
    perte_cumulee = 0.0
    nb_coups_total = 0

    for _ in range(nb_parties):
        graine_partie = int(generateur.integers(0, 1_000_000_000))
        resultat_partie = jouer_une_partie_entrainement(
            reseau=reseau,
            taux_apprentissage=taux_apprentissage,
            graine=graine_partie,
            epsilon=epsilon,
        )
        recompense_cumulee += resultat_partie["recompense_totale"]
        perte_cumulee += resultat_partie["perte_totale"]
        nb_coups_total += int(resultat_partie["nb_coups"])

    statistiques = StatistiquesEntrainement(
        nb_parties=nb_parties,
        nb_coups_total=nb_coups_total,
        recompense_moyenne=recompense_cumulee / max(nb_parties, 1),
        perte_moyenne=perte_cumulee / max(nb_parties, 1),
    )

    return reseau, statistiques
