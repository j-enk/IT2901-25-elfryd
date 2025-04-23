using Xunit;
using System;
using System.Threading.Tasks;
using BatterySensorAPI.Models;
using BatterySensorAPI.Services;
using BatterySensorAPI.Controllers;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using Moq;

public class UpdateConfigResponse
{
    public bool success { get; set; }
    public string result { get; set; }
}

namespace BatterySensorAPI.Tests
{
    public class ElfrydControllerTests
    {
        private readonly Mock<IElfrydApiClient> _mockElfrydClient;
        private readonly Mock<ILogger<ElfrydController>> _mockLogger;
        private readonly ElfrydController _controller;

        public ElfrydControllerTests()
        {
            _mockElfrydClient = new Mock<IElfrydApiClient>();
            _mockLogger = new Mock<ILogger<ElfrydController>>();
            _controller = new ElfrydController(_mockElfrydClient.Object, _mockLogger.Object);
        }



        // Tests for updating config
        [Fact]
        public async Task UpdateConfigValidCommand()
        {
            // Arrange
            var request = new ElfrydUpdateConfigRequest { Command = "battery 60" };
            var expectedResult = "{\"success\":true}";
            _mockElfrydClient
                .Setup(client => client.UpdateConfigAsync(request.Command))
                .ReturnsAsync(expectedResult);

            // Act
            var result = await _controller.UpdateConfig(request);

            // Assert
            var okResult = Assert.IsType<OkObjectResult>(result);
            var response = okResult.Value as UpdateConfigResponse;

            if (response == null)
            {
                var json = System.Text.Json.JsonSerializer.Serialize(okResult.Value);
                response = System.Text.Json.JsonSerializer.Deserialize<UpdateConfigResponse>(json);
            }

            Assert.NotNull(response);
            Assert.True(response.success);
            Assert.Equal(expectedResult, response.result);
        }

        [Fact]
        public async Task UpdateConfigNullCommand()
        {
            // Arrange
            var request = new ElfrydUpdateConfigRequest { Command = null };

            // Act
            var result = await _controller.UpdateConfig(request);

            // Assert
            Assert.IsType<BadRequestObjectResult>(result);
        }

        [Fact]
        public async Task UpdateConfigEmptyCommand()
        {
            // Arrange
            var request = new ElfrydUpdateConfigRequest { Command = "" };

            // Act
            var result = await _controller.UpdateConfig(request);

            // Assert
            Assert.IsType<BadRequestObjectResult>(result);
        }


        // Test that the controller handles exceptions correctly
        [Fact]
        public async Task UpdateConfigException()
        {
            // Arrange
            var request = new ElfrydUpdateConfigRequest { Command = "battery 60" };
            _mockElfrydClient
                .Setup(client => client.UpdateConfigAsync(request.Command))
                .ThrowsAsync(new Exception("Test exception"));

            // Act
            var result = await _controller.UpdateConfig(request);

            // Assert
            var statusCodeResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, statusCodeResult.StatusCode);
            Assert.Contains("Error updating", statusCodeResult.Value.ToString());

        }

        // Tests for getting config
        [Fact]
        public async Task GetConfigValidRequest()
        {
            // Arrange
            bool sendAll = false;
            int limit = 10;
            string expectedJsonResponse = "{\"configs\":[{\"name\":\"frequency\",\"value\":60}]}";

            _mockElfrydClient
                .Setup(client => client.GetConfigAsync(sendAll, limit))
                .ReturnsAsync(expectedJsonResponse);

            // Act
            var result = await _controller.GetConfig(sendAll, limit);

            // Assert
            var contentResult = Assert.IsType<ContentResult>(result);
            Assert.Equal("application/json", contentResult.ContentType);
            Assert.Equal(expectedJsonResponse, contentResult.Content);
        }

        [Fact]
        public async Task GetConfigException()
        {
            // Arrange
            _mockElfrydClient
                .Setup(client => client.GetConfigAsync(It.IsAny<bool>(), It.IsAny<int>()))
                .ThrowsAsync(new Exception("Test exception"));

            // Act
            var result = await _controller.GetConfig();

            // Assert
            var statusCodeResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, statusCodeResult.StatusCode);
            Assert.Equal("Error retrieving config from Elfryd API", statusCodeResult.Value);
        }



        // Tests for getting battery data

        [Fact]
        public async Task GetBatteryDataWithIncorrectID()
        {
            string batteryId = "2";
            int limit = 10;
            int hours = 24;

            // This response contains data from batteries we have not requested
            string unfilteredResponse = @"{
            ""data"": [
                {
                    ""id"": 1,
                    ""battery_id"": 1,
                    ""voltage"": 3824,
                    ""device_timestamp"": 1712841632
                },
                {
                    ""id"": 2,
                    ""battery_id"": 2,    
                    ""voltage"": 12333,
                    ""device_timestamp"": 1712851632
                },
                {
                    ""id"": 4,
                    ""battery_id"": 3,
                    ""voltage"": 3950,
                    ""device_timestamp"": 1712861632
                }
            ]
        }";

            _mockElfrydClient
                .Setup(client => client.GetBatteryDataAsync(batteryId, limit, hours))
                .ReturnsAsync(unfilteredResponse);

            // Act
            var result = await _controller.GetBatteryData(batteryId, limit, hours);

            // Assert
            // The controller should detect the inconsistency and return an error
            var statusCodeResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, statusCodeResult.StatusCode);
            Assert.Contains("inconsistent data", statusCodeResult.Value.ToString());
        }

        [Fact]
        public async Task GetBatteryDataWithCorrectID()
        {
            // Arrange
            string batteryId = "2";
            int limit = 10;
            int hours = 24;

            // This response contains data only for the requested battery_id
            string filteredResponse = @"{
            ""data"": [
                {
                    ""id"": 2,
                    ""battery_id"": 2,
                    ""voltage"": 12333,
                    ""device_timestamp"": 1712851632
                },
                {
                    ""id"": 3,
                    ""battery_id"": 2,
                    ""voltage"": 12244,
                    ""device_timestamp"": 1712851633
                }
            ]
        }";

            _mockElfrydClient
                .Setup(client => client.GetBatteryDataAsync(batteryId, limit, hours))
                .ReturnsAsync(filteredResponse);

            // Act
            var result = await _controller.GetBatteryData(batteryId, limit, hours);

            // Assert
            var contentResult = Assert.IsType<ContentResult>(result);
            Assert.Equal("application/json", contentResult.ContentType);
            Assert.Equal(filteredResponse, contentResult.Content);
        }

    }
}