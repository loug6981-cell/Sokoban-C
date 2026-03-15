#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

/* Constantes et touches */
#define DEPLACE 1000
#define TAILLE 12
#define MUR '#'
#define CAISSE '$'      
#define CIBLE '.'
#define SOKOBAN '@'
#define CAISSE_CIBLE '*'
#define SOKOBAN_CIBLE '+'
#define VIDE ' '
#define CAISSE '$' 
#define SOKOBAN '@'  
#define KEY_HAUT 'z' /* KEY signifie que la constante est utilise pour un touche*/
#define KEY_BAS 's'
#define KEY_GAUCHE 'q'
#define KEY_DROITE 'd'
#define KEY_RESET 'r'
#define KEY_QUIT 'x'
#define KEY_COME_BACK 'u'
#define KEY_UNZOOM '-'     
#define KEY_ZOOM '+'     
#define ZOOM_MIN 1   /*vision normal*/      
#define ZOOM_MAX 3   /*jusqu'a deux zoom possible */    


typedef char t_tabDeplacement[DEPLACE];  
typedef char t_Plateau[TAILLE][TAILLE];

/* fonction et procedures*/
void enregistrerDeplacements(const t_tabDeplacement t, int nb, const char fic[]);  
void chargerPartie(t_Plateau plateau, char fichier[]);
void enregistrerPartie(t_Plateau plateau, char fichier[]);
int  kbhit(void);
void afficher_entete(const t_Plateau plateau, const char *nomPartie, int nbDeplacements, int zoom); 
void afficher_plateau(const t_Plateau plateau, int zoom);                                          
bool deplacer(t_Plateau plateau, int dLig, int dCol, bool *pousseCaisse);
void rejouer_deplacements(t_Plateau plateau, const t_tabDeplacement t, int nb);
void appliquer_deplacement(t_Plateau plateau, char dep);
void sauvegarde_deplacements(const t_tabDeplacement t, int nbDeplacements);
bool gagne(const t_Plateau plateau, int totalCibles);
int  compter_cibles(const t_Plateau plateau);
void recharger_partie_initiale(t_Plateau plateauCourant, const t_Plateau plateauInitial);
void copier_plateau(t_Plateau dest, const t_Plateau src);

/* Programme principale */
int main(void)
{
    char nomPartie[50];
    t_Plateau plateau = {{0}};
    t_Plateau plateauInitial = {{0}};
    t_tabDeplacement tabDeplacements;  /*historique des mouvements*/
    int nbDeplacements = 0;   /*compteur afficher au joueur*/
    int nbDeplacementsMemorises = 0; /*combien de case de tabdéplacement sont utilisés*/
    int totalCibles = 0;
    int zoom = 1;                 /* etat du zoom x1 (vision normal) */

    /* saisie du niveau */

    printf("A quel niveau souhaite tu jouers ?\n");    
    if (scanf("%s", nomPartie) != 1) {          
        fprintf(stderr, "Lecture du nom de fichier impossible.\n");
        return EXIT_FAILURE;
    }

    /* Pour recommencer la partie */

    chargerPartie(plateau, nomPartie);
    copier_plateau(plateauInitial, plateau);
    totalCibles = compter_cibles(plateau);

    /* Affichage */
    system("clear");
    afficher_entete(plateau, nomPartie, nbDeplacements, zoom); 
    afficher_plateau(plateau, zoom);                           

    
    bool enCours = true;
    while (enCours && !gagne(plateau, totalCibles)) {  /* condition de victoire */
        int touche = '\0';
        if (kbhit()) {
            touche = getchar();

            bool mouv = false;
            bool pousseCaisse = false;
            switch (touche) {      /*position selon la touche et collision avec la caisse*/

                case KEY_HAUT:
                    mouv = deplacer(plateau, -1,  0, &pousseCaisse);
                    break;
                case KEY_BAS:
                    mouv = deplacer(plateau,  1,  0, &pousseCaisse);
                    break;
                case KEY_GAUCHE:
                    mouv = deplacer(plateau,  0, -1, &pousseCaisse);
                    break;
                case KEY_DROITE:
                    mouv = deplacer(plateau,  0,  1, &pousseCaisse);
                    break;

                /* Gestion du zoom et du unzoom avec + et - */
                case KEY_ZOOM:
                    if (zoom < ZOOM_MAX) {   /* zooom */
                        zoom++;
                    }
                    break;
                case KEY_UNZOOM:
                    if (zoom > ZOOM_MIN) {   /* unzoom */
                        zoom--;
                    }
                    break;

                case KEY_COME_BACK: /*u est la touche pour annuler un deplacement*/
                    if (nbDeplacementsMemorises > 0) { /*nombre de deplacement toujours positif*/
                        nbDeplacementsMemorises--; /*compteur pour revenir en arriere*/
                        nbDeplacements = nbDeplacementsMemorises; 
                        recharger_partie_initiale(plateau, plateauInitial); /*repartir du plateau initial */
                        rejouer_deplacements(plateau, tabDeplacements, nbDeplacementsMemorises);
                    }
                    break;

                case KEY_RESET: {  /*r pour recommencer*/
                    printf("Recommencer la partie ? (o/n) ");
                    int rep = getchar();

                    if (rep == '\n'){
                        rep = getchar();
                    }

                    if (rep == 'o') { /*tout reviens a zero mais reste dans le niveau ecrit*/
                        recharger_partie_initiale(plateau, plateauInitial);
                        nbDeplacements = 0;
                        nbDeplacementsMemorises = 0;
                        system("clear");   /*tout l affichage remis a zero*/
                        afficher_entete(plateau, nomPartie, nbDeplacements, zoom);
                        afficher_plateau(plateau, zoom);                           
                        continue;
                    }
                    break;
                }

                case KEY_QUIT: {
                    /* boucle abandonner et enregistrement */
                    printf("Sauvegarder la partie ? (o/n)\n");
                    int rep = getchar();
                    if (rep == '\n'){
                        rep = getchar();
                    }
                    if (rep == 'o') {
                        char fichierSauvegarde[50];
                        printf("Nom du fichier .sok a enregistrer : \n");
                        if (scanf("%s", fichierSauvegarde) == 1) {        
                            enregistrerPartie(plateau, fichierSauvegarde);
                            printf("Partie enregistree dans %s\n",
                                   fichierSauvegarde);
                        } else {
                            printf("Erreur d'enregistrement\n");
                        }
                    }
                    sauvegarde_deplacements(tabDeplacements, nbDeplacementsMemorises);
                    printf("vous avez abandonne\n");
                    enCours = false;
                    break;
                }

                default:
                    break;
            }

            /* memorisation des deplacements */
            if (mouv && nbDeplacementsMemorises < DEPLACE) { /*si un mouvement a lieu*/
                char codeDep = '\0';

                switch (touche) {  /*touche de deplacement assigner */

                    case KEY_GAUCHE:
                        if (pousseCaisse){
                            codeDep = 'G';   /*caisse toucher */
                        }else{
                            codeDep = 'g';   /*mouvement sans avoir toucher de caisse*/
                        }
                        break;
                        

                    case KEY_HAUT:
                        if (pousseCaisse){
                            codeDep = 'H';
                        }else{
                            codeDep = 'h';
                        }
                        break;
                        

                    case KEY_BAS:
                        if (pousseCaisse){
                            codeDep = 'B';
                        }else{
                            codeDep = 'b';
                        }
                        break;
                        

                    case KEY_DROITE:
                        if (pousseCaisse){
                            codeDep = 'D';
                        }else{
                            codeDep = 'd';
                        }
                        break;
                        

                    default:
                        break;
                }
                if (codeDep != '\0') {
                    tabDeplacements[nbDeplacementsMemorises] = codeDep; /*stoker dans tabdeplacement*/
                    nbDeplacementsMemorises++;
                }

            }

            /* affiche zoom apres deplacement ou zoom back*/
            if (mouv || touche == KEY_ZOOM || touche == KEY_UNZOOM || touche == KEY_COME_BACK) { 
                if (mouv) { /*compteur*/
                    nbDeplacements++;
                }
                system("clear");
                afficher_entete(plateau, nomPartie, nbDeplacements, zoom); 
                afficher_plateau(plateau, zoom);                           
            }
        }
    }

    if (enCours) {
        /* Sortie de la boucle car gagne est vrai */
        system("clear");
        afficher_entete(plateau, nomPartie, nbDeplacements, zoom); 
        afficher_plateau(plateau, zoom);                           
        printf("\nVous avez gagne!\n");
        sauvegarde_deplacements(tabDeplacements, nbDeplacementsMemorises);
    }

    return EXIT_SUCCESS;
}


/* affichage commandes et compteurs */
void afficher_entete(const t_Plateau plateau, const char *nomPartie, int nbDeplacements, int zoom) 
{
    printf("---------------------------------------------\n");
    printf("|                 SOKOBAN                   |\n");
    printf("---------------------------------------------\n");
    printf("Partie : %s\n", nomPartie);
    printf("Touches :\n");
    printf("  %c -> haut   | %c -> bas\n", KEY_HAUT, KEY_BAS);
    printf("  %c -> gauche | %c -> droite\n", KEY_GAUCHE, KEY_DROITE);
    printf("  %c -> recommencer | %c -> abandonner\n",KEY_RESET, KEY_QUIT);
    printf("  %c -> revenir en arriere\n", KEY_COME_BACK);
    printf("  %c -> zoomer | %c -> dezoomer\n", KEY_ZOOM, KEY_UNZOOM); 
    printf("Zoom actuel : x%d\n", zoom);                              
    printf("Nombre de deplacements : %d\n", nbDeplacements); /*diminue si la touche u est presse (car reviens en arrier)*/
    printf("---------------------------------------------\n");
}

/* Affiche le plateau */
void afficher_plateau(const t_Plateau plateau, int zoom) 
{
    for (int lig = 0; lig < TAILLE; lig++) {
        /* repete chaque ligne plusieurs fois pour le zoom vertical */
        for (int zv = 0; zv < zoom; zv++) {                
            for (int col = 0; col < TAILLE; col++) {
                char c = plateau[lig][col];
                if (c == CAISSE_CIBLE){
                    c = CAISSE;
                }    
                if (c == SOKOBAN_CIBLE){ 
                    c = SOKOBAN; 
                }
                /* repete le caractère pour le zoom horizontal */
                for (int zh = 0; zh < zoom; zh++) {         
                    putchar(c);
                }
            }
            putchar('\n');
        }
    }
}



/* deplacement avec les colonnes*/
bool deplacer(t_Plateau p, int dLig, int dCol, bool *pousseCaisse)
{
    if (pousseCaisse != NULL) {
        *pousseCaisse = false;
    }
    /* positionnement sokoban (pointeur aussi)*/
    int jLig = -1, jCol = -1;
    for (int lig = 0; lig < TAILLE; lig++) {
        for (int col = 0; col < TAILLE; col++) {
            if (p[lig][col] == SOKOBAN || p[lig][col] == SOKOBAN_CIBLE) {
                jLig = lig;
                jCol = col;
                break;
            }
        }
        if (jLig != -1) break;
    }
    if (jLig == -1) return false; /* pas de joueur */

    int aLig = jLig + dLig, aCol = jCol + dCol;  /* une case devant soko */
    int bLig = jLig + 2*dLig, bCol = jCol + 2*dCol;  /* case devant soko avec caisse */

    if (aLig < 0 || aLig >= TAILLE || aCol < 0 || aCol >= TAILLE)
        return false;

    char cur = p[jLig][jCol];   
    char a = p[aLig][aCol];

    /* mur donc ne peut pas avancer */
    if (a == MUR){
        return false;
    }

    /* toujours avoir une case qui permet a sokoban d'arriver en jeu */
    void leave_from_current(void) {
        if (cur == SOKOBAN_CIBLE) p[jLig][jCol] = CIBLE; 
        else p[jLig][jCol] = VIDE;  
    }

    /* si la case de devant est une cible ou un espace vide*/
    if (a == VIDE || a == CIBLE) {
        leave_from_current();
        p[aLig][aCol] = (a == CIBLE) ? SOKOBAN_CIBLE : SOKOBAN;
        return true;
    }

    /* si la caisse peut etre pousser et atteint la cible */
    if (a == CAISSE || a == CAISSE_CIBLE) {
        if (bLig < 0 || bLig >= TAILLE || bCol < 0 || bCol >= TAILLE)
            return false;
        char b = p[bLig][bCol];
        if (b == VIDE || b == CIBLE) {
            /* pousser la caisse */
            p[bLig][bCol] = (b == CIBLE) ? CAISSE_CIBLE : CAISSE;
            if (pousseCaisse != NULL) {
                *pousseCaisse = true;
            }
            /* appel fonstion ici */
            leave_from_current();
            p[aLig][aCol] = (a == CAISSE_CIBLE)
                            ? SOKOBAN_CIBLE : SOKOBAN;
            return true;
        }
        return false; /* case qui zst bloque */
    }

    return false;
}



/* pour gagner (toutes les cibles sont atteintes) */
bool gagne(const t_Plateau plateau, int totalCibles)
{
    int boxesOnTargets = 0;
    for (int lig = 0; lig < TAILLE; lig++) {
        for (int col = 0; col < TAILLE; col++) {
            if (plateau[lig][col] == CAISSE_CIBLE){
                boxesOnTargets++; /*ajoute 1 au compteur des caisses sur cible */
            }
        }
    }
    return boxesOnTargets == totalCibles;
}

/*compte les cibles (variable c) */
int compter_cibles(const t_Plateau plateau)
{
    int c = 0;
    for (int lig = 0; lig < TAILLE; lig++) {
        for (int col = 0; col < TAILLE; col++) {
            char ch = plateau[lig][col];
            if (ch == CIBLE || ch == SOKOBAN_CIBLE || ch == CAISSE_CIBLE)
                c++;
        }
    }
    return c;
}

void appliquer_deplacement(t_Plateau plateau, char dep)
{
    int dLig = 0;
    int dCol = 0;
    bool pousseCaisse = false;

    switch (dep) { /*les touches gGdDhHbB sont affectes aux deplacements*/
        case 'g':
        case 'G':
            dLig = 0;
            dCol = -1;
            break;
        case 'd':
        case 'D':
            dLig = 0;
            dCol = 1;
            break;
        case 'h':
        case 'H':
            dLig = -1;
            dCol = 0;
            break;
        case 'b':
        case 'B':
            dLig = 1;
            dCol = 0;
            break;
        default:
            break;
    }

    if (dLig != 0 || dCol != 0) {
        deplacer(plateau, dLig, dCol, &pousseCaisse);
    }
}

void rejouer_deplacements(t_Plateau plateau, const t_tabDeplacement t, int nb)
{
    for (int i = 0; i < nb; i++) {
        appliquer_deplacement(plateau, t[i]);
    }
}

void sauvegarde_deplacements(const t_tabDeplacement t,int nbDeplacements)
{
    printf("Tu veux sauvegarder les deplacements ? (o/n)\n");
    int rep = getchar();
    if (rep == '\n') {
        rep = getchar(); //essaie sans
    }
    if (rep == 'o') {
        char fichierDep[50];
        printf("donne le nom du fichier .dep :\n");
        if (scanf("%50s", fichierDep) == 1) {
            enregistrerDeplacements(t, nbDeplacements, fichierDep);
            printf("Deplacements enregistrers dans %s\n", fichierDep);
        } else {
            printf("Erreur\n");
        }
    }
}

/*copie du tableau de restart*/
void copier_plateau(t_Plateau dest, const t_Plateau src)
{
    for (int lig = 0; lig < TAILLE; lig++) {
        for (int col = 0; col < TAILLE; col++) {
            dest[lig][col] = src[lig][col];
        }
    }
}

void recharger_partie_initiale(t_Plateau plateauCourant, const t_Plateau plateauInitial)
{
    copier_plateau(plateauCourant, plateauInitial);
}

void chargerPartie(t_Plateau plateau, char fichier[])
{
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL){
        printf("fichier mal ecrit");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne = 0; ligne < TAILLE; ligne++){
            for (int colonne = 0; colonne < TAILLE; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrerPartie(t_Plateau plateau, char fichier[])
{
    FILE * f;
    char finDeLigne = '\n';

    f = fopen(fichier, "w");
    for (int ligne = 0; ligne < TAILLE; ligne++){
        for (int colonne = 0; colonne < TAILLE; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

int kbhit(void)
{
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF){
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}


void enregistrerDeplacements(const t_tabDeplacement t, int nb, const char fic[])
{
    FILE * f;

    f = fopen(fic, "w");
    fwrite(t,sizeof(char), nb, f);
    fclose(f);
}