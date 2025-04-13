# VM Setup Guide

This guide provides step-by-step instructions for setting up a new virtual machine (VM) to host the Elfryd MQTT broker infrastructure.

## 1. Creating an Ubuntu VM on Azure

1. Log in to the [Azure Portal](https://portal.azure.com)
2. Click "Create a resource" > "Compute" > "Virtual Machine"
3. Select "Ubuntu Server 22.04 LTS" or newer
4. Choose your VM size (at least 1GB RAM recommended)
5. Setup authentication (SSH key recommended)
6. Allow SSH (port 22)
7. Review and create

## 2. Configure Network Security

1. Go to your VM's "Networking" settings
2. Click "Add inbound port rule"
3. Add rules for the following ports:
   - MQTT (port 1883) - Internal bridge communication
   - MQTT over TLS (port 8885) - Secure device connections
   - API over HTTPS (port 443) - REST API access

## 3. Configure DNS Name

1. Go to your VM's "Overview" page
2. Click on the "Configure" link next to DNS name
3. Enter a DNS name prefix (this will be used for your TLS certificate)
4. Click "Save"
5. Note the full DNS name (e.g., `yourvm.westeurope.cloudapp.azure.com`)

## 4. Connect to the VM

Connect to your VM using SSH:

```bash
ssh yourusername@your-vm-dns-name
```

## 5. Setup Git and Clone the Repository

1. Install Git:

```bash
sudo apt update && sudo apt install -y git
```

2. Generate SSH key for GitHub:

```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
```

3. Copy the SSH public key:

```bash
cat ~/.ssh/id_ed25519.pub
```

4. Add the key to your GitHub account:
   - Go to GitHub > Settings > SSH and GPG keys
   - Click "New SSH key"
   - Paste your public key and save

5. Initialize empty repo:

```bash
git init elfryd
cd elfryd
```

6. Add the remote repository:

```bash
git remote add origin git@github.com:j-enk/IT2901-25-elfryd.git
```

7. Configure sparse checkout (to download only the broker code):

```bash
git config core.sparseCheckout true
echo "battery-sensor/broker" >> .git/info/sparse-checkout
```

8. Pull the `broker` directory:

```bash
git pull origin main
cd battery-sensor/broker
```

## 6. Run the Installation Script

Once you have the code on your VM, you can run the installation script to set up all required services:

```bash
sudo bash install.sh
```

The script will:
- Install Docker and required packages
- Generate TLS certificates for the MQTT server
- Generate a secure API key for protected endpoints
- Set up Docker containers for all services
- Start all services and run tests
- Create a certificate package for clients

## 7. Download Client Certificates

After installation, you need to download the client certificates to your local machine to enable secure MQTT connections:

```bash
scp yourusername@your-vm-dns-name:~/elfryd/battery-sensor/broker/elfryd_client_certs.tar.gz .
tar -xzf elfryd_client_certs.tar.gz
```

The certificates will be extracted to a `client_certs` directory containing the CA certificate required for TLS connections.

## VM Sizing Recommendations

For a production environment, we recommend the following VM specifications:

- **CPU**: 2 vCPUs minimum
- **RAM**: 2GB minimum, 4GB recommended for heavier loads
- **Disk**: 32GB minimum, SSD recommended
- **Network**: Standard networking with public IP

For development or testing, a smaller VM with 1 vCPU and 1GB RAM may be sufficient.

## Security Considerations

- Keep your API key secure - it provides access to all protected endpoints
- Regularly update your Ubuntu VM to patch security vulnerabilities
- Consider implementing IP restrictions for the management ports
- Enable Azure Security Center for additional security monitoring
- Rotate TLS certificates periodically (can be done using the `cleanup.sh` and `install.sh` scripts)

## Troubleshooting VM Issues

If you encounter issues with the VM:

1. Check if the VM is running (Azure portal > VM > Overview)
2. Verify network security rules allow traffic on required ports
3. Check disk space usage with `df -h`
4. Check system logs with `sudo journalctl -xe`
5. Verify Docker is running with `sudo systemctl status docker`

For more detailed troubleshooting of the broker services, refer to the [Scripts Documentation](scripts.md).