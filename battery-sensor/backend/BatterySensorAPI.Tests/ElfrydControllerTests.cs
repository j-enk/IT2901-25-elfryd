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

            var request = new ElfrydUpdateConfigRequest { Command = "battery 60" };
            var expectedResult = "{\"success\":true}";
            _mockElfrydClient
                .Setup(client => client.UpdateConfigAsync(request.Command))
                .ReturnsAsync(expectedResult);


            var result = await _controller.UpdateConfig(request);


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

            var request = new ElfrydUpdateConfigRequest { Command = null };


            var result = await _controller.UpdateConfig(request);


            Assert.IsType<BadRequestObjectResult>(result);
        }

        [Fact]
        public async Task UpdateConfigEmptyCommand()
        {

            var request = new ElfrydUpdateConfigRequest { Command = "" };


            var result = await _controller.UpdateConfig(request);


            Assert.IsType<BadRequestObjectResult>(result);
        }


        // Test that the controller handles exceptions correctly
        [Fact]
        public async Task UpdateConfigException()
        {

            var request = new ElfrydUpdateConfigRequest { Command = "battery 60" };
            _mockElfrydClient
                .Setup(client => client.UpdateConfigAsync(request.Command))
                .ThrowsAsync(new Exception("Test exception"));


            var result = await _controller.UpdateConfig(request);


            var statusCodeResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, statusCodeResult.StatusCode);
            Assert.Contains("Error updating", statusCodeResult.Value.ToString());

        }

        // Tests for getting config
        [Fact]
        public async Task GetConfigValidRequest()
        {

            int limit = 10;
            double hours = 168;
            double time_offset = 0;
            string expectedJsonResponse = "{\"configs\":[{\"name\":\"frequency\",\"value\":60}]}";

            _mockElfrydClient
                .Setup(client => client.GetConfigAsync(limit, hours, time_offset))
                .ReturnsAsync(expectedJsonResponse);


            var result = await _controller.GetConfig(limit, hours, time_offset);


            var contentResult = Assert.IsType<ContentResult>(result);
            Assert.Equal("application/json", contentResult.ContentType);
            Assert.Equal(expectedJsonResponse, contentResult.Content);
        }

        [Fact]
        public async Task GetConfigException()
        {

            _mockElfrydClient
                .Setup(client => client.GetConfigAsync(It.IsAny<int>(), It.IsAny<double>(), It.IsAny<double>()))
                .ThrowsAsync(new Exception("Test exception"));


            var result = await _controller.GetConfig();


            var statusCodeResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, statusCodeResult.StatusCode);
            Assert.Equal("Error retrieving config from Elfryd API", statusCodeResult.Value);
        }



        // Tests for getting battery data

        [Fact]
        public async Task GetBatteryDataWithIncorrectID()
        {
            int batteryId = 2;
            int limit = 10;
            double hours = 24;
            double time_offset = 0;

            // This response contains illegal data with an ID different from the requested
            string unfilteredResponse = @"
    [
        {
            ""id"": 1,
            ""battery_id"": 1,
            ""voltage"": 12524,
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
            ""voltage"": 11950,
            ""device_timestamp"": 1712861632
        }
    ]";

            _mockElfrydClient
                .Setup(client => client.GetBatteryDataAsync(batteryId, limit, hours, time_offset))
                .ReturnsAsync(unfilteredResponse);


            var result = await _controller.GetBatteryData(batteryId, limit, hours, time_offset);

            var objectResult = Assert.IsType<ObjectResult>(result);
            Assert.Equal(500, objectResult.StatusCode);
            Assert.Contains("Inconsistent battery data", objectResult.Value.ToString());
        }

        [Fact]
        public async Task GetBatteryDataWithCorrectID()
        {

            int batteryId = 2;
            int limit = 10;
            double hours = 24;
            double time_offset = 0;

            // This response contains data only for the requested battery_id
            string filteredResponse = @"
             [
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
            ]";

            _mockElfrydClient
                .Setup(client => client.GetBatteryDataAsync(batteryId, limit, hours, time_offset))
                .ReturnsAsync(filteredResponse);


            var result = await _controller.GetBatteryData(batteryId, limit, hours, time_offset);


            var contentResult = Assert.IsType<ContentResult>(result);
            Assert.Equal("application/json", contentResult.ContentType);
            Assert.Equal(filteredResponse, contentResult.Content);
        }

    }
}
