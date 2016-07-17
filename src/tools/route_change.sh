#########################################################################
# File Name: route_change.sh
# Author: Haiyang Zhou
# mail: geo.zhouhy@gmail.com
# Created Time: 一  9/14 11:10:35 2015
#########################################################################
#!/bin/bash

#lineend=`ifconfig | grep -n -E "^en1" | awk -F":" '{print $1}'`
#declare -i lineend
#linestart=`ifconfig | grep -n -E "^en0" | awk -F":" '{print $1}'`
#declare -i linestart
#linenum=$(( lineend-linestart ))
#lineend=$(( lineend-1 ))
#en0ip=`ifconfig | head -$lineend | tail -$linenum | grep -E "^\\tinet " | awk -F" " '{print $2}'`


gateway=""

for ip in `netstat -nr | grep 'default' | sed -E "s/ +/ /g" | awk '{print $2}'`
do
    if [ -z `echo $ip | grep -o -E "\d{1,3}\\.\d{1,3}\\.\d{1,3}\\.\d{1,3}"` ]; then
        continue;
    fi
    gateway=$ip
    break;
done
echo "gateway:"$gateway

for server in `cat ipcn.txt`
do
    sudo route $1 $server $gateway > /dev/null
done

#for domain in `cat domaincn.txt`
#do
#    serveraddr=`nslookup ${domain} | tr "\n" "," | awk -F":" '{print $NF}' | awk -F"," '{print $1}'`
#    addrchk=`echo -n $serveraddr | grep -E "\d{1,3}\\.\d{1,3}\\.\d{1,3}\\.\d{1,3}"`
#    if [ -z "$addrchk" ]; then continue; fi
#    sudo route add ${serveraddr}/32 $gateway
#done
