%{
#include <stdio.h>
#include "imageprocessing.h"
#include <FreeImage.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

void yyerror(char *c);
int yylex(void);

%}
%union {
  char    strval[50];
  int     ival;
}
%token <strval> STRING NUMERO
%token <ival> VAR IGUAL EOL ASPA VEZES DIVIDIDO COLCHETE_ESQ COLCHETE_DIR
%left SOMA

%%

PROGRAMA:
        PROGRAMA EXPRESSAO EOL
        |
        ;

EXPRESSAO:
    | STRING IGUAL STRING VEZES NUMERO{
        printf("Multiplicando o brilho da imagem %s por %s\n", $3, $5);
        struct timeval tempo_inicial, tempo_final, diferenca;
        imagem I = abrir_imagem($3);
        printf("Usando múltiplas threads...\n");
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_threads(&I, atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo para threads foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        salvar_imagem($1, &I);
        liberar_imagem(&I);
    }

    | STRING IGUAL STRING DIVIDIDO NUMERO{
        printf("Dividindo o brilho da imagem %s por %s\n", $3, $5);
        imagem I = abrir_imagem($3);
        aplicar_brilho_colunas(&I, 1/atof($5));
        salvar_imagem($1, &I);
        liberar_imagem(&I);
    }

    | STRING IGUAL STRING {
        printf("Copiando %s para %s\n", $3, $1);
        imagem I = abrir_imagem($3);
        printf("Li imagem %d por %d\n", I.width, I.height);
        salvar_imagem($1, &I);
        liberar_imagem(&I);
                          }
    | COLCHETE_ESQ STRING COLCHETE_DIR {
        printf("Encontrando intensidade maxima de %s\n", $2);
        imagem I = abrir_imagem($2);
        printf("Li imagem %d por %d\n", I.width, I.height);
        printa_max(&I);
        liberar_imagem(&I);
    }
    ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}

int main() {
  FreeImage_Initialise(0);
  yyparse();
  return 0;

}
