from __future__ import annotations

from pathlib import Path

import numpy as np

try:
    from sb3_contrib import MaskablePPO
except ImportError as exc:  # pragma: no cover - dependance optionnelle
    raise ImportError(
        "sb3-contrib est requis pour exporter le modele SB3 vers STM32."
    ) from exc


RACINE_PROJET = Path(__file__).resolve().parent.parent
CHEMIN_MODELE = Path(__file__).resolve().parent / "modeles_sb3" / "maskable_ppo_dames"
CHEMIN_HEADER = RACINE_PROJET / "projetDemo2026" / "Core" / "Inc" / "modele_policy_sb3_stm.h"
CHEMIN_SOURCE = RACINE_PROJET / "projetDemo2026" / "Core" / "Src" / "modele_policy_sb3_stm.c"


def formater_vecteur(valeurs: np.ndarray, indent: str = "  ") -> str:
    elements = ", ".join(f"{float(valeur):.8f}f" for valeur in valeurs)
    return f"{indent}{elements}"


def formater_matrice(matrice: np.ndarray, indent: str = "  ") -> str:
    lignes = []
    for ligne in matrice:
        lignes.append(f"{indent}{{{', '.join(f'{float(valeur):.8f}f' for valeur in ligne)}}}")
    return ",\n".join(lignes)


def generer_header(
    nb_entrees: int,
    nb_couche1: int,
    nb_couche2: int,
    nb_actions: int,
) -> str:
    return f"""#ifndef MODELE_POLICY_SB3_STM_H
#define MODELE_POLICY_SB3_STM_H

#define MODELE_POLICY_SB3_STM_NB_ENTREES {nb_entrees}U
#define MODELE_POLICY_SB3_STM_NB_COUCHE1 {nb_couche1}U
#define MODELE_POLICY_SB3_STM_NB_COUCHE2 {nb_couche2}U
#define MODELE_POLICY_SB3_STM_NB_ACTIONS {nb_actions}U

extern const float g_modelePolicySb3W1[MODELE_POLICY_SB3_STM_NB_COUCHE1][MODELE_POLICY_SB3_STM_NB_ENTREES];
extern const float g_modelePolicySb3B1[MODELE_POLICY_SB3_STM_NB_COUCHE1];
extern const float g_modelePolicySb3W2[MODELE_POLICY_SB3_STM_NB_COUCHE2][MODELE_POLICY_SB3_STM_NB_COUCHE1];
extern const float g_modelePolicySb3B2[MODELE_POLICY_SB3_STM_NB_COUCHE2];
extern const float g_modelePolicySb3W3[MODELE_POLICY_SB3_STM_NB_ACTIONS][MODELE_POLICY_SB3_STM_NB_COUCHE2];
extern const float g_modelePolicySb3B3[MODELE_POLICY_SB3_STM_NB_ACTIONS];

#endif /* MODELE_POLICY_SB3_STM_H */
"""


def generer_source(
    w1: np.ndarray,
    b1: np.ndarray,
    w2: np.ndarray,
    b2: np.ndarray,
    w3: np.ndarray,
    b3: np.ndarray,
) -> str:
    return f"""#include "modele_policy_sb3_stm.h"

const float g_modelePolicySb3W1[MODELE_POLICY_SB3_STM_NB_COUCHE1][MODELE_POLICY_SB3_STM_NB_ENTREES] = {{
{formater_matrice(w1)}
}};

const float g_modelePolicySb3B1[MODELE_POLICY_SB3_STM_NB_COUCHE1] = {{
{formater_vecteur(b1)}
}};

const float g_modelePolicySb3W2[MODELE_POLICY_SB3_STM_NB_COUCHE2][MODELE_POLICY_SB3_STM_NB_COUCHE1] = {{
{formater_matrice(w2)}
}};

const float g_modelePolicySb3B2[MODELE_POLICY_SB3_STM_NB_COUCHE2] = {{
{formater_vecteur(b2)}
}};

const float g_modelePolicySb3W3[MODELE_POLICY_SB3_STM_NB_ACTIONS][MODELE_POLICY_SB3_STM_NB_COUCHE2] = {{
{formater_matrice(w3)}
}};

const float g_modelePolicySb3B3[MODELE_POLICY_SB3_STM_NB_ACTIONS] = {{
{formater_vecteur(b3)}
}};
"""


def main() -> None:
    model = MaskablePPO.load(CHEMIN_MODELE)
    state = model.policy.state_dict()

    w1 = state["mlp_extractor.policy_net.0.weight"].detach().cpu().numpy().astype(np.float32)
    b1 = state["mlp_extractor.policy_net.0.bias"].detach().cpu().numpy().astype(np.float32)
    w2 = state["mlp_extractor.policy_net.2.weight"].detach().cpu().numpy().astype(np.float32)
    b2 = state["mlp_extractor.policy_net.2.bias"].detach().cpu().numpy().astype(np.float32)
    w3 = state["action_net.weight"].detach().cpu().numpy().astype(np.float32)
    b3 = state["action_net.bias"].detach().cpu().numpy().astype(np.float32)

    header = generer_header(
        nb_entrees=w1.shape[1],
        nb_couche1=w1.shape[0],
        nb_couche2=w2.shape[0],
        nb_actions=w3.shape[0],
    )
    source = generer_source(w1=w1, b1=b1, w2=w2, b2=b2, w3=w3, b3=b3)

    CHEMIN_HEADER.write_text(header, encoding="ascii")
    CHEMIN_SOURCE.write_text(source, encoding="ascii")

    print(f"Header genere : {CHEMIN_HEADER}")
    print(f"Source genere : {CHEMIN_SOURCE}")


if __name__ == "__main__":
    main()
