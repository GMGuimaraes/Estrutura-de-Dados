#include <stdio.h>

struct no
{
	int chave;
	no* prox;
};

struct circlcc
{
	no* cabeca;
};

circlcc* cria_nova_circlcc();
void imprime_circlcc(circlcc*);
no* busca_circlcc(circlcc*, int);

int main()
{
	circlcc* lista = cria_nova_circlcc();
	
	
}

circlcc* cria_nova_circlcc()
{
	circlcc* l = new circlcc;
	
	l->cabeca = new no;
	l->cabeca->prox = l->cabeca;
	
	return l;
}

//Função que imprime os elementos de uma lista com cabeça circular
void imprime_circlcc(circlcc* l)
{
	if (l->cabeca->prox == l->cabeca)
		printf("Lista vazia!");
	else
		for (no* ptr = l->cabeca->prox; ptr != l->cabeca; ptr = ptr->prox)
			printf("%d ", ptr->chave);
			
	printf("\n");
}

no* busca_circlcc(circlcc* l, int chave)
{
	no* ptr = l->cabeca->prox;
	
	l->cabeca->chave = chave;
	
	while (ptr->chave != chave)
		ptr = ptr->prox;
		
	if (ptr == l->cabeca)
		return NULL;
		
	return ptr;
}