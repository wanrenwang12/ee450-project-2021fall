

Name:   Dongdong Yang
USC ID: 7060108809

                                         Project Summary
	I have finished phase one, phase two and phase three. Two client can send input names to central, central server will send request to three query/compute servers separately. Then clients get correct results from central server.
-----------------------------------------------------------------------------------------------------


				    Code files description:
central.cpp:
	The central.cpp sends the input names from clients to serverT, then T will reply several message contains two names, like <nameA, nameB>. After receiving each message from T, central sends the message to serverS. Then central will receive several message like "nameA:scoreA, nameB>scoreB;", After that central transmits two inputs names and results from serverS to serverP, and get two different result messages from serverP. Finally, central sends two results message to clientA and clientB.

serverT.cpp:
	The serverT is a server can find all related path for two inputs. First, it reads edgelist.txt and stores all information into <name, name>. Then find related path by iteration. To avoid compute repeatedly, set "isPush" as a symbol to represent whether the path has been visited or not.

serverS.cpp:
	The serverS is a simple server which can query scores of all related path. To simplify the procedure, the serverS reads all scores and store as map<name, score> in advance. When receiving a message from central(format is 'nameA,nameB', the server analyses two names and find their scores in map. Then server send back to central

serverP.cpp:
	The server is a little complicated. There are two class, edge class and graph class. When receiving each message from central, serverP computing their matching gap, adding edge, and node to graph. Then by algorithm "dijkstra", serverP gets all nodes' shortest path and matching gap to start node( which is nameB). Finally, serverP finds nameB's path and matching gap and send them to central.

ClientA.cpp/clientB.cpp:
	The function of clientA and clientB are same. They check the input format and send them to central, then wait results from central.
----------------------------------------------------------------------------------------------------


			  The format of all the messages exchanged:
clientA.cpp/clientB.cpp:
	sendToCentral: inputname
	recvFromCentral: char array contains final results which can be 'cout' directly.

ServerT.cpp:
	recvFromCentral: two input name
	sendToCentral: all related path, 'name1, name2;'

ServerS.cpp:
	recvFromCentral: all related path, 'name1, name2;'
	sendToCentral: all related path and scores, 'name1:score1, name2>score2;'

ServerP.cpp:
	recvFromCentral: two input name, all related path and scores
	sendToCentral: final results, like" Found compatibility ...Compatibility score:..." Or "Found no compatibility..."

central.cpp:
	recvFromClientA/B: input name
	sendToT: input names
	recvFromT_sendToS: related path 'name1, name2;'
	recvFromS_sendToP: related path 'name1, name2;' and 'name1:score1,name2>score2;'
	recvFromP: Final results that can be displayed directly
-----------------------------------------------------------------------------------------------------


                               The idiosyncrasies of my project:
	My project work well under test case posted on the instruction. However, if the inputs from clients are incorrect or not included in edgelist.txt/scores.txt, the servers cannot figure out a way to solve or send the error message. The clients will get "Found no compatibility between "null" and "null" " as a result and show on terminal.
	And TAs said all inputs names will be less than 512 characters, so I set the size of buffer for sending and receiving as 1000. It should be enough under the most condition. However, if the names both are greater than 500, and with other information (scores, results etc.), it cannot transmit correct message between each other. But this problem can be solved by just increase the size of buffer. And I think no one has name that is greater than 500 characters. It is meaningless to increase the size.
	The third situation is two client send the same names. The central and other servers deal with this condition as usual. They cannot find two inputs are same, and reply to clients " Found no compatibility between name and name."
	
----------------------------------------------------------------------------------------------------
					Reused Code:
For socket:
1. Beej
2. https://www.geeksforgeeks.org/socket-programming-cc/

For format transfer:
1.https://stackoverflow.com/questions/16747915/c-converting-a-string-to-double
2.https://stackoverflow.com/questions/1786497/sprintf-double-precision-in-c/1786866

For dijistra:
1.https://blog.csdn.net/weixin_42375679/article/details/112466514

