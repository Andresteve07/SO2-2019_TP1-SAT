#!/bin/sh
# cd into repository directory
sudo useradd so2tp1sat -s /sbin/nologin -M
sudo cp scripts/so2_tp1-sat.service /lib/systemd/system/.
sudo chmod 755 /lib/systemd/system/so2_tp1-sat.service
sleep 2
sudo systemctl enable so2_tp1-sat.service
sleep 2
sudo systemctl start so2_tp1-sat.service
# loggin 
# sudo journalctl -f -u so2_tp1-sat
# stop service 
sudo systemctl stop so2_tp1-sat.service
# disables service 
sudo systemctl disable so2_tp1-sat.service