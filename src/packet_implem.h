#ifndef __PACKET_IMPLEM_H_
#define __PACKET_IMPLEM_H_

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintx_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <zlib.h>

/* Raccourci pour struct pkt */
typedef struct pkt pkt_t;

/* Types de paquets */
typedef enum {
	PTYPE_DATA = 1,
	PTYPE_ACK = 2,
} ptypes_t;

/* Taille maximale permise pour le payload */
#define MAX_PAYLOAD_SIZE 512
/* Taille maximale de Window */
#define MAX_WINDOW_SIZE 31

#define MAX_SEQNUM 255
/* Valeur de retours des fonctions */
typedef enum {
	PKT_OK = 0,     /* Le paquet a ete traite avec succes */
	E_TYPE,         /* Erreur liee au champs Type */
	E_LENGTH,       /* Erreur liee au champs Length  */
	E_CRC,          /* CRC invalide */
	E_WINDOW,       /* Erreur liee au champs Window */
	E_SEQNUM,       /* Numero de sequence invalide */
	E_NOMEM,        /* Pas assez de memoire */
	E_NOHEADER,     /* Le paquet n'a pas de header (trop court) */
	E_UNCONSISTENT, /* Le paquet est incoherent */
} pkt_status_code;

/* Alloue et initialise une struct pkt
 * @return: NULL en cas d'erreur */
pkt_t* pkt_new();
/* Libere le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associees
 */
void pkt_del(pkt_t*);

/*
 * Decode des donnees recues et cree une nouvelle structure pkt.
 * Le paquet recu est en network byte-order.
 * La fonction vérifie que:
 * - Le CRC32 des donnees recues est le meme que celui decode a la fin
 *   du flux de donnees
 * - Le type du paquet est valide
 * - La longeur du paquet est valide et coherente avec le nombre d'octets
 *   recus.
 *
 * @data: L'ensemble d'octets constituant le paquet recu
 * @len: Le nombre de bytes recus
 * @pkt: Une struct pkt valide
 * @post: pkt est la representation du paquet recu
 *
 * @return: Un code indiquant si l'operation a reussi ou representant
 *         l'erreur rencontree.
 */
pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt);

/*
 * Encode une struct pkt dans un buffer, pret a etre envoye sur le reseau
 * (c-a-d en network byte-order), incluant le CRC32 du header et payload.
 *
 * @pkt: La structure a encoder
 * @buf: Le buffer dans lequel la structure sera encodee
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets ecrit dans le buffer
 * @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
 *         le buffer est trop petit.
 */
pkt_status_code pkt_encode(const pkt_t*, char *buf, size_t *len);

/* Accesseurs pour les champs toujours presents du paquet.
 * Les valeurs renvoyees sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type     (const pkt_t*);
uint8_t  pkt_get_window   (const pkt_t*);
uint8_t  pkt_get_seqnum   (const pkt_t*);
uint16_t pkt_get_length   (const pkt_t*);
uint32_t pkt_get_timestamp(const pkt_t*);
uint32_t pkt_get_crc      (const pkt_t*);
/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
 * en a pas.
 */
const char* pkt_get_payload(const pkt_t*);

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapte.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
pkt_status_code pkt_set_type     (pkt_t*, const ptypes_t type);
pkt_status_code pkt_set_window   (pkt_t*, const uint8_t window);
pkt_status_code pkt_set_seqnum   (pkt_t*, const uint8_t seqnum);
pkt_status_code pkt_set_length   (pkt_t*, const uint16_t length);
pkt_status_code pkt_set_timestamp(pkt_t*, const uint32_t crc);
pkt_status_code pkt_set_crc      (pkt_t*, const uint32_t crc);
/* Defini la valeur du champs payload du paquet.
 * @data: Une succession d'octets representants le payload
 * @length: Le nombre d'octets composant le payload
 * @POST: pkt_get_length(pkt) == length */
pkt_status_code pkt_set_payload(pkt_t*,
                                const char *data,
                                const uint16_t length);


#endif  /* __PACKET_INTERFACE_H_ */
