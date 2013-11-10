#!/bin/bash

#make the bin directory
mkdir -p bin

#compile and make the binaries
g++ client/*.cpp -lcrypto -o bin/client
g++ server/*.cpp -o bin/server

#my ip address

myIP=$(ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1')
numNodes=$(wc -l < "$1")
numNodes=$(($numNodes + 1))
myIP="127.0.0.1"
nodeID=0

while read -r line || [ -n "$line" ]
do
    # echo $line
    IFS=':' read -a array <<< "$line"
    # echo "${array[2]}"
    if [ "${array[0]}" == "$myIP" ] ; then
    	# echo "matched"
    	#run the node terminal
    	port=$(echo ${array[1]} | grep -o "^[0-9]*")
    	folder=$(echo ${array[1]} | grep -o " .*"|sed "s/^[ ]*//g")
    	mkdir -p "$folder"
    	echo "Running a server on port number "$port
    	printf "$numNodes\n$nodeID\n$1" > bin/input$nodeID
    	# gnome-terminal -e "./bin/run.sh"
        gnome-terminal -e "bash -c \"./bin/server < bin/input$nodeID; exec bash\""
    fi
    nodeID=$(($nodeID + 1))
done < "$1"

#remove the temporary file to give inputs
# rm  bin/input*
# rm -f bin/run.sh