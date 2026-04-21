from __future__ import annotations
from pathlib import Path
from stable_baselines3.common.env_checker import check_env
from sb3_contrib import MaskablePPO
from env import DamesSb3Env


TOTAL_TIMESTEPS = 100_000
NB_COUPS_MAX = 150
SEED = 42
DOSSIER_MODELES = Path(__file__).resolve().parent / "modeles_sb3"


def main() -> None:
    DOSSIER_MODELES.mkdir(parents=True, exist_ok=True)

    env = DamesSb3Env(
        joueur_agent=1,
        nb_coups_max=NB_COUPS_MAX,
        alterner_couleur=True,
    )

    print("Verification de l'environnement Gymnasium...")
    check_env(env, warn=True)

    print("Creation du modele MaskablePPO...")
    modele = MaskablePPO(
        "MlpPolicy",
        env,
        verbose=1,
        gamma=0.99,
        n_steps=1024,
        batch_size=128,
        learning_rate=3e-4,
        seed=SEED,
        device="auto",
    )

    print(f"Debut entrainement sur {TOTAL_TIMESTEPS} timesteps...")
    modele.learn(total_timesteps=TOTAL_TIMESTEPS, progress_bar=False)

    chemin_modele = DOSSIER_MODELES / "maskable_ppo_dames"
    modele.save(chemin_modele)
    print(f"Modele sauvegarde dans : {chemin_modele}")


if __name__ == "__main__":
    main()
