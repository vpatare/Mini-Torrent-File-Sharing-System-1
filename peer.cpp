#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#define PORT 4444
#define CHUNK_BUFFER_SIZE 512000
#define TOTAL_HASH_SIZE 100000
#define BUFFER_SIZE 1024

using namespace std;



void bin2hex( unsigned char * src, int len, char * hex )
{
    int i, j;

    for( i = 0, j = 0; i < len; i++, j+=2 )
        sprintf( &hex[j], "%02x", src[i] );
}

size_t split(char *buffer, char *argv[], size_t argv_size)
{
    char *p, *start_of_word;
    int c;
    enum states { DULL, IN_WORD, IN_STRING } state = DULL;
    size_t argc = 0;

    for (p = buffer; argc < argv_size && *p != '\0'; p++) {
        c = (unsigned char) *p;
        switch (state) {
            case DULL:
                if (isspace(c)) {
                    continue;
                }

                if (c == '"') {
                    state = IN_STRING;
                    start_of_word = p + 1;
                    continue;
                }
                state = IN_WORD;
                start_of_word = p;
                continue;

            case IN_STRING:
                if (c == '"') {
                    *p = 0;
                    argv[argc++] = start_of_word;
                    state = DULL;
                }
                continue;

            case IN_WORD:
                if (isspace(c)) {
                    *p = 0;
                    argv[argc++] = start_of_word;
                    state = DULL;
                }
                continue;
        }
    }

    if (state != DULL && argc < argv_size)
        argv[argc++] = start_of_word;

    return argc;
}
vector <char*> test_split(const char *s)
{
    char buf[1024];
    size_t i, argc;
    char *argv[20];

    strcpy(buf, s);
    vector <char*> out;
    argc = split(buf, argv, 20);
    for (i = 0; i < argc; i++)
        out.push_back(argv[i]);

    return out;
}

typedef struct mtorrent {
    char* file;
    char *hash_of_hash;
    char* ip_port;
    int success;

}seeder_data;

seeder_data create_mtorrent(char* path, char* dest, char* tracker1, char* tracker2, char* ip_port) {

    seeder_data data;

    char temp_path[BUFFER_SIZE];
    strcpy(temp_path, path);
    struct stat filestat;
    stat(temp_path, &filestat);
    char* abs_path = realpath(temp_path, NULL);
    size_t size = filestat.st_size;
    FILE *in;
    FILE *out;
    size_t byte_read = 0;
    unsigned char buffer[CHUNK_BUFFER_SIZE];
    in = fopen(path, "rb");
    out = fopen(dest, "wb");
    fprintf(out, "%s\n%s\n%s\n%zu\n", tracker1, tracker2, abs_path, size);
    char str[2*SHA_DIGEST_LENGTH];
    char hash_of_hash[2*SHA_DIGEST_LENGTH];
    unsigned char obuf[SHA_DIGEST_LENGTH];
    unsigned int itr = 0;
    char total_hash[TOTAL_HASH_SIZE];
    while(1) {
        int n =fread(buffer, 1, sizeof(buffer), in);
        if(n <= 0)
            break;
        SHA1(buffer, n, obuf);
        bin2hex(obuf, sizeof(obuf), str);
        fwrite(str, 1, 20, out);
        fwrite("\n", 1, 1, out);

        if(itr == 0) {
            strncpy(total_hash, str, 20);
        }
        else{
            strncat(total_hash, str, 20);
        }
        itr++;

    }
    //printf("%s\n", total_hash);
    SHA1((unsigned char*)total_hash, strlen(total_hash), obuf);
    bin2hex(obuf, sizeof(obuf), hash_of_hash);
    memset(total_hash, 0, sizeof(total_hash));
    data.file = abs_path;
    data.hash_of_hash = hash_of_hash;
    data.ip_port = ip_port;
    data.success = 1;
    fclose(in);
    fclose(out);

    return data;

}

void peer_as_server(string ip_port) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in peerservaddr, peercliaddr;
    string peer_upload_ip_str = ip_port.substr(0, ip_port.find_first_of(":"));
    string peer_upload_port_str = ip_port.substr(peer_upload_ip_str.size() + 1);
    char peer_upload_ip[peer_upload_ip_str.size() + 1];
    //char peer_upload_port[peer_upload_port_str.size() + 1];
    strcpy(peer_upload_ip, peer_upload_ip_str.c_str());
    //strcpy(peer_upload_port, peer_upload_port_str.c_str());
    int peer_upload_port = stoi(peer_upload_port_str);

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("[-] Error creating peer server\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("[+] peer server created\n");

    memset(&peerservaddr, 0, sizeof(peerservaddr));
    memset(&peercliaddr, 0, sizeof(peercliaddr));
    printf("ip = %s port = %d\n", peer_upload_ip, peer_upload_port);
    peerservaddr.sin_family = AF_INET;
    peerservaddr.sin_addr.s_addr = inet_addr(peer_upload_ip);
    peerservaddr.sin_port = htons(peer_upload_port);

    if ( bind(sockfd, (const struct sockaddr *)&peerservaddr,
              sizeof(peerservaddr)) < 0 )
    {
        printf("[-] Error in binding peer server\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    char* client_ip_upload_port = argv[1];
    char* tracker1_ip_port = argv[2];
    char* tracker2_ip_port = argv[3];
    int clientSocket, ret;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    string client_upload(client_ip_upload_port);

    thread peer_upload(peer_as_server, client_upload);
    peer_upload.detach();

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");



    while(1) {
        fgets(buffer, 1024, stdin);
        //getline(buffer, 1024);
        vector<char *> command = test_split(buffer);
        //printf("%s\n", command[0]);
        seeder_data data;
        if (strcmp(command[0], "share") == 0) {
            data = create_mtorrent(command[1], command[2], tracker1_ip_port, tracker2_ip_port, argv[1]);
            if(data.success == 1) {
                char to_send[BUFFER_SIZE];
                strcpy(to_send, data.file);
                strcat(to_send, " ");
                strcat(to_send, data.hash_of_hash);
                strcat(to_send, " ");
                strcat(to_send, data.ip_port);
                strcat(to_send, "\n");
                send(clientSocket, to_send, strlen(to_send), 0);
                printf("SUCCESS\n");
                //printf("file = %s\n", data.file);
                //printf("hash = %s\n", data.hash_of_hash);
                //printf("port_ip = %s\n", data.ip_port);

            }
            else
                printf("not executed\n");
            bzero(buffer, sizeof(buffer));
        }


        if (strcmp(command[0], "get") == 0) {
            FILE* fp = fopen(command[1], "rb");


        }


        if (strcmp(command[0], "exit") == 0) {
            send(clientSocket, command[0], sizeof(command[0]), 0);
            close(clientSocket);
            printf("disconnecting\n");
            exit(1);
        }
        bzero(buffer, sizeof(buffer));

    }

    return 0;
}
