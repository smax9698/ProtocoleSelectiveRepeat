#include "packet_implem.h"

struct __attribute__((__packed__)) pkt {
	uint8_t type : 3;
	uint8_t window : 5;
	uint8_t seqnum : 8;
	uint16_t length : 16;
	uint32_t timestamp : 32;
	uint32_t crc : 32;
	char * payload;
};


pkt_t* pkt_new()
{
	pkt_t * new_packet = (pkt_t *)malloc(sizeof(pkt_t));
	if(new_packet == NULL){
		fprintf(stderr, "malloc erreur : %s\n",strerror(errno));
	}
	new_packet->type = 0;
	new_packet->window = 0;
	new_packet->seqnum = 0;
	new_packet->length = 0;
	new_packet->timestamp = 0; // mettre le temps actuel ?
	new_packet->crc = 0;
	new_packet->payload = NULL;
	return new_packet;
}

void pkt_del(pkt_t *pkt)
{
	free(pkt->payload);
	free(pkt);
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
	pkt_status_code err_status = PKT_OK;

	int deb_crc = len-4;

	// décodage du crc
	uLong crc = crc32(0L,Z_NULL,0);
	memcpy((void *)&crc,(const void *)&data[deb_crc],4);

	// recalcul du crc
	uLong crcVerif;
	crcVerif = crc32(0L,(const Bytef *)data,len-4);

	if(crcVerif != ntohl(crc)){
		return E_CRC;
	}
	// récupération du champs type
	uint8_t type_int = (uint8_t) data[0] >> 5;

	// vérification que le champs a une valeur sensée et set
	err_status = pkt_set_type(pkt,type_int);
	if(err_status != PKT_OK){
				return E_TYPE;
	}

	// récupération du champs length et set. (manipulation de bits)
	uint16_t l;
	memcpy((void *)&l,(const void *)&data[2],sizeof(l));
  l = ntohs(l);
	pkt_set_length(pkt,l);

	// vérifaction de la concordance entre la longueur donnée en paramètre et la longueur contenue dans le paquet
	if((uint16_t) len != l + 12){
		return E_LENGTH;
	}

	// récupération du champs window et set
	pkt_set_window(pkt,(uint8_t)(data[0] << 3) >> 3);

	// récupération du champs seqnum et set
	pkt_set_seqnum(pkt,(uint8_t)data[1]);

	// récupération du champs timestamp et set. (manipulation de bits)
	uint32_t timest;
	memcpy((void *)&timest,(const void*)&data[4],sizeof(timest));
	pkt_set_timestamp(pkt,ntohl(timest));

	// buffer de la taille indiquée par le champs length du paquet
	char * payload = (char *)malloc(sizeof(char)*l);

	// remplissage du buffer
	memcpy((void *)payload,(const void *)&data[8],sizeof(char)*l);

	// set du payload
	err_status = pkt_set_payload(pkt,payload,l);
	if(err_status != PKT_OK){
		return err_status;
	}

	pkt_set_crc(pkt,crc);

	return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
	// verifie qu'il y a assez de place dans le buffer
	size_t tot_length = pkt_get_length(pkt) + 12;
	if(*len < tot_length){
		return E_NOMEM;
	}

	// premier byte : type + window
	uint8_t type_and_window = (pkt_get_type(pkt) << 5) + pkt_get_window(pkt);
	buf[0] = (char) type_and_window;

	// byte 2 : seqnum
	uint8_t seqnum = pkt_get_seqnum(pkt);
	buf[1] = (char) seqnum;

	// bytes 3 et 4 : length
	uint16_t length;
	length = htons(pkt_get_length(pkt));
	memcpy((void *)&buf[2],(const void *)&length,sizeof(length));

	// encodage de timestamp bytes 4 à 7
	uint32_t timestamp = 0;
	timestamp = htonl(pkt_get_timestamp(pkt));
	memcpy((void *)&buf[4],(const void *)&timestamp,sizeof(timestamp));

	// encodage du payload
	memcpy((void *)&buf[8],(const void *)pkt_get_payload(pkt),pkt_get_length(pkt));

	// calcul du crc
	uLong crc = crc32(0L,Z_NULL,0);
	crc = htonl(crc32(crc,(const Bytef *)buf,8+pkt_get_length(pkt)));

	// encodage du crc
	memcpy((void *)&buf[8+pkt_get_length(pkt)],(const void *)&crc,sizeof(uLong));

	// mise à jour de len : nombre de bytes effectivement encodés
  memcpy((void *)len,(const void *)&tot_length,sizeof(size_t));

	return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t* pkt)
{
	ptypes_t pt = pkt->type;
	return pt;
}

uint8_t pkt_get_window(const pkt_t* pkt)
{
	return pkt->window;
}

uint8_t pkt_get_seqnum(const pkt_t* pkt)
{
	return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t* pkt)
{
	return pkt->length;
}

uint32_t pkt_get_timestamp(const pkt_t* pkt)
{
	return pkt->timestamp;
}

uint32_t pkt_get_crc(const pkt_t* pkt)
{
	return pkt->crc;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
	return pkt->payload;
}

pkt_status_code pkt_set_type(pkt_t * pkt, const ptypes_t type)
{
	if(type == PTYPE_DATA || type == PTYPE_ACK){
		pkt->type = type;
		return PKT_OK;
	}
	return E_TYPE;
}

pkt_status_code pkt_set_window(pkt_t * pkt, const uint8_t window)
{
	pkt->window = window;
	return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t * pkt, const uint8_t seqnum)
{
	pkt->seqnum = seqnum;
	return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t * pkt, const uint16_t length)
{
	pkt->length = length;
	return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t * pkt, const uint32_t timestamp)
{
	pkt->timestamp = timestamp;
	return PKT_OK;
}

pkt_status_code pkt_set_crc(pkt_t * pkt, const uint32_t crc)
{
	pkt->crc = crc;
	return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t * pkt,const char *data,const uint16_t length)
{
	pkt->payload = (char *)malloc(sizeof(char)*length);

	if(pkt->payload == NULL){
		return E_NOMEM;
	}

	memcpy(pkt->payload,data,length);
	pkt_set_length(pkt,length);

	return PKT_OK;
}
