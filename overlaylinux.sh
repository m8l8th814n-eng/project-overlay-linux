#!/bin/bash
TEXT="overlaylinux is hard"
CHARS="!@#$%^&*()_+{}|:<>?~a󰣇bcdefghijklmnXXopÅÖÖqrstuvwÄÄxDy!zA!BCD?EFGHIJKLMNOPQRST󰝚UVWXYZ01󱄛23456789"
COLORS=("\033[31m" "\033[32m" "\033[33m" "\033[34m" "\033[35m" "\033[36m" "\033[91m" "\033[92m" "\033[93m" "\033[94m" "\033[95m" "\033[96m")
RESET="\033[0m"

clear
echo -e "\n\n\t"

for (( i=0; i<${#TEXT}; i++ )); do
    for (( j=0; j<8; j++ )); do
        RAND_INDEX=$(( RANDOM % ${#CHARS} ))
        RAND_CHAR="${CHARS:$RAND_INDEX:1}"
        RAND_COLOR="${COLORS[$(( RANDOM % ${#COLORS[@]} ))]}"
        
        echo -ne "${RAND_COLOR}${RAND_CHAR}${RESET}"
        sleep 0.04
        echo -ne "\b"
    done
    
    ACTUAL_CHAR="${TEXT:$i:1}"
    if (( RANDOM % 4 == 0 )); then
        echo -ne "\033[1;37m${ACTUAL_CHAR}${RESET}" 
    else
        echo -ne "\033[1;36m${ACTUAL_CHAR}${RESET}"
    fi
    sleep 0.1
done

sleep 0.5
echo -ne "\r\t"
for (( i=0; i<${#TEXT}; i++ )); do
    if (( RANDOM % 5 == 0 )); then
        RAND_INDEX=$(( RANDOM % ${#CHARS} ))
        RAND_COLOR="${COLORS[$(( RANDOM % ${#COLORS[@]} ))]}"
        echo -ne "${RAND_COLOR}${CHARS:$RAND_INDEX:1}${RESET}"
    else
        echo -ne "\033[1;36m${TEXT:$i:1}${RESET}"
    fi
done
sleep 0.1
echo -ne "\r\t\033[1;36ooverlaylinux is advanced\033[0m\n\n"

