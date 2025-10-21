#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
typedef enum {MOT, TUB, INF, SUP, SPP, NL, FIN} LEX;
static LEX getlex(char *mot){
    enum {Neutre, Spp, Equote, Emot } etat=Neutre;
    int c;
    char *w;
    w=mot;
    
    printf("%s>$", getenv("PWD"));
    
    while ((c=getchar()) != EOF){
        switch(etat){
        case Neutre:
            switch(c){
            case '<':
                return (INF);
            case '>':
                etat=Spp;
                continue;
            case '|':
                return (TUB);
            case '"':
                etat=Equote;
                continue;
            case ' ':
            case '\t':
                continue;
            case '\n':
                return(NL);
            default:
                etat=Emot;
                *w++=c;
                continue;
            }
        case Spp:
            if(c=='>') 
                return(SPP);
            ungetc(c,stdin);
            return(SUP);
        case Equote:
            switch(c){
            case '\\':
                *w++=c;
                continue;
            case '"':
                *w='\0';
                return(MOT);
            default:
                *w++=c;
                continue;
            }
        case Emot:
            switch(c){
            case '|':
            case '<':
            case '>':
            case ' ':
            case '\t':
            case '\n':
                ungetc(c,stdin);
                *w='\0';
                return(MOT);
            default:
                *w++=c;
                continue;
            }
        }
    }
    return(FIN);
}

void main(int argc, char *argv[]){
    char mot[200];
    while(1)
        switch(getlex(mot)){
        case MOT:
            printf("MOT: %s\n",mot);
            break;
        case TUB:
            printf("TUBE\n");
            break;
        case INF:
            printf("REDIRECTION ENTREE\n");
            break;
        case SUP:
            printf("REDIRECTION SORTIE\n");
            break;
        case SPP:
            printf("REDIRECTION AJOUT\n");
            break;
        case NL:
            printf("NOUVELLE LIGNE \n");
            break;
        case FIN:
            printf("FIN \n");
            exit(0);
        }
}
