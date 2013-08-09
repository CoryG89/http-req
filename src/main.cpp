#include <iostream>
#include <cstring>

#include "WinsockHelpers.h"

using namespace std;

#define PAUSE() cout << "\nPress enter to continue..\n"; cin.get();
#define EXIT(val) PAUSE(); return val;


int main(int argc, char * argv[])
{
	char * host = NULL;
	char * path = NULL;
	char * method = NULL;

	if (argc < 2) {
		cout << "\nUsage:\n\n\thttp-req host_name [req_path] [req_method]\n\n";
	} else  {
		/** The host to send the request is the first and only required argument
			if the following two arguments are omitted, the given host is sent
			an HTTP GET request for / the root path ( / ) **/
		host = argv[1];

		if (argc > 2) {
			path = argv[2];
		} else {
			path = "/";
		}

		if (argc > 3) {
			method = argv[3];
		} else {
			method = "GET";
		}


		WSADATA wsaData = init_winsock(true);

		char *http_res = NULL;
		http_req(host, method, path, http_res);

		cout << http_res << endl;
		WSA_CLEANUP();
	}
	EXIT(0);
}