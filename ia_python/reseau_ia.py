"""Reseau de neurones.

Architecture V1 :
- 51 entrees
- 16 neurones caches
- 1 sortie

Fonctions d'activation :
- couche cachee : tanh
- sortie : tanh

Tableau recapitulatif entre les maths et le code Python :

Maths   -> Code Python
x       -> vecteur_entree
W1      -> self.poids_entree_cachee
b1      -> self.biais_cachee
z1      -> pre_activations_cachee
a1      -> activations_cachee
W2      -> self.poids_cachee_sortie
b2      -> self.biais_sortie
z2      -> pre_activations_sortie
a2      -> activations_sortie
sortie  -> float(activations_sortie[0])
"""

from __future__ import annotations
import numpy as np
from regles_ia import NB_ENTREES_IA


NB_NEURONES_CACHES_IA = 16
NB_SORTIES_IA = 1


class ReseauIA:
    """Petit reseau dense pour evaluer un etat de jeu."""

    def __init__(
        self,
        nb_entrees: int = NB_ENTREES_IA,
        nb_neurones_caches: int = NB_NEURONES_CACHES_IA,
        graine: int | None = None,
    ) -> None:
        self.nb_entrees = nb_entrees
        self.nb_neurones_caches = nb_neurones_caches
        self.nb_sorties = NB_SORTIES_IA
        self.generateur = np.random.default_rng(graine)

        self.poids_entree_cachee = np.zeros(
            (self.nb_neurones_caches, self.nb_entrees), dtype=np.float32
        )
        self.biais_cachee = np.zeros(self.nb_neurones_caches, dtype=np.float32)
        self.poids_cachee_sortie = np.zeros(
            (self.nb_sorties, self.nb_neurones_caches), dtype=np.float32
        )
        self.biais_sortie = np.zeros(self.nb_sorties, dtype=np.float32)

        self.reinitialiser_poids()

    def reinitialiser_poids(self) -> None:
        """Initialise les poids avec une loi uniforme de type Xavier simple."""
        limite_entree_cachee = np.sqrt(6.0 / (self.nb_entrees + self.nb_neurones_caches))
        limite_cachee_sortie = np.sqrt(6.0 / (self.nb_neurones_caches + self.nb_sorties))

        self.poids_entree_cachee = self.generateur.uniform(
            low=-limite_entree_cachee,
            high=limite_entree_cachee,
            size=(self.nb_neurones_caches, self.nb_entrees),
        ).astype(np.float32)

        self.poids_cachee_sortie = self.generateur.uniform(
            low=-limite_cachee_sortie,
            high=limite_cachee_sortie,
            size=(self.nb_sorties, self.nb_neurones_caches),
        ).astype(np.float32)

        self.biais_cachee.fill(0.0)
        self.biais_sortie.fill(0.0)

    def predire(self, entrees: np.ndarray) -> float:
        """Retourne le score estime pour un etat."""
        _, _, _, activations_sortie = self.propagation_avant(entrees)
        return float(activations_sortie[0])

    def propagation_avant(
        self, entrees: np.ndarray
    ) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
        """Calcule la propagation avant complete."""
        vecteur_entree = self._valider_et_convertir_entrees(entrees)

        pre_activations_cachee = (
            self.poids_entree_cachee @ vecteur_entree + self.biais_cachee
        )
        activations_cachee = self._tanh(pre_activations_cachee)

        pre_activations_sortie = (
            self.poids_cachee_sortie @ activations_cachee + self.biais_sortie
        )
        activations_sortie = self._tanh(pre_activations_sortie)

        return (
            vecteur_entree,
            pre_activations_cachee,
            activations_cachee,
            activations_sortie,
        )

    def apprendre(
        self,
        entrees: np.ndarray,
        cible: float,
        taux_apprentissage: float = 0.01,
    ) -> dict[str, float]:
        """Effectue une etape d'apprentissage par retropropagation.

        Retourne quelques informations utiles pour suivre l'entrainement.
        """
        (
            vecteur_entree,
            _pre_activations_cachee,
            activations_cachee,
            activations_sortie,
        ) = self.propagation_avant(entrees)

        cible_vecteur = np.array([cible], dtype=np.float32)
        erreur_sortie = activations_sortie - cible_vecteur
        delta_sortie = erreur_sortie * self._derivee_tanh_depuis_activation(
            activations_sortie
        )

        gradient_poids_cachee_sortie = np.outer(delta_sortie, activations_cachee)
        gradient_biais_sortie = delta_sortie

        delta_cachee = (
            self.poids_cachee_sortie.T @ delta_sortie
        ) * self._derivee_tanh_depuis_activation(activations_cachee)

        gradient_poids_entree_cachee = np.outer(delta_cachee, vecteur_entree)
        gradient_biais_cachee = delta_cachee

        self.poids_cachee_sortie -= taux_apprentissage * gradient_poids_cachee_sortie
        self.biais_sortie -= taux_apprentissage * gradient_biais_sortie
        self.poids_entree_cachee -= taux_apprentissage * gradient_poids_entree_cachee
        self.biais_cachee -= taux_apprentissage * gradient_biais_cachee

        perte = 0.5 * float(np.sum(erreur_sortie**2))

        return {
            "sortie": float(activations_sortie[0]),
            "cible": float(cible),
            "erreur": float(erreur_sortie[0]),
            "perte": perte,
        }

    def _valider_et_convertir_entrees(self, entrees: np.ndarray) -> np.ndarray:
        vecteur_entree = np.asarray(entrees, dtype=np.float32).reshape(-1)

        if vecteur_entree.shape[0] != self.nb_entrees:
            raise ValueError(
                f"Nombre d'entrees invalide : {vecteur_entree.shape[0]} "
                f"au lieu de {self.nb_entrees}."
            )

        return vecteur_entree

    @staticmethod
    def _tanh(x: np.ndarray) -> np.ndarray:
        return np.tanh(x).astype(np.float32)

    @staticmethod
    def _derivee_tanh_depuis_activation(activation: np.ndarray) -> np.ndarray:
        return (1.0 - activation**2).astype(np.float32)
