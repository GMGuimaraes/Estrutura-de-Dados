#include <stdio.h>

//Definição do nó
struct no
{
	int chave;
	no* prox;
};

//Definição de uma lista simplesmente encadeada sem cabeça
struct lsc
{
	no* inicio;
};

//Prototipação das funções
lsc* cria_nova_lsc();
void imprime_lsc(lsc*);
no* busca_lsc(lsc*, int);
void insere_antes_lsc(lsc*, int, int);
void remove_chave_lsc(lsc*, int);

//Função principal 
int main()
{
	lsc* lista = cria_nova_lsc();
	
	imprime_lsc(lista);
	
	insere_antes_lsc(lista, 10, 100);
	imprime_lsc(lista);
	
	no* p = busca_lsc(lista, 10);
	
	if (p)
		printf("A chave %d foi encontrada!\n", 10);
	else
		printf("A chave %d não foi encontrada!\n", 10);
		
	insere_antes_lsc(lista, 20, 10);
	imprime_lsc(lista);
	
	insere_antes_lsc(lista, 30, 10);
	imprime_lsc(lista);
	
	remove_chave_lsc(lista, 30);
	imprime_lsc(lista);
	
	remove_chave_lsc(lista, 20);
	imprime_lsc(lista);
		
	return 0;
}

//Cria nova LSC vazia
lsc* cria_nova_lsc()
{
	lsc* l = new lsc;
	
	l->inicio = NULL;
	
	return l;
}

void imprime_lsc(lsc* l)
{
	if (!l->inicio)
		printf("Lista vazia!");
	else
		for (no* ptr = l->inicio; ptr; ptr = ptr->prox)
			printf("%d ", ptr->chave);
			
	printf("\n");
}

no* busca_lsc(lsc* l, int chave)
{
	no* ptr;
	
	ptr = l->inicio;
	
	while (ptr && ptr->chave != chave)
		ptr = ptr->prox;
		
	return ptr;
}

void insere_antes_lsc(lsc* l, int nova_chave, int x)
{
	no *p, *q, *novo;
	
	novo = new no;
	novo->chave = nova_chave;
	
	p = NULL;
	q = l->inicio;
	
	while (q && q->chave != x)
	{
		p = q;
		q = q->prox;
	}
	
	novo->prox = q;
	 
	if (p)
		p->prox = novo;
	else
		l->inicio = novo;
}

void remove_chave_lsc(lsc* l, int chave)
{
	no *p, *q;
	
	p = NULL;
	q = l->inicio;
	
	while (q && q->chave != chave)
	{
		p = q;
		q = q->prox;
	}
	
	if (q)//Encontrei, vou remover
	{
		if (p)
			p->prox = q->prox;
		else
			l->inicio = q->prox;
	
		delete q;
	}
	else
		printf("Chave não encontrada!\n");
	
	
}