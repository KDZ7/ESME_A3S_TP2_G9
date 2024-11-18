#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "protocol_messag.h"

#define MEM_SIZE sysconf(_SC_PAGE_SIZE)

int main(int argc, char *argv[])
{
    char *device = "/dev/m0", *type = NULL, *data = NULL;
    int fd;
    struct protocol_message *protocol_msg;

    // Récupérer les arguments
    for (int i = 1; i < argc; i++)
    {
        // Récupérer le nom du device
        if (strcmp(argv[i], "-d") == 0)
            device = argv[i + 1];
        // Récupérer le type du message
        else if (strcmp(argv[i], "-t") == 0)
            type = argv[i + 1];
        // Récupérer les données du message
        else if (strcmp(argv[i], "-m") == 0)
            data = argv[i + 1];
    }

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        perror("Erreur ouverture device");
        return EXIT_FAILURE;
    }

    if (!strcmp(type, "w") || !strcmp(type, "write"))
    {
        if (data == NULL)
        {
            fprintf(stderr, "Données manquantes pour le type de message\n");
            close(fd);
            return EXIT_FAILURE;
        }
        size_t data_size = strlen(data) + 1;
        protocol_msg = malloc(sizeof(struct protocol_message) + data_size);
        if (protocol_msg == NULL)
        {
            perror("Erreur d'allocation mémoire");
            close(fd);
            return EXIT_FAILURE;
        }
        protocol_msg->length = sizeof(struct protocol_message) + data_size;
        protocol_msg->type = MSG_TYPE_WRITE;
        protocol_msg->pid = getpid();
        memcpy(protocol_msg->data, data, data_size);
        if (write(fd, protocol_msg, protocol_msg->length) < 0)
        {
            perror("Erreur d'écriture");
            free(protocol_msg);
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Message envoyé: \n length=%u, \n type=%u, \n pid=%u, \n data=%s\n", protocol_msg->length, protocol_msg->type, protocol_msg->pid, protocol_msg->data);
        free(protocol_msg);
    }
    else if (!strcmp(type, "r") || !strcmp(type, "read"))
    {
        size_t msg_size = sizeof(struct protocol_message) + MEM_SIZE;
        protocol_msg = malloc(msg_size);
        if (protocol_msg == NULL)
        {
            perror("Erreur d'allocation mémoire");
            close(fd);
            return EXIT_FAILURE;
        }
        if (read(fd, protocol_msg, msg_size) < 0)
        {
            perror("Erreur de lecture");
            free(protocol_msg);
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Message reçu: \n length=%u, \n type=%u, \n pid=%u, \n data=%s\n", protocol_msg->length, protocol_msg->type, protocol_msg->pid, protocol_msg->data);
        free(protocol_msg);
    }
    else
    {
        fprintf(stderr, "Type de message inconnu\n");
        close(fd);
        return EXIT_FAILURE;
    }
    close(fd);
    return EXIT_SUCCESS;
}