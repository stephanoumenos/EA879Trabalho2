
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "imageprocessing.h"
#include <FreeImage.h>
#include <alloca.h>

#define n_threads 4

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

void* brilho_thread(void* argumentos)
{
    brilho_args* informacoes=(brilho_args*) argumentos; 
    float intensidade=informacoes->intensidade;
    imagem* I=informacoes->I;
    unsigned int i,idx;
    unsigned int linha=informacoes->linha;
    pthread_mutex_unlock(&trava);
    for (i=0; i<(I->width); i++){
        idx=linha*I->width+i;
        if(I->r[idx] * intensidade <= 255 ){
            I->r[idx] *= intensidade;
        }
        else I->r[idx] = 255;
        if(I->g[idx] * intensidade <= 255 ){
            I->g[idx] *= intensidade;
        }
        else I->g[idx] = 255;
        if(I->b[idx] * intensidade <= 255 ){
            I->b[idx] *= intensidade;
        }
        else I->b[idx] = 255;
    }
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
     unsigned int i;
     do{
         for (i=0; i<n_threads; i++){
            pthread_mutex_lock(&trava);
            argumentos->linha=linha++;
            pthread_create(&threads[i],NULL,brilho_thread,argumentos); 
         }
         for (i=0; i<n_threads; i++)
            pthread_join(threads[i],NULL);
     } while(linha<I->height);
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
