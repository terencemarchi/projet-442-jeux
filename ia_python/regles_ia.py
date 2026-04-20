"""Regles simplifiees du jeu de dames pour le mode IA.

Hypotheses retenues :
- plateau 10x10
- uniquement les cases jouables
- pions simples uniquement, pas de dames
- deplacement diagonal simple vers l'avant
- prise simple d'un seul pion vers l'avant
- pas de prise multiple
- pas de prise obligatoire
- pas de regle du maximum
"""

from __future__ import annotations
from dataclasses import dataclass
import numpy as np

TAILLE_PLATEAU_IA = 10
NB_CASES_JOUABLES_IA = 50
NB_LIGNES_PIONS_IA = 4
NB_ENTREES_IA = 51

PION_NOIR = -1
CASE_VIDE = 0
PION_BLANC = 1

JOUEUR_NOIR = -1
JOUEUR_BLANC = 1

GAGNANT_AUCUN = 0


@dataclass(frozen=True)
class CoupIA:
    ligne_depart: int
    colonne_depart: int
    ligne_arrivee: int
    colonne_arrivee: int
    est_prise: bool


@dataclass
class EtatJeuIA:
    plateau: np.ndarray
    joueur_courant: int
    partie_terminee: bool
    gagnant: int
    nb_coups_joues: int


def creer_plateau_initial() -> np.ndarray:
    """Cree le plateau de depart."""
    plateau = np.zeros((TAILLE_PLATEAU_IA, TAILLE_PLATEAU_IA), dtype=np.int8)

    for ligne in range(NB_LIGNES_PIONS_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            if est_case_jouable(ligne, colonne):
                plateau[ligne, colonne] = PION_BLANC

    for ligne in range(TAILLE_PLATEAU_IA - NB_LIGNES_PIONS_IA, TAILLE_PLATEAU_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            if est_case_jouable(ligne, colonne):
                plateau[ligne, colonne] = PION_NOIR

    return plateau


def initialiser_partie() -> EtatJeuIA:
    """Initialise une partie simplifiee."""
    return EtatJeuIA(
        plateau=creer_plateau_initial(),
        joueur_courant=JOUEUR_BLANC,
        partie_terminee=False,
        gagnant=GAGNANT_AUCUN,
        nb_coups_joues=0,
    )


def copier_etat(etat: EtatJeuIA) -> EtatJeuIA:
    """Retourne une copie profonde de l'etat."""
    return EtatJeuIA(
        plateau=etat.plateau.copy(),
        joueur_courant=etat.joueur_courant,
        partie_terminee=etat.partie_terminee,
        gagnant=etat.gagnant,
        nb_coups_joues=etat.nb_coups_joues,
    )


def est_case_jouable(ligne: int, colonne: int) -> bool:
    return (ligne + colonne) % 2 == 1


def est_dans_plateau(ligne: int, colonne: int) -> bool:
    return 0 <= ligne < TAILLE_PLATEAU_IA and 0 <= colonne < TAILLE_PLATEAU_IA


def compter_pions(etat: EtatJeuIA, joueur: int) -> int:
    return int(np.count_nonzero(etat.plateau == joueur))


def generer_coups_possibles(etat: EtatJeuIA) -> list[CoupIA]:
    """Genere tous les coups possibles pour le joueur courant.
    Les prises restent facultatives et seulement vers l'avant.
    """
    coups: list[CoupIA] = []
    direction = 1 if etat.joueur_courant == JOUEUR_BLANC else -1
    adversaire = -etat.joueur_courant

    for ligne in range(TAILLE_PLATEAU_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            if etat.plateau[ligne, colonne] != etat.joueur_courant:
                continue

            for delta_colonne in (-1, 1):
                ligne_arrivee = ligne + direction
                colonne_arrivee = colonne + delta_colonne

                if (
                    est_dans_plateau(ligne_arrivee, colonne_arrivee)
                    and est_case_jouable(ligne_arrivee, colonne_arrivee)
                    and etat.plateau[ligne_arrivee, colonne_arrivee] == CASE_VIDE
                ):
                    coups.append(
                        CoupIA(
                            ligne_depart=ligne,
                            colonne_depart=colonne,
                            ligne_arrivee=ligne_arrivee,
                            colonne_arrivee=colonne_arrivee,
                            est_prise=False,
                        )
                    )

            for delta_colonne in (-2, 2):
                ligne_arrivee = ligne + (2 * direction)
                colonne_arrivee = colonne + delta_colonne
                ligne_intermediaire = ligne + direction
                colonne_intermediaire = colonne + (delta_colonne // 2)

                if not est_dans_plateau(ligne_arrivee, colonne_arrivee):
                    continue

                if not est_case_jouable(ligne_arrivee, colonne_arrivee):
                    continue

                if etat.plateau[ligne_arrivee, colonne_arrivee] != CASE_VIDE:
                    continue

                if etat.plateau[ligne_intermediaire, colonne_intermediaire] != adversaire:
                    continue

                coups.append(
                    CoupIA(
                        ligne_depart=ligne,
                        colonne_depart=colonne,
                        ligne_arrivee=ligne_arrivee,
                        colonne_arrivee=colonne_arrivee,
                        est_prise=True,
                    )
                )

    return coups


def coup_est_valide(etat: EtatJeuIA, coup: CoupIA) -> bool:
    """Verifie si un coup appartient a la liste des coups possibles."""
    return coup in generer_coups_possibles(etat)


def appliquer_coup(etat: EtatJeuIA, coup: CoupIA) -> EtatJeuIA:
    """Applique un coup et retourne le nouvel etat."""
    if not coup_est_valide(etat, coup):
        raise ValueError("Coup invalide pour l'etat fourni.")

    nouvel_etat = copier_etat(etat)
    joueur = nouvel_etat.joueur_courant

    nouvel_etat.plateau[coup.ligne_depart, coup.colonne_depart] = CASE_VIDE
    nouvel_etat.plateau[coup.ligne_arrivee, coup.colonne_arrivee] = joueur

    if coup.est_prise:
        ligne_capturee = (coup.ligne_depart + coup.ligne_arrivee) // 2
        colonne_capturee = (coup.colonne_depart + coup.colonne_arrivee) // 2
        nouvel_etat.plateau[ligne_capturee, colonne_capturee] = CASE_VIDE

    nouvel_etat.nb_coups_joues += 1
    nouvel_etat.joueur_courant = -joueur
    evaluer_fin_partie(nouvel_etat)
    return nouvel_etat


def evaluer_fin_partie(etat: EtatJeuIA) -> None:
    """Met a jour l'etat de fin de partie."""
    nb_blancs = compter_pions(etat, JOUEUR_BLANC)
    nb_noirs = compter_pions(etat, JOUEUR_NOIR)

    if nb_blancs == 0:
        etat.partie_terminee = True
        etat.gagnant = JOUEUR_NOIR
        return

    if nb_noirs == 0:
        etat.partie_terminee = True
        etat.gagnant = JOUEUR_BLANC
        return

    if len(generer_coups_possibles(etat)) == 0:
        etat.partie_terminee = True
        etat.gagnant = -etat.joueur_courant
        return

    etat.partie_terminee = False
    etat.gagnant = GAGNANT_AUCUN


def convertir_etat_en_entrees(etat: EtatJeuIA) -> np.ndarray:
    """Convertit l'etat du jeu en vecteur de 51 entrees."""
    entrees = np.zeros(NB_ENTREES_IA, dtype=np.float32)
    index = 0

    for ligne in range(TAILLE_PLATEAU_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            if est_case_jouable(ligne, colonne):
                entrees[index] = float(etat.plateau[ligne, colonne])
                index += 1

    entrees[NB_CASES_JOUABLES_IA] = float(etat.joueur_courant)
    return entrees


def calculer_recompense(
    etat_avant: EtatJeuIA,
    etat_apres: EtatJeuIA,
    joueur_reference: int | None = None,
) -> float:
    """Calcule la recompense pour un joueur.

    Recompenses retenues pour la V1 :
    - +1 si un pion adverse est pris
    - -1 si un pion du joueur de reference est perdu
    - +10 si victoire
    - -10 si defaite
    - -0.01 par coup
    """
    if joueur_reference is None:
        joueur_reference = etat_avant.joueur_courant

    adversaire = -joueur_reference

    pions_adverses_avant = compter_pions(etat_avant, adversaire)
    pions_adverses_apres = compter_pions(etat_apres, adversaire)
    pions_joueur_avant = compter_pions(etat_avant, joueur_reference)
    pions_joueur_apres = compter_pions(etat_apres, joueur_reference)

    recompense = -0.01
    recompense += float(pions_adverses_avant - pions_adverses_apres)
    recompense -= float(pions_joueur_avant - pions_joueur_apres)

    if etat_apres.partie_terminee:
        if etat_apres.gagnant == joueur_reference:
            recompense += 10.0
        elif etat_apres.gagnant == adversaire:
            recompense -= 10.0

    return recompense


def afficher_plateau_ascii(etat: EtatJeuIA) -> str:
    """Retourne une representation texte du plateau."""
    symboles = {
        PION_BLANC: "B",
        PION_NOIR: "N",
        CASE_VIDE: ".",
    }

    lignes: list[str] = []
    for ligne in range(TAILLE_PLATEAU_IA):
        ligne_texte: list[str] = []
        for colonne in range(TAILLE_PLATEAU_IA):
            if not est_case_jouable(ligne, colonne):
                ligne_texte.append(" ")
            else:
                ligne_texte.append(symboles[int(etat.plateau[ligne, colonne])])
        lignes.append(" ".join(ligne_texte))
    return "\n".join(lignes)
