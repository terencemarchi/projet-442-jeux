from __future__ import annotations
from dataclasses import dataclass
from typing import Any
import numpy as np

import gymnasium as gym
from gymnasium import spaces

from rules import CASE_VIDE
from rules import CoupIA
from rules import EtatJeuIA
from rules import TAILLE_PLATEAU_IA
from rules import appliquer_coup
from rules import calculer_recompense
from rules import convertir_etat_en_entrees
from rules import est_case_jouable
from rules import generer_coups_possibles
from rules import initialiser_partie

NB_CASES_JOUABLES = 50
NB_ACTIONS = NB_CASES_JOUABLES * NB_CASES_JOUABLES
RECOMPENSE_ACTION_INVALIDE = -5.0

@dataclass(frozen=True)
class CoupEncode:
    index_depart: int
    index_arrivee: int


def construire_cases_jouables() -> list[tuple[int, int]]:
    """Construit la table 0..49 -> coordonnees plateau."""
    cases: list[tuple[int, int]] = []
    for ligne in range(TAILLE_PLATEAU_IA):
        for colonne in range(TAILLE_PLATEAU_IA):
            if est_case_jouable(ligne, colonne):
                cases.append((ligne, colonne))
    return cases


CASES_JOUABLES = construire_cases_jouables()
COORD_VERS_INDEX = {coord: index for index, coord in enumerate(CASES_JOUABLES)}


class DamesSb3Env(gym.Env):
    """Environnement SB3 pour apprendre une politique de coups.
    L'agent joue contre un adversaire aleatoire
    """

    metadata = {"render_modes": ["human"]}

    def __init__(
        self,
        joueur_agent: int = 1,
        nb_coups_max: int = 150,
        alterner_couleur: bool = False,
    ) -> None:
        super().__init__()
        self.joueur_agent_initial = joueur_agent
        self.joueur_agent = joueur_agent
        self.nb_coups_max = nb_coups_max
        self.alterner_couleur = alterner_couleur

        self.observation_space = spaces.Box(
            low=-2.0,
            high=2.0,
            shape=(51,),
            dtype=np.float32,
        )
        self.action_space = spaces.Discrete(NB_ACTIONS)

        self.etat: EtatJeuIA | None = None
        self._rng = np.random.default_rng()
        self._index_episode = 0

    def reset(
        self,
        *,
        seed: int | None = None,
        options: dict[str, Any] | None = None,
    ) -> tuple[np.ndarray, dict[str, Any]]:
        super().reset(seed=seed)
        self._rng = np.random.default_rng(seed)
        self.etat = initialiser_partie()

        if self.alterner_couleur:
            self.joueur_agent = 1 if (self._index_episode % 2 == 0) else -1
            self._index_episode += 1
        else:
            self.joueur_agent = self.joueur_agent_initial

        if self.etat.joueur_courant != self.joueur_agent:
            self._jouer_tour_adverse()

        return self._observation(), self._info()

    def step(
        self, action: int
    ) -> tuple[np.ndarray, float, bool, bool, dict[str, Any]]:
        assert self.etat is not None, "reset() doit etre appele avant step()."

        etat_avant = self.etat
        coup = self._decoder_action_vers_coup(action)
        coups_valides = generer_coups_possibles(etat_avant)

        if coup is None or coup not in coups_valides:
            observation = self._observation()
            info = self._info()
            info["action_invalide"] = True
            return observation, RECOMPENSE_ACTION_INVALIDE, True, False, info

        self.etat = appliquer_coup(etat_avant, coup)
        recompense = calculer_recompense(
            etat_avant, self.etat, joueur_reference=self.joueur_agent
        )

        if self.etat.partie_terminee:
            return self._observation(), recompense, True, False, self._info()

        if self.etat.nb_coups_joues >= self.nb_coups_max:
            recompense += calculer_recompense(
                self.etat,
                self.etat,
                joueur_reference=self.joueur_agent,
                match_nul=True,
            )
            return self._observation(), recompense, False, True, self._info()

        recompense += self._jouer_tour_adverse()

        termine = bool(self.etat.partie_terminee)
        tronque = bool(
            (not self.etat.partie_terminee)
            and (self.etat.nb_coups_joues >= self.nb_coups_max)
        )

        if tronque:
            recompense += calculer_recompense(
                self.etat,
                self.etat,
                joueur_reference=self.joueur_agent,
                match_nul=True,
            )

        return self._observation(), recompense, termine, tronque, self._info()

    def render(self) -> None:
        assert self.etat is not None, "reset() doit etre appele avant render()."
        print(self.etat.plateau)

    def action_masks(self) -> np.ndarray:
        """Masque d'actions valide pour MaskablePPO.

        True = action autorisee
        False = action interdite
        """
        assert self.etat is not None, "reset() doit etre appele avant action_masks()."
        masque = np.zeros(NB_ACTIONS, dtype=bool)

        if self.etat.partie_terminee or self.etat.joueur_courant != self.joueur_agent:
            return masque

        for coup in generer_coups_possibles(self.etat):
            masque[self._encoder_coup(coup)] = True

        return masque

    def _observation(self) -> np.ndarray:
        assert self.etat is not None
        return convertir_etat_en_entrees(self.etat)

    def _info(self) -> dict[str, Any]:
        assert self.etat is not None
        return {
            "joueur_agent": self.joueur_agent,
            "joueur_courant": self.etat.joueur_courant,
            "nb_coups_joues": self.etat.nb_coups_joues,
            "gagnant": self.etat.gagnant,
            "nb_actions_valides": int(np.count_nonzero(self.action_masks())),
        }

    def _jouer_tour_adverse(self) -> float:
        assert self.etat is not None
        if self.etat.partie_terminee:
            return 0.0

        coups_adverses = generer_coups_possibles(self.etat)
        if not coups_adverses:
            return 0.0

        index = int(self._rng.integers(0, len(coups_adverses)))
        coup_adverse = coups_adverses[index]
        etat_avant = self.etat
        self.etat = appliquer_coup(self.etat, coup_adverse)
        return calculer_recompense(
            etat_avant, self.etat, joueur_reference=self.joueur_agent
        )

    def _encoder_coup(self, coup: CoupIA) -> int:
        index_depart = COORD_VERS_INDEX[(coup.ligne_depart, coup.colonne_depart)]
        index_arrivee = COORD_VERS_INDEX[(coup.ligne_arrivee, coup.colonne_arrivee)]
        return (index_depart * NB_CASES_JOUABLES) + index_arrivee

    def _decoder_action_vers_coup(self, action: int) -> CoupIA | None:
        index_depart = int(action) // NB_CASES_JOUABLES
        index_arrivee = int(action) % NB_CASES_JOUABLES

        if not (0 <= index_depart < NB_CASES_JOUABLES):
            return None
        if not (0 <= index_arrivee < NB_CASES_JOUABLES):
            return None

        ligne_depart, colonne_depart = CASES_JOUABLES[index_depart]
        ligne_arrivee, colonne_arrivee = CASES_JOUABLES[index_arrivee]

        piece_depart = int(self.etat.plateau[ligne_depart, colonne_depart]) if self.etat else CASE_VIDE
        if piece_depart == CASE_VIDE:
            est_prise = False
        else:
            est_prise = abs(ligne_arrivee - ligne_depart) == 2

        return CoupIA(
            ligne_depart=ligne_depart,
            colonne_depart=colonne_depart,
            ligne_arrivee=ligne_arrivee,
            colonne_arrivee=colonne_arrivee,
            est_prise=est_prise,
        )
