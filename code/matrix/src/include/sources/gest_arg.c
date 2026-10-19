#include "../headers/gest_arg.h"

arg * parse_arg(int argc, char ** argv){
    /*
    ON va faire un seule programme capable de gerer plusierus entrées
    pour cela 3 a 5 arguments pour tout gerer:
    le premier sert a indiquer si on fait un vrai calcul avec
    des fichiers d'entree ou ci on fait un test avec les matrices
    predefinie dans le porgramme
    argv[1] == 0 alors on execute les matrice classqie du programme
    dans la taille souhaité dans argv 2, avec ci on le souhaite du 
    blockig via argv[3]

    si argv[1] = 1 alors on va faire le calcul sur les matrice presentes
    dans les fichiers (idealement peux importe que ca soit un .mat ou .txt)
    avec argv[2] = A, argv[3] = B, et argv[4] taille du blocking
    et enfin ci besoin argv[5] pour faire une ecriture de la matrice
    resultat dans le fichier de l'argument.
    */
    if(argc< 3){
        printf("Le programme a besoin au minimum d'une taille de matrice pour l'execution\n");
        printf("Mode disponible : \n\t{1} mult celon les mat. predefini\n\t\t ./summa 1 size_mat size_blocking \n\t{2} celon les matrice dans les fichier en entrées\n\t\t ./summa 2 A.mat B.mat size_block (C.mat)\n");
        printf("exemple :\n\t ./summa 1 1024 32\n");
        printf("exemple :\n\t ./summa 2 A.mat B.mat 32 C.mat\n");
        exit(EXIT_FAILURE);
    }

    arg * ARG = malloc(sizeof(arg));


    ARG->mode = atoi(argv[1]);
    //printf("mode %d\n",ARG->mode);
    ARG->block = 0;
    if(ARG->mode == 1){
        ARG->size = atoi(argv[2]);
        if (argc >= 4)
            ARG->block = atoi(argv[3]);
        if(argc == 5)
            ARG->file_C = argv[4];
        if(argc >  5)
            printf("les argument apres le nom de fichier sont ignoré\n");
        //printf("taille = %d, block = %d, C  = %s\n",ARG->size,ARG->block,ARG->file_C);
    }
    else if(ARG->mode == 2){
        if(argc<4){
            printf("il n y a pas assez d'argument\n");
            exit(EXIT_FAILURE);
        }
        ARG->file_A = argv[2];
        ARG->file_B = argv[3];
        
        if(argc >=5)
            ARG->block = atoi(argv[4]);
        if(argc == 6)
            ARG->file_C = argv[5];
        if(argc >  6)
            printf("les argument apres le nom de fichier C sont ignoré\n");

    }else{
        printf("Mode %d pas encore existant/!\\\n",ARG->mode);
        exit(EXIT_FAILURE);
    }

    return ARG;
}



matrix_param * init_p0(arg * ARG,int lg_grid,short need_tmp_C,short need_blocking){
    int * A;
    int *B;
    int * A_blk = NULL, *B_blk = NULL;
    int *C,*tmp_C = NULL;
    int size = ARG->size;
    int to_resize,old_size = size;
    int dim_blk;
    srand(42);

    matrix_param * param = malloc(sizeof(matrix_param));

    if(ARG->mode == 1){
        printf("old size : %d\n",old_size);
        A = malloc(old_size*old_size*sizeof(int));
        B = calloc(old_size*old_size,sizeof(int));
        
        //initialisation des donnees
        for(int i = 0;i<old_size;i++){
            for(int j = 0;j<old_size;j++){
                A[i*old_size+j] = i*old_size+j+1;
                //rand()%100;
                //B[i*old_size+j] = rand()%100;
                
                //i*old_size+j+1;
            }
            B[i*old_size +i] = 1;//matrice identité
        }

        to_resize = (lg_grid - old_size%lg_grid)%lg_grid;//indique le nombre de ligne manquante (on reste sur des matrice carrées)
    
        size = old_size+to_resize;
        if (to_resize){
            A = resize(A,old_size,old_size,size,size);
            B = resize(B,old_size,old_size,size,size);
        }
    }
    else{
        Matrix M_A = lecture(ARG->file_A);
        Matrix M_B = lecture(ARG->file_B);
        A = M_A.data;
        B = M_B.data;

        old_size = max(M_A.cols,M_A.rows,M_B.cols,M_B.rows);
        
        to_resize = (lg_grid - old_size%lg_grid)%lg_grid;//indique le nombre de ligne manquante (on reste sur des matrice carrées)
    
        size = old_size+to_resize;

        if(size != old_size || size != min(M_A.cols,M_A.rows,M_B.cols,M_B.rows)){
            A = resize(A,M_A.rows,M_A.cols,size,size);
            B = resize(B,M_B.rows,M_B.cols,size,size);
        }
    }
    
    dim_blk = size/lg_grid;

    if(need_blocking){
        // -- Preparation de la matrice pour l'envoie
        int ** res = to_blocking_dbl_lin(A,B,lg_grid,dim_blk,size);
        A_blk = res[0];
        B_blk = res[1];
        free(res);
    }

    C = calloc(size*size,sizeof(int));
    if(need_tmp_C)
        tmp_C = malloc(size*size*sizeof(int));
    
    printf(" P0 a fini son taf\n");
    
    param->matrix_A = A;
    param->matrix_B = B;
    param->matrix_C = C;
    param->A_blk = A_blk;
    param->B_blk = B_blk;
    param->matrix_tmp_C = tmp_C;
    param->size = size;
    param->old_size = old_size;
    param->to_resize = to_resize;

    return param;

}

