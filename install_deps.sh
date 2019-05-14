mkdir build
mkdir doc
mkdir bin
sudo iptables -t nat -I OUTPUT -p tcp -o lo --dport 443 -j REDIRECT --to-ports 1409
sudo iptables -t nat -I PREROUTING -p tcp -o lo --dport 443 -j REDIRECT --to-ports 1409
sudo iptables -t nat -I OUTPUT -p tcp -o lo --dport 80 -j REDIRECT --to-ports 1408
sudo iptables -t nat -I PREROUTING -p tcp -o lo --dport 80 -j REDIRECT --to-ports 1408
sudo apt install iptables-persistent
