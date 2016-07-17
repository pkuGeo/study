#########################################################################
# File Name: create_vpn_setting.sh
# Author: Haiyang Zhou
# mail: geo.zhouhy@gmail.com
# Created Time: 三  9/23 11:27:54 2015
#########################################################################
#!/bin/bash

echo '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>PPTP</key>
  <dict>
    <key>UserConfigs</key>
    <array>'
cat $1 | grep -E "(\[.*\])|(^PhoneNumber)" | tr "\n" "," | sed "s/,$//g" | sed "s/,\[/:\[/g" | sed "s/PhoneNumber=//g" | sed "s/\[//g" | sed "s/\]//g" | tr ":," "\n\t" | sort | tr "\t" ":" | while read line
do
    vpn_name=`echo $line | cut -d ":" -f1`
    vpn_host=`echo $line | cut -d ":" -f2`
    echo "      <dict>
        <key>EAP</key>
        <dict/>
        <key>IPSec</key>
        <dict/>
        <key>PPP</key>
        <dict>
          <key>AuthName</key>
          <string>v1214563</string>
          <key>AuthPasswordEncryption</key>
          <string>Keychain</string>
          <key>CCPEnabled</key>
          <integer>1</integer>
          <key>CCPMPPE128Enabled</key>
          <integer>1</integer>
          <key>CCPMPPE40Enabled</key>
          <integer>1</integer>
          <key>CommRemoteAddress</key>
          <string>${vpn_host}</string>
          <key>UserDefinedName</key>
          <string>${vpn_name}</string>
        </dict>
      </dict>"
done

echo '    </array>
  </dict>
</dict>
</plist>'
