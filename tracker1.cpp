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
#include <fstream>
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
        recv(newSocket, buffer, 1, 0);

        //exit
        if (strcmp(buffer, "0") == 0) {
            //recv(newSocket, buffer, 1024, 0);
            printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
            //terminate();
            //close(newSocket);
            break;
        }


        //share
        if(strcmp(buffer, "1") == 0){                       //controls share only
            recv(newSocket, buffer, 1024, 0);

            string temp(buffer);
            istringstream iss(temp);
            vector<string> result((istream_iterator<string>(iss)), istream_iterator<string>());

            //printf("%s\n", buffer);
            seeder_file = fopen("seeders.txt", "ab");
            //string client_address = inet_ntoa(newAddr.sin_addr);   //getting clients address and port
            //int cli_port = ntohs(newAddr.sin_port);
            //string client_port = to_string(cli_port);
            string client_ip_port = result[2];
            fwrite(buffer,1, strlen(buffer), seeder_file);
            fclose(seeder_file);
            fwrite("\n", 1, 1, seeder_file);

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

        else if(strcmp(buffer, "2") == 0) {
            recv(newSocket, buffer, 1024, 0);
            //printf("%s\n", buffer);
            string hash(buffer);
            vector<pair<string, string> > found_hashes;
            if(seeder_list.find(hash) == seeder_list.end()){
                printf("hash not found\n");
                int zero = 0;
                int temp = htonl(zero);
                send(newSocket, &temp, sizeof(temp) , 0);
            }
            else {
                printf("found\n");
                found_hashes = seeder_list[hash];
                int total_found = found_hashes.size();
                for (int i = 0; i < found_hashes.size(); i++) {
                    cout << "file = " << found_hashes[i].first << "\n" << "ip port = " << found_hashes[i].second << "\n";
                }
                int temp = htonl(total_found);
                send(newSocket, &temp, sizeof(temp), 0);
                for(int i = 0; i < total_found; i++) {
                    char file_path[found_hashes[i].first.size() + 1];
                    char ip_port[found_hashes[i].second.size() + 1];
                    strcpy(file_path, found_hashes[i].first.c_str());
                    strcpy(ip_port, found_hashes[i].second.c_str());
                    send(newSocket, file_path, strlen(file_path), 0);
                    send(newSocket, ip_port, strlen(ip_port), 0);
                }
            }
        }
        bzero(buffer, 1024);

    }

    close(newSocket);
    //printf("here\n");


}

int main(int argc, char* argv[]){


    char* tracker1_ip_port = argv[1];
    char* tracker2_ip_port = argv[2];

    string tracker1_ip_port_str(tracker1_ip_port);
    string tracker1_ip_str = tracker1_ip_port_str.substr(0, tracker1_ip_port_str.find_first_of(":"));
    string tracker1_port_str = tracker1_ip_port_str.substr(tracker1_ip_str.size() + 1);
    char tracker1_ip[tracker1_ip_str.size() + 1];
    //char peer_upload_port[peer_upload_port_str.size() + 1];
    strcpy(tracker1_ip, tracker1_ip_str.c_str());
    //strcpy(peer_upload_port, peer_upload_port_str.c_str());
    int tracker1_port = stoi(tracker1_port_str);

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
    serverAddr.sin_port = htons(tracker1_port);
    serverAddr.sin_addr.s_addr = inet_addr(tracker1_ip);

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


    ifstream infile("seeders.txt");

    string line;

    while(getline(infile, line)) {
        istringstream iss(line);
        vector<string> result((istream_iterator<string>(iss)), istream_iterator<string>());
        if (seeder_list.find(result[1]) == seeder_list.end()) {
            //printf("not found\n");
            vector<pair<string, string> > second;
            second.push_back(pair<string, string>(result[0], result[2]));
            seeder_list.insert(pair<string, vector<pair<string, string> > >(result[1], second));
        } else {
            //printf("found\n");
            seeder_list[result[1]].push_back(pair<string, string>(result[0], result[2]));
        }
    }
    /*for (auto i = seeder_list.begin(); i != seeder_list.end(); i++) {
        cout << i->first << "==> \n";
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            cout << "file=" << j->first << "\n" << " ip_port=" << j->second << "\n";
        }

    }*/
    while(1){
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if(newSocket < 0){
            printf("[-]connection denied\n");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        thread cl(client, newSocket, newAddr);
        cl.detach();

        //break;
    }

    close(newSocket);


    return 0;
}

