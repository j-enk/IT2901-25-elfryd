using System.IO;
using Microsoft.Extensions.Configuration;
using Microsoft.AspNetCore.Hosting;

namespace backend.API
{
    static class ConfigurationManager
    {

        public static IConfiguration AppSetting { get; }
        static ConfigurationManager()
        {

            var enviroment = System.Environment.GetEnvironmentVariable("ASPNETCORE_ENVIRONMENT");

            var jsonString = enviroment == "Development" ? "appsettings.Development.json" : "appsettings.json";

            AppSetting = new ConfigurationBuilder()
                    .SetBasePath(Directory.GetCurrentDirectory())
                    .AddJsonFile(jsonString)
                    .AddEnvironmentVariables()
                    .Build();
        }
    }
}
