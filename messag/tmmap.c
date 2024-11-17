/* exemple d'utilisation de la fct mmap */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

int main(int N, char *P[])
{
int f1, f2, i;
char c,*p;

     if (N != 2 ) {
       fprintf(stderr,"Utilisation : %s fichier_ou_device !\n",P[0]);
       exit(1);
     }
     if ((f1 = open(P[1],O_RDWR)) < 0) {
        perror("open 1"); exit(2);
     }
     if ((f2 = open(P[1],O_RDWR)) < 0) {
        perror("open 2"); exit(2);
     }
     p = mmap(0, 2048, PROT_READ|PROT_WRITE,MAP_SHARED ,f1,0);
     if (p == (void*)-1) {
        perror("mmap"); exit(3);
     }
     close(f1);
     i=9;
     printf("Le caractere no %d est '%c'\n", i+1, p[i]);
     p[i]++;
     if (lseek(f2,(long)i,0) == (off_t)-1)  perror("lseek");
     if (read(f2,&c,1) == -1) {
        perror("read"); exit(4);
     }
     printf("Le caractere no %d est '%c'\n", i+1, c);
     close(f2);
}


