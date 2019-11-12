openssl req -newkey rsa:2048 -sha256 -keyout clasdigital.pem -out clasdigitalrequest.pem \
	    -batch -subj "/C=DE/ST=Hessen/L=Frankfurt am Main/O=Johann Wolfgang Goethe-Universitaet Frankfurt am Main/OU=Fachbereich 10/CN=www.clas-digital.uni-frankfurt.de" \
	    -reqexts SAN \
	    -config <(cat /etc/ssl/openssl.cnf <(printf "[SAN]\nsubjectAltName=DNS:www.clas-digital.uni-frankfurt.de,DNS:clas-digital.uni-frankfurt.de"))
