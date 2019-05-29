mkdir build
mkdir doc
mkdir bin
sudo iptables -A PREROUTING -t nat -p tcp --dport 443 -j REDIRECT --to-ports 1409
sudo iptables -A PREROUTING -t nat -p tcp --dport 80 -j REDIRECT --to-ports 1408
sudo iptables -t nat -A OUTPUT -o lo -p tcp --dport 443 -j REDIRECT --to-port 1409
sudo iptables -t nat -A OUTPUT -o lo -p tcp --dport 80 -j REDIRECT --to-port 1408
sudo apt install iptables-persistent
