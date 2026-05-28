cat > Makefile << 'EOF'
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
	cat > deb-packages/myrpc-client/DEBIAN/control << 'EOF'
Package: myrpc-client
Version: 1.0.0
Architecture: amd64
Maintainer: Student <student@example.com>
Description: myRPC client utility
EOF
	dpkg-deb --build deb-packages/myrpc-client myrpc-client_1.0.0_amd64.deb
	
	mkdir -p deb-packages/myrpc-server/DEBIAN
	mkdir -p deb-packages/myrpc-server/usr/bin
	mkdir -p deb-packages/myrpc-server/etc/myRPC
	mkdir -p deb-packages/myrpc-server/lib/systemd/system
	cp src/server/myRPC-server deb-packages/myrpc-server/usr/bin/
	cp config/myRPC.conf deb-packages/myrpc-server/etc/myRPC/
	cp config/users.conf deb-packages/myrpc-server/etc/myRPC/
	cp scripts/myRPC-server.service deb-packages/myrpc-server/lib/systemd/system/ 2>/dev/null || true
	cat > deb-packages/myrpc-server/DEBIAN/control << 'EOF'
Package: myrpc-server
Version: 1.0.0
Architecture: amd64
Maintainer: Student <student@example.com>
Description: myRPC server daemon
EOF
	cat > deb-packages/myrpc-server/DEBIAN/postinst << 'EOF'
#!/bin/bash
if [ -f /lib/systemd/system/myRPC-server.service ]; then systemctl daemon-reload; fi
mkdir -p /var/log
touch /var/log/myrpc-server.log
chmod 644 /var/log/myrpc-server.log
EOF
	chmod +x deb-packages/myrpc-server/DEBIAN/postinst
	dpkg-deb --build deb-packages/myrpc-server myrpc-server_1.0.0_amd64.deb
	@echo "DEB пакеты созданы:"
	@ls *.deb

install:
	dpkg -i myrpc-client_1.0.0_amd64.deb
	dpkg -i myrpc-server_1.0.0_amd64.deb

uninstall:
	dpkg -r myrpc-client myrpc-server
EOF