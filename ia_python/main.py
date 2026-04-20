"""Point d'entree minimal pour tester les regles IA."""

from entrainement import lancer_entrainement
from regles_ia import afficher_plateau_ascii
from regles_ia import convertir_etat_en_entrees
from regles_ia import generer_coups_possibles
from regles_ia import initialiser_partie
from reseau_ia import ReseauIA


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
    _, statistiques = lancer_entrainement(nb_parties=5, taux_apprentissage=0.01, graine=42)
    print(f"Nombre de parties : {statistiques.nb_parties}")
    print(f"Nombre de coups total : {statistiques.nb_coups_total}")
    print(f"Recompense moyenne : {statistiques.recompense_moyenne:.4f}")
    print(f"Perte moyenne : {statistiques.perte_moyenne:.4f}")


if __name__ == "__main__":
    main()
