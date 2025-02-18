using System;
using System.IO;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using backend.API.Data.Services;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using MQTTnet.Client.Publishing;
using static IMQTTClientService;

namespace backend.API.MQTT
{
    public class MQTTClientService : IMQTTClientService
    {
        MqttFactory _mqttFactory;
        IMqttClient _client;
        IMqttClientOptions _clientoptions;

        private readonly IConfiguration _configuration;

        private readonly IServiceProvider _serviceProvider;
        public MQTTClientService(IConfiguration configuration, IServiceProvider serviceProvider)
        {
            _configuration = configuration;
            _serviceProvider = serviceProvider;

            var randomID = Guid.NewGuid().ToString();
            _mqttFactory = new MqttFactory();
            _client = _mqttFactory.CreateMqttClient();
            X509Certificate2 caCrt = new X509Certificate2(File.ReadAllBytes("ca.crt"));
            _clientoptions = new MqttClientOptionsBuilder()
                                        .WithClientId("Server_" + randomID)
                                        .WithTcpServer(ConfigurationManager.AppSetting["MQTT:url"], Int32.Parse(ConfigurationManager.AppSetting["MQTT:port"]))
                                        .WithCredentials(ConfigurationManager.AppSetting["MQTT:Username"], ConfigurationManager.AppSetting["MQTT:Password"])
                                        .WithTls(new MqttClientOptionsBuilderTlsParameters()
                                        {
                                            UseTls = true,
                                            SslProtocol = System.Security.Authentication.SslProtocols.Tls12,
                                            CertificateValidationHandler = (certContext) =>
                                            {
                                                X509Chain chain = new X509Chain();
                                                chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                                                chain.ChainPolicy.RevocationFlag = X509RevocationFlag.ExcludeRoot;
                                                chain.ChainPolicy.VerificationFlags = X509VerificationFlags.NoFlag;
                                                chain.ChainPolicy.VerificationTime = DateTime.Now;
                                                chain.ChainPolicy.UrlRetrievalTimeout = new TimeSpan(0, 0, 0);
                                                chain.ChainPolicy.CustomTrustStore.Add(caCrt);
                                                chain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;

                                                // convert provided X509Certificate to X509Certificate2
                                                var x5092 = new X509Certificate2(certContext.Certificate);

                                                return chain.Build(x5092);
                                            }
                                        })
                                        .WithCleanSession()
                                        .Build();


            /* runs on broker connect */
            _client.UseConnectedHandler(async e =>
                {
                    Console.WriteLine("Established connection to broker");

                    string topic = "nrf/info";
                    var topicFilter = new MqttTopicFilterBuilder()
                                      .WithTopic(topic)
                                      .Build();

                    await _client.SubscribeAsync(topicFilter);

                    Console.WriteLine("Subscribed to topic '" + topic + "'");
                });

            _client.UseDisconnectedHandler(async e =>
            {
                Console.WriteLine("Disconnected from broker");
                await Task.Delay(TimeSpan.FromSeconds(5));

                try
                {
                    await _client.ConnectAsync(_clientoptions, CancellationToken.None);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            });

            /* runs on message received */
            _client.UseApplicationMessageReceivedHandler(e =>
            {
                string message = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);
                string topic = e.ApplicationMessage.Topic;

                if (topic == "nrf/info")
                {
                    string[] messageParts = message.Split('?');
                    long lockBoxID = long.Parse(messageParts[0]);
                    string value = messageParts[1];


                    if (value == "PONG")
                    {
                        Console.WriteLine("Lockbox " + lockBoxID + " pong");

                        using (var scope = _serviceProvider.CreateScope())
                        {
                            var _lockBoxService = scope.ServiceProvider.GetRequiredService<LockBoxesService>();
                            var result = _lockBoxService.lockBoxNewResponse(lockBoxID);

                            if (!result) Console.WriteLine("Failed to update lockbox last response");
                            else Console.WriteLine("Updated lockbox last response");
                        }
                    }

                    string[] valueParts = value.Split('=');
                    string command = valueParts[0];
                    string commandValue = valueParts[1][0].ToString();

                    switch (command)
                    {
                        case "KEY_STATUS":
                            Console.WriteLine("Lockbox " + lockBoxID + " key status: " + commandValue);

                            using (var scope = _serviceProvider.CreateScope())
                            {
                                var _keyService = scope.ServiceProvider.GetRequiredService<KeyService>();
                                var result = _keyService.AddNewKeyReport(lockBoxID, commandValue == "1" ? KeyReportStatus.Present : KeyReportStatus.Gone);

                                if (!result) Console.WriteLine("Failed to add key report");
                                else Console.WriteLine("Added key report");
                            }

                            break;

                        case "KEY_EVENT":
                            Console.WriteLine("Lockbox " + lockBoxID + " key event: " + commandValue);

                            using (var scope = _serviceProvider.CreateScope())
                            {
                                var _keyService = scope.ServiceProvider.GetRequiredService<KeyService>();

                                var result = _keyService.AddNewKeyEvent(lockBoxID, commandValue == "1" ? KeyEventActions.Returned : KeyEventActions.Removed);

                                if (!result) Console.WriteLine("Failed to add key event");
                                else Console.WriteLine("Added key event");


                            }

                            break;
                        case "LOCK_STATUS":
                            Console.WriteLine("Lockbox " + lockBoxID + " lock status: " + commandValue);

                            using (var scope = _serviceProvider.CreateScope())
                            {
                                var _lockBoxService = scope.ServiceProvider.GetRequiredService<LockBoxesService>();
                                var result = _lockBoxService.setLockBoxStatus(lockBoxID, commandValue == "1");

                                if (!result) Console.WriteLine("Failed to update lockbox status");
                                else Console.WriteLine("Updated lockbox status");
                            }
                            break;
                        case "LOCK_EVENT":
                            Console.WriteLine("Lockbox " + lockBoxID + " lock event: " + commandValue);
                            using (var scope = _serviceProvider.CreateScope())
                            {
                                var _lockBoxService = scope.ServiceProvider.GetRequiredService<LockBoxesService>();
                                var result = _lockBoxService.setLockBoxStatus(lockBoxID, commandValue == "1");

                                if (!result) Console.WriteLine("Failed to update lockbox status");
                                else Console.WriteLine("Updated lockbox status");
                            }
                            break;
                        default:
                            Console.WriteLine("Unknown command: " + command);
                            break;
                    }
                }

            });

            /* CONNECTS TO BROKER */
            try
            {
                _client.ConnectAsync(_clientoptions).Wait();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }

        public async Task<MqttClientPublishResult> PublishMQTTMessage(string topic, string payload)
        {
            var message = new MqttApplicationMessageBuilder()
                              .WithTopic(topic)
                              .WithPayload(payload)
                              .WithAtLeastOnceQoS()
                              .Build();

            var result = await this._client.PublishAsync(message);
            return result;
        }

        public async Task<MqttClientPublishResult> IssueMQTTCommand(string topic, MQTTCommands payload, long lockBoxID)
        {

            string payloadString = lockBoxID.ToString() + "?" + payload.ToString();

            var message = new MqttApplicationMessageBuilder()
                              .WithTopic(topic)
                              .WithPayload(payloadString)
                              .WithResponseTopic("nrf/info")
                              .WithAtLeastOnceQoS()
                              .Build();

            var result = await this._client.PublishAsync(message);
            return result;
        }
    }
}