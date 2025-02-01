using Xunit;
using System;
using System.Linq;
using backend.API.Data;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using Microsoft.EntityFrameworkCore;
using System.Collections.Generic;
using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;

namespace backend.API.Data.Services
{
    public class BookingsServiceTest
    {
        BookingsService _BookingsService;
        BookingVM _testBooking;
        LockBoxContext context;

        public BookingsServiceTest()
        {
            var dbOptionsBuilder = new DbContextOptionsBuilder<LockBoxContext>().UseInMemoryDatabase(Guid.NewGuid().ToString());
            context = new LockBoxContext(dbOptionsBuilder.Options);
            _BookingsService = new BookingsService(context);


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

            context.Users.Add(new User
            {
                Name = "Test Testesen",
                UserName = "test@test.test",
                PhoneNumber = "12345678",
                Email = "test@test.test",
            });
            context.SaveChanges();

            _testBooking = new BookingVM
            {
                StartTime = DateTime.Now,
                EndTime = DateTime.Now.AddMinutes(60),
                Status = 'P',
                Comment = "This is a comment",
                UserID = ((IdentityUser)context.Users.Where(u => u.Name == "Test Testesen").ToList()[0]).Id,
                BoatID = 1
            };

        }

        [Fact]
        public void TestAddingBookingExceedingTimeLimitFails()
        {
            var boat = context.Boats.FirstOrDefault(b => b.BoatID == 1);
            var MaxBookingLimit = boat.MaxBookingLimit;
            _testBooking.EndTime = DateTime.Now.AddHours(MaxBookingLimit);
            var exception = Assert.Throws<ArgumentException>(() => _BookingsService.AddBooking(_testBooking));
        }

        [Fact]
        public void TestAddingBookingWithoutExistingBoatFails()
        {
            _testBooking.BoatID = 124243141;
            var exceptionBoatDoesNotExist = Assert.Throws<ArgumentException>(() => _BookingsService.AddBooking(_testBooking));
            Assert.Equal("Boat does not exist.", exceptionBoatDoesNotExist.Message);
        }

        [Fact]
        public void TestAddingOverlappingBookingFails()
        {
            _BookingsService.AddBooking(_testBooking);
            var exceptionOverlapping = Assert.Throws<ArgumentException>(() => _BookingsService.AddBooking(_testBooking));
            Assert.Equal("Booking or charging time overlaps another booking.", exceptionOverlapping.Message);
        }

        [Fact]
        public void TestBookingStartTimeBeforeEndTimeFails()
        {
            _testBooking.StartTime = DateTime.Now.AddHours(12);
            _testBooking.EndTime = DateTime.Now.AddHours(11);
            var exceptionStartBeforeEnd = Assert.Throws<ArgumentException>(() => _BookingsService.AddBooking(_testBooking));
            Assert.Equal("Start time must be earlier than end time.", exceptionStartBeforeEnd.Message);
        }

        [Fact]
        public void TestBookingIsAddedCorrectly()
        {
            _BookingsService.AddBooking(_testBooking);
            var bookingsList = _BookingsService.GetBookings();
            Assert.True(bookingsList.Count == 1);
            Assert.True(bookingsList[0].Comment == "This is a comment");
        }

        [Fact]
        public void TestGetBookingsWithinTimeSpan()
        {
            var bookings = _BookingsService.GetBookingsTimeSpan(DateTime.Now, DateTime.Now.AddMinutes(1), 1);
            Assert.True(bookings.Count == 0);

            _BookingsService.AddBooking(_testBooking);
            _testBooking.StartTime = DateTime.Now.AddHours(5);
            _testBooking.EndTime = DateTime.Now.AddHours(6);
            _BookingsService.AddBooking(_testBooking);

            bookings = _BookingsService.GetBookingsTimeSpan(DateTime.Now, DateTime.Now.AddHours(2), 1);
            Assert.True(bookings.Count == 1);
            bookings = _BookingsService.GetBookingsTimeSpan(DateTime.Now, DateTime.Now.AddHours(12), 1);
            Assert.True(bookings.Count == 2);
        }

        [Fact]
        public void TestGetBookingsInWeek()
        {
            _BookingsService.AddBooking(_testBooking);
            _testBooking.StartTime = DateTime.Now.AddDays(7);
            _testBooking.EndTime = DateTime.Now.AddDays(7).AddHours(1);
            _BookingsService.AddBooking(_testBooking);

            var bookings = _BookingsService.GetBookingsInWeek(DateTime.Now, 1);
            Assert.True(bookings.Count == 1);
            bookings = _BookingsService.GetBookingsInWeek(DateTime.Now.AddDays(7), 1);
            Assert.True(bookings.Count == 1);

            _testBooking.StartTime = DateTime.Now.AddHours(2);
            _testBooking.EndTime = DateTime.Now.AddHours(3);
            _BookingsService.AddBooking(_testBooking);
            bookings = _BookingsService.GetBookingsInWeek(DateTime.Now, 1);
            Assert.True(bookings.Count == 2);

        }

        [Fact]
        public void TestGetUserBookings()
        {
            var UserID = ((IdentityUser)context.Users.Where(u => u.Name == "Test Testesen").ToList()[0]).Id;
            var bookings = _BookingsService.GetUserBookings(UserID, 1);
            Assert.True(bookings.Count == 0);
            _BookingsService.AddBooking(_testBooking);
            bookings = _BookingsService.GetUserBookings(UserID, 1);
            Assert.True(bookings.Count == 1);
        }

        [Fact]
        public void TestGetBookingById()
        {
            _BookingsService.AddBooking(_testBooking);
            var booking = _BookingsService.GetBooking(1);
            Assert.True(booking.BookingOwner == "Test Testesen");
            Assert.True(booking.Comment == "This is a comment");
        }

        [Fact]
        public void TestUpdateBooking()
        {
            _BookingsService.AddBooking(_testBooking);
            var temp = new BookingVM
            {
                StartTime = _testBooking.StartTime,
                EndTime = _testBooking.EndTime,
                Status = _testBooking.Status,
                Comment = "This is an updated comment",
                UserID = _testBooking.UserID,
                BoatID = _testBooking.BoatID
            };
            var booking = _BookingsService.UpdateBooking(1, temp);
            var bookings = _BookingsService.GetBookings();
            Assert.True(bookings.Count == 1);
            Assert.True(bookings[0].Comment == "This is an updated comment");

        }

        [Fact]
        public void TestDeleteBooking()
        {
            _BookingsService.AddBooking(_testBooking);
            var bookings = _BookingsService.GetBookings();
            Assert.True(bookings.Count == 1);
            _BookingsService.DeleteBooking(1);
            bookings = _BookingsService.GetBookings();
            Assert.True(bookings.Count == 0);
        }

        [Fact]
        public void TestHasBooked()
        {
            var UserID = ((IdentityUser)context.Users.Where(u => u.Name == "Test Testesen").ToList()[0]).Id;
            var hasBooked = _BookingsService.GetHasBooked(UserID);
            Assert.Null(hasBooked);
            _BookingsService.AddBooking(_testBooking);
            hasBooked = _BookingsService.GetHasBooked(UserID);
            Assert.NotNull(hasBooked);
        }

        [Fact]
        public void TestGetUserIdFromBooking()
        {
            var UserID = ((IdentityUser)context.Users.Where(u => u.Name == "Test Testesen").ToList()[0]).Id;
            _BookingsService.AddBooking(_testBooking);
            var UserIDFromBooking = _BookingsService.GetBookingOwner(1);
            Assert.Equal(UserID, UserIDFromBooking);
        }

    }
}
