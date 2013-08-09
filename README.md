http-req
========

Windows command-line application allowing basic http requests from the command
line. Uses Winsock 2 if available, capable of using Winsock 1 as well.

Usage
-----

    http-req HOSTNAME [PATH] [METHOD]

The last two path and method parameters are optional and the default is a `GET`
request to the root path `/`. In order to send a `HEAD` request to host
`coryg89.github.io` for the path `/index.html` you would enter the
following command.

    http-req coryg89.github.io /index.html HEAD

Build
------
The project includes solution and project files for Visual Studio and can only
be built using the Windows Visual C++ compiler in it's current state, but could
be easily adapted to use a Unix/BSD socket implementation as well.