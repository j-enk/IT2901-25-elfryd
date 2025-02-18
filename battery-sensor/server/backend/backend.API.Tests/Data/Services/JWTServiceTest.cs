using Xunit;
using backend.API.Data.Models;
using System.Collections.Generic;
using Microsoft.Extensions.Configuration;
using System.IO;
using System.Configuration;
using backend.API;

namespace backend.API.Data.Services
{
    public class JWTServiceTest
    {
        User _testUser;
        JWTService _JWTService;

        public JWTServiceTest()
        {
            _testUser = new User
            {
                Name = "TestUser",
                UserName = "test@testmail.com",
                PhoneNumber = "12345678",
                Email = "test@testmail.com"
            };
            var appSettingsPath = Path.Combine(Directory.GetParent(Directory.GetCurrentDirectory()).Parent.Parent.Parent.FullName + "/backend.API/", "appsettings.json");
            var config = new ConfigurationBuilder()
                .AddJsonFile(appSettingsPath)
                .AddUserSecrets<Program>()
                .Build();

            _JWTService = new JWTService(config);
        }

        [Fact]
        public void JWTService_Can_Generate_TokenPair()
        {
            var roles = new List<string> { "admin" };

            var tokenPair = _JWTService.GenerateTokenPair(_testUser, roles);

            Assert.NotNull(tokenPair.AccessToken);
            Assert.NotNull(tokenPair.RefreshToken);
        }

        [Fact]
        public void JWTService_Can_Generate_Valid_AuthToken()
        {
            var roles = new List<string> { "admin" };

            var authToken = _JWTService.GenerateAuthToken(_testUser, roles);
            var uuid = _JWTService.ValidateAndGetUUID(authToken);

            Assert.Equal(uuid, _testUser.Id);
        }

        [Fact]
        public void JWTService_Returns_Null_On_Expired_Token()
        {
            var expiredAccessToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6Im1hcnRpbnNrYXR2ZWR0QGdtYWlsLmNvbSIsIm5hbWUiOiJPbGEgTm9yZG1hbm4iLCJzdWIiOiJjMGMxNzMyZi03ZmMxLTRiOTItOTY5MC04ZDA3NzU4ZGVmZGQiLCJyb2xlcyI6IkFkbWluIiwiZXhwIjoxNjc2OTA2ODg3LCJpc3MiOiJodHRwOi8vbG9jYWxob3N0OjUwMDEiLCJhdWQiOiJodHRwOi8vbG9jYWxob3N0OjMwMDAifQ.2FjOX7M1u0WQr7fLKoIP7Eyt4jDlDFxmjU64vj2vV5w";

            var uuid = _JWTService.ValidateAndGetUUID(expiredAccessToken);
            Assert.Null(uuid);
        }

        [Fact]
        public void JWTService_Returns_Null_On_Tampered_Token()
        {
            var roles = new List<string> { "admin" };
            var authToken = _JWTService.GenerateAuthToken(_testUser, roles);

            var invalidAccessToken = authToken.Remove(10, 1);

            var uuid = _JWTService.ValidateAndGetUUID(invalidAccessToken);
            Assert.Null(uuid);
        }

        [Fact]
        public void JWTService_Can_Generate_Valid_RefreshToken()
        {
            var refreshToken = _JWTService.GenerateRefreshToken(_testUser);
            var uuid = _JWTService.ValidateAndGetUUID(refreshToken);
            Assert.Equal(uuid, _testUser.Id);
        }
    }
}
