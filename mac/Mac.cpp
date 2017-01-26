#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "Mac.h"

using namespace phy;

void Mac::ColocaNoBuffer(char c)
{
	input_buffer[buf_fim] = c;
	buf_fim = (buf_fim + 1) % TAM_BUF;
}
char Mac::RetiraDoBuffer()
{
	char r;
	r = input_buffer[buf_ini];
	buf_ini = (buf_ini + 1) % TAM_BUF;
	return r;
}
double tempo_decorrido(TIMEB* time_buffer)
{
	TIMEB curr_time;
	ftime(&curr_time);
	
	int secs = curr_time.time - time_buffer->time;
	int msecs = 0;

	if (curr_time.millitm==time_buffer->millitm)
		return (double)secs;

	else if (curr_time.millitm>time_buffer->millitm)
		msecs = (int)(curr_time.millitm - time_buffer->millitm);

	else
	{
		secs--;
		msecs = (int)((curr_time.millitm+1000) - time_buffer->millitm);
	}
	
	if (secs==0)
		return ((double)msecs)/1000;
	else
		return ((double)secs)+(((double)msecs)/1000);
}

void CopiaQuadro(Quadro * dest, Quadro * org)
{
	memcpy(dest, org, sizeof(Quadro));
}
void ResetQuadro(Quadro * q, bool Estado)
{
	memset(q, 0, sizeof(Quadro));
	q->Transmitido = Estado;
}
char CalculaFCS(Quadro * q)
{
	char FCS;
	FCS = q->Destino ^ q->Origem ^ q->TamanhoMsg;
	for (int i = 0; i < q->TamanhoMsg; i++) 
		FCS = FCS ^ q->Mensagem[i];
}
void PreencheMsgQuadro(Quadro * q, char * v)
{
	int i;
	for (i = 0; i < q->TamanhoMsg; i++)
		q->Mensagem[i] = v[i];
	for (; i < 255; i++)
		q->Mensagem[i] = 0;
}

Mac::Mac(MAC mac_addr)
{
	uart = new Uart();
	buf_saida_ini = 0;
	buf_saida_fim = 0;
	
	for (int i = 0; i < TAM_BUF_SAIDA; i++)
		ResetQuadro(&(outputData[i]), true);
	ResetQuadro(&inputData1, false);
	ResetQuadro(&inputData2, false);
	buf_ini = 0;
	buf_fim = 0;
	macAddress = mac_addr;
}

Mac::~Mac()
{
	delete uart;
}

bool Mac::Transmite()
{
	char buf;
	char volta = 0;
	int qtd_volta = 0;
	if (outputData[buf_saida_ini].Transmitido) return true;
	uart->mainLoop();
	uart->setErrorProb(errorProb); // Seta probabilidade de erro
	uart->mainLoop();
	if (!uart->dataIndication())
	{
		
		switch (outputData[buf_saida_ini].numBytesTransmitidos)
		{
			case 0:
				/* Uma nova tentativa foi iniciada */
				numTentativas++;
								
				buf = outputData[buf_saida_ini].Destino;
				break;
			case 1:
				buf = outputData[buf_saida_ini].Origem;
				break;
			case 2:
				buf = outputData[buf_saida_ini].TamanhoMsg;
				break;
			default:
				if (outputData[buf_saida_ini].numBytesTransmitidos == outputData[buf_saida_ini].TamanhoQuadro - 1)
				{
					buf = outputData[buf_saida_ini].FCS;
				} else {
					buf = outputData[buf_saida_ini].Mensagem[outputData[buf_saida_ini].numBytesTransmitidos - 3];		
				}
				break;
		}
		uart->dataRequest(buf);
		
		do {
			uart->mainLoop();
			if (uart->dataIndication())	
			{
				volta = uart->dataReceive();				
				qtd_volta++;
			}
		} while (uart->dataIndication());
		if ((qtd_volta != 1) || volta != buf)
		{
			/* Paramos de transmitir pois detectou-se uma colisao */
			printf("Colisao ou mensagem diferente devido a probabilidade de erro.\n");
			outputData[buf_saida_ini].numBytesTransmitidos = 0;
			return false;
		} else {
			this->ColocaNoBuffer(volta);
			outputData[buf_saida_ini].numBytesTransmitidos++;
			if (outputData[buf_saida_ini].numBytesTransmitidos == outputData[buf_saida_ini].TamanhoQuadro) 
			{
				outputData[buf_saida_ini].Transmitido = true;
				buf_saida_ini = (buf_saida_ini + 1) % TAM_BUF_SAIDA;
			}
			return true;
		}
		
	} else {
		/* Percebeu que o meio estava ocupado e nao transmitiu evitando colisao */
		return true;
	}
}
bool Mac::Recebe(bool timeout)
{
	unsigned char buf;
	
	// Depois de ficar 
	if (timeout) inputData1.numBytesTransmitidos = 0;
	
	uart->mainLoop();
	if (uart->dataIndication())
	{
		buf = uart->dataReceive();
		this->ColocaNoBuffer(buf);		
	}
	
	if (buf_ini != buf_fim)
	{
		
		buf = this->RetiraDoBuffer();

		switch (inputData1.numBytesTransmitidos)
		{
			case 0:
				printf("Destino: %c\n", buf);
				inputData1.Destino = buf;
				break;
			case 1:
				printf("Origem: %c\n", buf);
				inputData1.Origem = buf;
				break;
			case 2:
				printf("Tamanho: %d\n", buf);
				inputData1.TamanhoMsg = buf;
				inputData1.TamanhoQuadro = (buf < 16 ? 16 : buf) + 4;
				break;
			default:
				if (inputData1.numBytesTransmitidos == inputData1.TamanhoQuadro - 1)
				{
					inputData1.FCS = buf;
				} else {
					inputData1.Mensagem[inputData1.numBytesTransmitidos - 3] = buf;
				}
				break;
		}
		inputData1.numBytesTransmitidos++;
		if (inputData1.numBytesTransmitidos == inputData1.TamanhoQuadro)
		{
		   if ( (rand() % 100) > lossProb )  // else, mensagem perdida!
   		   inputData1.Transmitido = true;
		}
		return true;
	} else {
		return false;
	}
}
void Mac::dataRequest(MAC dest, char * msg, int tamanho)
{
	int i;
	if (((buf_saida_fim + 1) % TAM_BUF_SAIDA) == buf_saida_ini) return; // jogamos o quadro fora por falta de buffer de saida.
	
	
	outputData[buf_saida_fim].numBytesTransmitidos = 0;
	outputData[buf_saida_fim].Destino = dest;
	outputData[buf_saida_fim].Origem = macAddress;
	outputData[buf_saida_fim].TamanhoMsg = tamanho;
	outputData[buf_saida_fim].TamanhoQuadro = (tamanho < 16 ? 16 : tamanho) + 4;
	outputData[buf_saida_fim].Transmitido = false;
	PreencheMsgQuadro(&(outputData[buf_saida_fim]), msg);	
	outputData[buf_saida_fim].FCS = CalculaFCS(&(outputData[buf_saida_fim]));
	numTentativas = 0;
	
	printf("Mensagem %d \"", buf_saida_fim);
	for (i = 0; i < tamanho; i++) printf("%c", msg[i]);
	printf("\" empacotada: %d bytes.\nDestino: %c\nTamanho no quadro: %d\n\n", tamanho, outputData[buf_saida_fim].Destino, outputData[buf_saida_fim].TamanhoMsg);
	buf_saida_fim = (buf_saida_fim + 1) % TAM_BUF_SAIDA;
}
bool Mac::dataIndication()
{
   return dataIndicationFlag;
}
unsigned int Mac::dataReceive(char * msg)
{
	unsigned int tam;
	if (!dataIndicationFlag) return 0;

	memcpy(msg, inputData2.Mensagem, tam = inputData2.TamanhoMsg);
	ResetQuadro(&inputData2, false);
	dataIndicationFlag = false;
	return tam;
}

/* Acrescentada depois para poder fornecer o MAC de origem do quadro.
*/
unsigned int Mac::dataReceive(char * msg, MAC * mac_origem)
{
	unsigned int tam;
	if (!dataIndicationFlag) return 0;
	
	*mac_origem = inputData2.Origem;
	memcpy(msg, inputData2.Mensagem, tam = inputData2.TamanhoMsg);
	ResetQuadro(&inputData2, false);
	dataIndicationFlag = false;
	return tam;
}


void Mac::setLossProbability(unsigned int prob)
{
	lossProb = prob;
	printf("Probabilidade de perda: %d\n", lossProb);
}
void Mac::setErrorProbability(unsigned int prob)
{
	errorProb = prob;
   printf("Probabilidade de erro: %d\n", errorProb);
}
void Mac::mainLoop()
{
	bool colisao = false;
	bool timeout; // = "Ja passou tempo suficiente para receber";
	bool backoff; // = "ja passou tempo suficiente para nova tentativa de envio";
	
	timeout = (tempo_decorrido(&ultimaRecepcao) > IFS);
	backoff = (tempo_decorrido(&ultimaColisao) > waitTime);
	
	
	if ((outputData[buf_saida_ini].numBytesTransmitidos > 0) || (backoff && timeout))
	{
		colisao = !this->Transmite();
		if (colisao)
		{
			/* Ocorreu uma colisao */
			ftime(&ultimaColisao);
			if (numTentativas == 1)
			{
				// Sorteia um tempo aleatorio entre entre 1 e 10 ms
				waitTime = 0.001 + 9.0 * ((double)(rand() % 1000)/1000.0);
				
			} else if (numTentativas <= 10)	{
				waitTime *= 2;
			} else if (numTentativas > 16)
			{
				/* Desistimos de transmitir o quadro */
				outputData[buf_saida_ini].Transmitido = true;
				buf_saida_ini = (buf_saida_ini + 1) % TAM_BUF_SAIDA;
			}
		
		}
	}
	
	if (this->Recebe(timeout))
	{
		//setar a ultima hora que recebeu dado
		if (!colisao)
		{
			ftime(&ultimaRecepcao);
		}
		
		if (inputData1.Transmitido)
		{
			if ((inputData1.FCS == CalculaFCS(&inputData1))
			    && ((inputData1.Destino == macAddress) || (inputData1.Destino == 0xFF)))
			    // 0xFF = endereco para broadcast -> manda para todas as maquinas
			{
				dataIndicationFlag = true;
				CopiaQuadro(&inputData2, &inputData1);
			}
			ResetQuadro(&inputData1, false);	
		}

	}
}
MAC Mac::getMacAddr()
{
	return macAddress;
}
