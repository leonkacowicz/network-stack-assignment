#include "util.h"
#include <time.h>
#include <string.h>
#include "mac/Mac.h"


int main(int argc,char **argv) {
	Mac* mac;
	
   bool first_address = true;
	bool aborted = false;
	char addr = -1;
	char addr_dest = -1;
	char buf[255];
	char buf_entrada[256];
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

	mac = new Mac(addr);	

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
				   mac->dataRequest(addr_dest, buf, indice);	   
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
				   mac->setErrorProbability(ErrorProbability);
				   break;
			   case '-':
				   // diminui a probabilidade de erro
				   ErrorProbability = ErrorProbability - 10;
				   if (ErrorProbability < 0) ErrorProbability = 0;
				   mac->setErrorProbability(ErrorProbability);
				   break;
				case '>':
				   // aumenta a probabilidade de perda
				   LossProbability = LossProbability + 10;
				   if (LossProbability > 100) LossProbability = 100;
				   mac->setLossProbability(LossProbability);
				   break;
			   case '<':
				   // diminui a probabilidade de perda
				   LossProbability = LossProbability - 10;
				   if (LossProbability < 0) LossProbability = 0;
				   mac->setLossProbability(LossProbability);
				   break;
			   case '[':
				   // envia uma mensagem predefinida pequena
				   while ( indice<strlen(preDefinida1) )
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
			      if (indice == 255)
			      {
				      printf(">> Enviando agora a mensagem...\n");
				      mac->dataRequest(addr_dest, buf, indice);
				      indice = 0;
				      first_address = true;
			      }
				   break;
		   }//switch
		}
		
		mac->mainLoop();
		if (mac->dataIndication())
		{
			num_bytes_msg_recebida = mac->dataReceive(buf_entrada);
			printf("received %d bytes: \"", num_bytes_msg_recebida);
			for (i = 0; i < num_bytes_msg_recebida; i++) printf("%c", buf_entrada[i]);
			printf("\"\n");
			
		}
	}
	close_port();
	printf("closed! \n");
	delete mac;
	return 0;
}
