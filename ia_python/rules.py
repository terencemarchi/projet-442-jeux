"""Rules simplifiees du jeu de dames pour le mode IA.

Hypotheses retenues :
- plateau 10x10
- uniquement les cases jouables
- promotion en dame sur la derniere ligne
- dame courte : deplacement d'une case en diagonale dans les 4 directions
- dame courte : prise simple par saut de 2 cases dans les 4 directions
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
DAME_BLANCHE = 2
DAME_NOIRE = -2

JOUEUR_NOIR = -1
JOUEUR_BLANC = 1

GAGNANT_AUCUN = 0

MALUS_PAR_COUP = -0.1
BONUS_PRISE = 2.0
MALUS_PERTE = -1.0
BONUS_DAME = 4.0
BONUS_VICTOIRE = 100.0
MALUS_DEFAITE = -10.0
MALUS_NULLE = -10.0


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


def est_dame(piece: int) -> bool:
    return piece in (DAME_BLANCHE, DAME_NOIRE)


def piece_appartient_au_joueur(piece: int, joueur: int) -> bool:
    if joueur == JOUEUR_BLANC:
        return piece in (PION_BLANC, DAME_BLANCHE)
    return piece in (PION_NOIR, DAME_NOIRE)


def piece_est_adverse(piece: int, joueur: int) -> bool:
    if joueur == JOUEUR_BLANC:
        return piece in (PION_NOIR, DAME_NOIRE)
    return piece in (PION_BLANC, DAME_BLANCHE)


def compter_pieces(etat: EtatJeuIA, joueur: int) -> int:
    if joueur == JOUEUR_BLANC:
        return int(np.count_nonzero(np.isin(etat.plateau, [PION_BLANC, DAME_BLANCHE])))
    return int(np.count_nonzero(np.isin(etat.plateau, [PION_NOIR, DAME_NOIRE])))


def compter_dames(etat: EtatJeuIA, joueur: int) -> int:
    piece_dame = DAME_BLANCHE if joueur == JOUEUR_BLANC else DAME_NOIRE
    return int(np.count_nonzero(etat.plateau == piece_dame))


def generer_coups_possibles(etat: EtatJeuIA) -> list[CoupIA]:
    """Genere tous les coups possibles pour le joueur courant."""
    coups: list[CoupIA] = []
    direction = 1 if etat.joueur_courant == JOUEUR_BLANC else -1

    for ligne in range(TAILLE_PLATEAU_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            piece = int(etat.plateau[ligne, colonne])

            if not piece_appartient_au_joueur(piece, etat.joueur_courant):
                continue

            if est_dame(piece):
                directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
                directions_prise = [(-2, -2), (-2, 2), (2, -2), (2, 2)]
            else:
                directions = [(direction, -1), (direction, 1)]
                directions_prise = [(2 * direction, -2), (2 * direction, 2)]

            for delta_ligne, delta_colonne in directions:
                ligne_arrivee = ligne + delta_ligne
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

            for delta_ligne, delta_colonne in directions_prise:
                ligne_arrivee = ligne + delta_ligne
                colonne_arrivee = colonne + delta_colonne
                ligne_intermediaire = ligne + (delta_ligne // 2)
                colonne_intermediaire = colonne + (delta_colonne // 2)

                if not est_dans_plateau(ligne_arrivee, colonne_arrivee):
                    continue

                if not est_case_jouable(ligne_arrivee, colonne_arrivee):
                    continue

                if etat.plateau[ligne_arrivee, colonne_arrivee] != CASE_VIDE:
                    continue

                if not piece_est_adverse(
                    int(etat.plateau[ligne_intermediaire, colonne_intermediaire]),
                    etat.joueur_courant,
                ):
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
    piece = int(nouvel_etat.plateau[coup.ligne_depart, coup.colonne_depart])

    nouvel_etat.plateau[coup.ligne_depart, coup.colonne_depart] = CASE_VIDE
    nouvel_etat.plateau[coup.ligne_arrivee, coup.colonne_arrivee] = piece

    if coup.est_prise:
        ligne_capturee = (coup.ligne_depart + coup.ligne_arrivee) // 2
        colonne_capturee = (coup.colonne_depart + coup.colonne_arrivee) // 2
        nouvel_etat.plateau[ligne_capturee, colonne_capturee] = CASE_VIDE

    if piece == PION_BLANC and coup.ligne_arrivee == (TAILLE_PLATEAU_IA - 1):
        nouvel_etat.plateau[coup.ligne_arrivee, coup.colonne_arrivee] = DAME_BLANCHE
    elif piece == PION_NOIR and coup.ligne_arrivee == 0:
        nouvel_etat.plateau[coup.ligne_arrivee, coup.colonne_arrivee] = DAME_NOIRE

    nouvel_etat.nb_coups_joues += 1
    nouvel_etat.joueur_courant = -joueur
    evaluer_fin_partie(nouvel_etat)
    return nouvel_etat


def evaluer_fin_partie(etat: EtatJeuIA) -> None:
    """Met a jour l'etat de fin de partie."""
    nb_blancs = compter_pieces(etat, JOUEUR_BLANC)
    nb_noirs = compter_pieces(etat, JOUEUR_NOIR)

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
    match_nul: bool = False,
) -> float:
    """Calcule la recompense pour un joueur."""
    if joueur_reference is None:
        joueur_reference = etat_avant.joueur_courant

    adversaire = -joueur_reference

    pions_adverses_avant = compter_pieces(etat_avant, adversaire)
    pions_adverses_apres = compter_pieces(etat_apres, adversaire)
    pions_joueur_avant = compter_pieces(etat_avant, joueur_reference)
    pions_joueur_apres = compter_pieces(etat_apres, joueur_reference)
    dames_joueur_avant = compter_dames(etat_avant, joueur_reference)
    dames_joueur_apres = compter_dames(etat_apres, joueur_reference)
    dames_adverses_avant = compter_dames(etat_avant, adversaire)
    dames_adverses_apres = compter_dames(etat_apres, adversaire)

    recompense = MALUS_PAR_COUP
    recompense += BONUS_PRISE * float(pions_adverses_avant - pions_adverses_apres)
    recompense += MALUS_PERTE * float(pions_joueur_avant - pions_joueur_apres)
    recompense += BONUS_DAME * float(dames_joueur_apres - dames_joueur_avant)
    recompense -= BONUS_DAME * float(dames_adverses_apres - dames_adverses_avant)

    if etat_apres.partie_terminee:
        if etat_apres.gagnant == joueur_reference:
            recompense += BONUS_VICTOIRE
        elif etat_apres.gagnant == adversaire:
            recompense += MALUS_DEFAITE
        elif etat_apres.gagnant == GAGNANT_AUCUN:
            recompense += MALUS_NULLE

    if match_nul:
        recompense += MALUS_NULLE

    return recompense


def afficher_plateau_ascii(etat: EtatJeuIA) -> str:
    """Retourne une representation texte du plateau."""
    symboles = {
        PION_BLANC: "B",
        PION_NOIR: "N",
        DAME_BLANCHE: "b",
        DAME_NOIRE: "n",
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
