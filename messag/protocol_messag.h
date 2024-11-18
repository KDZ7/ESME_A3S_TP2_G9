#ifndef __PROTOCOL_MESSAGE_H
#define __PROTOCOL_MESSAGE_H

#include <linux/types.h>

#define MSG_TYPE_WRITE 0x01
#define MSG_TYPE_READ 0x02

/*
 * Structure du protocole de message
 * length : taille du message (sizeof(type) + sizeof(pid) + sizeof(data))
 * type : type du message (MSG_TYPE_WRITE, MSG_TYPE_READ)
 * pid : pid du processus qui a envoyé le message (terminal de l'utilisateur)
 * data : données du message
 *
 * description :
 *    Cette structure permet d'avoir une convention (protocole) à respecter
 *    pour une validation des messages envoyés entre le noyau et l'utilisateur.
 *
 */
struct protocol_message
{
    __u32 length;
    __u8 type;
    __u32 pid;
    __u8 data[];
};

#endif // __PROTOCOL_MESSAGE_H