#include "util.h"
#include <time.h>
#include <string.h>
#include "llc/Llc.h"

typedef enum 
{
	CMD_DATAGRAM = 1,
	CMD_LISTEN,
	CMD_CON,
	CMD_SEND,
	CMD_DISC,
	
} CMD;

int main(int argc,char **argv) {

	int comando = -1;
	SAP sap_local = -1;
	SAP sap_remoto = -1;
	MAC mac_remoto;
	MAC mac_local;
	
	bool bsap_local = false;
	bool bsap_remoto = false;		
	bool bmac_remoto = false;	
	
	
	char tmp;
	
	char mensagem[481];
	int posicao_msg = 0;
	
	
	Llc* llc;
	
	setbuf(stdout, NULL);
	fprintf(stdout, "Digite o caracter identificador da maquina: ");
	open_port();
	
	while (!kbhit());
	mac_local = readch();
	
	printf("%c\n", mac_local);

	llc = new Llc(mac_local);	

	while (1)
	{		
		if (comando == -1)
		{
			printf("\nEscolha o comando:\n1. Datagrama nao confiavel\n2. CV: Ouvir\n3. CV: Conectar\n4. CV: Enviar\n5. CV: Desconectar\n\nSua escolha: ");
		} else if (!bsap_local && (comando != CMD_DATAGRAM)) {
			printf("\nDigite o SAP local: ");
		} else if (!bsap_remoto && (comando == CMD_CON)) {
			printf("\nDigite o SAP remoto: ");
		} else if (!bmac_remoto && (comando != CMD_LISTEN) && (comando != CMD_SEND) && (comando != CMD_DISC)) {
			printf("\nDigite o MAC remoto: ");
		} else if ((posicao_msg == 0) && ((comando == 1) || (comando == CMD_SEND))) {
			printf("\nDigite a mensagem: ");
		}
		
		
		while (!kbhit()) 
		{
			llc->mainLoop();
		}
		
		tmp = readch();
		if (tmp == 'q') break;
		
		
		if (comando == -1)
		{
			if ((tmp >= '1') && (tmp <= '5')) comando = tmp - '0';
			printf("%c", tmp);
			
		} else if (!bsap_local && (comando != CMD_DATAGRAM)) {
			printf("%c\n", tmp);
			if ((tmp >= '0') && (tmp <= '3'))
			{
				bsap_local = true;
				sap_local = tmp - '0';
				
				if (comando == CMD_DISC)
				{
					llc->hungup(sap_local);
					bsap_local = false;
					comando = -1;
				} else if (comando == CMD_LISTEN) {
					llc->listen(sap_local);
					bsap_local = false;
					comando = -1;
				}
				
				
			}
		} else if (!bsap_remoto && (comando == CMD_CON)) {
			if ((tmp >= '0') && (tmp <= '3')) 
			{
				bsap_remoto = true;
				sap_remoto = tmp - '0';
			}
			printf("%c", tmp);
		} else if (!bmac_remoto && (comando != CMD_LISTEN) && (comando != CMD_SEND) && (comando != CMD_DISC)) {
			bmac_remoto = true;
			mac_remoto = tmp;
			printf("%c", tmp);
		} else if ((comando == CMD_DATAGRAM) || (comando == CMD_SEND)) {
			if ((tmp == '\r') || (tmp == '\n') || (posicao_msg == 480))
			{
				if (comando == CMD_DATAGRAM)
				{
					llc->sendDatagram(0, 0, mac_remoto, mensagem, posicao_msg);
					posicao_msg = 0;
					bmac_remoto = false;
					comando = -1;
				} else if (comando == CMD_SEND) {
					llc->send(sap_local, mensagem, posicao_msg);
					posicao_msg = 0;
					bmac_remoto = false;
					bsap_local = false;
					comando = -1;
				}
				
			}
		
			mensagem[posicao_msg++] = tmp;
			printf("%c", tmp);
		} 
		
		
	}


/*
	printf("Digite o endereco de destino, a mensagem e Enter para enviar:\n");
	
	while (!aborted) 
	{ 
		
		if (kbhit()) 
		{

			tmp = readch();

			switch (tmp)
			{
				case 10:
				case 13:
					printf(">> Enviando agora a mensagem...\n");
					//mac->dataRequest(addr_dest, buf, indice);
					llc->sendDatagram(0, 0, addr_dest, buf, indice);
					
					indice = 0;
					first_address = true;
					break;
				case 'q':
					aborted = true;
					break;
				case '+':
					// aumenta a probabilidade de erro
					ErrorProbability = ErrorProbability + 10;
					if (ErrorProbability > 100) ErrorProbability = 100;
					//mac->setErrorProbability(ErrorProbability);
					break;
				case '-':
					// diminui a probabilidade de erro
					ErrorProbability = ErrorProbability - 10;
					if (ErrorProbability < 0) ErrorProbability = 0;
					//mac->setErrorProbability(ErrorProbability);
					break;
				case '>':
					// aumenta a probabilidade de perda
					LossProbability = LossProbability + 10;
					if (LossProbability > 100) LossProbability = 100;
					//mac->setLossProbability(LossProbability);
					break;
				case '<':
					// diminui a probabilidade de perda
					LossProbability = LossProbability - 10;
					if (LossProbability < 0) LossProbability = 0;
					//mac->setLossProbability(LossProbability);
					break;
				case '[':
					// envia uma mensagem predefinida pequena
					while(indice<strlen(preDefinida1))
							buf[indice++] = preDefinida1[indice];
					break;
				case ']':
					// envia uma mensagem predefinida grande
					while ( indice<strlen(preDefinida2) )
							buf[indice++] = preDefinida2[indice];
					break;
				default:
					//Typing the destiny address
					if (first_address)
					{
						printf("Endereco de destino = %c\n", tmp);
						if (tmp == '!')
							addr_dest = 0xFF;
						else
							addr_dest = tmp;
						first_address = false;
						break;
					}

					printf("%c", tmp);
					buf[indice++] = tmp;
					if (indice == 480)
					{
						printf(">> Enviando agora a mensagem...\n");
						//mac->dataRequest(addr_dest, buf, indice);
						llc->sendDatagram(0, 0, addr_dest, buf, indice);
						
						indice = 0;
						first_address = true;
					}
					break;
			}//switch
		}
		
		llc->mainLoop();
		
		MAC mac_remoto;
		SAP sap_remoto;
		
		num_bytes_msg_recebida = llc->receiveDatagram (0, &sap_remoto, &mac_remoto, buf_entrada);
		if (num_bytes_msg_recebida)
		{
			//num_bytes_msg_recebida = mac->dataReceive(buf_entrada);
			printf("received %d bytes: \"", num_bytes_msg_recebida);
			for (i = 0; i < num_bytes_msg_recebida; i++) printf("%c", buf_entrada[i]);
			printf("\"\n");
			
		}
	}
	*/
	close_port();
	printf("closed! \n");
	delete llc;
	return 0;
	
}

/*
int main(int argc,char **argv) {
	Llc* llc;
	
	bool first_address = true;
	bool aborted = false;
	char addr = -1;
	char addr_dest = -1;
	char buf[480];
	char buf_entrada[481];
	int indice = 0;
	char tmp;
	int num_bytes_msg_recebida;
	int ErrorProbability = 0;
	int LossProbability = 0;
	int i;
	
	char preDefinida1[] = "puc-rio";
	char preDefinida2[] = "Este eh o nosso trabalho 2 de redes. Parece que tudo esta funcionando perfeitamente.";

	puts("Trabalho 2 de Redes");
	puts("Componentes: Andrei, Leon e Rafael");
	puts("Comandos:");
	puts("'q' : Sai do programa.");
	puts("'[' : Coloca a string pre-definida de 7 bytes no buffer.");
	puts("']' : Coloca a stirng pre-definida de 84 bytes no buffer.");
	puts("'+' : Aumenta a probabilidade de erro.");
	puts("'-' : Diminui a probabilidade de erro.");
	puts("'>' : Aumenta a probabilidade de perda de quadro.");
	puts("'<' : Diminui a probabilidade de perda de quadro.");
	puts("'!' : Endereco de broadcast (=0xFF).");

	setbuf(stdout, NULL);
	fprintf(stdout, "Digite o caracter identificador da maquina: ");
	open_port();
	while (addr == -1)
	{
		if (kbhit()) addr = readch();
	}

	printf("%c\n", addr);
	
	if (addr == 'q')
	{
		close_port();
		printf("closed! \n");
		return 0; 
	}	

	llc = new Llc(addr);	

	printf("Digite o endereco de destino, a mensagem e Enter para enviar:\n");
	
	while (!aborted) 
	{ 
		
		if (kbhit()) 
		{

			tmp = readch();

			switch (tmp)
			{
				case 10:
				case 13:
					printf(">> Enviando agora a mensagem...\n");
					//mac->dataRequest(addr_dest, buf, indice);
					llc->sendDatagram(0, 0, addr_dest, buf, indice);
					
					indice = 0;
					first_address = true;
					break;
				case 'q':
					aborted = true;
					break;
				case '+':
					// aumenta a probabilidade de erro
					ErrorProbability = ErrorProbability + 10;
					if (ErrorProbability > 100) ErrorProbability = 100;
					//mac->setErrorProbability(ErrorProbability);
					break;
				case '-':
					// diminui a probabilidade de erro
					ErrorProbability = ErrorProbability - 10;
					if (ErrorProbability < 0) ErrorProbability = 0;
					//mac->setErrorProbability(ErrorProbability);
					break;
				case '>':
					// aumenta a probabilidade de perda
					LossProbability = LossProbability + 10;
					if (LossProbability > 100) LossProbability = 100;
					//mac->setLossProbability(LossProbability);
					break;
				case '<':
					// diminui a probabilidade de perda
					LossProbability = LossProbability - 10;
					if (LossProbability < 0) LossProbability = 0;
					//mac->setLossProbability(LossProbability);
					break;
				case '[':
					// envia uma mensagem predefinida pequena
					while(indice<strlen(preDefinida1))
							buf[indice++] = preDefinida1[indice];
					break;
				case ']':
					// envia uma mensagem predefinida grande
					while ( indice<strlen(preDefinida2) )
							buf[indice++] = preDefinida2[indice];
					break;
				default:
					//Typing the destiny address
					if (first_address)
					{
						printf("Endereco de destino = %c\n", tmp);
						if (tmp == '!')
							addr_dest = 0xFF;
						else
							addr_dest = tmp;
						first_address = false;
						break;
					}

					printf("%c", tmp);
					buf[indice++] = tmp;
					if (indice == 480)
					{
						printf(">> Enviando agora a mensagem...\n");
						//mac->dataRequest(addr_dest, buf, indice);
						llc->sendDatagram(0, 0, addr_dest, buf, indice);
						
						indice = 0;
						first_address = true;
					}
					break;
			}//switch
		}
		
		llc->mainLoop();
		
		MAC mac_remoto;
		SAP sap_remoto;
		
		num_bytes_msg_recebida = llc->receiveDatagram (0, &sap_remoto, &mac_remoto, buf_entrada);
		if (num_bytes_msg_recebida)
		{
			//num_bytes_msg_recebida = mac->dataReceive(buf_entrada);
			printf("received %d bytes: \"", num_bytes_msg_recebida);
			for (i = 0; i < num_bytes_msg_recebida; i++) printf("%c", buf_entrada[i]);
			printf("\"\n");
			
		}
	}
	close_port();
	printf("closed! \n");
	delete llc;
	return 0;
}*/
