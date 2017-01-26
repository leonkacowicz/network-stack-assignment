#ifndef UART_H_
#define UART_H_

namespace phy {
	class Uart {
		protected:
			float errorProb;
			bool dataIndicationFlag;
			char outputData;
			char inputData;

		public:
			Uart();
			~Uart();
			void dataRequest(char data);
			bool dataIndication();
			char dataReceive();

			void setErrorProb(float prob);
			float getErrorProb();
			static bool setInterfaceName(char* interface);
			void mainLoop();
	};
}

#endif /*UART_H_*/
