using System.Threading.Tasks;
using MQTTnet.Client.Publishing;

public interface IMQTTClientService
{

    public enum MQTTCommands
    {
        OPEN,
        CLOSE,
        KEY_STATUS,
        LOCK_STATUS,
        PING,
    };

    Task<MqttClientPublishResult> PublishMQTTMessage(string topic, string message);

    Task<MqttClientPublishResult> IssueMQTTCommand(string topic, MQTTCommands message, long lockBoxID);
}