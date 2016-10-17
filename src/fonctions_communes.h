#ifndef __FONCTION_COMMUNES_H_
#define __FONCTION_COMMUNES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>

/*
 * @argc:
 * - argc : nombre d'arguments contenus dans argv.
 * - argv : arguments entrés par l'utilisateur.
 * - file_name : nom du fichier ou il faut aller lire/ecrire.
 * - host_name : nom ou adresse de l'hôte sur/depuis lequel envoyer/recevoir des données.
 * - port : port sur lequel communiquer.
 * @pre : argc = 3 ou 5, file_name, host_name et port sont non-mallocé et initialisé à NULL pour file_name
 * et host_name.
 * @post : file_name, host_name et port ont été remplis correctement.
 * @return : 0 en cas de succès et -1 en cas d'erreur. (un message explicatif est mis sur la sortie standard).
 *
 */
int read_entries(int argc, char *argv[], char **file_name, char **host_name, uint16_t * port);

#endif
