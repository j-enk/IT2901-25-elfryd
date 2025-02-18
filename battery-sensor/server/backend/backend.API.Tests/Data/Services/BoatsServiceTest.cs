using Xunit;
using System;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using backend.API.Data;
using Microsoft.EntityFrameworkCore;
using System.Collections.Generic;

namespace backend.API.Data.Services
{

    public class BoatsServiceTest
    {
        BoatVM _testBoat;
        BoatsService _BoatsService;

        public BoatsServiceTest()
        {
            var dbOptionsBuilder = new DbContextOptionsBuilder<LockBoxContext>().UseInMemoryDatabase(Guid.NewGuid().ToString());
            var context = new LockBoxContext(dbOptionsBuilder.Options);
            _BoatsService = new BoatsService(context);

            context.LockBoxes.Add(new LockBox()
            {
                Topic = "nrf/command",
                isOpen = false
            });
            context.SaveChanges();

            _testBoat = new BoatVM
            {
                Name = "Elfryd 2",
                ChargingTime = 50,
                WeatherLimit = 20,
                AdvanceBookingLimit = 90,
                MaxBookingLimit = 8,
                LockBoxID = 1
            };
        }

        [Fact]
        public void TestBoatIsAddedToDb()
        {
            Assert.True(_BoatsService.GetBoats().Count == 0);
            _BoatsService.AddBoat(_testBoat);
            Assert.True(_BoatsService.GetBoats().Count == 1);
            List<BoatListVM> boats = _BoatsService.GetBoats();
            Assert.True(boats[0].Name == "Elfryd 2");
        }

        [Fact]
        public void TestGetBoatById()
        {
            _BoatsService.AddBoat(_testBoat);
            var boat = _BoatsService.GetBoat(1);
            Assert.NotNull(boat);
            Assert.True(boat.Name == "Elfryd 2");
        }

        [Fact]
        public void TestBoatIsUpdated()
        {
            _BoatsService.AddBoat(_testBoat);
            var boatBeforeUpdate = _testBoat;
            boatBeforeUpdate.Name = "Elfryd 3";

            var updatedBoat = _BoatsService.UpdateBoat(1, boatBeforeUpdate);
            Assert.True(updatedBoat.Name == "Elfryd 3");
            Assert.True(_BoatsService.GetBoat(1).Name == "Elfryd 3");
        }

        [Fact]
        public void TestBoatIsDeleted()
        {
            _BoatsService.AddBoat(_testBoat);
            Assert.True(_BoatsService.GetBoats().Count == 1);
            _BoatsService.DeleteBoat(1);
            Assert.True(_BoatsService.GetBoats().Count == 0);
        }

    }

}