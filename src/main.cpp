#include <iostream>
#include <cstring>

#include "WinsockHelpers.h"

using namespace std;

#define PAUSE() \
	cout << endl << "Press enter to continue.."; \
	cin.get(); \
	cout << endl;

int main(int argc, char * argv[])
{
	char * host = NULL;
	char * path = NULL;
	char * method = NULL;

	if (argc < 2) {
		cout << "Usage:\n\n\t";
		cout << "http-req host_name [req_path] [req_method]" << endl << endl;
	} else  {
		host = argv[1];
		if (argc > 2) {
			int pathLen = strlen(argv[2]);
			cout << endl << "pathLen: " << pathLen << endl;
		
			path = new char[pathLen + 2];
			path[0] = '/';

			int i;
			for (i = 1; i <= pathLen; i++) 
				path[i] = argv[2][i-1];

			path[pathLen + 1] = 0;

		} else {
			path = "/";
		}

		if (argc > 3) {
			method = argv[3];
		} else {
			method = "GET";
		}
	}

	WSADATA wsaData = init_winsock(true);

	char *http_res = NULL;
	http_req(host, method, path, http_res);

	cout << http_res << endl;

	WSA_CLEANUP();
	PAUSE();
	return 0;
}