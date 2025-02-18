using Xunit;
using System;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using backend.API.Data;
using Microsoft.EntityFrameworkCore;
using System.Collections.Generic;

namespace backend.API.Data.Services
{
    public class BookingPeriodsServiceTest
    {
        BookingPeriodsService _BookingsPeriodsService;
        BookingPeriodVM _testBookingPeriod;
        public BookingPeriodsServiceTest()
        {
            var dbOptionsBuilder = new DbContextOptionsBuilder<LockBoxContext>().UseInMemoryDatabase(Guid.NewGuid().ToString());
            var context = new LockBoxContext(dbOptionsBuilder.Options);
            _BookingsPeriodsService = new BookingPeriodsService(context);

            _testBookingPeriod = new BookingPeriodVM
            {
                Name = "Sommer 2023",
                StartDate = new DateTime(2023, 05, 1),
                EndDate = new DateTime(2023, 09, 1),
                BookingOpens = 8,
                BookingCloses = 22,
                BoatID = 1
            };

            context.LockBoxes.Add(new LockBox()
            {
                Topic = "nrf/command",
                isOpen = false
            });
            context.SaveChanges();

            context.Boats.Add(new Boat
            {
                Name = "Elfryd 2",
                ChargingTime = 50,
                WeatherLimit = 20,
                AdvanceBookingLimit = 90,
                MaxBookingLimit = 8,
                LockBoxID = 1
            });
            context.SaveChanges();
        }
        [Fact]
        public void TestAddBookingPeriod()
        {
            Assert.Empty(_BookingsPeriodsService.GetBookingPeriods());
            var bookingPeriod = _BookingsPeriodsService.AddBookingPeriod(_testBookingPeriod);
            Assert.True(bookingPeriod is BookingPeriod);
            var bookingPeriods = _BookingsPeriodsService.GetBookingPeriods();
            Assert.NotEmpty(bookingPeriods);
            Assert.True(bookingPeriods[0].Name == "Sommer 2023");
        }

        [Fact]
        public void TestGetBookingPeriodByName()
        {
            var bookingPeriod = _BookingsPeriodsService.AddBookingPeriod(_testBookingPeriod);
            var getBookingPeriod = _BookingsPeriodsService.GetBookingPeriod(bookingPeriod.Name, bookingPeriod.BoatID);
            Assert.True(getBookingPeriod.Name == bookingPeriod.Name);
            Assert.True(getBookingPeriod is BookingPeriodReadableVM);

            var nonExistingBookingPeriod = _BookingsPeriodsService.GetBookingPeriod("this does not exist", 123);
            Assert.True(nonExistingBookingPeriod is null);
        }

        [Fact]
        public void TestUpdateBookingPeriod()
        {
            var bookingPeriod = _BookingsPeriodsService.AddBookingPeriod(_testBookingPeriod);
            var updatedBookingPeriod = new BookingPeriodVM()
            {
                Name = bookingPeriod.Name,
                StartDate = DateTime.Now.AddDays(7), // Updated StartDate and EndDate
                EndDate = DateTime.Now.AddDays(14),
                BookingOpens = 8,
                BookingCloses = 22,
                BoatID = bookingPeriod.BoatID
            };

            _BookingsPeriodsService.UpdateBookingPeriod(bookingPeriod.Name, bookingPeriod.BoatID, updatedBookingPeriod);
            var newBookingPeriod = _BookingsPeriodsService.GetBookingPeriod(updatedBookingPeriod.Name, updatedBookingPeriod.BoatID);

            Assert.True(newBookingPeriod.StartDate == updatedBookingPeriod.StartDate);
            Assert.True(newBookingPeriod.EndDate == updatedBookingPeriod.EndDate);

            updatedBookingPeriod.Name = "New Name";
            Assert.Throws<InvalidOperationException>(() => _BookingsPeriodsService.UpdateBookingPeriod(bookingPeriod.Name, bookingPeriod.BoatID, updatedBookingPeriod));
        }

        [Fact]
        public void TestDeleteBookingPeriod()
        {
            var bookingPeriod = _BookingsPeriodsService.AddBookingPeriod(_testBookingPeriod);

            _BookingsPeriodsService.DeleteBookingPeriod(bookingPeriod.Name, bookingPeriod.BoatID);
            var deletedBookingPeriod = _BookingsPeriodsService.GetBookingPeriod(bookingPeriod.Name, bookingPeriod.BoatID);

            Assert.Null(deletedBookingPeriod);
        }

    }
}