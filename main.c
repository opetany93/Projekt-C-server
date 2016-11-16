#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "dlib.h"

#define BUFLEN 64  			//Max length of buffer
#define PORT 666    		//The port on which to listen for incoming data

float (*operation)(float, float);

struct params
{
	float a;
	float b;
	char  *operation;
}params;

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	uint8_t space;
	uint8_t buf_temp[BUFLEN];
	struct sockaddr_in si_server, si_client;

    int s, err = 0, slen = sizeof(si_client), recv_len;

    char buf[BUFLEN];

    //create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *) &si_server, 0, sizeof(si_server));

    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(PORT);
    si_server.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_server, sizeof(si_server) ) == -1)
    {
        die("bind");
    }

    //keep listening for data
    while(1)
    {
    	printf("Waiting for data...");
        fflush(stdout);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_client, &slen)) == -1)
        {
            die("recvfrom()");
        }

        //print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port));
		printf("Data: %s\n" , buf);

		for(i=0; i < BUFLEN; i++)
		{
			if(buf[i] == ' ')
			{
				space = i + 1;
				break;
			}

			buf_temp[i] = buf[i];
		}

		params.a = (float)atof(buf_temp);

		for(i=0; i < BUFLEN;i++) buf_temp[i]='\0';

		for(i = 0; i < BUFLEN; i++)
		{
			if(buf[space + i] == ' ')
			{
				space = i + 1;
				break;
			}

			buf_temp[i] = buf[space + i];
		}

		params.b = (float)atof(buf_temp);

		for(i = 0; i < BUFLEN; i++) buf_temp[i]='\0';

		for(i = 0; i < BUFLEN; i++)
		{
			if(buf[space + i] == ' ')
			{
				break;
			}

			buf_temp[i] = buf[space + i];
		}

		operation = buf_temp;

		printf("Pierwszy wspolczynnik: %f\n" , params.a);
		printf("Drugi wspolczynnik: %f\n" , params.b);
		printf("Operacja: %s\n" , operation);

		memset((char *) &buf, 0, sizeof(buf));

//        if (!strcmp((params.operation),"add"))
//        {
//        	operation = add;
//        }
//        else if(!strcmp((params.operation),"subtract"))
//        {
//        	operation = subtract;
//        }
//        else if(!strcmp((params.operation),"multiply"))
//        {
//        	operation = multiply;
//        }
//        else if(!strcmp((params.operation),"divide"))
//        {
//        	operation = divide;
//        }
//        else
//        {
//        	err = -1;
//        }
//
//        //if(err == 0) sprintf(buf, "Result: %f", (*operation)(params.a, params.b));
//        //else if(err == -1) sprintf(buf, "Wrong parameters! Please try again.");
//
//        //now reply the client with the result
//        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_client, slen) == -1)
//        {
//            die("sendto()");
//        }
    }

    close(s);

	return 0;
}
