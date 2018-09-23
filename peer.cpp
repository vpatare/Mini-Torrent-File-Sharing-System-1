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
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <thread>

#define PORT 4444
#define CHUNK_BUFFER_SIZE 512000
#define TOTAL_HASH_SIZE 100000
#define BUFFER_SIZE 1024
#define print(x) printf("%s", x);

using namespace std;

//exit = send 0
//share = send 1
//get = send 2
//show downloads= send 3
//remove = send 4


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

void peer_client() {
    ;
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
        //fwrite("\n", 1, 1, out);

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
    fwrite("\n", 1, 1, out);
    fwrite(hash_of_hash, 1, 40, out);
    fclose(in);
    fclose(out);

    return data;

}


//thread
void peer_as_server(string ip_port) {
    int sockfd, nbytes, newsocket;
    char buffer[BUFFER_SIZE];
    char bigbuffer[1500];
    int opt = 1;
    struct sockaddr_in peerservaddr, peercliaddr;
    socklen_t addrlen;
    string peer_upload_ip_str = ip_port.substr(0, ip_port.find_first_of(":"));
    string peer_upload_port_str = ip_port.substr(peer_upload_ip_str.size() + 1);
    char peer_upload_ip[peer_upload_ip_str.size() + 1];
    //char peer_upload_port[peer_upload_port_str.size() + 1];
    strcpy(peer_upload_ip, peer_upload_ip_str.c_str());
    //strcpy(peer_upload_port, peer_upload_port_str.c_str());
    int peer_upload_port = stoi(peer_upload_port_str);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("[-] Error creating peer server\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("[+] peer server created\n");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
               &opt, sizeof(opt));

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
    else {
        printf("[+] Bind with peer server\n");
    }

    if(listen(sockfd, 10) == 0){
        printf("[+]Listening....\n");
    }else{
        printf("[-]Error in binding.\n");
    }
    while(1) {
        newsocket = accept(sockfd, (struct sockaddr *) &peerservaddr, &addrlen);
        //printf("coam\n");
        if (newsocket < 0) {
            printf("[--]connection from peer server denied");
            exit(1);
        }
        printf("[++] Connection from peer server accepted for %s:%d\n", inet_ntoa(peerservaddr.sin_addr),
               ntohs(peerservaddr.sin_port));
        recv(newsocket, buffer, 1024, 0);
        cout << "filepath recieved = " << buffer << "\n";
        char fname[100];
        strcpy(fname, buffer);

        FILE *fp = fopen(fname, "rb");
        if (fp == NULL) {
            printf("file not opened\n");
            exit(1);
        }

        struct stat filestat;
        stat(fname, &filestat);
        size_t size = filestat.st_size;
        long temp_size = htonl(size);
        double temp = (double) size / (1500 * 1.0);
        int residue = size % 1500;
        cout << "parts float = " << temp << "\n";
        int parts = ceil(temp);
        cout << "parts = " << parts << "\n";
        send(newsocket, &parts, sizeof(parts), 0);
        send(newsocket, &residue, sizeof(residue), 0);
        char temp_recv[1];

        for (int i = 0; i < parts; i++) {
            bzero(bigbuffer, sizeof(bigbuffer));
            fread(bigbuffer, 1, sizeof(bigbuffer), fp);
            cout << "*i = " << i << "->" << bigbuffer << "\n---------------\n";
            write(newsocket, bigbuffer, sizeof(bigbuffer));
            read(newsocket, temp_recv, 1);
        }
        cout << "sending done\n";

    }
}


int main(int argc, char* argv[]){
    char* client_ip_upload_port = argv[1];
    char* tracker1_ip_port = argv[2];
    char* tracker2_ip_port = argv[3];
    int clientSocket, ret;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    char rcvbuffer[1024];
    char bigbuffer[1500];


    //converting tracker1 into proper form
    string tracker1_ip_port_str(tracker1_ip_port);
    string tracker1_ip_str = tracker1_ip_port_str.substr(0, tracker1_ip_port_str.find_first_of(":"));
    string tracker1_port_str = tracker1_ip_port_str.substr(tracker1_ip_str.size() + 1);
    char tracker1_ip[tracker1_ip_str.size() + 1];
    //char peer_upload_port[peer_upload_port_str.size() + 1];
    strcpy(tracker1_ip, tracker1_ip_str.c_str());
    //strcpy(peer_upload_port, peer_upload_port_str.c_str());
    int tracker1_port = stoi(tracker1_port_str);


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
    serverAddr.sin_port = htons(tracker1_port);
    serverAddr.sin_addr.s_addr = inet_addr(tracker1_ip); // server address

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
        printf("%s\n", command[2]);
        char save_location[100];
        strcpy(save_location, command[2]);
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
                send(clientSocket, "1", 1, 0);
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
            char *hash = (char*) malloc(40 * sizeof(char));
            printf("path to save = %s\n", command[2]);
            FILE* fp = fopen(command[1], "r");
            char c;
            int line_count = 0;
            int i = 0;
            while ((c = getc(fp)) != EOF) {
                //printf("line no = %d\n", line_count);
                if(line_count == 5) {
                    //printf("%d\n", i);
                    hash[i] = c;
                    i++;
                }
                else {
                    if(c == '\n')
                        line_count++;
                }
            }
            hash[i] = '\0';
            //printf("hash = %s", hash);
            send(clientSocket, "2", 1, 0);
            send(clientSocket, hash, strlen(hash), 0);
            int total_found;
            recv(clientSocket, &total_found, sizeof(total_found), 0);
            int found = ntohl(total_found);
            printf("%d found\n", found);
            vector < pair <string, string> > found_files;
            bzero(buffer, BUFFER_SIZE);
            for(int i = 0; i < found; i++) {
                recv(clientSocket, buffer, BUFFER_SIZE, 0); //get filename
                send(clientSocket, "1", 1, 0);
                printf("recieved filename = %s\n", buffer);
                string file_path(buffer);
                cout << "in string = " << file_path << "\n";
                bzero(buffer, BUFFER_SIZE);
                recv(clientSocket, buffer, BUFFER_SIZE, 0); //get ip port to connect to
                send(clientSocket, "1", 1, 0);
                string ip_port(buffer);
                printf("ip_port recieved = %s\n", buffer);
                cout << "in string = " << ip_port << "\n";
                bzero(buffer, BUFFER_SIZE);
                found_files.push_back(make_pair(file_path, ip_port));
            }
            for(int i = 0; i < found; i++) {
                cout << "filepath = "<< found_files[i].first << " ip_port = " << found_files[i].second << "\n";
            }
            if(found > 0) {
                printf("inside found > 0\n");
                //close(clientSocket);
                //print("client socket closed\n");
                string remote_file_path_str = found_files[0].first;
                cout << "string file path = " << remote_file_path_str <<"\n";
                char remote_file_path[remote_file_path_str.size() + 1];
                strcpy(remote_file_path, remote_file_path_str.c_str());

                string ip_port_peer_connect =  found_files[0].second;
                string ip_peer_connect_str = ip_port_peer_connect.substr(0, ip_port_peer_connect.find_first_of(":"));
                string port_peer_connect_str = ip_port_peer_connect.substr(ip_peer_connect_str.size() + 1);
                char ip_peer_connect[ip_peer_connect_str.size() + 1];
                //char peer_upload_port[peer_upload_port_str.size() + 1];
                strcpy(ip_peer_connect, ip_peer_connect_str.c_str());
                //strcpy(peer_upload_port, peer_upload_port_str.c_str());
                int port_peer_connect = stoi(port_peer_connect_str);
                cout << "ip = " << ip_peer_connect << " port = " << port_peer_connect << "\n";
                print("mem cleared\n");

                int peersocket;
                int opt = 1;
                struct sockaddr_in peerservaddr;
                if ( (peersocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
                    printf("socket for p2p error\n");
                    exit(EXIT_FAILURE);
                }

                peerservaddr.sin_family = AF_INET;
                peerservaddr.sin_port = htons(port_peer_connect);
                peerservaddr.sin_addr.s_addr = inet_addr(ip_peer_connect); // server address

                ret = connect(peersocket, (struct sockaddr*)&peerservaddr, sizeof(peerservaddr));
                if(ret < 0){
                    printf("[-]Error in connection to downloading peer.\n");
                    exit(1);
                }
                printf("[+]Connected to download peer.\n");
                printf("filepath = %s length = %d\n", remote_file_path, strlen(remote_file_path));
                send(peersocket, remote_file_path, sizeof(remote_file_path), 0);
                bzero(buffer, BUFFER_SIZE);
                int parts;
                int residue;
                recv(peersocket, &parts, sizeof(parts), 0);
                recv(peersocket, &residue, sizeof(residue), 0);
                cout << "parts = " << parts << " residue = " << residue<< "\n";
                cout << "command2 = " << save_location << "\n";
                FILE* wr = fopen(save_location, "ab");
                if(wr == NULL) {
                    printf("cant open write file on peer\n");
                    exit(1);
                }
                int i = 0;
                while(i != parts) {
                    read(peersocket, bigbuffer, sizeof(bigbuffer));
                    write(peersocket, "1", 1);
                    fputs(bigbuffer, wr);
                    i++;
                }
                cout << "recieving done\n";
                close(peersocket);
                fclose(wr);
            }

        }


        if (strcmp(command[0], "exit") == 0) {
            send(clientSocket, "0", 1, 0);
            //send(clientSocket, command[0], sizeof(command[0]), 0);
            close(clientSocket);
            printf("disconnecting\n");
            //bzero(buffer, sizeof(buffer));
            exit(1);
        }
        bzero(buffer, sizeof(buffer));

    }

    return 0;
}
