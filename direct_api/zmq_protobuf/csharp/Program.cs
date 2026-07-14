using Google.Protobuf;
using NetMQ;
using NetMQ.Sockets;
using Proto.NavApi;

const string defaultHost = "127.0.0.1";
const int processorSettingsRequestPort = 60501;
const int targetDataPublishPort = 61502;

var environmentHost = Environment.GetEnvironmentVariable("SONASOFT_HOST");
var host = args.Length > 0
    ? args[0]
    : string.IsNullOrWhiteSpace(environmentHost)
        ? defaultHost
        : environmentHost;

try
{
    Console.WriteLine($"Connecting to SonaSoft at {host}");

    var response = GetProcessorSettings(host);
    Console.WriteLine($"GetProcessorSettings result code: {response.Result?.Code}");
    if (response.Settings is not null)
    {
        PrintProcessorSettings(response.Settings);
    }

    Console.WriteLine("\nWaiting for one TargetData message...");
    var targetData = ReceiveTargetData(host);
    PrintTargetDataSummary(targetData);
    return 0;
}
catch (Exception error) when (error is TimeoutException or InvalidProtocolBufferException or NetMQException)
{
    Console.Error.WriteLine($"Example failed: {error.Message}");
    return 1;
}
finally
{
    NetMQConfig.Cleanup();
}

static GetProcessorSettingsResponse GetProcessorSettings(string host)
{
    using var socket = new RequestSocket();
    socket.Connect($"tcp://{host}:{processorSettingsRequestPort}");

    var request = new GetProcessorSettingsRequest();
    if (!socket.TrySendFrame(TimeSpan.FromSeconds(5), request.ToByteArray()))
    {
        throw new TimeoutException("request timed out while sending");
    }

    if (!socket.TryReceiveFrameBytes(TimeSpan.FromSeconds(5), out var reply))
    {
        throw new TimeoutException("request timed out waiting for response");
    }

    return GetProcessorSettingsResponse.Parser.ParseFrom(reply);
}

static TargetData ReceiveTargetData(string host)
{
    using var socket = new SubscriberSocket();
    socket.SubscribeToAnyTopic();
    socket.Connect($"tcp://{host}:{targetDataPublishPort}");

    if (!socket.TryReceiveFrameBytes(TimeSpan.FromSeconds(10), out var payload))
    {
        throw new TimeoutException("timed out waiting for TargetData");
    }

    return TargetData.Parser.ParseFrom(payload);
}

static void PrintProcessorSettings(ProcessorSettings settings)
{
    Console.WriteLine("Processor settings");
    Console.WriteLine($"  System type: {settings.SystemType}");
    Console.WriteLine($"  Field of view: {settings.Fov}");
    Console.WriteLine($"  Detect bottom: {settings.DetectBottom}");
    Console.WriteLine($"  Auto squelch: {settings.SquelchlessInwaterDetector}");
    Console.WriteLine(
        $"  Squelch range: {settings.MinInwaterSquelch} - {settings.MaxInwaterSquelch}");
    Console.WriteLine($"  Current squelch: {settings.InwaterSquelch}");
}

static void PrintTargetDataSummary(TargetData targetData)
{
    Console.WriteLine("TargetData update");
    Console.WriteLine($"  Target groups: {targetData.Groups.Count}");
    Console.WriteLine($"  Bottom bins: {targetData.Bottom.Count}");
    if (targetData.Heading is not null)
    {
        Console.WriteLine($"  Heading: {targetData.Heading.Heading_}");
    }

    if (targetData.Position is not null)
    {
        Console.WriteLine($"  Position: {targetData.Position.Lat}, {targetData.Position.Lon}");
    }

    if (targetData.GridDescription is not null)
    {
        Console.WriteLine($"  Grid max range: {targetData.GridDescription.MaxRange}");
    }
}
