#!/bin/sh
cd /tmp
sudo useradd so2_sat -s /sbin/nologin -M
sudo mv so2_tp1-sat.service /lib/systemd/system/.
sudo chmod 755 /lib/systemd/system/so2_tp1-sat.service
sleep 5
sudo systemctl enable sleepservice.service
sudo systemctl start sleepservice
sudo journalctl -f -u sleepservice
