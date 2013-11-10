#!/bin/bash

if [ "$1" == "clean" ] ; then
    rm -rf bin
    echo "Cleaning done!"
fi

if [ "$1" == "client" ] ; then
    
    #make the bin directory
    mkdir -p bin
    mkdir -p received

    #compile and make the binaries
    g++ client/*.cpp -lcrypto -o bin/client

    #my ip address
    myIP=$(ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1')
    numNodes=$(wc -l < "$2")
    numNodes=$(($numNodes + 1))

    echo "Running client on port number "$3
    printf "$numNodes\n$2\n$myIP" > bin/inputClient

    gnome-terminal -e "bash -c \"./bin/client; exec bash\""


fi

if [ "$1" == "server" ] ; then

    #make the bin directory
    mkdir -p bin

    g++ server/*.cpp -o bin/server

    #my ip address
    myIP=$(ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1')
    numNodes=$(wc -l < "$2")
    numNodes=$(($numNodes + 1))
    # myIP="127.0.0.1"
    nodeID=0

    while read -r line || [ -n "$line" ]
    do
        # echo $line
        IFS=':' read -a array <<< "$line"
        # echo "${array[0]}"
        if [ "${array[0]}" == "$myIP" ] ; then
        	#echo "matched"
        	#run the node terminal
        	port=$(echo ${array[1]} | grep -o "^[0-9]*")
        	folder=$(echo ${array[1]} | grep -o " .*"|sed "s/^[ ]*//g")
        	mkdir -p "$folder"
        	echo "Running a server on port number "$port
        	printf "$numNodes\n$nodeID\n$2" > bin/input$nodeID
        	# gnome-terminal -e "./bin/run.sh"
             gnome-terminal -t "Node $nodeID" -e "bash -c \"./bin/server < bin/input$nodeID; exec bash\""
        fi
        nodeID=$(($nodeID + 1))
    done < "$2"
fi