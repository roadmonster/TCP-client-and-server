/**
 * Project 1 http_cli source code
 * This program send the request to the server and display the http response on the stderr and
 * redirect the body of the response to the intended file if needed, otherwise, will display the body.
 *
 *@author Hao Li
 * CPSC 5510 Computer Network
 * This is a free software released to the public
 */
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;
class Util{
public:
    Util(string url){
        this->request = parse_out_http(url);
        this->host = getHost();
        this->resource_uri = getResourceURI();
        this->resource_format = getResourceFormat();
        this->port = getPort();
    }

    int getPortNum(){
        return this->port;
    }

    string getHostName(){
        return this->host;
    }

    string getResourceName(){
        return this->resource_uri;
    }

    string getFormat(){
        return this->resource_format;
    }

    string create_response(){
        const string connection_status = "close";
        string eol = "\r\n";
        string request_line = "GET " + resource_uri + " " + "HTTP/1.1" + eol;
        string header_line1 = "Connection: " + connection_status + eol;
        string header_line2 = "Host: " + host + eol;

        string req = request_line + header_line1 + header_line2 + eol;
        return req;
    }
private:
    int port;
    string host;
    string resource_uri;
    string resource_format;
    string request;

    string parse_out_http(string url){
        int idx1 = url.find_first_of(":") + 3;
        string url2 = url.substr(idx1);
//        cout<<"request: "<<url2<<endl;
        return url2;
    }

    string getHost(){
        string host;
        int idx2 = request.find_first_of(":");
        if(idx2 == -1){
            int idx3 = request.find_first_of("/");
            if( idx3 == -1){
                return request;
            }
            else{
                return request.substr(0, idx3);
            }
        }
        host = request.substr(0,idx2);
//        cout<<"Host:"<<host<<endl;
        return host;
    }

    string getResourceURI(){
        int idx1 = request.find_first_of("/");
        string res;
        if(idx1 == -1){
            res = "/";
        }else{
            res = request.substr(idx1);
        }

//        cout<<"resource: "<<res<<endl;
//        cout<<"resource.length: "<<res.length()<<endl;
        return res;
    }

    string getResourceFormat(){
        string res = resource_uri.substr(resource_uri.find_first_of(".") +1);
//        cout<<"format: "<<res<<endl;
        return res;
    }

    int getPort(){
        int port;
        int idx1 = request.find_first_of("/");
        int colon_index = request.find_first_of(":");

        if(colon_index == -1){
            port = 80;
        }
        else{
            int port_index = colon_index + 1;
            string port_num = request.substr(port_index, idx1-port_index);
            stringstream stuff(port_num);
            stuff >> port;
        }
//        cout<<"Port: "<<port<<endl;
        return port;
    }
};

class Client{
public:
    Client(int port, string address){

        this->server_port = to_string(port).c_str();
        this->server_address = address.c_str();
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = 0;
        hints.ai_protocol = 0;          /* Any protocol */
        this->sockfd = connect_server();
    }

    void send_msg(Util* util) {
        int total = 0, n;

        char resp[256];
        strcpy(resp, util->create_response().c_str());
        int len = util->create_response().length();
        while ((n = send(this->sockfd, resp, len - total, 0)) > 0)
            total += n;

        if (n == -1) {
            cerr<<"Error: sending failed.\n";
            exit(EXIT_FAILURE);
        }
    }
    string receive_msg(){
//        string server_resp = "";
//
//        char resp[256];
//        while(recv(this->sockfd, &resp, 256, 0) > 0){
//            server_resp.append(resp);
//        }

        char resp;
        string res = "";

        while(read(this->sockfd, &resp, 1) > 0){
            res.push_back(resp);

        }

        return res;

    }

    void display(string res){

        int index = 0; //index to loop through the http
        for(index = 0; index < res.size(); index++){
            cerr<<res[index];
            if(res[index+1] == '\r')
                if(res[index+2] == '\n')
                    if(res[index+3] == '\r')
                        if(res[index+4] == '\n'){
                            break;
                        }
        }

        index += 5; //plus 4 to point to the beginning of the body
        res = res.substr(index); // get the body

        cout.write(res.c_str(), res.length());
    }
private:
    int sockfd;
    const char* server_port;
    const char* server_address;
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    int connect_server(){
        int s = getaddrinfo(server_address, server_port, &hints, &result);
        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }

        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sockfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
            if (sockfd == -1)
                continue;

            if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
                break;                  /* Success */

            close(sockfd);
        }
        if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not connect\n");
            exit(EXIT_FAILURE);
        }

        freeaddrinfo(result);
        return sockfd;
    }
};
int main(int argc, char const *argv[]){
    if (argc < 2) {
        fprintf(stderr,"usage %s url (optional: > filename) \n", argv[0]);
        exit(0);
    }

    string url = argv[1];
    Util util(url);
    cerr<<util.create_response();

    Client cli (util.getPortNum(), util.getHostName());
//    cout<<"going to send msg: "<<endl;
    cli.send_msg(&util);
//    cout<<"msg sent"<<endl;
    string serv_resp = cli.receive_msg();
//    cout<<"trying to receive msg\n";
    cli.display(serv_resp);

}


