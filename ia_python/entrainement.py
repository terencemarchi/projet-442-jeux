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


GAMMA_PAR_DEFAUT = 0.9
ECHELLE_CIBLE = 10.0


@dataclass
class StatistiquesEntrainement:
    nb_parties: int
    nb_coups_total: int
    recompense_moyenne: float
    perte_moyenne: float


@dataclass
class StatistiquesEvaluation:
    nb_parties: int
    nb_coups_total: int
    nb_victoires_ia: int
    nb_defaites_ia: int
    nb_nulles: int
    recompense_moyenne_ia: float


def interpoler_epsilon(
    index_partie: int,
    nb_parties: int,
    epsilon_depart: float,
    epsilon_fin: float,
) -> float:
    """Fait decroitre epsilon lineairement au fil de l'entrainement."""
    if nb_parties <= 1:
        return epsilon_fin

    progression = index_partie / float(nb_parties - 1)
    return float(epsilon_depart + ((epsilon_fin - epsilon_depart) * progression))


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


def choisir_coup_politique(
    reseau: ReseauIA,
    etat: EtatJeuIA,
    generateur: np.random.Generator,
    epsilon: float,
):
    """Choisit un coup selon une politique epsilon-greedy."""
    if float(generateur.random()) < epsilon:
        return choisir_coup_aleatoire(etat, generateur)
    return choisir_meilleur_coup(reseau, etat)


def calculer_cible_v2(
    reseau: ReseauIA,
    etat_avant: EtatJeuIA,
    etat_apres: EtatJeuIA,
    gamma: float,
) -> tuple[float, float]:
    """Calcule une cible TD simple pour la sortie du reseau.

    Le score du reseau est interprete globalement :
    - positif = favorable aux blancs
    - negatif = favorable aux noirs

    On convertit donc la recompense locale du joueur actif vers cette meme
    convention avant d'ajouter la valeur future.
    """
    joueur_actif = etat_avant.joueur_courant
    recompense_locale = calculer_recompense(
        etat_avant, etat_apres, joueur_reference=joueur_actif
    )
    recompense_alignee = float(joueur_actif) * recompense_locale

    if etat_apres.partie_terminee:
        cible_brute = recompense_alignee
    else:
        entrees_apres = convertir_etat_en_entrees(etat_apres)
        valeur_future = reseau.predire(entrees_apres)
        cible_brute = recompense_alignee + (gamma * valeur_future)

    cible_normalisee = float(np.clip(cible_brute / ECHELLE_CIBLE, -1.0, 1.0))
    return recompense_locale, cible_normalisee


def jouer_un_coup_et_entrainer(
    reseau: ReseauIA,
    etat: EtatJeuIA,
    generateur: np.random.Generator,
    taux_apprentissage: float,
    epsilon: float,
    gamma: float,
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
    coup = choisir_coup_politique(reseau, etat, generateur, epsilon)

    if coup is None:
        return etat, 0.0, 0.0

    etat_avant = etat
    entrees = convertir_etat_en_entrees(etat_avant)
    etat_apres = appliquer_coup(etat_avant, coup)
    recompense, cible = calculer_cible_v2(reseau, etat_avant, etat_apres, gamma)

    informations_apprentissage = reseau.apprendre(
        entrees=entrees,
        cible=cible,
        taux_apprentissage=taux_apprentissage,
    )

    return etat_apres, recompense, informations_apprentissage["perte"]


def jouer_une_partie_entrainement(
    reseau: ReseauIA,
    taux_apprentissage: float = 0.01,
    graine: int | None = None,
    nb_coups_max: int = 200,
    epsilon: float = 0.2,
    gamma: float = GAMMA_PAR_DEFAUT,
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
            gamma=gamma,
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


def jouer_une_partie_evaluation(
    reseau: ReseauIA,
    joueur_ia: int,
    graine: int | None = None,
    nb_coups_max: int = 200,
    epsilon_ia: float = 0.0,
) -> dict[str, float]:
    """Joue une partie sans apprentissage entre l'IA et une politique aleatoire."""
    etat = initialiser_partie()
    generateur = np.random.default_rng(graine)

    recompense_ia_totale = 0.0
    nb_coups_effectifs = 0

    while (not etat.partie_terminee) and (nb_coups_effectifs < nb_coups_max):
        joueur_actif = etat.joueur_courant
        etat_avant = etat

        if joueur_actif == joueur_ia:
            coup = choisir_coup_politique(reseau, etat, generateur, epsilon_ia)
        else:
            coup = choisir_coup_aleatoire(etat, generateur)

        if coup is None:
            break

        etat = appliquer_coup(etat, coup)

        if joueur_actif == joueur_ia:
            recompense_ia_totale += calculer_recompense(
                etat_avant, etat, joueur_reference=joueur_ia
            )

        nb_coups_effectifs += 1

    match_nul = (not etat.partie_terminee) and (nb_coups_effectifs >= nb_coups_max)

    return {
        "nb_coups": float(nb_coups_effectifs),
        "gagnant": float(etat.gagnant),
        "match_nul": 1.0 if match_nul else 0.0,
        "recompense_ia_totale": recompense_ia_totale,
    }


def lancer_entrainement(
    nb_parties: int = 100,
    taux_apprentissage: float = 0.01,
    graine: int | None = None,
    epsilon_depart: float = 0.2,
    epsilon_fin: float = 0.05,
    gamma: float = GAMMA_PAR_DEFAUT,
    intervalle_progression: int = 0,
) -> tuple[ReseauIA, StatistiquesEntrainement]:
    """Lance plusieurs parties d'entrainement et retourne le reseau appris."""
    reseau = ReseauIA(graine=graine)
    generateur = np.random.default_rng(graine)

    recompense_cumulee = 0.0
    perte_cumulee = 0.0
    nb_coups_total = 0

    for index_partie in range(nb_parties):
        epsilon_courant = interpoler_epsilon(
            index_partie=index_partie,
            nb_parties=nb_parties,
            epsilon_depart=epsilon_depart,
            epsilon_fin=epsilon_fin,
        )
        graine_partie = int(generateur.integers(0, 1_000_000_000))
        resultat_partie = jouer_une_partie_entrainement(
            reseau=reseau,
            taux_apprentissage=taux_apprentissage,
            graine=graine_partie,
            epsilon=epsilon_courant,
            gamma=gamma,
        )
        recompense_cumulee += resultat_partie["recompense_totale"]
        perte_cumulee += resultat_partie["perte_totale"]
        nb_coups_total += int(resultat_partie["nb_coups"])

        if (
            intervalle_progression > 0
            and (((index_partie + 1) % intervalle_progression) == 0 or (index_partie + 1) == nb_parties)
        ):
            print(
                "Entrainement : partie "
                f"{index_partie + 1}/{nb_parties} terminee "
                f"(epsilon={epsilon_courant:.4f})"
            )

    statistiques = StatistiquesEntrainement(
        nb_parties=nb_parties,
        nb_coups_total=nb_coups_total,
        recompense_moyenne=recompense_cumulee / max(nb_parties, 1),
        perte_moyenne=perte_cumulee / max(nb_parties, 1),
    )

    return reseau, statistiques


def lancer_evaluation(
    reseau: ReseauIA,
    nb_parties: int = 50,
    graine: int | None = None,
    nb_coups_max: int = 200,
    epsilon_ia: float = 0.0,
    intervalle_progression: int = 0,
) -> StatistiquesEvaluation:
    """Evalue le reseau contre un joueur aleatoire.

    Les couleurs alternent d'une partie a l'autre pour limiter le biais.
    """
    generateur = np.random.default_rng(graine)

    nb_coups_total = 0
    nb_victoires_ia = 0
    nb_defaites_ia = 0
    nb_nulles = 0
    recompense_ia_cumulee = 0.0

    for index_partie in range(nb_parties):
        joueur_ia = 1 if (index_partie % 2 == 0) else -1
        graine_partie = int(generateur.integers(0, 1_000_000_000))
        resultat = jouer_une_partie_evaluation(
            reseau=reseau,
            joueur_ia=joueur_ia,
            graine=graine_partie,
            nb_coups_max=nb_coups_max,
            epsilon_ia=epsilon_ia,
        )

        nb_coups_total += int(resultat["nb_coups"])
        recompense_ia_cumulee += resultat["recompense_ia_totale"]

        if resultat["match_nul"] != 0.0:
            nb_nulles += 1
        elif int(resultat["gagnant"]) == joueur_ia:
            nb_victoires_ia += 1
        else:
            nb_defaites_ia += 1

        if (
            intervalle_progression > 0
            and (((index_partie + 1) % intervalle_progression) == 0 or (index_partie + 1) == nb_parties)
        ):
            print(
                f"Evaluation : partie {index_partie + 1}/{nb_parties} terminee"
            )

    return StatistiquesEvaluation(
        nb_parties=nb_parties,
        nb_coups_total=nb_coups_total,
        nb_victoires_ia=nb_victoires_ia,
        nb_defaites_ia=nb_defaites_ia,
        nb_nulles=nb_nulles,
        recompense_moyenne_ia=recompense_ia_cumulee / max(nb_parties, 1),
    )
