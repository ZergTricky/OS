#include <string.h>
#include <stdio.h>
#include "zmq.h"

typedef struct MD
{
	int clientId;
	int messageNumber;
	char message[128];
} MessageData;

int main(int argc, char const *argv[])
{
	void* context = zmq_ctx_new();
	void* serverSocket = zmq_socket(context, ZMQ_PULL);
	zmq_connect(serverSocket, "tcp://127.0.0.1:200");
	zmq_connect(serverSocket, "tcp://127.0.0.1:5000");


	printf("Starting...\n");
	for (;;)
	{
		// printf("Trying to get...\n");
		zmq_msg_t message;
		zmq_msg_init(&message);
		int status = zmq_msg_recv(&message, serverSocket, ZMQ_DONTWAIT);
		if(status < 0){
			zmq_msg_close(&message);
			continue;
		}
		MessageData *m = (MessageData *)zmq_msg_data(&message);
		printf("Message from client: %d  messageId: %d message: %s\n", m->clientId, m->messageNumber, m->message);
		zmq_msg_close(&message);
	}
	zmq_close(serverSocket);
	zmq_ctx_destroy(context);

	return 0;
}