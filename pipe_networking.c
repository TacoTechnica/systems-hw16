#include "pipe_networking.h"

#define TO_SERVER_FIFO "server_very_secure_fifo"
#define TO_CLIENT_FIFO "client_super_secure_fifo"

char server_side_buffer[HANDSHAKE_BUFFER_SIZE];
char client_side_buffer[HANDSHAKE_BUFFER_SIZE];

/*=========================
    server_handshake
    args: int * to_client

    Perofrms the server side pipe 3 way handshake.
    Sets *to_client to the file descriptor to the downstream pipe.

    returns the file descriptor for the upstream pipe.
=========================*/
int server_handshake(int *to_client) {

    // Create server "from client" fifo
    if (mkfifo(TO_SERVER_FIFO, 0644) == -1) {
        printf("Error creating from_clients pipe in server handshake!\n");
        exit(1);
    }
    printf("Server opens its fifo...\n");
    int from_client = open(TO_SERVER_FIFO, O_RDONLY);

    // Wait for client to send us their fifo
    printf("Server waiting for client to send us their fifo...\n");
    read(from_client, server_side_buffer, HANDSHAKE_BUFFER_SIZE);
    printf("Server received client fifo! Fifo value: \"%s\" \n", server_side_buffer);

    // Send data back to client for confirmation
    *to_client = open(server_side_buffer, O_WRONLY);

    write(*to_client, server_side_buffer, HANDSHAKE_BUFFER_SIZE); // 
    printf("wtf server\n");
    // STOP AFTER THIS?

    // Now wait for the client to send us a verification signal, completing the handshake
    char status;
    read(from_client, &status, 1);
    if (status == 1) {
        printf("Server received client verification %d: SUCCESS!\n", status);
    } else {
        printf("Server received client verification %d: FAILURE! :(\n", status);
    }
    return 0;
}


/*=========================
    client_handshake
    args: int * to_server

    Perofrms the client side pipe 3 way handshake.
    Sets *to_server to the file descriptor for the upstream pipe.

    returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

    char *to_client_fifo = TO_CLIENT_FIFO;

    // Create private fifo
    if (mkfifo(to_client_fifo, 0644) == -1) {
        printf("Error creating from_server pipe in client handshake!\n");
        exit(1);
    }

    // Connect to server fifo
    printf("Client opens its fifo...\n");
    *to_server = open(TO_SERVER_FIFO, O_WRONLY);
    if (*to_server == -1) {
        printf("Client failed to open server public pipe\n");
        exit(1);
    }
    // Send our client fifo to server
    printf("Client sends fifo to server\n");
    write(*to_server, to_client_fifo, HANDSHAKE_BUFFER_SIZE);//strlen(to_client_fifo));
    // (now server waits for our fifo name)

    // Now, we should get our data back from the server as
    // a confirmation


    // Actually open our client's receiving-data file
    int from_server = open(to_client_fifo, O_RDONLY);
    // STOPS AFTER THIS ?
    read(from_server, client_side_buffer, HANDSHAKE_BUFFER_SIZE);//strlen(to_client_fifo));
    printf("wtf client\n");

    printf("Client received server data! Received: \"%s\"", client_side_buffer);

    int success = 0; // TODO: Dejankify
    // Now, we verify whether the server's data is correct!
    if (!strcmp(client_side_buffer, to_client_fifo)) {
        // Correct! Yay!
        success = 1;
        printf("Client verified server data, correct! Sending good call...\n");
    } else {
        // Incorrect! uh oh
        success = 0;
        printf("Client verified server data, INCORRECT! Sending good call...\n");
    }
    write(*to_server, &success, 1);

    return 0;
}
