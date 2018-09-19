#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <pthread.h>
#include <thread>

using namespace std;

#define PORT 4444

char buffer[1024];
map<string, vector<pair<string, string> > > seeder_list;
FILE* seeder_file;
struct sockaddr_in newAddr;


void client(int newSocket, struct sockaddr_in newAddr)
{

    while(1) {
        bzero(buffer, 1024);
        //printf("%s\n", buffer);
        recv(newSocket, buffer, 1024, 0);
        if (strcmp(buffer, "exit") == 0) {
            printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
            break;
        } else {                       //controls share only
            //printf("%s\n", buffer);
            seeder_file = fopen("seeders.txt", "ab");
            string client_address = inet_ntoa(newAddr.sin_addr);   //getting clients address and port
            int cli_port = ntohs(newAddr.sin_port);
            string client_port = to_string(cli_port);
            string client_ip_port = client_address + ":" + client_port;
            fwrite(buffer,1, strlen(buffer), seeder_file);
            fclose(seeder_file);
            fwrite("\n", 1, 1, seeder_file);
            string temp(buffer);
            istringstream iss(temp);
            vector<string> result((istream_iterator<string>(iss)), istream_iterator<string>());
            if (seeder_list.find(result[1]) == seeder_list.end()) {
                //printf("not found\n");
                vector<pair<string, string> > second;
                second.push_back(pair<string, string>(result[0], client_ip_port));
                seeder_list.insert(pair<string, vector<pair<string, string> > >(result[1], second));
            } else {
                //printf("found\n");
                seeder_list[result[1]].push_back(pair<string, string>(result[0], client_ip_port));
            }

            //printing seeder_list data structure
            for (auto i = seeder_list.begin(); i != seeder_list.end(); i++) {
                cout << i->first << "==> \n";
                for (auto j = i->second.begin(); j != i->second.end(); j++) {
                    cout << "file=" << j->first << "\n" << " ip_port=" << j->second << "\n";
                }

            }
        }
    }

    close(newSocket);


}

int main(){

    map<string, vector<pair<string, string> > > seeder_list;

    int sockfd, ret;
    int opt = 1;
    struct sockaddr_in serverAddr;


    int newSocket;

    socklen_t addr_size;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
               &opt, sizeof(opt));

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", 4444);

    if(listen(sockfd, 10) == 0){
        printf("[+]Listening....\n");
    }else{
        printf("[-]Error in binding.\n");
    }
    while(1){
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if(newSocket < 0){
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        thread cl(client, newSocket, newAddr);
        cl.detach();
    }

    close(newSocket);


    return 0;
}

