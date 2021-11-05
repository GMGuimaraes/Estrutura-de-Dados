/* coloring.c
codigo exemplo de uso do glpk para MIP (mixed integer programming)

Encontra uma coloraca otima (com menor numero de cores) para um grafo G de entrada.
A coloracao atribui uma cor para cada vertice de G de modo que vertices adjacentes (vizinhos, ou seja, que tem aresta conectando-os) tenham cores diferentes.

Formato do arquivo de entrada:
n m (n=|V| e m=|E| do grafo)
i j (uma linha para cada aresta (i,j) do grafo)

Modelo de PLI para coloracao:

min y1 + y2 + y3 + ... + yn
s.a.:
   xi1 + xj1 <=y1, para cada aresta (i,j) em G
   xi2 + xj2 <=y2, para cada aresta (i,j) em G
   ...
   xin + xjn <=yn, para cada aresta (i,j) em G
   xi1 + xi2 + xi3 + ... + xin >=1, para cada vertice i em G

variaveis: yi e xik sao binarias.

Obs.:
1) Ha uma variavel y para cada cor possivel (cores 1 a n). Neste caso, yi=1, se e somente se a cor i for selecionada para a coloracao otima.
2) Ha uma variavel xik para cada vertice i e cor k. Neste caso, xik=1, se e somente se o vertice i for colorido com a cor k.
3) A ultima restricao forca cada vertice ser colorido pelo menos com 1 cor.
4) As demais restricoes forcam que os vertices adjacentes tenham cores diferentes.

IMPORTANTE! O modelo não trata os casos em que o grafo de entrada possui vertices isolados. 
Ha varias solucoes, por exemplo:

1) Como todos os vertices isolados podem ser coloridos com uma mesma cor, basta remove-los do grafo e usar uma das cores utilizadas para colori-las.
Note que se o grafo tiver apenas vertices isolados, uma cor deve ser usada para colori-las!

2) Ou, inclua as restricoes: xik <= yk, para cada vertice isolado i e cada cor k. (TAREFA!)

3) Ou inclua as restricoes: x1k + x2k + x3k + ... + xnk <= yk, para cada cor k.

*/

#include <stdio.h>
#include <stdlib.h>
#include <glpk.h>
#include <time.h>

#define EPSILON 0.000001

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) 
#endif

int carga_instancia(FILE* fin, int *n, int *m, int ***A)
{
  int i,j, count;

  fscanf(fin,"%d %d\n", n, m);
  PRINTF("%d %d\n", *n,*m);

  *A=(int**) malloc(sizeof(int*)*(*n+1));
  for(i=0;i<*n+1;i++){
    (*A)[i]=(int*) malloc(sizeof(int)*(*n+1));
    for(j=0;j<*n+1;j++){
      (*A)[i][j]=0;
    }
  }

  count=0;
  while(!feof(fin)){
    fscanf(fin, "%d %d\n", &i,&j);
    PRINTF("%d %d\n", i,j);
    if (i<1 || i>*n || j < 1 || j > *n){
      return 0;
    }
    if(!(*A)[i][j]){
      (*A)[i][j]=(*A)[j][i]=1;
      count++;
    }
  }
  if(count!=*m){
    if(2*count==*m){
      *m=count;
      return 1;
    }
    return 0;
  }
  return 1;
}


int carga_lp(glp_prob **lp, int n, int m, int **A)
{
  int *ia, *ja, rows, cols, i,j, nz, k, aresta;
  double *ar;
  char name[80];

  rows=m*n+n; // xik+xjk <= yk  e sum_{k}x_ik >=1, each i
  cols=n+n*n; // y_k e x_ik

  // Aloca matriz de coeficientes
  ia=(int*)malloc(sizeof(int)*(3*(m*n)+n*n+1));
  ja=(int*)malloc(sizeof(int)*(3*(m*n)+n*n+1));
  ar=(double*)malloc(sizeof(double)*(3*(m*n)+n*n+1));
      
  // Cria problema de PL
  *lp = glp_create_prob();
  glp_set_prob_name(*lp, "coloring");
  glp_set_obj_dir(*lp, GLP_MIN);

  // Configura restricoes
  glp_add_rows(*lp, rows);
  for(i=0;i<m*n;i++){ /* edge1: x11 + x21 - y1 <= 0, aresta (1,2) */
    name[0]='\0';
    sprintf(name,"edge%d", i+1);
    glp_set_row_name(*lp, i+1, name);
    glp_set_row_bnds(*lp, i+1, GLP_UP, 0.0, 0.0);
  }

  for(j=0;j<n;j++,i++){ /* cover1: x11 + x12 + x13 + ... + x1n >= 1*/
    name[0]='\0';
    sprintf(name,"cover%d", i+1);
    glp_set_row_name(*lp, i+1, name);
    glp_set_row_bnds(*lp, i+1, GLP_FX, 1.0, 1.0);
  }

  // Configura variaveis
  glp_add_cols(*lp, cols);
  for(i=0;i<n;i++){
    name[0]='\0';
    sprintf(name,"y%d", i+1); /* as primeiras n variaveis referem-se `as variaveis y1, y2, ..., yn*/
    glp_set_col_name(*lp, i+1, name);
    glp_set_col_bnds(*lp, i+1, GLP_DB, 0.0, 1.0);
    glp_set_obj_coef(*lp, i+1, 1.0);
    glp_set_col_kind(*lp, i+1, GLP_BV); // especifica que a variaval yi eh binaria
  }

  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      name[0]='\0';
      sprintf(name,"x%d,%d", j+1, i+1); 
      /* as demais n*n variaveis referem-se `as variaveis 
	 x11, x21, x31, ..., xn1,x21,...,x2n, ..., xn1,..., xnn, nesta ordem 
	 ou seja,
	 a variavel de indice n+1 e' a variavel x11
	 a variavel de indice n+2 e' a variavel x12
	 ...
	 genericamente falando, a variavel de indice n+i*n+j+1 e' a variavel xij
	 ...
	 a variavel de indice n+n*n e' a variavel xnn
      */
      glp_set_col_name(*lp, n+i*n+j+1, name);
      glp_set_col_bnds(*lp, n+i*n+j+1, GLP_LO, 0.0, 0.0);
      //      glp_set_obj_coef(*lp, n+i*n+j+1, 0.0);
      glp_set_col_kind(*lp, n+i*n+j+1, GLP_BV); // especifica que a variavel xij eh binaria
    }
  }

  // Configura matriz de coeficientes ...

  //  ... nas restricoes de aresta (edge), ou seja, xik + xjk <= yk, para cada aresta (i,j) e cor k
  /* note que as restricoes xik + xjk <= yk eh equivalente a 1.0xik + 1.0xjk - 1.0yk <= 0.0 */
  for(nz=1,k=0;k<n;k++){ // para cada cor k
    for(aresta=1,i=1;i<=n;i++){ // para cada vertice i
      for(j=i+1;j<=n;j++){ // para cada vertice j
	if(A[i][j]>0){ // se existir aresta (i,j) no grafo entao coloca os coeficientes das variaveis na restricao de aresta
	  /* o coeficiente da (n+i+k*n)-esima variavel, ou seja, da variavel xik na restricao de aresta e' 1.0 */
	  ia[nz] = aresta+k*m; ja[nz] = n+i+k*n; ar[nz++] =  1.0; 
	  /* o coeficiente da (n+j+k*n)-esima variavel, ou seja, da variavel xjk na restricao de aresta e' 1.0 */
	  ia[nz] = aresta+k*m; ja[nz] = n+j+k*n; ar[nz++] =  1.0; 
	  /* o coeficiente da k-esima variavel, ou seja, da variavel yk na restricao de aresta e' -1.0 */
	  ia[nz] = aresta+k*m; ja[nz] = k+1; ar[nz++] =  -1.0; 
	  aresta++;
	}
      }
    }
  }

  // ... nas restricoes de cobertura (cover), i.e., xi1+xi2+xi3+...xik >= 1, para cada vertice i
  for(i=1;i<=n;i++){ // para cada vertice i
    for(k=0;k<n;k++){ // para cada cor k
      /* o coeficiente da (n+i+k*n)-esima variavel, ou seja, da variavel xik na restricao de cobertura  e' 1.0 */
      ia[nz] = m*n + i; ja[nz] = n+i+k*n; ar[nz++]= 1.0;
    }
  }  

  
  // Carrega PL
  glp_load_matrix(*lp, nz-1, ia, ja, ar);

#ifdef DEBUG
      PRINTF("\n---LP gravado em coloring.lp");
      glp_write_lp(*lp, NULL,"coloring.lp");
#endif

  // libera memoria
  free(ia); free(ja); free(ar);
  return 1;
}
int main(int argc, char **argv)
{     glp_prob *lp;
      double z;
      FILE *fin, *graph, *graphSol;
      int n, m, i,j,k, status, tipo;
      int **A;
      double valor;
      clock_t antes, agora;
      char filename[80];

      glp_smcp param_lp;

      if(argc<3){
	PRINTF("Sintaxe: Coloring <grafo> <tipo>\n\t<grafo>: grafo a ser colorido\n\t<tipo>:1=relaxacao linear, 2=solucao inteira\n");
	exit(1);
      }
      fin=fopen(argv[1], "r");
      if(!fin){
	printf("Problema na abertura do arquivo: %s\n", argv[1]);
	exit(1);
      }

      if(!carga_instancia(fin, &n, &m, &A)){
	PRINTF("Problema na carga da instancia\n");
	exit(1);
      }

      tipo = atoi(argv[2]);
      if(tipo!=1 && tipo!=2){
	printf("Tipo invalido\nUse: tipo =1 p/ relaxacao linear ou tipo=2 p/ solucao inteira");
	exit(1);
      }

      sprintf(filename, "%s.sol.gr", argv[1]);
      graphSol=fopen(filename, "w");
      if(!graphSol){
	printf("\nproblema na criacao do arquivo de saida: %s", filename);
	exit(1);
      }
      sprintf(filename, "%s.gr", argv[1]);
      graph=fopen(filename, "w");
      if(!graph){
	printf("\nproblema na criacao do arquivo de saida: %s", filename);
	exit(1);
      }

      // desabilita saidas do GLPK no terminal
      glp_term_out(GLP_OFF);

#ifdef DEBUG
      PRINTF("n=%d m=%d\n", n, m);
      for(i=1;i<=n;i++){
	for(j=1;j<=n;j++){
	  if(A[i][j]>0){
	    PRINTF("(%d, %d)\n", i,j);
	  }
	}
      }
#endif  

      // carga do lp
      carga_lp(&lp, n,m,A);

      // configura simplex
      glp_init_smcp(&param_lp);
      param_lp.msg_lev = GLP_MSG_OFF;


      antes=clock();
      // Executa Solver de PL
      glp_simplex(lp, &param_lp);
      if(tipo==2){
	glp_intopt(lp, NULL);
      }
      agora=clock();

      if(tipo==2){
	status=glp_mip_status(lp);
	printf("\nstatus=%d\n", status);
      }
      // Recupera solucao
      if(tipo==1)
	z = glp_get_obj_val(lp);
      else
	z = glp_mip_obj_val(lp);

      fprintf(graphSol, "graph G {\n");
      fprintf(graph, "graph G {\n");
      for(i=0;i<n;i++){
	if(tipo==1)
	  valor=glp_get_col_prim(lp, i+1);
	else
	  valor=glp_mip_col_val(lp, i+1);
	if(valor>EPSILON){
	  PRINTF("y%d = %g\n", i+1, valor);
	}
      }
      for(k=0;i<n+n*n;k++){
	for(j=0;j<n;j++,i++){	  
	  if(tipo==1)
	    valor=glp_get_col_prim(lp, i+1);
	  else 
	    valor=glp_mip_col_val(lp, i+1);
	  if(valor>EPSILON){
	    PRINTF("x%d,%d = %g\n", j+1,k+1, valor);
	    fprintf(graphSol, "%d[label=\"%d\",fillcolor=\"%lf %lf %lf\", style=filled]\n", j+1, j+1, (k+1.0)/n, 0.7,0.7);
	  }
	}
      }

      for(i=1;i<=n;i++){
	for(j=i+1;j<=n;j++){
	  if(A[i][j]){
	    fprintf(graphSol, "%d -- %d\n", i, j);
	  }
	}
      }

      for(i=1;i<=n;i++){
	fprintf(graph, "%d[label=\"%d\",fillcolor=\"%lf %lf %lf\", style=filled]\n", i, i, 0.5, 0.5,0.5);
      }
      for(i=1;i<=n;i++){
	for(j=i+1;j<=n;j++){
	  if(A[i][j]){
	    fprintf(graph, "%d -- %d\n", i, j);
	  }
	}
      }
      
      fprintf(graph, "}\n");
      fprintf(graphSol, "}\n");
      fclose(graphSol);
      fclose(graph);
#ifdef DEBUG	
      PRINTF("\n---solucao gravada em coloring.sol");
      // Grava solucao
      if (tipo==1)
	glp_print_sol(lp, "coloring.sol");
      else
	glp_print_mip(lp, "coloring.sol");
#endif

      PRINTF("\n\n\n**** z =%g tempo=%g\n", z, ((double)agora-antes)/CLOCKS_PER_SEC);
      printf("%s\t%g\t%g\n", argv[1],z,((double)agora-antes)/CLOCKS_PER_SEC);

      // Libera memoria alocada
      for(i=0;i<n+1;i++)
	free (A[i]);
      free(A);
      // Destroi problema
      glp_delete_prob(lp);
      return 0;
}

/* eof */
