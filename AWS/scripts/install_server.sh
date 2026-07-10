#!/usr/bin/env bash
# =====================================================
# InfinityTable Server Bootstrap Script
# Run on a fresh Amazon Linux 2023 / Ubuntu 22.04
# =====================================================
set -euo pipefail
exec > /var/log/it_install.log 2>&1

echo "=== InfinityTable Server Install ==="
date

OS_ID=$(. /etc/os-release && echo $ID)

if [ "$OS_ID" = "amzn" ] || [ "$OS_ID" = "centos" ] || [ "$OS_ID" = "rhel" ]; then
    PKG="dnf"
else
    PKG="apt-get"
    apt-get update -y
fi

# Install dependencies
if [ "$PKG" = "dnf" ]; then
    dnf update -y
    dnf install -y docker docker-compose-plugin nginx awscli htop wget unzip netcat
else
    apt-get install -y docker.io docker-compose nginx awscli htop wget unzip netcat
fi

# Enable Docker
systemctl enable --now docker
usermod -aG docker "${SUDO_USER:-ec2-user}" 2>/dev/null || true

# Create directories
mkdir -p /opt/infinitytable/{Saved,Mods,Logs,Config,Backups}
chmod 755 /opt/infinitytable

# Copy configs
cp /tmp/it-deploy/docker-compose.aws.yml /opt/infinitytable/docker-compose.yml
cp /tmp/it-deploy/infinitytable.service  /etc/systemd/system/
cp /tmp/it-deploy/infinitytable.conf     /etc/nginx/conf.d/
rm -f /etc/nginx/conf.d/default.conf

# Enable services
systemctl daemon-reload
systemctl enable --now nginx
systemctl enable --now infinitytable

# Wait for server to come up
echo "Waiting for game server..."
for i in $(seq 1 30); do
    if nc -zu 127.0.0.1 7777; then
        echo "Server is UP on port 7777!"
        break
    fi
    sleep 2
done

echo "=== Install Complete ==="
echo "Game port:  7777/UDP"
echo "Query port: 7778/TCP"
echo "Health:     http://$(curl -s http://169.254.169.254/latest/meta-data/public-ipv4)/health"
