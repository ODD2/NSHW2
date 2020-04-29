# Introduction
NSHW2 is a project that provides shell access through TLS/SSL connection. The connection requires both end certificates to be valid.

Bellow are the required libraries:
  - libssl-dev
  
Bellow are the techniques that're used in this project:
 * [socket]
 * [epoll]
 * [fork]
 * [ssl]
 
[socket]: <http://man7.org/linux/man-pages/man2/socket.2.html>
[epoll]:<http://man7.org/linux/man-pages/man7/epoll.7.html>
[fork]:<http://man7.org/linux/man-pages/man2/fork.2.html>
[ssl]:<https://www.openssl.org/docs/manmaster/man3/>

### Installation

NSHW2 requires [openssl](https://www.openssl.org/source/) to run.

Install the libssl-dev dependency
```sh
$ sudo apt-get update
$ sudo apt-get install libssl-dev
```

NSHW2 provides default certificates for both server and client.
To validate those certificates, we need to install the CA that signed them.
```sh
$ ./install_ca_sh
```
The script will them prompt a dialog, select 'yes' in the first dialog.
On the next dialog, scroll down to the bottom until you meet OD.crt.
press space to select it then press enter to finish the installation.

These process will install the required ca into /etc/ssl/certs, which is the place NSHW2 client/server searches for trusted ca.

Finally, open two terminals.
```sh
#terminal 1
./NSHW2_server
```
```sh
#terminal 2
./NSHW2_client
```


### Notice
NSHW2 now serves **only on localhost(127.0.0.1):5000**.
The certificate for server is located in the ./server/cert.pem.
The certificate for client is located in the ./client/cert.pem.

Replacing it changes the certificate provided to server/client .
**Before replacing, check the validity in advance**.
```sh
openssl verify $cert_loc
```
