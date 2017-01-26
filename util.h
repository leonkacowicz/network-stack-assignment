#ifndef UTIL_H_
#define UTIL_H_

extern "C" {
	#include <stdlib.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <errno.h>
	#include <signal.h>
	#include <termios.h>
	
	#include <sys/time.h>
	#include <sys/mman.h>
	
	int peek = -1;
	struct termios orig, new_term;
	
	void open_port()
	{
	    if (tcgetattr(0,&orig)==-1) 
	    {
	        printf("Could not tcgetattr");
	        exit(1);
	    }
	
	    new_term = orig;
		
	    /* settings for raw mode */
	    new_term.c_lflag     &= ~ICANON;
	    new_term.c_lflag     &= ~ECHO;
	    new_term.c_lflag     &= ~ISIG;
	    new_term.c_cc[VMIN]  =  1;
	    new_term.c_cc[VTIME] =  0;
		
	    if (tcsetattr(0,TCSANOW,&new_term)==-1) 
	    {
	        printf("Could not tcsetattr");
	        exit(1);
	    }	
	}
	
	void close_port()
	{
	    tcsetattr(0, TCSANOW, &orig);
	}
	
	int kbhit()
	{
	    char ch;
	    int  nread;
	
	    if(peek != -1) return 1;
	
	    new_term.c_cc[VMIN] = 0;
	    tcsetattr(0, TCSANOW, &new_term);
	    nread = read(0, &ch, 1);
	    new_term.c_cc[VMIN] = 1;
	    tcsetattr(0, TCSANOW, &new_term);
	
	    if(nread == 1)
	    {
	        peek = ch;
	        return 1;
	    }
	
	    return 0;
	}
	
	int readch()
	{
	    char ch;
	
	    if(peek != -1)
	    {
	        ch   = peek;
	        peek = -1;
	        return ch;
	    }
	
	    read(0, &ch, 1);
	    return ch;
	}
}


#endif /*UTIL_H_*/
