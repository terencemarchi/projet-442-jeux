from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import numpy as np

try:
    from sb3_contrib import MaskablePPO
except ImportError as exc:  # pragma: no cover - dependance optionnelle
    raise ImportError(
        "sb3-contrib est requis pour utiliser test.py."
    ) from exc

from env import DamesSb3Env


NB_EPISODES_EVAL = 200
NB_COUPS_MAX = 150
SEED = 42
CHEMIN_MODELE = (
    Path(__file__).resolve().parent / "modeles_sb3" / "maskable_ppo_dames"
)


@dataclass
class StatistiquesEvaluationSb3:
    nb_episodes: int
    nb_victoires: int
    nb_defaites: int
    nb_nulles: int
    recompense_moyenne: float
    nb_coups_moyen: float


def evaluer_modele(
    chemin_modele: Path,
    nb_episodes: int,
    nb_coups_max: int,
    seed: int,
) -> StatistiquesEvaluationSb3:
    env = DamesSb3Env(
        joueur_agent=1,
        nb_coups_max=nb_coups_max,
        alterner_couleur=True,
    )
    modele = MaskablePPO.load(chemin_modele)
    generateur = np.random.default_rng(seed)

    nb_victoires = 0
    nb_defaites = 0
    nb_nulles = 0
    recompense_cumulee = 0.0
    nb_coups_cumules = 0

    for index_episode in range(nb_episodes):
        observation, info = env.reset(seed=int(generateur.integers(0, 1_000_000_000)))
        termine = False
        tronque = False
        recompense_episode = 0.0

        while not (termine or tronque):
            masque_actions = env.action_masks()
            action, _ = modele.predict(
                observation,
                action_masks=masque_actions,
                deterministic=True,
            )
            observation, recompense, termine, tronque, info = env.step(int(action))
            recompense_episode += float(recompense)

        recompense_cumulee += recompense_episode
        nb_coups_cumules += int(info["nb_coups_joues"])

        if tronque:
            nb_nulles += 1
        elif int(info["gagnant"]) == int(info["joueur_agent"]):
            nb_victoires += 1
        else:
            nb_defaites += 1

        print(f"Evaluation SB3 : episode {index_episode + 1}/{nb_episodes} termine")

    return StatistiquesEvaluationSb3(
        nb_episodes=nb_episodes,
        nb_victoires=nb_victoires,
        nb_defaites=nb_defaites,
        nb_nulles=nb_nulles,
        recompense_moyenne=recompense_cumulee / max(nb_episodes, 1),
        nb_coups_moyen=nb_coups_cumules / max(nb_episodes, 1),
    )


def main() -> None:
    print(f"Chargement du modele : {CHEMIN_MODELE}")
    statistiques = evaluer_modele(
        chemin_modele=CHEMIN_MODELE,
        nb_episodes=NB_EPISODES_EVAL,
        nb_coups_max=NB_COUPS_MAX,
        seed=SEED,
    )

    print()
    print("Resultats evaluation SB3 :")
    print(f"Nombre d'episodes : {statistiques.nb_episodes}")
    print(f"Victoires : {statistiques.nb_victoires}")
    print(f"Defaites : {statistiques.nb_defaites}")
    print(f"Nulles : {statistiques.nb_nulles}")
    print(f"Recompense moyenne : {statistiques.recompense_moyenne:.4f}")
    print(f"Nombre moyen de coups : {statistiques.nb_coups_moyen:.2f}")


if __name__ == "__main__":
    main()
