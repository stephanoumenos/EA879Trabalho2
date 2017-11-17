
#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H


typedef struct {
  unsigned int width, height;
  float *r, *g, *b;
} imagem;

imagem abrir_imagem(char *nome_do_arquivo);
void salvar_imagem(char *nome_do_arquivo, imagem *I);
void liberar_imagem(imagem *i);
void altera_linha(unsigned int linha, imagem* I, float intensidade);
void altera_coluna(unsigned int coluna, imagem* I, float intensidade);
void aplicar_brilho_threads(imagem *I, float intensidade);
void printa_max(imagem *I);
void aplicar_brilho_processos(imagem* I, float intensidade);
void aplicar_brilho_linhas(imagem* I, float intensidade);
void aplicar_brilho_colunas(imagem* I, float intensidade);

#endif
