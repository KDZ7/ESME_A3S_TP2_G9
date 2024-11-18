#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define NUM_THREADS 10

void *write_to_device(void *arg)
{
    char *device = (char *)arg;
    char command[0xFF];
    snprintf(command, sizeof(command), "./test_protocol -d %s -t write -m \"Je suis le thread %lu\"", device, (unsigned long)pthread_self());
    int ret = system(command);
    if (ret != 0)
        fprintf(stderr, "Erreur lors de l'écriture du message\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    char *device = "/dev/m0";
    if (argc < 2)
        device = argv[1];

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_create(&threads[i], NULL, write_to_device, (void *)device) != 0)
        {
            perror("Erreur lors de la création du thread");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Erreur lors de la terminaison du thread");
            return EXIT_FAILURE;
        }
    }
    printf("Tous les threads ont terminé\n");
    return EXIT_SUCCESS;
}
