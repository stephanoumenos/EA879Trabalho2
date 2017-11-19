%{
#include <stdio.h>
#include "imageprocessing.h"
#include <FreeImage.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

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
        char str_temp[1000];
        printf("Multiplicando o brilho da imagem %s por %s\n", $3, $5);
        struct timeval tempo_inicial, tempo_final, diferenca;
        // Thread
        printf("Usando múltiplas threads...\n\n");
        imagem I = abrir_imagem($3);
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_threads(&I, atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo para threads foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        strcpy(str_temp, "thread_");
        strcat(str_temp, $3);
        //salvar_imagem(str_temp, &I);
        liberar_imagem(&I);
        // Processos
        printf("Usando múltiplos processos...\n\n");
        I = abrir_imagem($3);
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_processos(&I, atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo para processos foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        strcpy(str_temp, "processos_");
        strcat(str_temp, $3);
        //salvar_imagem(str_temp, &I);
        liberar_imagem(&I);
        // Linhas
        printf("Varrendo pelas linhas...\n\n");
        I = abrir_imagem($3);
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_linhas(&I, atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo pelas linhas foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        strcpy(str_temp, "linhas_");
        strcat(str_temp, $3);
        //salvar_imagem(str_temp, &I);
        liberar_imagem(&I);
        // Colunas
        printf("Varrendo pelas colunas...\n\n");
        I = abrir_imagem($3);
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_colunas(&I, atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo pelas colunas foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        strcpy(str_temp, "colunas_");
        strcat(str_temp, $3);
        //salvar_imagem(str_temp, &I);
        liberar_imagem(&I);
    }

    | STRING IGUAL STRING DIVIDIDO NUMERO{
        printf("Dividindo o brilho da imagem %s por %s\n", $3, $5);
        struct timeval tempo_inicial, tempo_final, diferenca;
        imagem I = abrir_imagem($3);
        // Thread
        printf("Usando múltiplas threads...\n");
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_threads(&I, 1/atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo para threads foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        //salvar_imagem("thread.jpg", &I);
        liberar_imagem(&I);
        // Linhas
        I = abrir_imagem($3);
        printf("Varrendo pelas linhas...\n");
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_linhas(&I, 1/atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo pelas linhas foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        //salvar_imagem("linhas.jpg", &I);
        liberar_imagem(&I);
        // Colunas
        I = abrir_imagem($3);
        printf("Varrendo pelas colunas...\n");
        gettimeofday(&tempo_inicial,NULL);
        aplicar_brilho_colunas(&I, 1/atof($5));
        gettimeofday(&tempo_final,NULL);
        timersub(&tempo_final,&tempo_inicial,&diferenca);
        printf("O tempo pelas colunas foi: %ld.%06ld segundos\n", diferenca.tv_sec, diferenca.tv_usec);
        salvar_imagem("colunas.jpg", &I);
        //salvar_imagem($1, &I);
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
