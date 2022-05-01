#include <time.h>
#include <string.h>
#include <stdio.h>
#include "zmq.h"
#include "string"
#include <unistd.h>
#include "map"


typedef struct MD
{
	int clientId;
	int messageNumber;
	char message[128];
} MessageData;

using namespace std;

int main(int argc, char const *argv[])
{
	int id = 5;
	void* context = zmq_ctx_new();
	printf("Publisher %d Starting...\n", id);

	void* publishSocket = zmq_socket(context, ZMQ_PUSH);

	string str = "tcp://127.0.0.1:5000";
	zmq_bind(publishSocket, str.c_str());
	int count = 0;
	for (;;)
	{
		MessageData md;
		memcpy(md.message, "Hello world\0", strlen("Hello world\0")+1);
		md.messageNumber = count;
		zmq_msg_t zmqMessage;
		zmq_msg_init_size(&zmqMessage, sizeof(MessageData));
		memcpy(zmq_msg_data(&zmqMessage), &md, sizeof(MessageData));

		printf("Sending: - %d\n", count);
		// zmq_send(publishSocket, "lol", 3, ZMQ_SNDMORE);
		int send = zmq_msg_send(&zmqMessage, publishSocket, ZMQ_DONTWAIT);
		zmq_msg_close(&zmqMessage);

		usleep(1000 * 1000 * 1);
		count++;
	}
	zmq_close(publishSocket);
	zmq_ctx_destroy(context);

	return 0;
}


