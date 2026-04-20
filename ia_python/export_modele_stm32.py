"""Exporte les poids Python actuels vers un module C pour la STM32."""

from __future__ import annotations

from pathlib import Path

import numpy as np


RACINE_PROJET = Path(__file__).resolve().parents[1]
DOSSIER_EXPORTS_CSV = Path(__file__).resolve().parent / "exports_csv"
FICHIER_SORTIE_C = (
    RACINE_PROJET / "projetDemo2026" / "Core" / "Src" / "modele_ia_stm.c"
)


def formatter_flottant(valeur: float) -> str:
    return f"{float(valeur):.7f}f"


def exporter_modele_stm32() -> Path:
    poids_entree_cachee = np.loadtxt(
        DOSSIER_EXPORTS_CSV / "W1_poids_entree_cachee.csv", delimiter=","
    )
    biais_cachee = np.loadtxt(
        DOSSIER_EXPORTS_CSV / "b1_biais_cachee.csv", delimiter=","
    ).reshape(-1)
    poids_cachee_sortie = np.loadtxt(
        DOSSIER_EXPORTS_CSV / "W2_poids_cachee_sortie.csv", delimiter=","
    ).reshape(-1)
    biais_sortie = float(
        np.loadtxt(DOSSIER_EXPORTS_CSV / "b2_biais_sortie.csv", delimiter=",")
    )

    lignes: list[str] = []
    lignes.append('#include "modele_ia_stm.h"')
    lignes.append("")
    lignes.append(
        "const float g_modeleIaPoidsEntreeCachee"
        "[MODELE_IA_STM_NB_NEURONES_CACHES][REGLES_IA_NB_ENTREES] = {"
    )
    for ligne in poids_entree_cachee:
        valeurs = ", ".join(formatter_flottant(valeur) for valeur in ligne)
        lignes.append(f"  {{{valeurs}}},")
    lignes.append("};")
    lignes.append("")
    lignes.append(
        "const float g_modeleIaBiaisCachee[MODELE_IA_STM_NB_NEURONES_CACHES] = {"
    )
    lignes.append("  " + ", ".join(formatter_flottant(valeur) for valeur in biais_cachee))
    lignes.append("};")
    lignes.append("")
    lignes.append(
        "const float g_modeleIaPoidsCacheeSortie[MODELE_IA_STM_NB_NEURONES_CACHES] = {"
    )
    lignes.append(
        "  " + ", ".join(formatter_flottant(valeur) for valeur in poids_cachee_sortie)
    )
    lignes.append("};")
    lignes.append("")
    lignes.append(f"const float g_modeleIaBiaisSortie = {formatter_flottant(biais_sortie)};")

    FICHIER_SORTIE_C.write_text("\n".join(lignes) + "\n")
    return FICHIER_SORTIE_C


if __name__ == "__main__":
    fichier = exporter_modele_stm32()
    print(f"Module STM32 genere : {fichier}")
