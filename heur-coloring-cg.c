/* heur-coloring-cg.c
codigo exemplo de aplicacao de heuristica com o metodo de geracao de colunas
 usando o solver do glpk para resolver o problema mestre restrito e o pricing

Encontra a relaxacao linear do problema da coloracao de vertices para um grafo G de entrada. Em seguida, resolve o PLI
considerando apenas as colunas geradas na relaxação linear do nó raiz.
A coloracao atribui uma cor para cada vertice de G de modo que vertices adjacentes (vizinhos, ou seja, que tem aresta conectando-os) tenham cores diferentes.

Formato do arquivo de entrada:
n m (n=|V| e m=|E| do grafo)
i j (uma linha para cada aresta (i,j) do grafo)

Modelo de PLI para coloracao:

min lambda1 + lambda2 + lambda3 + ... + lambdap
s.a.:
   \sum_{p \in P} aip lambda_p >=1, para cada vertices i em G

onde aip = 1, se o vertice i estah no conjunto independente p
lambdap = 1, se o conjunto independente p eh selecionado na coloracao

Obs.:
1) Ha uma variavel p para cada conjunto independente possivel (que potencialmente eh exponencial em n=|V|).
2) A ideia eh comecar com um subconjunto P contendo apenas alguns conjuntos independentes (de fato, um conjunto indenpendente de tamanho unitario eh criado para conter cada vertice).
3) O problema de pricing eh executado para encontrar um conjunto independente de custo reduzido negativo para entrar em P.
4) Se nao houver nenhum conj. indep. com esta propriedade, o programa para, pois a solucao atual eh a solucao otima da relaxacao linear.

*/

#include <stdio.h>
#include <stdlib.h>
#include <glpk.h>
#include <time.h>

#define EPSILON 0.00000001

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) 
#endif

int n, m;
int **A;
typedef struct{
  int i;
  int j;
} edgeT;
edgeT *E;

int carga_instancia(FILE *fin)
{
  int i,j, edges;

  fscanf(fin, "%d %d\n", &n, &m);
  E=(edgeT*)malloc(sizeof(edgeT)*m);
  A=(int**)malloc(sizeof(int*)*n);
  for(i=0;i<n;i++){
    A[i]=(int*)malloc(sizeof(int)*n);
    for(j=0;j<n;j++){
      A[i][j]=-1;
    }
  }
  edges=0;
  while(!feof(fin)){
    fscanf(fin, "%d %d\n", &i, &j);
    --i;--j;
    A[i][j]=A[j][i]=edges;
    E[edges].i=i;
    E[edges].j=j;
    edges++;
  }

#ifdef DEBUG
  for(i=0;i<m;i++){
    printf("\nEdge%d: (%d,%d)", i+1, E[i].i+1, E[i].j+1);
  }
#endif

  if(edges!=m){
    return 0;
  }
  return 1;
}

int carga_pmr(glp_prob **lp)
{
  int *ia, *ja, i, nz,k;
  double *ar;
  int rows, cols;
  char name[100];

  rows=n; // restricao de cobertura, um para cada vertice
  cols=n; // inicialmente, n c.i.'s unitarios, cada um contendo 1 vertice de V
  
  // Aloca matriz de coeficientes
  ia=(int*)malloc(sizeof(int)*(n+1));
  ja=(int*)malloc(sizeof(int)*(n+1));
  ar=(double*)malloc(sizeof(double)*(n+1));
  
  
      // Cria problema de PL
  *lp = glp_create_prob();
  glp_set_prob_name(*lp, "coloring");
  glp_set_obj_dir(*lp, GLP_MIN);
  
  // Configura restricoes
  glp_add_rows(*lp, rows);
  for(i=0;i<rows;i++){
    name[0]='\0';
    sprintf(name,"cover%d", i+1);
    glp_set_row_name(*lp, i+1, name);
    glp_set_row_bnds(*lp, i+1, GLP_LO, 1, 0);
  }

  // Configura variaveis
  glp_add_cols(*lp, cols);
  for(i=0;i<cols;i++){
    name[0]='\0';
    sprintf(name,"lambda%d", i+1);
    glp_set_col_name(*lp, i+1, name);
    glp_set_col_bnds(*lp, i+1, GLP_LO, 0.0, 0.0);
    glp_set_obj_coef(*lp, i+1, 1.0);
    glp_set_col_kind(*lp, i+1, GLP_BV); // especifica que a variaval lambda eh binaria
  }
  
  // Configura matriz de coeficientes
  for(nz=1,k=0;k<n;k++){ // para cada vertice
    ia[nz] = k+1; ja[nz] = k+1; ar[nz] =  1; /* a[i][i] =  1 */
    nz++;
  }


  // Carrega PL
  PRINTF("\nCarregando mestre...");
  glp_load_matrix(*lp, nz-1, ia, ja, ar);

#ifdef DEBUG
  // Grava PL do PMR inicial
  PRINTF("\n---- LP do PMR inicial gravado em coloracao-pmr.lp");
  glp_write_lp(*lp, NULL,"coloracao-pmr.lp");
  getchar();
#endif
  
  // desaloca matrizes
  free(ia); free(ja); free(ar);
  return 1;
}

int carga_pricing(glp_prob **lp)
{
  int *ia, *ja, rows, cols, i, nz;
  double *ar;
  char name[80];// nome da restricao

  rows=m; // uma restricao para cada aresta
  cols=n; // uma variavel para cada vertice

  // Aloca matriz de coeficientes
  ia=(int*)malloc(sizeof(int)*(2*m+1));
  ja=(int*)malloc(sizeof(int)*(2*m+1));
  ar=(double*)malloc(sizeof(double)*(2*m+1));
      
  // Cria problema de PL
  *lp = glp_create_prob();
  glp_set_prob_name(*lp, "IS");
  glp_set_obj_dir(*lp, GLP_MAX);

  // Configura restricoes
  glp_add_rows(*lp, rows);
  for(i=0;i<m;i++){
    sprintf(name, "edge%d", i+1);
    glp_set_row_name(*lp, i+1, name);
    glp_set_row_bnds(*lp, i+1, GLP_UP, 0.0, 1);
  }
  
  // Configura variaveis
  glp_add_cols(*lp, cols);
  for(i=0;i<cols;i++){
    name[0]='\0';
    sprintf(name,"x%d", i+1); /* as n variaveis referem-se `as variaveis x1, x2, ..., xn*/
    glp_set_col_name(*lp, i+1, name); 
    glp_set_col_bnds(*lp, i+1, GLP_DB, 0.0, 1.0);
    glp_set_obj_coef(*lp, i+1, 1);
    glp_set_col_kind(*lp, i+1, GLP_BV); // especifica que a variaval xi eh binaria
  }

  // Configura matriz de coeficientes ... xi + xj <= 1, para cada aresta (i,j)
  nz=1;
  for(i=0;i<m;i++){
    ia[nz]=i+1;
    ja[nz]=E[i].i+1;
    ar[nz]=1;
    nz++;
    ia[nz]=i+1;
    ja[nz]=E[i].j+1;
    ar[nz]=1;
    nz++;
  }

  // Carrega PL
  glp_load_matrix(*lp, nz-1, ia, ja, ar);

  // libera memoria
  free(ia); free(ja); free(ar);
  return 1;
}




/* eof */