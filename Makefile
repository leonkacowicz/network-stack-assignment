main: main.cpp phy/Uart.h phy/Uart.cpp mac/Mac.h mac/Mac.cpp llc/Llc.h llc/Llc.cpp llc/Sap.h llc/Sap.cpp
	g++ main.cpp phy/Uart.cpp mac/Mac.cpp llc/Llc.cpp llc/Sap.cpp -g -o main
