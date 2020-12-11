#include <stdio.h>

//Estrutura de um nó
struct no
{
	int chave;
	no* prox;
};

//Estrutura de uma lista com cabeça
struct lcc
{
	no* cabeca;
};

//Prototipação
lcc* cria_nova_lcc();
void imprime_lcc(lcc*);
void insere_lcc(lcc*, int, no*);
no* busca_lcc(lcc*, int);
void remove_sucessor_lcc(lcc*, no*);
void remove_chave_lcc(lcc*, int);
void insere_antes_lcc(lcc*, int, int);
void insere_pos_lcc(lcc*, int, int);

//Função principal
int main()
{
	lcc* lista = cria_nova_lcc();
	imprime_lcc(lista);
	
	insere_lcc(lista, 10, lista->cabeca);
	imprime_lcc(lista);
	
	insere_lcc(lista, 20, lista->cabeca->prox);
	imprime_lcc(lista);
	
	no* p = busca_lcc(lista, 20);
	
	if (p)
		insere_lcc(lista, 30, p);
	imprime_lcc(lista);
	
	p = busca_lcc(lista, 30);
	
	if (p)
		insere_lcc(lista, 40, p);
	imprime_lcc(lista);
	
	remove_sucessor_lcc(lista, p);
	imprime_lcc(lista);
	
	/*p = busca_lcc(lista, 10);
	
	remove_sucessor_lcc(lista, p);
	imprime_lcc(lista);
	
	remove_sucessor_lcc(lista, lista->cabeca);
	imprime_lcc(lista);
	
	remove_sucessor_lcc(lista, lista->cabeca);
	imprime_lcc(lista);
	
	insere_lcc(lista, 10, lista->cabeca);
	imprime_lcc(lista);
	*/
	
	remove_chave_lcc(lista, 20);
	imprime_lcc(lista);
	
	remove_chave_lcc(lista, 40);
	imprime_lcc(lista);
	
	remove_chave_lcc(lista, 10);
	imprime_lcc(lista);
	
	remove_chave_lcc(lista, 30);
	imprime_lcc(lista);
	
	insere_lcc(lista, 50, lista->cabeca);
	imprime_lcc(lista);
	
	insere_antes_lcc(lista, 60, 50);
	imprime_lcc(lista);
	
	insere_antes_lcc(lista, 70, 1000);
	imprime_lcc(lista);
	
	insere_pos_lcc(lista, 80, 2);
	imprime_lcc(lista);
	
	return 0;
}

//Função que cria e retorna um ponteiro para uma nova lista com cabeça vazia
lcc* cria_nova_lcc()
{
	lcc* l = new lcc;//Aloca a nova lista
	
	l->cabeca = new no;//Cria nó cabeca
	l->cabeca->prox = NULL;//Faz o prox de cabeça ser NULL, para indicar que a lista está vazia
	
	return l;	
}

//Função que imprime os elementos de uma lista com cabeça
void imprime_lcc(lcc* l)
{
	if (l->cabeca->prox == NULL)//A lista está vazia if (!l->cabeca->prox)
		printf("Lista vazia!");
	else
		for (no* ptr = l->cabeca->prox; ptr != NULL; ptr = ptr->prox)
			printf("%d ", ptr->chave);
			
	printf("\n");
}

//Insere depois de um dado nó
void insere_lcc(lcc* l, int y, no* p)
{
	no* novo = new no;
	
	novo->chave = y;
	novo->prox = p->prox;
	p->prox = novo;
}

//Busca uma chave na lista
no* busca_lcc(lcc* l, int y)
{
	no* ptr = l->cabeca->prox;
	
	while (ptr != NULL && ptr->chave != y)
		ptr = ptr->prox;
		
	return ptr;
}

//Remove o sucessor de um dado nó da lista
void remove_sucessor_lcc(lcc* l, no* p)
{
	no* lixo;
	
	lixo = p->prox;
	p->prox = lixo->prox;
	
	delete lixo;
}

//Remove o nó com a chave solicitada
void remove_chave_lcc(lcc* l, int chave)
{
	no *p, *q;
	
	p = l->cabeca;
	q = l->cabeca->prox;//q = p->prox;
	
	while (q && q->chave != chave)
	{
		p = q;
		q = q->prox;
	}
	
	if (q)//Achei, posso remover!
	{
		p->prox = q->prox;
		
		delete q;
	}
}

void insere_antes_lcc(lcc* l, int nova_chave, int x)
{
	no *p, *q, *novo;
	
	novo = new no;
	novo->chave = nova_chave;
	
	p = l->cabeca;
	q = l->cabeca->prox;
	
	while (q && q->chave != x)
	{
		p = q;
		q = q->prox;
	}
	
	novo->prox = q;
	p->prox = novo;
}

void insere_pos_lcc(lcc* l, int nova_chave, int pos)
{
	no *p, *q, *novo;
	int i;
	
	novo = new no;
	novo->chave = nova_chave;
	
	p = l->cabeca;
	q = l->cabeca->prox;
	i = 0;
	
	while (q && i < pos)
	{
		p = q;
		q = q->prox;
		i++;
	}
	
	novo->prox = q;
	p->prox = novo;
}