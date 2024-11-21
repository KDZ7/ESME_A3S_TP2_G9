# ESME_A3S_TP2_G9
Groupe 9: ESME A3S (TP2)

# TP Drivers Linux Embarqué

## Introduction
Ce projet consiste à développer un driver de périphérique pour Linux permettant de gérer des messages via un protocole de communication personnalisé. Le développement a été réalisé étape par étape, avec un historique des commits documentant les solutions de chaque exercice. Pour examiner l'évolution du projet, consultez les commits organisés par exercices : exo2, exo3, exo4/exo5.

## Structure du Projet (Fichiers Ajoutés)

### 1. **Protocole de Message : `protocol_messag.h`**
Ce fichier d'en-tête définit la structure du protocole de message que nous avons utilisé. Il spécifie les types de messages et la manière dont les données sont organisées pour être échangées avec le driver :
- **`length`** : Taille totale du message, incluant les champs `type`, `pid` et `data`.
- **`type`** : Type de message (`MSG_TYPE_WRITE` pour l’écriture, `MSG_TYPE_READ` pour la lecture).
- **`pid`** : PID du processus émetteur.
- **`data`** : Contenu du message.

### 2. **Programmes de Test**
- **`test_protocol`** : Programme permettant de tester l’écriture et la lecture de messages en utilisant notre protocole de communication. Il interagit avec le périphérique `/dev/m0` pour s'assurer que les messages sont correctement transmis et reçus entre l'espace utilisateur et le noyau. Notre étude s'est inspirée du protocole Netlink pour la communication entre l'utilisateur et le noyau ([documentation Netlink](https://manpages.ubuntu.com/manpages/focal/fr/man7/netlink.7.html)).
- **`test_mutex`** : Programme qui lance plusieurs threads simultanément, chaque thread exécutant `test_protocol` pour écrire dans la mémoire partagée `MEM1`. Ce test permet de vérifier la gestion de la concurrence et la synchronisation des accès. En analysant les messages générés par le noyau, nous avons pu tirer des conclusions sur l'efficacité de notre synchronisation.

## Détails de l'Implémentation

### Exercice 3 : Implémentation du Protocole
Pour simplifier les tests, nous avons créé un programme nommé `test_protocol.c`. Ce programme utilise le protocole défini dans `protocol_messag.h` pour structurer les messages envoyés et reçus par le driver. L’objectif est de s’assurer que le format des messages respecte les exigences de notre protocole.

### Exercices 4 et 5 : Gestion de la Concurrence
Lors de ces exercices, nous avons rencontré un problème de concurrence : lorsque plusieurs processus écrivaient simultanément dans `MEM1`, des conflits survenaient. Ces problèmes étaient visibles dans les messages de débogage du noyau, indiquant que la mémoire partagée était mal synchronisée.

### Solution : Synchronisation avec Mutex
Pour résoudre ce problème, nous avons exploré des méthodes de synchronisation autres que les sémaphores, en utilisant des **mutex**. Les mutex sont particulièrement adaptés pour protéger des sections critiques, comme l’accès concurrent à la mémoire `MEM1`.

## Vérification de la Synchronisation
Nous avons développé le programme `test_mutex` pour vérifier l'efficacité de notre synchronisation. Ce programme crée plusieurs threads qui exécutent `test_protocol` pour écrire dans `MEM1`. Grâce à l'analyse des messages du noyau avec `dmesg`, nous avons confirmé que le problème de concurrence était résolu et que les écritures se faisaient de manière correcte et synchronisée.

## Conclusion
Ce projet nous a permis de créer un driver fonctionnel pour Linux qui gère des messages en respectant un protocole défini. Nous avons également appris à gérer les problèmes de concurrence en utilisant des mécanismes de synchronisation appropriés, comme les mutex, pour assurer l'intégrité des données partagées.
