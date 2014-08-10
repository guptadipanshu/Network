The program run using the following commands in two termnial or computer by-
./reliable_receiver UDP_port filename_to_write
ie ./reliable_receiver 192.168.56.12:8080 output.txt
Sender side
./reliable_sender receiver_hostname receiver_port filename_to_xfer bytes_to_xfer
i.e  
./reliable_sender receiver_hostname 192.168.56.12:8080 inputfile.txt 300000


TCP RENO- The code implements TCP RENO LOGIC (Cummalative ACKS and Go back to N protocol). It will transfer files from one machine to other despite any congestion and packet drop, the entire tcp logic here is implemented using UDP sockets. Efficency is also a major factor that is taken care of in the code for implementing tcp logic. The code uses sliding window logic and has a dynamic buffer at receiver end to manage packets that are not received inorder.
