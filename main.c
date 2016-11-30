#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dlfcn.h>

#define BUFLEN 64  			//Max length of buffer
#define PORT 666    		//The port on which to listen for incoming data

struct params
{
	float a;
	float b;
	float (*fun)(float, float);
}params;

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	uint8_t space = 0;
	char buf_temp[BUFLEN];

	// variables for dynamic library
	void* lib_handle;
	char* error_msg;

	// variables for socket
	struct sockaddr_in si_server, si_client;
    int s, slen = sizeof(si_client), recv_len;

    // buffer for incoming data
    char buf[BUFLEN];

    float (*operation)(float, float);

    lib_handle = dlopen("/root/C_Projects/Projekt-C-server/libdlib.so", RTLD_LAZY);

    if (!lib_handle)
    {
    	fprintf(stderr, "Error during dlopen(): %s\n", dlerror());
    	exit(1);
    }

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
		printf("\nReceived packet from %s:%d\n", inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port));
		printf("Data received: %s\n" , buf);

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
				space = space + i + 1;
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

		operation = dlsym(lib_handle, buf_temp);

		error_msg = dlerror();
		if (error_msg)
		{
		    sprintf(buf, "Wrong parameter of operation! Please try again.\n");

		    //now reply the client with the result
			if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr*) &si_client, slen) == -1)
			{
				die("sendto()");
			}

		    exit(1);
		}

		memset((char *) &buf, 0, sizeof(buf));

		sprintf(buf, "Result: %.3f.\n", operation(params.a, params.b));
		printf("Sent packet: %s", buf);

        //now reply the client with the result
        if (sendto(s, buf, sizeof(buf), 0, (struct sockaddr*) &si_client, slen) == -1)
        {
            die("sendto()");
        }

        memset((char *) &buf, 0, sizeof(buf));
    }

    close(s);

	return 0;
}
