#!/bin/bash
APP_ID=$(ps -eaf | grep  -i "[n]ohup ./vivent" | awk '{print $2}')
if [ -n "${APP_ID}" ]
then
	echo "A Vivent process ($APP_ID) is already running!"
	echo "Do you want to kill it and start an other? [y/n then ENTER]"
	read ANSWER
	if [ 'y' == "${ANSWER}" ]
	then
		sudo kill  "${APP_ID}"
		echo "Process ${APP_ID} has been killed and an other started!"
		echo " "
		sudo rm stdout.txt
		sudo nohup ./vivent > stdout.txt &
		sleep 2
		sudo cat stdout.txt
	elif [ 'n' == "${ANSWER}" ]
	then
		echo "Process ${APP_ID} has NOT been killed!"
	else
		echo "Type y or n!"
	fi		
else
	sudo rm stdout.txt
	sudo nohup ./vivent > stdout.txt &
	sleep 2
	sudo cat stdout.txt
fi
