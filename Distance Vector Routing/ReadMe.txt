TO run the code
Deploy several VM having IP address from 192.168.56.12- 192.168.56.19
then to select one VM as main sender run the command "./router [filetotransfer] [destination ip eg. 5(to send to address 192.168.56.15) ] " in that VM 
Run the command ./router in other remaining VM
use command like to do some fun testing-
	stop this host from receiving from 192.168.56.19: sudo iptables -I INPUT -s 192.168.56.19 -j DROP
	stop this host from sending to 192.168.56.19: sudo iptables -I OUTPUT -d 192.168.56.19 -j DROP
	delete that first rule if present: sudo iptables -D INPUT -s 192.168.56.19 -j DROP
	delete all rules: sudo iptables --flush
	
Distance Vector- The code here implements the actual distance vector routing protocol which routers use in real world to solve routing problem. The code can recover from any link failures in between transmission and can dynamically calculate the shortest path using distance vector. The code also solves the count to infinity problem that may occur in distance vector.
	
