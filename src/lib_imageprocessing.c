
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "imageprocessing.h"
#include <FreeImage.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>

#define n_threads 8
#define n_processos 500

pthread_mutex_t trava;

/*
imagem abrir_imagem(char *nome_do_arquivo);
void salvar_imagem(char *nome_do_arquivo);
void liberar_imagem(imagem *i);
 */

imagem abrir_imagem(char *nome_do_arquivo) {
  FIBITMAP *bitmapIn;
  int x, y;
  RGBQUAD color;
  imagem I;

  bitmapIn = FreeImage_Load(FIF_JPEG, nome_do_arquivo, 0);

  if (bitmapIn == 0) {
    printf("Erro! Nao achei arquivo - %s\n", nome_do_arquivo);
  } else {
    printf("Arquivo lido corretamente!\n");
   }

  x = FreeImage_GetWidth(bitmapIn);
  y = FreeImage_GetHeight(bitmapIn);

  I.width = x;
  I.height = y;
  I.r = malloc(sizeof(float) * x * y);
  I.g = malloc(sizeof(float) * x * y);
  I.b = malloc(sizeof(float) * x * y);

   for (int i=0; i<x; i++) {
     for (int j=0; j <y; j++) {
      int idx;
      FreeImage_GetPixelColor(bitmapIn, i, j, &color);

      idx = i + (j*x);

      I.r[idx] = color.rgbRed;
      I.g[idx] = color.rgbGreen;
      I.b[idx] = color.rgbBlue;
    }
   }
  return I;

}

void liberar_imagem(imagem *I) {
  free(I->r);
  free(I->g);
  free(I->b);
}

typedef struct __brilho_args
{
    float intensidade;
    imagem* I;
    unsigned int linha;
} brilho_args;

void altera_linha(unsigned int linha, imagem *I, float intensidade){
    //printf("BANANA\n");
    unsigned int i,idx;
    for (i=0; i<(I->width); i++){
	  idx=linha*I->width+i;
	  if(I->r[idx] * intensidade <= 255 )
	      I->r[idx] *= intensidade;
	  else I->r[idx] = 255;
	  if(I->g[idx] * intensidade <= 255 )
	      I->g[idx] *= intensidade;
	  else I->g[idx] = 255;
	  if(I->b[idx] * intensidade <= 255 )
	      I->b[idx] *= intensidade;
	  else I->b[idx] = 255;
    }
}

void altera_coluna(unsigned int coluna, imagem *I, float intensidade){
    unsigned int i,idx;
    for (i=0; i<(I->height); i++){
	  idx=coluna+i*I->width;
	  if(I->r[idx] * intensidade <= 255 )
	      I->r[idx] *= intensidade;
	  else I->r[idx] = 255;
	  if(I->g[idx] * intensidade <= 255 )
	      I->g[idx] *= intensidade;
	  else I->g[idx] = 255;
	  if(I->b[idx] * intensidade <= 255 )
	      I->b[idx] *= intensidade;
	  else I->b[idx] = 255;
    }
}

void* brilho_thread(void* argumentos)
{
    brilho_args* informacoes=(brilho_args*) argumentos; 
    float intensidade=informacoes->intensidade;
    imagem* I=informacoes->I;
    unsigned int i,idx;
    unsigned int linha=informacoes->linha;
    pthread_mutex_unlock(&trava);
    altera_linha(linha, I, intensidade);
    return NULL;
}

void salvar_imagem(char *nome_do_arquivo, imagem *I) {
  FIBITMAP *bitmapOut;
  RGBQUAD color;

  printf("Salvando imagem %d por %d...\n", I->width, I->height);
  bitmapOut = FreeImage_Allocate(I->width, I->height, 24, 0, 0, 0);

   for (int i=0; i<I->width; i++) {
     for (int j=0; j<I->height; j++) {
      int idx;

      idx = i + (j*I->width);
      color.rgbRed = I->r[idx];
      color.rgbGreen = I->g[idx];
      color.rgbBlue = I->b[idx];

      FreeImage_SetPixelColor(bitmapOut, i, j, &color);
    }
  }

  FreeImage_Save(FIF_JPEG, bitmapOut, nome_do_arquivo, JPEG_DEFAULT);
  printf("Imagem salva com sucesso\n");
}

void aplicar_brilho_threads(imagem *I, float intensidade)
{
    /* Muda o brilho da imagem por um fator linear intensidade que
     * pode ir de 0 a 1 */
     brilho_args* argumentos=alloca(sizeof(brilho_args));
     argumentos->intensidade=intensidade;
     argumentos->I=I;
     pthread_t threads[n_threads];
     unsigned int linha=0;
     unsigned int i,j;
     do{
         for (i=0; i<n_threads; i++){
            pthread_mutex_lock(&trava);
            if(linha==I->height){
                for (j=0; j<i; ++j)
                    pthread_join(threads[j],NULL);
                return; // Acabou
            }
            argumentos->linha=linha++;
            pthread_create(&threads[i],NULL,brilho_thread,argumentos); 
         }
         for (i=0; i<n_threads; i++)
            pthread_join(threads[i],NULL);
     } while(1);
}

void aplicar_brilho_processos(imagem *I, float intensidade)
{
    /* Muda o brilho da imagem por um fator linear intensidade que
     * pode ir de 0 a 1 */
    pid_t pid;
    unsigned int i,j;
    unsigned int linha=0;
    // Readable and writable
    int protection = PROT_READ | PROT_WRITE;
    
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    imagem* shared_memory=mmap(NULL,sizeof(*I), protection, visibility, 0, 0);
    shared_memory->width=I->width;
    shared_memory->height=I->height;
    (shared_memory->r)= (float*) mmap(NULL,sizeof(float)*I->height*I->width, protection, visibility, 0, 0);
    (shared_memory->g)= (float*) mmap(NULL,sizeof(float)* I->height*I->width, protection, visibility, 0, 0);
    (shared_memory->b)= (float*) mmap(NULL,sizeof(float)* I->height*I->width, protection, visibility, 0, 0);

    for(i=0;i<(I->width*I->height);++i){
        /* Copia imagem para memória compartilhada */
        shared_memory->r[i] = I->r[i];
        shared_memory->g[i] = I->g[i];
        shared_memory->b[i] = I->b[i];
    }


    while(linha<I->height){
        for(i=0;i<n_processos;++i){
            if(linha==I->height){
                for(j=0;j<i;j++)
                    wait(NULL);
                goto copia;
            }
            pid = fork(); 
            if(pid==0){ // Filho
                //printf("%u\n", linha);
                altera_linha(linha, shared_memory, intensidade);
                exit(0);
            }
            linha++;
        }
        for(i=0;i<n_processos;++i){
            wait(NULL);
        }
    }
copia:
    for(i=0;i<(I->width*I->height);++i){
        /* Copia imagem da memória compartilhada para a imagem original */
        I->r[i] = shared_memory->r[i];
        I->g[i] = shared_memory->g[i];
        I->b[i] = shared_memory->b[i];
    }
}

void aplicar_brilho_linhas(imagem* I, float intensidade)
{
    for(unsigned int linha=0;linha<I->height;++linha)
        altera_linha(linha, I, intensidade);
}

void aplicar_brilho_colunas(imagem* I, float intensidade)
{
    for(unsigned int colunas=0;colunas<I->width;++colunas)
        altera_coluna(colunas, I, intensidade);
}

void printa_max(imagem *I){

    unsigned int i;
    float max = 0;
    for (i=0; i<(I->width)*(I->height); i++) {
        if(I->r[i] + I->g[i] + I->b[i] > max)
            max = I->r[i] + I->g[i] + I->b[i];
    }
    printf("O pixel de intensidade maxima vale %f\n",max);

}
