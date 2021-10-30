cp synchronizer.service /etc/systemd/system/
cp .synchronizer_conf /etc/
mkdir /etc/systemd/system/synchroniser
cp synchroniser /etc/systemd/system/synchroniser/synchroniser
systemctl start synchronizer.service
systemctl status synchronizer.service
