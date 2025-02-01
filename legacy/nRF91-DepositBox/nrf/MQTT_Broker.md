# Setting up MQTT Broker on an Azure Virtual Machine

## Create an Ubuntu Virtual Server  

1. Open the Azure portal. From the main menu, select Create a resource.
2. Search for Virtual machine and select Create.
3. Use the Image: **Ubuntu 20.04 LTS - Gen2.**  
4. The size is recommended to be **Standard_B1s/B1ls** for being affordable while still reliable. 
5. Choose authentication type after your own preferences. 
6. Disk is recommended to be **Standard SSD/HDD**
7. Select Review + create.
8. Select Create.

*It takes about a minute to deploy the virtual machine.* 


## Configure the virtual machine

1. Select **Go to resource**.
2. Select **DNS name not configured**. Then set up the DNS name and set the desired **idle timeout** value. We recommend that you increase the idle timeout.
3. Select **Save**.
4. Go back to the **Overview** Pane
5. Copy the DNS name you created earlier to Notepad, because it is required when configuring the certificates.
6. Select the **Networking** pane to set up inbound port rules by using the **Add inbound port rule** button. Add the following rule:

| Destination port     | Protocol | 	Name    | Priority | Description |
| ----------- | ----------- | ----------- | ----------- | ----------- |
| 8885      | TCP       | Port_8885      | 311       | MQTT, encrypted, authenticated       |


## Connect to the virtual machine from your desktop
1. Select the **Connect (SSH)** pane.
2. Follow the instructions on that pane to connect. 
3. Depending on the authentication type you chose when creating the Virtual Machine, you need to either use the **SSH private key file**, or the **Username & Password** combination to connect. 

*PuTTY is one of the most well-known and used SSH clients, and also  my client of choice while connecting to VMs.* 


## Configure Ubuntu

1. Apply any OS updates 

```sh
sudo apt update && sudo apt -y upgrade
```

2. Install the required software:

```sh
sudo apt install -y mosquitto mosquitto-clients python3-pip && \
sudo pip3 install paho-mqtt
```  


## Help secure the Mosquitto MQTT broker
The following steps set up certificates to help secure the communications between the NRF9160 and the MQTT broker. The result will be a self-signed certificate that's valid for two years (730 days).

1. Create a Bash CommonName variable set to the DNS name of your Ubuntu server(example: kystlaget-mqtt.westeurope.cloudapp.azure.com):

```sh
CommonName=<Your Ubuntu Server DNS Name>
```  

2. Create a temporary working folder:

```sh
mkdir -p ~/mosquitto_certs && cd ~/mosquitto_certs
```  

3. Create the self-signed CA certificate:

```sh
openssl req -new -x509 -days 730 -nodes -extensions v3_ca -keyout ca.key -out ca.crt
```  

4. Create the server certificate:

```sh
openssl genrsa -out server.key 2048 && \
openssl req -new -out server.csr -key server.key -subj "/CN=$CommonName" && \
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 730 && \
openssl rsa -in server.key -out server.key
```  

5. Grant read access to the keys:

```sh
sudo chmod a+r *.key
```  

6. Copy certificates to the Mosquitto folder:

```sh
sudo cp ca.crt /etc/mosquitto/ca_certificates &&
sudo cp server.crt /etc/mosquitto/ca_certificates &&
sudo cp server.key /etc/mosquitto/ca_certificates
```  

7. Import the CA certificate to your local computer:  

On your own computer, open CMD in the folder you want to download the certificate in, and then use the command below. *(Replace the parts required)*

```sh
scp <VM_USERNAME>@<VM_DNS>:~/mosquitto_certs/* .
```


## Create the username and password for authentication
The following command will create the MQTT broker's password file and will prompt you to enter a password for the username. 

```sh
sudo mosquitto_passwd -c /etc/mosquitto/passwd USERNAME
```

*Command to create users:* 

```sh
sudo mosquitto_passwd -b /etc/mosquitto/passwd USERNAME PASSWORD
```

*Command to delete users:*

```sh
mosquitto_passwd -D /etc/mosquitto/passwd USERNAME
```


## Configure the Mosquitto MQTT broker
You need to tell the Mosquitto broker what ports to listen on and where the certificates are.
1. Use the following command to open the default.conf file
```sh
sudo nano /etc/mosquitto/conf.d/default.conf
```

2. Copy the configuration below into the file. 

```sh
per_listener_settings true

listener 1883 localhost
allow_anonymous true


listener 8885
allow_anonymous false
password_file /etc/mosquitto/passwd
cafile /etc/mosquitto/ca_certificates/ca.crt
keyfile /etc/mosquitto/ca_certificates/server.key
certfile /etc/mosquitto/ca_certificates/server.crt
```

## Test the Mosquitto broker
A useful troubleshooting tip for the Mosquitto broker is to start in interactive mode to ensure that there are no problems.

1. You need to stop Mosquitto first, and then start in interactive mode. Select Ctrl+C to exit Mosquitto interactive mode, and then enter the following command:

```sh
sudo systemctl stop mosquitto && sudo mosquitto -c /etc/mosquitto/conf.d/default.conf
```

2. You can use the mosquitto_pub and mosquitto_sub tools to test that the broker is properly setup. 

Subscribing to the test/path topic
```sh
mosquitto_sub -h <DNS_NAME> -t "test/path" -p 8885 --cafile <CA.crt PATH> --username <USERNAME> --pw <PASSWORD>
```
Writing to the test/path topic
```sh
mosquitto_pub -h <DNS_NAME> -t "test/path" -m "Hello" -p 8885 --cafile <CA.crt PATH> --username <USERNAME> --pw <PASSWORD>
```

## Start the Mosquitto MQTT broker in daemon mode
1. Stop the interactive instance of the Mosquitto Broker by selecting Ctrl+C.
2. Start the Mosquitto broker in daemon mode:
```sh
sudo systemctl enable mosquitto && sudo systemctl start mosquitto
```

### Credits:
*This short setup guide is based on a more detailed guide from Microsoft, and personalized for use for this project.*

*https://docs.microsoft.com/en-us/learn/modules/altair-azure-sphere-deploy-mqtt-broker/*
