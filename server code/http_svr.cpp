/**
 * Main function
 */
#include <iostream>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstring>

#include <map>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <ctime>
#include <algorithm>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

/**
 * This class does all the parsing and validating for the server class
 */
class Util2{

public:

    /**
     * Ctor
     * @param req ptr to the client request string
     */
    Util2(string* req){
        this->code_info = setup_code_info();
        this->mime_info = setup_mime_info();
        this->resource = retri_resource(req);

        this->filename = retri_filename();
        this->path = retri_path();
        this->connection_status = retri_connection_status(req);
    }

    /*
     * Destructor
     */
    ~Util2(){
        delete[]code_info;
        delete[]mime_info;
        this->code_info = nullptr;
        this->mime_info = nullptr;

    }

    /**
     * Validate the client request, change the error code when needed
     * @param req client request string
     * @param code error code to be changed
     */
    void validate_request(string* req, int* code){
        vector<string> methods;
        methods.push_back("GET");
        methods.push_back("POST");
        methods.push_back("DELETE");
        methods.push_back("PATCH");

        int index = req->find_first_of("/");
        string method= req->substr(0, index-1);

        int http_index = req->find("HTTP");

        //int connection_index = req->find("Connection");

        int host_index = req->find("Host");


        if(count(methods.begin(), methods.end(), method) == 0 || http_index == -1||host_index ==-1)
        {
            *code = 400;
            return;
        }

        if(method.compare("GET") != 0){
            *code = 501;
            return;
        }
    }

    /**
     * Create the response header message
     * @param code error code to be included
     * @return the response header
     */
    string create_response_msg(int code){

        string curr_time = get_curr_time();
        string connection_status = get_connection_status();
        string file_path = get_path();
        string EOL = "\r\n";
        string protocol = "HTTP/1.1";
        string line1 = protocol + " " + to_string(code) + " " + code_info->at(code) + EOL;
        string line2 = "Connection: " + connection_status + EOL;
        string line3 = "Date: " + curr_time + EOL;
        string response;

        if(code == 200){
            string lastModTime = get_mod_time(file_path);
            string line4 = "Last Modified: " + lastModTime + EOL;
            string MIME = get_mime(filename);
            string line6 = "Content Type: " + MIME + EOL + EOL;
            int file_len = get_file_len(file_path);
            string line5 = "Content Length: " + to_string(file_len) + EOL;
            response = line1 + line2 + line3 + line4 + line5 + line6;
        }
        else
            response = line1 + line2 + line3 + EOL;
        return response;
    }

    /*
     * Getter for filename
     */
    string get_filename(){
        return this->filename;
    }

    /**
     * Getter for path
     * @return
     */
    string get_path(){
        return this->path;
    }

    /**
     * Getter for connection status
     * @return
     */
    string get_connection_status(){
        return this->connection_status;
    }

    /**
     * Testing the path of the request exist or not
     * @param p path of the file
     * @param s
     * @return true if exist or vice versa
     */
    bool demo_exists(const fs::path& p, fs::file_status s = fs::file_status{})
    {
        if(fs::status_known(s) ? fs::exists(s) : fs::exists(p)){

            return true;
        }

        else{

            return false;
        }

    }


    /**
     * Wrapper method for accessing and returnning the resources on the server
     * @param code Error code
     * @return char array representation for the resource
     */
    char* read_resource(int* code){

        //find the format of the resource
        string format = filename.substr(filename.find_first_of(".") + 1);

        //judge if the format is supported
        vector<string> formats;
        formats.emplace_back("txt");
        formats.emplace_back("jpg");
        formats.emplace_back("jpeg");
        formats.emplace_back("png");
        formats.emplace_back("html");
        formats.emplace_back("htm");

        if(count(formats.begin(),formats.end(),format) == 0){
            *code = 501;
            return nullptr;
        }
        //read text file
        if(format.compare("txt") == 0 || format.compare("html") == 0 || format.compare("htm") == 0){
            return read_target();
        }
        else{

            char*img = read_image(code);
            if(*code == 501){
                delete[] img;
                return nullptr;
            }
            return img;
        }

    }

    /**
     * Get the length of the resouce
     * @param file_path path for the resource
     * @return integer length of the resource
     */
    int get_file_len(string file_path){
        struct stat stat_buffer;

        if(stat(file_path.c_str(), &stat_buffer) == -1){
            cerr<<"stat()";
        }
        int length = stat_buffer.st_size;
        return length;
    }

private:
    map<int, string>* code_info;
    map<string,string>* mime_info;
    string filename;
    string resource;
    string path;
    string connection_status;

    /**
     * Read text based resources
     * @return char array of the text resource
     */
    char* read_target(){
        ifstream text(this->path, ifstream::binary);
        char* buffer = nullptr;
        int len;
        if(text){
            text.seekg(0, text.end);
            len = text.tellg();
            cout<<"len: "<<len<<endl;

            text.seekg(0, text.beg);

            buffer = new char[len +1];

            text.read(buffer, len);

        }
        buffer[len] = '\0';
        text.close();

        return buffer;
    }

    /**
     * Access the binary files like image. Only image format is currently supported
     * @param code error code
     * @return char array
     */
    char* read_image(int* code){
        ifstream img(this->path, ifstream::in |ifstream::binary);

        if(img){
            img.seekg(0, img.end);
            int len = img.tellg();
            img.seekg(0, img.beg);
            char* res = new char[len];
            for(int i = 0; i < len; i++)
                res[i] = '5';

            bool bit = img.eof();

            img.read(res, len);
            img.close();
            return res;
        } else{
            *code = 501;
            return nullptr;
        }
    }

    /**
     * Parse the resource from the request
     * @param req client's request
     * @return resouce string
     */
    string retri_resource(string* req){
        int index = req->find_first_of("/");
        int index2 = req->find_first_of("\r");
        return req->substr(index, index2-9-index); // to be tested
    }

    /**
     * Parse the filename
     * @return
     */
    string retri_filename(){
        string filename;
        int index = resource.find_first_of(".");

        if(index == -1){
            filename = "index.html";
        }
        else
            filename = resource.substr(resource.find_last_of("/"));
        return filename;
    }


    /**
     * Set up the MIME info dictionary
     * @return established dictionary
     */
    map<string, string>* setup_mime_info(){
        map<string, string>* mime = new map<string, string>();
        mime->insert(pair<string, string>("txt", "txt/plain"));
        mime->insert(pair<string, string>("htm", "txt/htm"));
        mime->insert(pair<string, string>("html", "txt/html"));
        mime->insert(pair<string, string>("css", "txt/css"));
        mime->insert(pair<string, string>("jpg", "image/jpeg"));
        mime->insert(pair<string, string>("jpeg", "image/jpeg"));
        mime->insert(pair<string, string>("png", "image/png"));
        return mime;
    }

    /**
     * Set up error code dict
     * @return error code dict
     */
    map<int, string>* setup_code_info(){
        map<int, string>* code_info = new map<int, string>();
        code_info->insert(pair<int, string> (200, "OK"));
        code_info->insert(pair<int, string> (400, "Bad Request"));
        code_info->insert(pair<int, string> (404, "Not Found"));
        code_info->insert(pair<int, string> (501, "Not Implemented"));
        return code_info;
    }

    /**
     * Ge the MIME from the filename
     * @param filename
     * @return MIME type
     */
    string get_mime(string filename){
        int ext_idx = filename.find_first_of(".");
        string ext = filename.substr(ext_idx+1);
        if(mime_info->count(ext))
            return mime_info->at(ext);
        else
            return "Not implemented";
    }

    /**
     * Parse the path
     * @return path string
     */
    string retri_path(){
        string path;
        string root = "web_root";

        // format: "/a.jpg"
        if(resource.find(".")!= string::npos){

            path = resource;
        }
        // format: "/"
        else if(resource.length() == 1){
            path = resource + "index.html";
            cout<<"path = resource + index.html\n";
        }
        // format "/foo/"
        else if(resource.find_last_of("/") != resource.find_first_of("/")){
            path = resource + "index.html";
        }
        // format "/foo"
        else{
            path = resource + "/index.html";
        }
        path = root + path;
        return path;
    }

    /**
     * Parse the connections status
     * @param req
     * @return connection status string
     */
    string retri_connection_status(string* req){
        int index = req->find("Connection");
        string con_status;
        if(index == string::npos){
            con_status = "close";
            return con_status;
        }

        string temp_str = req->substr(index + 12);
        int len = temp_str.find_first_of("\r");

        con_status = temp_str.substr(0, len);
        return con_status;
    }

    /**
     * Get the last modified time in UTC
     * @param file_path
     * @return string representatio of the last mod time
     */
    string get_mod_time(string file_path){
        struct stat stat_buffer;
        if(stat(file_path.c_str(), &stat_buffer) == -1){
            cerr<<"Error in getting file status";
            return NULL;
        }
        else{
            struct timespec last_modify = stat_buffer.st_mtimespec;
            char lastModDate[80];
            strftime(lastModDate, sizeof(lastModDate), "%c %Z", gmtime(reinterpret_cast<const time_t *>(&(last_modify))));
            string lastModTime(lastModDate);
            return lastModDate;
        }
    }

    /**
     * Get current time in UTC
     * @return
     */
    string get_curr_time(){
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = gmtime(&rawtime);

        strftime(buffer,sizeof(buffer),"%c %Z",timeinfo);
        std::string curr_time(buffer);
        return curr_time;
    }
};

/**
 * This class does the logic of receiving and responding the request.
 * Error codes generated in cases of unsupported request.
 */
class Server{
public:

    /**
     * Ctor intialize and build the listening socket
     * @param portno port number
     */
    Server(const char* portno){
        this->sockfd = build_socket();
        this->portno = strtol(portno, NULL, 0);
        this->clilen = sizeof(cli_addr);

        prep_socket();

        if (::bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
            err_exit("Error: binding failed\n");
        }
        if(!listen_socket()){
            err_exit("Error: can not listening on the given socket\n");
        }
    }


    /**
     * Server listening and waiting for connection in multithreaded way
     */
    void standby(){

        while(true){
            newsockfd = accept(sockfd,
                               (struct sockaddr *) &cli_addr, reinterpret_cast<socklen_t *>(&clilen));
            //error handling
            if (newsockfd < 0){
                err_exit("ERROR on accept\n");
            }


            pid_t pid = fork();

            //error handling
            if (pid < 0)
                err_exit("ERROR on fork\n");
            //child thread
            else if (pid == 0){
                std::cout << "client connected\n";
                string req = get_msg(newsockfd); //problematic

                cout<<req<<endl;

                Util2 * util = new Util2(&req);
                int code = 200;

                util->validate_request(&req, &code);

                string filename = util->get_filename();
                string path = util->get_path();

                string err_resp;

                int total_len = 0;
                if(code !=200){
                    cout<<"error code: "<<code <<endl;
                    err_resp = util->create_response_msg(code);
                    cout<<err_resp;
                    char* r = new char[err_resp.length() + 1];
                    strcpy(r, err_resp.c_str());

                    send_msg(newsockfd, r, strlen(r));
                    delete []r;
                    exit(EXIT_FAILURE);
                }
                if (util->demo_exists(path) == false){
                    code = 404;
                    err_resp = util->create_response_msg(code);
                    cout<<err_resp;
                    char* r = new char[err_resp.length() + 1];
                    strcpy(r, err_resp.c_str());

                    send_msg(newsockfd, r, strlen(r));
                    delete []r;
                    exit(EXIT_FAILURE);
                }
                char* r = nullptr;
                string response;
                char* body = util->read_resource(&code);
                int body_len = util->get_file_len(path);

                response = util->create_response_msg(code);
                r = new char[response.length() + 1];
                strcpy(r, response.c_str());
                total_len = strlen(r) + body_len;

                if(code != 501){
                    memcpy(&r[strlen(r)], body, body_len);
                    delete[]body;
                }

                send_msg(newsockfd, r, total_len);
                delete [] r;
                exit(0);
            }
            close(newsockfd);

        }
    }

private:
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /**
     * create the socket
     * @return the socket file descriptor
     */
    int build_socket(){
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0){
            err_exit("ERROR opening socket\n");
        }
        return sock;
    }

    /**
     * initalize the server info for the socket
     */
    void prep_socket(){
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(portno);
    }

    /**
     * wrap method for listenting
     * @return true if successfully listenting with 5 available queueing place for clients
     */
    bool listen_socket(){
        if(listen(sockfd, 5)<0)
            return false;
        return true;
    }

    /**
     * Get the request from the client
     * @param socket_no
     * @return the request
     */
    string get_msg(int socket_no){
        char resp;
        string res = "";

        //read the request on the socket, break the loop when "\r\n\r\n" is received
        while(read(socket_no, &resp, 1) > 0){
            res.push_back(resp);
            if (res.find("\r\n\r\n") != -1)
                break;
        }
        return res;
    }

    /**
     * send the response back to the client
     * @param socket_no
     * @param msg reponse
     * @param len length of the message to be sent back
     */
    void send_msg(int socket_no,  char* msg, int len){

        int count = 0, index = 0;

        while((count = send(socket_no, msg, len-index, 0)) > 0){
            index += count;
        }
        if(count == -1){
            err_exit("Error in sending\n");
        }


    }

    /**
     * Utility method to print error and quit.
     * @param msg
     */
    void err_exit(string msg){
        cerr<<msg;
        exit(EXIT_FAILURE);
    }


};
int main(int argc, char const *argv[]) {

    if (argc < 2) {
        cerr<<"ERROR, no port provided\n";
        exit(EXIT_FAILURE);
    }
    Server server(argv[1]);// possible bug

    server.standby();
}


