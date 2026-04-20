"""Point d'entree minimal pour tester les regles IA."""

from pathlib import Path
from statistics import mean

from entrainement import lancer_evaluation
from entrainement import lancer_entrainement
from regles_ia import afficher_plateau_ascii
from regles_ia import convertir_etat_en_entrees
from regles_ia import generer_coups_possibles
from regles_ia import initialiser_partie
from reseau_ia import ReseauIA

NB_PARTIES_ENTRAINEMENT = 1000
NB_PARTIES_EVALUATION = 200
GAMMA = 0.9
EPSILON_DEPART = 0.30
EPSILON_FIN = 0.05
INTERVALLE_AFFICHAGE_ENTRAINEMENT = 50
INTERVALLE_AFFICHAGE_EVALUATION = 25

ACTIVER_BENCHMARK = True
GRAINES_BENCHMARK = [11, 42, 123]
NB_PARTIES_ENTRAINEMENT_BENCHMARK = 150
NB_PARTIES_EVALUATION_BENCHMARK = 60


def afficher_resume_benchmark(
    nom_strategie: str,
    resultats: list[dict[str, float]],
) -> None:
    taux_victoires = [resultat["taux_victoires"] for resultat in resultats]
    taux_defaites = [resultat["taux_defaites"] for resultat in resultats]
    recompenses = [resultat["recompense_moyenne"] for resultat in resultats]

    print(f"Strategie : {nom_strategie}")
    print(f"  Graines testees : {len(resultats)}")
    print(f"  Taux moyen de victoire : {mean(taux_victoires):.2f}%")
    print(f"  Taux moyen de defaite : {mean(taux_defaites):.2f}%")
    print(f"  Recompense moyenne IA : {mean(recompenses):.4f}")


def executer_benchmark() -> None:
    strategies = [
        {
            "nom": "epsilon fixe",
            "epsilon_depart": 0.20,
            "epsilon_fin": 0.20,
        },
        {
            "nom": "epsilon decroissant",
            "epsilon_depart": EPSILON_DEPART,
            "epsilon_fin": EPSILON_FIN,
        },
    ]

    print()
    print("Benchmark multi-graines...")

    for strategie in strategies:
        resultats_strategie: list[dict[str, float]] = []

        for index_graine, graine in enumerate(GRAINES_BENCHMARK, start=1):
            print(
                f"Benchmark {strategie['nom']} : "
                f"graine {index_graine}/{len(GRAINES_BENCHMARK)} "
                f"(seed={graine})"
            )

            reseau_entraine, _ = lancer_entrainement(
                nb_parties=NB_PARTIES_ENTRAINEMENT_BENCHMARK,
                taux_apprentissage=0.01,
                graine=graine,
                epsilon_depart=strategie["epsilon_depart"],
                epsilon_fin=strategie["epsilon_fin"],
                gamma=GAMMA,
                intervalle_progression=0,
            )

            statistiques = lancer_evaluation(
                reseau_entraine,
                nb_parties=NB_PARTIES_EVALUATION_BENCHMARK,
                graine=graine,
                intervalle_progression=0,
            )

            resultats_strategie.append(
                {
                    "taux_victoires": 100.0
                    * statistiques.nb_victoires_ia
                    / max(statistiques.nb_parties, 1),
                    "taux_defaites": 100.0
                    * statistiques.nb_defaites_ia
                    / max(statistiques.nb_parties, 1),
                    "recompense_moyenne": statistiques.recompense_moyenne_ia,
                }
            )

        afficher_resume_benchmark(strategie["nom"], resultats_strategie)


def main() -> None:
    etat = initialiser_partie()
    coups = generer_coups_possibles(etat)
    reseau = ReseauIA(graine=42)
    entrees = convertir_etat_en_entrees(etat)
    score_initial = reseau.predire(entrees)

    print("Etat initial :")
    print(afficher_plateau_ascii(etat))
    print()
    print(f"Joueur courant : {etat.joueur_courant}")
    print(f"Nombre de coups possibles : {len(coups)}")
    print(f"Score initial du reseau : {score_initial:.4f}")

    print()
    print("Test entrainement minimal...")
    reseau_entraine, statistiques = lancer_entrainement(
        nb_parties=NB_PARTIES_ENTRAINEMENT,
        taux_apprentissage=0.01,
        graine=42,
        epsilon_depart=EPSILON_DEPART,
        epsilon_fin=EPSILON_FIN,
        gamma=GAMMA,
        intervalle_progression=INTERVALLE_AFFICHAGE_ENTRAINEMENT,
    )
    print(f"Nombre de parties : {statistiques.nb_parties}")
    print(f"Nombre de coups total : {statistiques.nb_coups_total}")
    print(f"Recompense moyenne : {statistiques.recompense_moyenne:.4f}")
    print(f"Perte moyenne : {statistiques.perte_moyenne:.4f}")

    print()
    print("Evaluation contre un joueur aleatoire...")
    statistiques_evaluation = lancer_evaluation(
        reseau_entraine,
        nb_parties=NB_PARTIES_EVALUATION,
        graine=42,
        intervalle_progression=INTERVALLE_AFFICHAGE_EVALUATION,
    )
    print(f"Nombre de parties : {statistiques_evaluation.nb_parties}")
    print(f"Nombre de coups total : {statistiques_evaluation.nb_coups_total}")
    print(f"Victoires IA : {statistiques_evaluation.nb_victoires_ia}")
    print(f"Defaites IA : {statistiques_evaluation.nb_defaites_ia}")
    print(f"Nulles : {statistiques_evaluation.nb_nulles}")
    print(
        "Recompense moyenne IA : "
        f"{statistiques_evaluation.recompense_moyenne_ia:.4f}"
    )

    dossier_csv = Path(__file__).resolve().parent / "exports_csv"
    reseau_entraine.sauvegarder_csv(dossier_csv)
    print(f"CSV exportes dans : {dossier_csv}")

    if ACTIVER_BENCHMARK:
        executer_benchmark()


if __name__ == "__main__":
    main()
