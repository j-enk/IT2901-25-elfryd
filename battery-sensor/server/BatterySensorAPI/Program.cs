// Program.cs
using BatterySensorAPI.Services;
using DotNetEnv;

DotNetEnv.Env.Load();

var builder = WebApplication.CreateBuilder(args);

// Add services to the container
builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();



builder.Services.AddSingleton<IElfrydApiClient, ElfrydApiClient>(sp => {
    var configuration = sp.GetRequiredService<IConfiguration>();
    var logger = sp.GetRequiredService<ILogger<ElfrydApiClient>>();
    
    var hostName = Environment.GetEnvironmentVariable("HOST_NAME");
    var apiKey = Environment.GetEnvironmentVariable("API_KEY");
    logger.LogInformation($"Base URL: {hostName}");

    if (!string.IsNullOrEmpty(hostName) && !hostName.StartsWith("http://") && !hostName.StartsWith("https://"))
    {
        hostName = "https://" + hostName;
        logger.LogInformation($"Protocol added to base URL: {hostName}");
    }

    if (string.IsNullOrEmpty(apiKey)){
        logger.LogWarning("API_KEY environment variable is not set. Elfryd API client will not be initialized.");
    }

    HttpClientHandler handler = null;
    if (configuration.GetValue<bool>("ElfrydApi:AllowSelfSignedCertificate", false))
    {
        logger.LogWarning("Using unsafe certificate validation for Elfryd API");
        handler = new HttpClientHandler
        {
            ServerCertificateCustomValidationCallback = 
                HttpClientHandler.DangerousAcceptAnyServerCertificateValidator
        };
    }
    
    return new ElfrydApiClient(hostName, apiKey, handler);
});

// Add CORS support
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowAll", 
        builder => builder
            .AllowAnyOrigin()
            .AllowAnyMethod()
            .AllowAnyHeader());
});

var app = builder.Build();

// Configure the HTTP request pipeline
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();
app.UseCors("AllowAll");
app.UseAuthorization();
app.MapControllers();

app.Run();
public partial class Program { }