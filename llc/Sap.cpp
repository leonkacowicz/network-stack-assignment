#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include "Sap.h"


Sap::Sap(SAP idsap, Mac * mac_layer_obj, bool circuitoVirtual)
{
	int i;
	mac = mac_layer_obj;
	in_buffer_ini = 0;
	in_buffer_fim = 0;
	_circuitoVirtual = circuitoVirtual;
	_status = _status_ant = SAP_STATUS_SLEEPING;
	sap_local = idsap;
	for (i = 0; i < NUM_BUFFERS; i++)
		in_buffer[i].msg = new char[MAX_SDU_SIZE]; //(char *)malloc(MAX_SDU_SIZE * sizeof(char));
		
}

Sap::~Sap()
{
	int i;
	for (i = 0; i < NUM_BUFFERS; i++)
		free(in_buffer[i].msg);	
}

void Sap::sendDatagram (SAP sap_remoto, MAC mac_remoto, char * msg, int tamanho)
{	
	char msg2[MAX_MAC_MSG_SIZE];
	if (tamanho > MAX_SDU_SIZE) return;
	if (tamanho > MAX_MAC_MSG_SIZE - 4)
	{
		memcpy(msg2 + 4, msg, MAX_MAC_MSG_SIZE - 4);
		msg2[0] = sap_remoto;
		msg2[1] = sap_local;
		msg2[2] = CTRL_UI;
		msg2[3] = 0;
		mac->dataRequest(mac_remoto, msg2, MAX_MAC_MSG_SIZE);
		
		msg += MAX_MAC_MSG_SIZE - 4;
		
		memcpy(msg2 + 4, msg, tamanho - (MAX_MAC_MSG_SIZE - 4));
		msg2[0] = sap_remoto;
		msg2[1] = sap_local;		
		msg2[2] = CTRL_UI;
		msg2[3] = 0;
		mac->dataRequest(mac_remoto, msg2, 4 + tamanho - (MAX_MAC_MSG_SIZE - 4));
	} else {
		memcpy(msg2 + 4, msg, tamanho);
		
		/* Cabecalho do PDU */
		msg2[0] = sap_remoto;
		msg2[1] = sap_local;
		msg2[2] = CTRL_UI;
		msg2[3] = 0;
		mac->dataRequest(mac_remoto, msg2, tamanho + 4);			
	}	
}

int Sap::receiveDatagram (SAP * sap_remoto, MAC * mac_remoto, char * msg)
{
	/* Aqui apenas entrega para a camada superior o conteudo previamente bufferizado*/
	int buffer_atual;	
	if (in_buffer_ini == in_buffer_fim) 
		return 0; // NaSAP_STATUS_SLEEPINGo tem nada no buffer, retorna dizendo q o conteudo tem tamanho zero.

	buffer_atual = in_buffer_ini;
	in_buffer_ini = (in_buffer_ini + 1) % NUM_BUFFERS;
	
	// Em caso contrario passa o conteudo a diante.
	*sap_remoto = in_buffer[buffer_atual].origem;
	*mac_remoto = in_buffer[buffer_atual].mac_remoto;
	memcpy(msg, in_buffer[buffer_atual].msg, in_buffer[buffer_atual].tamanho);
	

	return in_buffer[buffer_atual].tamanho;
}



void Sap::RecebeDatagrama(char * msg, MAC mac_remoto, SAP sap_remoto, int tamanho)
{

	// Esta funcao eh usada pela classe llc para bufferizar os datagramas que forem chegando.
	
	if ((in_buffer_fim + 1) % NUM_BUFFERS == in_buffer_ini) return;
	
	// Neste caso estamos recebendo o primeiro quadro MAC de um quadro LLC que foi quebrado.
	in_buffer[in_buffer_fim].origem = sap_remoto;
	in_buffer[in_buffer_fim].destino = sap_local;
	in_buffer[in_buffer_fim].mac_remoto = mac_remoto;
	in_buffer[in_buffer_fim].tamanho = tamanho;	
	printf("tamanho = %d\n", in_buffer[in_buffer_fim].tamanho);
	memcpy(in_buffer[in_buffer_fim].msg, msg, tamanho * sizeof(char));
	in_buffer_fim = (in_buffer_fim + 1) % NUM_BUFFERS;
	
	
}


void Sap::listen()
{
	if (_status != SAP_STATUS_SLEEPING) return;
	if (_circuitoVirtual == false) return;
	
	printf("Sap local %d ouvindo.\n", sap_local);
	_status_ant = _status;
	_status = SAP_STATUS_LISTENING;	
}

SAP_STATUS Sap::status()
{
	return _status;
}

void Sap::call(SAP sap_remoto, MAC mac_remoto)
{
	char msg[4];
	printf("Sap local %d pede conexao ao sap remoto #%d da estacao #%c\n", sap_local, sap_remoto, mac_remoto);
	msg[0] = sap_remoto;
	msg[1] = sap_local;
	msg[2] = CTRL_CON_REQ;
	msg[3] = 0;
	mac->dataRequest(mac_remoto, msg, 4);
	
	_status_ant = _status;
	_status = SAP_STATUS_CONNECTING;
}


void Sap::hangup()
{
	char msg[4];
	if (_status != SAP_STATUS_CONNECTED) return;
	
	printf("Sap local %d desconectou do sap remoto #%d da estacao #%c\n", sap_local, sap_remoto_con, mac_remoto_con);
	msg[0] = sap_remoto_con;
	msg[1] = sap_local;
	msg[2] = CTRL_DISC;
	msg[3] = 0;
	mac->dataRequest(mac_remoto_con, msg, 4);
	
	
	_status = _status_ant;
	
}

void Sap::ProcessaQuadro(MAC mac_remoto, char * msg, int tamanho)
{
	SAP sap_remoto;
	sap_remoto = msg[1];
	if (_circuitoVirtual)
	{
		if (msg[2] == CTRL_CON_ACP)
		{
			ConexaoAceita(sap_remoto, mac_remoto);
			return;
		}
		if (msg[2] == CTRL_CON_REJ)
		{
			ConexaoRejeitada(sap_remoto, mac_remoto);
			return;
		}	
		if (msg[2] == CTRL_CON_REQ)
		{
			ConexaoRecebida(sap_remoto, mac_remoto);
			return;
		}
	} else {
		if (msg[2] == CTRL_UI)
		{
			RecebeDatagrama(&(msg[4]), mac_remoto, sap_remoto, tamanho - 4);
			return;
		}	
	}
	
}

void Sap::ConexaoAceita(SAP sap_remoto, MAC mac_remoto)
{
	printf("Conexao com o sap %d da estacao %c aceita.\n", sap_remoto, mac_remoto);
	
	_status_ant = _status;
	_status = SAP_STATUS_CONNECTED;
}

void Sap::ConexaoRejeitada(SAP sap_remoto, MAC mac_remoto)
{
	printf("Conexao com o sap %d da estacao %c rejeitada.\n", sap_remoto, mac_remoto);
	_status = _status_ant;
	
}

void Sap::ConexaoRecebida(SAP sap_remoto, MAC mac_remoto)
{
	char msg[4];
	
	printf("Pedido de conexao do sap %d da estacao %c recebido: ", sap_remoto, mac_remoto);
	if (_status == SAP_STATUS_LISTENING)
	{
		printf(" aceito.\n");
		_status_ant = _status;
		_status = SAP_STATUS_CONNECTED;
		
		sap_remoto_con = sap_remoto;
		mac_remoto_con = mac_remoto;
		
		msg[0] = sap_remoto_con;
		msg[1] = sap_local;
		msg[2] = CTRL_CON_ACP;
		msg[3] = 0;
		mac->dataRequest(mac_remoto_con, msg, 4);		
			
	} else if (_status == SAP_STATUS_CONNECTED) {
		if ((mac_remoto_con == mac_remoto) && (sap_remoto_con == sap_remoto))
		{
		
			/* Se receber um pedido de conexao de um sap remoto ao qual ele ja acha que esta conectado,
			por exemplo se a confirmacao da conexao for perdida, manda uma outra confirmacao. */
			printf(" retransmitindo quadro de confirmacao.\n");
			msg[0] = sap_remoto;
			msg[1] = sap_local;
			msg[2] = CTRL_CON_ACP;
			msg[3] = 0;
			mac->dataRequest(mac_remoto, msg, 4);
		} else {
		
			/* Se receber um pedido de conexao de um sap remoto que nao seja o que ele acha q conectado, envia uma msg de rejeicao.*/
			printf(" rejeitando.\n");
			msg[0] = sap_remoto;
			msg[1] = sap_local;
			msg[2] = CTRL_CON_REJ;
			msg[3] = 0;
			mac->dataRequest(mac_remoto, msg, 4);
		}
	
	}
	
	
}

int Sap::send (char * msg, int tamanho)
{
	// A fazer...
}

