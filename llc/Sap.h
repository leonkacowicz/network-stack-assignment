#ifndef SAP_H_
#define SAP_H_

#include "../mac/Mac.h"

#define NUM_BUFFERS 4
#define MAX_SDU_SIZE 480

typedef enum
{
	SAP_STATUS_SLEEPING = 0,
	SAP_STATUS_LISTENING = 1,
	SAP_STATUS_CONNECTING = 2,
	SAP_STATUS_CONNECTED = 3,
	SAP_STATUS_ERROR = 4
} SAP_STATUS;

typedef enum
{
	CTRL_I = 1, // informacao numerada
	CTRL_RR = 2, // receiver ready
	CTRL_RNR = 3, // receiver not ready
	CTRL_REJ = 4, // quadro rejeitado
	CTRL_CON_REQ = 5, // pede conexao
	CTRL_CON_ACP = 6, // aceita conexao
	CTRL_CON_REJ = 7, // rejeita conexao
	CTRL_DISC = 8, // desfaz conexao
	CTRL_UI = 10 // informacao nao numerada (para datagrama nao confiavel)
} CTRLCOD;

typedef unsigned char SAP;
typedef unsigned char CTRL;
typedef unsigned char SEQN;

typedef struct
{
	SAP destino;
	SAP origem;
	CTRL controle;
	SEQN sequencia;
	char * msg;
	int tamanho; // Estes nao sao transmitidos no quadro LLC
	MAC mac_remoto;
} PDU;



class Sap
{
	private:
		Mac * mac;
		PDU in_buffer[NUM_BUFFERS];
		PDU out_buffer;
		SAP_STATUS _status;
		SAP_STATUS _status_ant;
		
		SAP sap_local;
		SAP sap_remoto_con;
		MAC mac_remoto_con;
		
		char numero_seq;
		char numero_rec;
		
		int in_buffer_ini;
		int in_buffer_fim;
		
		void RecebeDatagrama();
		void RecebeCircuito();
		
		bool _circuitoVirtual;
		
		void ConexaoRejeitada(SAP sap_remoto, MAC mac_remoto);
		void ConexaoAceita(SAP sap_remoto, MAC mac_remoto);
		void RecebeDatagrama(char * msg, MAC mac_remoto, SAP sap_remoto, int tamanho);
		void ConexaoRecebida(SAP sap_remoto, MAC mac_remoto);
	public:
		Sap (SAP idsap, Mac * mac_layer_obj, bool circuitoVirtual);
		~Sap ();
		void call (SAP sap_remoto, MAC mac_remoto);
		void listen();
		void hangup();
		SAP_STATUS status();
		int send (char * msg, int tamanho);
		int receive (char * msg);
		void sendDatagram (SAP sap_remoto, MAC mac_remoto, char * msg, int tamanho);
		int receiveDatagram (SAP * sap_remoto, MAC * mac_remoto, char * msg);

		void mainLoop();
		void setLossProbability (float prob);
		void ProcessaQuadro(MAC mac_remoto, char * msg, int tamanho);

};


#endif
