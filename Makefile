# Общий Makefile для проекта myRPC

.PHONY: all clean deb install uninstall

all:
	$(MAKE) -C libmysyslog
	$(MAKE) -C src/server
	$(MAKE) -C src/client

clean:
	$(MAKE) -C libmysyslog clean
	$(MAKE) -C src/server clean
	$(MAKE) -C src/client clean
	rm -rf deb-packages
	rm -f *.deb

deb: all
	mkdir -p deb-packages/myrpc-client/DEBIAN
	mkdir -p deb-packages/myrpc-client/usr/bin
	cp src/client/myRPC-client deb-packages/myrpc-client/usr/bin/
	echo 'Package: myrpc-client' > deb-packages/myrpc-client/DEBIAN/control
	echo 'Version: 1.0.0' >> deb-packages/myrpc-client/DEBIAN/control
	echo 'Architecture: amd64' >> deb-packages/myrpc-client/DEBIAN/control
	echo 'Maintainer: Student <student@example.com>' >> deb-packages/myrpc-client/DEBIAN/control
	echo 'Description: myRPC client utility' >> deb-packages/myrpc-client/DEBIAN/control
	dpkg-deb --build deb-packages/myrpc-client myrpc-client_1.0.0_amd64.deb
	mkdir -p deb-packages/myrpc-server/DEBIAN
	mkdir -p deb-packages/myrpc-server/usr/bin
	mkdir -p deb-packages/myrpc-server/etc/myRPC
	mkdir -p deb-packages/myrpc-server/lib/systemd/system
	cp src/server/myRPC-server deb-packages/myrpc-server/usr/bin/
	cp config/myRPC.conf deb-packages/myrpc-server/etc/myRPC/
	cp config/users.conf deb-packages/myrpc-server/etc/myRPC/
	cp scripts/myRPC-server.service deb-packages/myrpc-server/lib/systemd/system/ 2>/dev/null || true
	echo 'Package: myrpc-server' > deb-packages/myrpc-server/DEBIAN/control
	echo 'Version: 1.0.0' >> deb-packages/myrpc-server/DEBIAN/control
	echo 'Architecture: amd64' >> deb-packages/myrpc-server/DEBIAN/control
	echo 'Maintainer: Student <student@example.com>' >> deb-packages/myrpc-server/DEBIAN/control
	echo 'Description: myRPC server daemon' >> deb-packages/myrpc-server/DEBIAN/control
	echo '#!/bin/bash' > deb-packages/myrpc-server/DEBIAN/postinst
	echo 'if [ -f /lib/systemd/system/myRPC-server.service ]; then systemctl daemon-reload; fi' >> deb-packages/myrpc-server/DEBIAN/postinst
	echo 'mkdir -p /var/log' >> deb-packages/myrpc-server/DEBIAN/postinst
	echo 'touch /var/log/myrpc-server.log' >> deb-packages/myrpc-server/DEBIAN/postinst
	echo 'chmod 644 /var/log/myrpc-server.log' >> deb-packages/myrpc-server/DEBIAN/postinst
	chmod +x deb-packages/myrpc-server/DEBIAN/postinst
	dpkg-deb --build deb-packages/myrpc-server myrpc-server_1.0.0_amd64.deb
	@echo "DEB пакеты созданы:"
	@ls *.deb

install:
	dpkg -i myrpc-client_1.0.0_amd64.deb
	dpkg -i myrpc-server_1.0.0_amd64.deb

uninstall:
	dpkg -r myrpc-client myrpc-server