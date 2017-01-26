#include <stdio.h>
#include "Llc.h"


Llc::Llc(MAC mac_addr)
{
	int i;
	mac = new Mac(mac_addr);
	for (i = 0; i < NUM_SAPS; i++)
		saps[i] = new Sap(i, mac, i > 0);
		
}

Llc::~Llc()
{
	int i;
	for (i = 0; i < NUM_SAPS; i++)
		delete saps[i];
	delete mac;
}


void Llc::sendDatagram (SAP sap_local, SAP sap_remoto, MAC mac_remoto, char * msg, int tamanho)
{
	if (sap_local >= NUM_SAPS) return;
	
	saps[sap_local]->sendDatagram (sap_remoto, mac_remoto, msg, tamanho);
	
}

int Llc::receiveDatagram (SAP sap_local, SAP * sap_remoto, MAC * mac_remoto, char * msg)
{
	int tamanho;
	
	tamanho = saps[sap_local]->receiveDatagram(sap_remoto, mac_remoto, msg);
	if (tamanho > 0)
	{
		printf("Entrei; %d\n", tamanho);
	}
	return tamanho;
}

void Llc::mainLoop()
{
	MAC mac_remoto;
	SAP sap_local;
	int tamanho;
	int i;
	mac->mainLoop();
	if (mac->dataIndication())
	{
		
		tamanho = mac->dataReceive(in_buffer, &mac_remoto);
		saps[in_buffer[0]]->ProcessaQuadro(mac_remoto, in_buffer, tamanho);
		
		/*
		printf("tamanho recebido mac %d\n", tamanho);
		//printf("data indication; %d bytes: sap_destino: %d; sap_origem: %d; mac_origem: %d; \n", tamanho, in_buffer[0], in_buffer[1], mac_remoto);
		//for (i = 4; i < tamanho; i++) printf("%c", in_buffer[i]);
		
		saps[in_buffer[0]]->RecebeDatagrama(&(in_buffer[4]), mac_remoto, in_buffer[1], tamanho - 4);
		*/
		
	}
}

void Llc::listen (SAP sap_local)
{
	if (sap_local >= NUM_SAPS) return;
	
	saps[sap_local]->listen();
}
void Llc::call (SAP sap_local, SAP sap_remoto, MAC mac_remoto)
{
	if (sap_local >= NUM_SAPS) return;
	
	saps[sap_local]->call(sap_remoto, mac_remoto);
}

void Llc::hungup (SAP sap_local)
{
	if (sap_local >= NUM_SAPS) return;
	
	saps[sap_local]->hangup();
}

SAP_STATUS Llc::status (SAP sap_local)
{
	if (sap_local >= NUM_SAPS) return SAP_STATUS_ERROR;
	return saps[sap_local]->status();
}

int Llc::send (SAP sap_local, char * msg, int tamanho)
{
	saps[sap_local]->send(msg, tamanho);
}
/*
	private:
		Sap * saps[NUM_SAPS];
	public:
		Llc (char mac_addr);
		~Llc ();
		void call (SAP sap_local, SAP sap_remoto, char mac_remoto);
		void listen (SAP sap_local);
		void hungup (SAP sap_local);
		SAP_STATUS status (SAP sap_local);
		int send (SAP sap_local, char * msg, int tamanho);
		int receive (SAP sap_local, char * msg);
		void sendDatagram (SAP sap_local, SAP sap_remoto, char mac_remoto, char * msg, int tamanho);
		int receiveDatagram (SAP sap_local, SAP * sap_remoto, char * mac_remoto, char * msg);
		void mainLoop();
		void setLossProbability (float prob);
		*/


