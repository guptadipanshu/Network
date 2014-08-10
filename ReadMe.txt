There are three folder in the directory -
HTTP CLIENT SERVER- implements a http client server to transfer file from server end to client end on requests made by client using C socket library.

TCP RENO- The code implements TCP RENO LOGIC (Cummalative ACKS and Go back to N protocol). It will transfer files from one machine to other despite any congestion and packet drop, the entire tcp logic here is implemented using UDP sockets. Efficency is also a major factor that is taken care of in the code for implementing tcp logic. The code uses sliding window logic and has a dynamic buffer at receiver end to manage packets that are not received inorder.

Distance Vector- The code here implements the actual distance vector routing protocol which routers use in real world to solve routing problem. The code can recover from any link failures in between transmission and can dynamically calculate the shortest path using distance vector. The code also solves the count to infinity problem that may occur in distance vector.




Below are steps how to setup test enviornment-
Have an Ubuntu 14.04 VM running on VirtualBox.

VirtualBox's default network setup is a NAT interface to the
outside world, provided by the host computer.First,You should use that internet access to apt-get install any programs you'll need, 
like gcc, make, gdb.Install iperf and tcpdump - iperf.
Replace the network interface. Make sure the VM is fully shut down, and go to 
its Settings->Network section. Switch the Adapter Type from NAT to "host-only", and click ok. 

Now that you have one VM properly set up, you can make copies of it. Once the VM is fully shut 
down, use the clone function, and be sure to choose the "reinitialize MAC addresses" option.
Once you have your VMs, you'll need to make sure they get different IP addresses - right now
they'll default to using the same one. Inside each VM, edit the /etc/rc.local file, and on 
the line before exit 0, add 

sudo ifconfig eth0 address

where address is 192.168.56.x. (For instance, the test setup use 192.168.56.12 - 192.168.56.19).

