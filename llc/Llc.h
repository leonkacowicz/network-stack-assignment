#ifndef LLC_H_
#define LLC_H_
#define NUM_SAPS 4
#include "Sap.h"


class Llc
{
	private:
		Sap * saps[NUM_SAPS];
		Mac * mac;
		char in_buffer[MAX_MAC_MSG_SIZE];
	public:
		Llc (MAC mac_addr);
		~Llc ();
		void call (SAP sap_local, SAP sap_remoto, MAC mac_remoto);
		void listen (SAP sap_local);
		void hungup (SAP sap_local);
		SAP_STATUS status (SAP sap_local);
		int send (SAP sap_local, char * msg, int tamanho);
		int receive (SAP sap_local, char * msg);
		void sendDatagram (SAP sap_local, SAP sap_remoto, MAC mac_remoto, char * msg, int tamanho);
		int receiveDatagram (SAP sap_local, SAP * sap_remoto, MAC * mac_remoto, char * msg);
		void mainLoop();
		void setLossProbability (float prob);
};

#endif
