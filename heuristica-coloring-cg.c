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
int main(int argc, char **argv)
{     
  glp_prob *lp, *pric;
  double z, *dual, *xstar, z_pric, z_heur;
      FILE *fin;
      int i,j, *ind, icol;
      char name[80];
      int it, masterCols;
      int colsgen;
      int nels, *index;
      double *val;

      glp_iocp param;
      glp_smcp param_lp;
      clock_t antes, agora;

      double valor;

      if(argc<2){
	printf("Sintaxe: Coloring <grafo>\n");
	exit(1);
      }
      
      fin=fopen(argv[1], "r");
      if(!fin){
	printf("Problema na abertura do arquivo: %s\n", argv[1]);
	exit(1);
      }
      
      if(!carga_instancia(fin)){
	printf("Problema na carga da instancia\n");
	exit(1);
      }

      // desabilita saidas do GLPK no terminal
      glp_term_out(GLP_OFF);

      // Carrega Problema Mestre Restrito
      carga_pmr(&lp);

      // Carrega Pricing
      carga_pricing(&pric);

      // Aloca memoria
      index=(int*)malloc(sizeof(int)*(n+1));
      val=(double*)malloc(sizeof(double)*(n+1));
      dual=(double*)malloc(sizeof(double)*(n+1));
      xstar=(double*)malloc(sizeof(double)*(n+1));
      ind=(int*)malloc(sizeof(int)*(n+1));
	
      // Configura parametros do B&B no glpk
      glp_init_iocp(&param);
      param.presolve = GLP_ON;
      param.msg_lev = GLP_MSG_OFF;
      param.tm_lim = 3000;

      // Configura parametros do Simplex
      glp_init_smcp(&param_lp);
      param_lp.msg_lev = GLP_MSG_OFF;

      for(i=1;i<=n;i++){
	ind[i]=i;
      }

      it=0;
      colsgen=0;
      antes=clock();
      do{
	// Executa Solver de PL para resolver PMR
	glp_simplex(lp, &param_lp);
	it++;

	// Recupera solucao
	z = glp_get_obj_val(lp);
	printf("\n****z=%g\n****it=%d\n\n", z, it);

	masterCols=glp_get_num_cols(lp);
	for(i=0;i<masterCols;i++){
	  valor=glp_get_col_prim(lp, i+1);
	  if(valor>EPSILON){
	    printf("\n\tx%d (%g)= {", i+1, valor );

	    nels=glp_get_mat_col(lp, i+1, index, val);
	    for(j=1;j<=nels;j++){
	      printf("%d (%g)", index[j], val[j]);
	    }
	    printf("}");
	  }
	}

	// recupera duais
	// seta peso das variaveis no pricing
	for(i=0;i<n;i++){
	  dual[i+1]=glp_get_row_dual(lp, i+1);
	  PRINTF("\n\tdual de %d = %g", i+1, dual[i+1]);
	  glp_set_obj_coef(pric, i+1, dual[i+1]);
	}

#ifdef DEBUG
	// Grava PL do Pricing atual
	name[0]='\0';
	sprintf(name,"pricing%d.lp", it);

	PRINTF("\n---- LP gravado em %s", name);
	glp_write_lp(pric,NULL,name);
#endif

	// resolve pricing
	glp_intopt(pric, &param);
	// recupera valor da solucao 
	z_pric = glp_mip_obj_val(pric);
	// recupera a solucao otima 
	for(i=1;i<=n;i++){
	    xstar[i]=glp_mip_col_val(pric, i);
	}
	//	testa se existe coluna de c.r. negativo
	printf("\n****zPric=%g", z_pric);
	if(1 - z_pric < - EPSILON){
	  // adiciona 1 nova coluna no PMR
	  icol=glp_add_cols(lp, 1);
	  name[0]='\0';
	  sprintf(name,"n%d", icol);
	  glp_set_col_name(lp, icol, name);
	  glp_set_col_bnds(lp, icol, GLP_LO, 0.0, 0.0);
	  glp_set_obj_coef(lp, icol, 1.0);
          glp_set_col_kind(lp, icol, GLP_BV); // especifica que a variaval eh binaria
	  printf("\n\tNova coluna:{ ");
	  for(i=1;i<=n;i++){
	    if(xstar[i]>EPSILON)
	      printf("%d (%g) ", i, xstar[i]);
	  }
	  printf("}\n");
	
	  glp_set_mat_col(lp, icol, n,ind,xstar); // seta os coeficientes da nova coluna no PMR	  
	  colsgen++;
	}
	else{
	  break;
	}
#ifdef DEBUG
	// Grava PL do PMR atual
	name[0]='\0';
	sprintf(name,"coloring%d.lp", it);
	
	PRINTF("\n---- LP gravado em %s", name);
	glp_write_lp(lp, NULL,name);
	getchar();
#endif
	  
      } while(1);
	
#ifdef DEBUG      
      PRINTF("\n---- solucao gravada em coloring-cg.sol");
      glp_print_sol(lp, "coloring-cg.sol");
#endif

      agora=clock();
      printf("\n\n\n****z=%g\n****it=%d tempo=%g\n****colsgen=%d\n\n", z, it, ((double)agora-antes)/CLOCKS_PER_SEC, colsgen);

      glp_write_lp(lp, NULL,"pmr-relax.lp");
      // resolve PMR as PLI
      glp_intopt(lp, &param);
      // recupera valor da solucao 
      z_heur = glp_mip_obj_val(lp);
      // recupera a solucao otima 
      masterCols=glp_get_num_cols(lp);
      for(i=0;i<masterCols;i++){
        //        valor=glp_get_col_prim(lp, i+1);
        valor=glp_mip_col_val(lp, i+1);
        if(valor>EPSILON){
          printf("\n\tx%d (%g)= {", i+1, valor );
          
          nels=glp_get_mat_col(lp, i+1, index, val);
          for(j=1;j<=nels;j++){
            printf("%d (%g)", index[j], val[j]);
          }
	    printf("}");
        }
      }
      agora=clock();

      printf("\n\n\n****z=%g\n****it=%d tempo=%g\n****colsgen=%d\n\n", z_heur, it, ((double)agora-antes)/CLOCKS_PER_SEC, colsgen);
      // Destroi problema
      glp_delete_prob(lp);
      glp_delete_prob(pric);

      // libera memoria
      free(dual);
      free(xstar);
      free(ind);
      free(index);
      free(val);

      return 0;
}