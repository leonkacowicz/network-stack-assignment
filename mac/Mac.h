#ifndef MAC_H_
#define MAC_H_
#include "../phy/Uart.h"
#include <sys/timeb.h>

#define TAM_BUF 1000
#define TAM_BUF_SAIDA 1000
#define MAX_MAC_MSG_SIZE 255
#define IFS 0.1 /* em segundos */

using namespace phy;


typedef unsigned char MAC;

typedef struct
{
	MAC Destino;
	MAC Origem;
	unsigned char TamanhoMsg;
	char Mensagem[256];
	char FCS;
	short int TamanhoQuadro;
	bool Transmitido;
	short int numBytesTransmitidos;
} Quadro;

typedef struct timeb TIMEB;

class Mac
{
	private:
		void ColocaNoBuffer(char c);
		char RetiraDoBuffer();

		char input_buffer[TAM_BUF];
		int buf_ini;
		int buf_fim;
		unsigned int errorProb;
		unsigned int lossProb;
		
		
		bool dataIndicationFlag;
		
		int numTentativas;
		double waitTime;
		TIMEB ultimaColisao;
		TIMEB ultimaRecepcao;
		
		Quadro outputData[TAM_BUF_SAIDA];
		int buf_saida_ini;
		int buf_saida_fim;
		
		Quadro inputData1;
		Quadro inputData2;

		
		MAC macAddress;
		phy::Uart * uart;

		bool Transmite();
		bool Recebe(bool timeout);
	public:
		Mac(MAC mac_addr);
		~Mac();
		void dataRequest(MAC dest, char * msg, int tamanho);
		bool dataIndication();
		unsigned int dataReceive(char * msg);
		unsigned int dataReceive(char * msg, MAC * mac_origem);
		
		void setLossProbability(unsigned int prob);
		void setErrorProbability(unsigned int prob);
		void mainLoop();
		MAC getMacAddr();
};


#endif /*MAC_H_*/
