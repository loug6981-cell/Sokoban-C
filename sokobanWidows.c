#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>

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

#define KEY_HAUT 'z'
#define KEY_BAS 's'
#define KEY_GAUCHE 'q'
#define KEY_DROITE 'd'
#define KEY_RESET 'r'
#define KEY_QUIT 'x'
#define KEY_COME_BACK 'u'
#define KEY_UNZOOM '-'
#define KEY_ZOOM '+'

#define ZOOM_MIN 1
#define ZOOM_MAX 3

typedef char t_tabDeplacement[DEPLACE];
typedef char t_Plateau[TAILLE][TAILLE];

/* prototypes */
void enregistrerDeplacements(const t_tabDeplacement t, int nb, const char fic[]);
void chargerPartie(t_Plateau plateau, char fichier[]);
void enregistrerPartie(t_Plateau plateau, char fichier[]);
void afficher_entete(const char *nomPartie, int nbDeplacements, int zoom);
void afficher_plateau(const t_Plateau plateau, int zoom);
bool deplacer(t_Plateau plateau, int dLig, int dCol, bool *pousseCaisse);
void rejouer_deplacements(t_Plateau plateau, const t_tabDeplacement t, int nb);
void appliquer_deplacement(t_Plateau plateau, char dep);
void sauvegarde_deplacements(const t_tabDeplacement t, int nbDeplacements);
bool gagne(const t_Plateau plateau, int totalCibles);
int compter_cibles(const t_Plateau plateau);
void recharger_partie_initiale(t_Plateau plateauCourant, const t_Plateau plateauInitial);
void copier_plateau(t_Plateau dest, const t_Plateau src);

int main(void)
{
    char nomPartie[50];
    t_Plateau plateau = {{0}};
    t_Plateau plateauInitial = {{0}};
    t_tabDeplacement tabDeplacements;

    int nbDeplacements = 0;
    int nbDeplacementsMemorises = 0;
    int totalCibles = 0;
    int zoom = 1;

    printf("A quel niveau souhaite tu jouer ?\n");
    scanf("%s", nomPartie);

    chargerPartie(plateau, nomPartie);
    copier_plateau(plateauInitial, plateau);
    totalCibles = compter_cibles(plateau);

    system("cls");
    afficher_entete(nomPartie, nbDeplacements, zoom);
    afficher_plateau(plateau, zoom);

    bool enCours = true;

    while (enCours && !gagne(plateau, totalCibles))
    {
        if (_kbhit())
        {
            char touche = _getch();
            bool mouv = false;
            bool pousseCaisse = false;

            switch (touche)
            {
            case KEY_HAUT:
                mouv = deplacer(plateau, -1, 0, &pousseCaisse);
                break;

            case KEY_BAS:
                mouv = deplacer(plateau, 1, 0, &pousseCaisse);
                break;

            case KEY_GAUCHE:
                mouv = deplacer(plateau, 0, -1, &pousseCaisse);
                break;

            case KEY_DROITE:
                mouv = deplacer(plateau, 0, 1, &pousseCaisse);
                break;

            case KEY_ZOOM:
                if (zoom < ZOOM_MAX) zoom++;
                break;

            case KEY_UNZOOM:
                if (zoom > ZOOM_MIN) zoom--;
                break;

            case KEY_COME_BACK:
                if (nbDeplacementsMemorises > 0)
                {
                    nbDeplacementsMemorises--;
                    nbDeplacements = nbDeplacementsMemorises;

                    recharger_partie_initiale(plateau, plateauInitial);
                    rejouer_deplacements(plateau, tabDeplacements, nbDeplacementsMemorises);
                }
                break;

            case KEY_RESET:
            {
                printf("Recommencer la partie ? (o/n) ");
                char rep = _getch();

                if (rep == 'o')
                {
                    recharger_partie_initiale(plateau, plateauInitial);
                    nbDeplacements = 0;
                    nbDeplacementsMemorises = 0;

                    system("cls");
                    afficher_entete(nomPartie, nbDeplacements, zoom);
                    afficher_plateau(plateau, zoom);
                }
                break;
            }

            case KEY_QUIT:
            {
                printf("Sauvegarder la partie ? (o/n)\n");
                char rep = _getch();

                if (rep == 'o')
                {
                    char fichierSauvegarde[50];
                    printf("Nom du fichier .sok : ");
                    scanf("%s", fichierSauvegarde);

                    enregistrerPartie(plateau, fichierSauvegarde);
                }

                sauvegarde_deplacements(tabDeplacements, nbDeplacementsMemorises);

                printf("vous avez abandonne\n");
                enCours = false;
                break;
            }

            default:
                break;
            }

            if (mouv && nbDeplacementsMemorises < DEPLACE)
            {
                char codeDep = '\0';

                if (touche == KEY_GAUCHE) codeDep = pousseCaisse ? 'G' : 'g';
                if (touche == KEY_DROITE) codeDep = pousseCaisse ? 'D' : 'd';
                if (touche == KEY_HAUT) codeDep = pousseCaisse ? 'H' : 'h';
                if (touche == KEY_BAS) codeDep = pousseCaisse ? 'B' : 'b';

                if (codeDep != '\0')
                {
                    tabDeplacements[nbDeplacementsMemorises++] = codeDep;
                }
            }

            if (mouv || touche == KEY_ZOOM || touche == KEY_UNZOOM || touche == KEY_COME_BACK)
            {
                if (mouv) nbDeplacements++;

                system("cls");
                afficher_entete(nomPartie, nbDeplacements, zoom);
                afficher_plateau(plateau, zoom);
            }
        }
    }

    if (enCours)
    {
        system("cls");
        afficher_entete(nomPartie, nbDeplacements, zoom);
        afficher_plateau(plateau, zoom);

        printf("\nVous avez gagne!\n");

        sauvegarde_deplacements(tabDeplacements, nbDeplacementsMemorises);
    }

    return EXIT_SUCCESS;
}

void afficher_entete(const char *nomPartie, int nbDeplacements, int zoom)
{
    printf("---------------------------------------------\n");
    printf("|                 SOKOBAN                   |\n");
    printf("---------------------------------------------\n");
    printf("Partie : %s\n", nomPartie);

    printf("z haut | s bas\n");
    printf("q gauche | d droite\n");
    printf("r recommencer | x quitter\n");
    printf("u annuler\n");

    printf("+ zoom | - dezoom\n");

    printf("Zoom : x%d\n", zoom);
    printf("Deplacements : %d\n", nbDeplacements);

    printf("---------------------------------------------\n");
}

void afficher_plateau(const t_Plateau plateau, int zoom)
{
    for (int lig = 0; lig < TAILLE; lig++)
    {
        for (int zv = 0; zv < zoom; zv++)
        {
            for (int col = 0; col < TAILLE; col++)
            {
                char c = plateau[lig][col];

                if (c == CAISSE_CIBLE) c = CAISSE;
                if (c == SOKOBAN_CIBLE) c = SOKOBAN;

                for (int zh = 0; zh < zoom; zh++)
                    putchar(c);
            }
            printf("\n");
        }
    }
}

bool deplacer(t_Plateau p, int dLig, int dCol, bool *pousseCaisse)
{
    if (pousseCaisse) *pousseCaisse = false;

    int jLig=-1,jCol=-1;

    for(int i=0;i<TAILLE;i++)
        for(int j=0;j<TAILLE;j++)
            if(p[i][j]==SOKOBAN||p[i][j]==SOKOBAN_CIBLE)
            { jLig=i; jCol=j; }

    if(jLig==-1) return false;

    int aLig=jLig+dLig;
    int aCol=jCol+dCol;

    int bLig=jLig+2*dLig;
    int bCol=jCol+2*dCol;

    char cur=p[jLig][jCol];
    char a=p[aLig][aCol];

    if(a==MUR) return false;

    if(a==VIDE||a==CIBLE)
    {
        p[jLig][jCol]=(cur==SOKOBAN_CIBLE)?CIBLE:VIDE;
        p[aLig][aCol]=(a==CIBLE)?SOKOBAN_CIBLE:SOKOBAN;
        return true;
    }

    if(a==CAISSE||a==CAISSE_CIBLE)
    {
        char b=p[bLig][bCol];

        if(b==VIDE||b==CIBLE)
        {
            p[bLig][bCol]=(b==CIBLE)?CAISSE_CIBLE:CAISSE;
            p[jLig][jCol]=(cur==SOKOBAN_CIBLE)?CIBLE:VIDE;
            p[aLig][aCol]=(a==CAISSE_CIBLE)?SOKOBAN_CIBLE:SOKOBAN;

            if(pousseCaisse) *pousseCaisse=true;

            return true;
        }
    }

    return false;
}

bool gagne(const t_Plateau plateau,int totalCibles)
{
    int c=0;

    for(int i=0;i<TAILLE;i++)
        for(int j=0;j<TAILLE;j++)
            if(plateau[i][j]==CAISSE_CIBLE)
                c++;

    return c==totalCibles;
}

int compter_cibles(const t_Plateau plateau)
{
    int c=0;

    for(int i=0;i<TAILLE;i++)
        for(int j=0;j<TAILLE;j++)
            if(plateau[i][j]==CIBLE||plateau[i][j]==CAISSE_CIBLE||plateau[i][j]==SOKOBAN_CIBLE)
                c++;

    return c;
}

void copier_plateau(t_Plateau dest,const t_Plateau src)
{
    for(int i=0;i<TAILLE;i++)
        for(int j=0;j<TAILLE;j++)
            dest[i][j]=src[i][j];
}

void recharger_partie_initiale(t_Plateau plateauCourant,const t_Plateau plateauInitial)
{
    copier_plateau(plateauCourant,plateauInitial);
}

void chargerPartie(t_Plateau plateau,char fichier[])
{
    FILE *f=fopen(fichier,"r");

    if(!f)
    {
        printf("Erreur fichier\n");
        exit(EXIT_FAILURE);
    }

    for(int i=0;i<TAILLE;i++)
    {
        for(int j=0;j<TAILLE;j++)
            fread(&plateau[i][j],sizeof(char),1,f);

        fgetc(f);
    }

    fclose(f);
}

void enregistrerPartie(t_Plateau plateau,char fichier[])
{
    FILE *f=fopen(fichier,"w");

    for(int i=0;i<TAILLE;i++)
    {
        for(int j=0;j<TAILLE;j++)
            fwrite(&plateau[i][j],sizeof(char),1,f);

        fputc('\n',f);
    }

    fclose(f);
}

void enregistrerDeplacements(const t_tabDeplacement t,int nb,const char fic[])
{
    FILE *f=fopen(fic,"w");
    fwrite(t,sizeof(char),nb,f);
    fclose(f);
}

void sauvegarde_deplacements(const t_tabDeplacement t,int nbDeplacements)
{
    printf("Sauvegarder les deplacements ? (o/n)\n");

    char rep=_getch();

    if(rep=='o')
    {
        char fichier[50];

        printf("\nNom du fichier .dep : ");
        scanf("%s",fichier);

        enregistrerDeplacements(t,nbDeplacements,fichier);
    }
}