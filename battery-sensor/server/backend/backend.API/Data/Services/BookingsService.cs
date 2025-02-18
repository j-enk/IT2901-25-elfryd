using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;

namespace backend.API.Data.Services
{
    public class BookingsService
    {
        private LockBoxContext _context;

        public BookingsService(LockBoxContext context)
        {
            _context = context;
        }

        private Boolean validateBookingTime(DateTime startTime, DateTime endTime)
        {
            return startTime < endTime;
        }

        private TimeSpan getChargingTime(DateTime startTime, DateTime endTime, int chargingTime)
        {
            TimeSpan bookingTimeSpan = endTime - startTime;
            TimeSpan chargingPeriod = bookingTimeSpan * chargingTime / 100;

            return chargingPeriod;
        }

        public BookingCalenderViewVM AddBooking(BookingVM booking)
        {
            // TODO: move validate into functions, a lot of overlapping validate logic in UpdateBooking and AddBooking

            if (!validateBookingTime(booking.StartTime, booking.EndTime))
            {
                throw new ArgumentException("Starttiden må være før sluttiden.");
            }

            // Logic for checking that booking is within a booking period
            bool isBookingPeriod = false;
            foreach (var BookingPeriod in _context.BookingPeriods)
            {

                if ((booking.StartTime >= BookingPeriod.StartDate && booking.EndTime <= BookingPeriod.EndDate) &&
                    (booking.StartTime.Hour >= BookingPeriod.BookingOpens && ((booking.EndTime.Hour <= BookingPeriod.BookingCloses && booking.EndTime.Minute == 0) || booking.EndTime.Hour < BookingPeriod.BookingCloses)))
                {
                    isBookingPeriod = true;
                }
            }

            if (!isBookingPeriod)
            {
                throw new ArgumentException("Booking må være i en bookingperiode.");
            }

            Boat boat;
            try
            {
                boat = _context.Boats.Single(b => b.BoatID == booking.BoatID);
            }
            catch
            {
                throw new ArgumentException("Båten finnes ikke.");
            }
            TimeSpan maxTime = new TimeSpan(boat.MaxBookingLimit, 0, 0);

            if (booking.EndTime - booking.StartTime > maxTime && !booking.isOfficial)
            {
                throw new ArgumentException($"Booking kan ikke være lenger enn {boat.MaxBookingLimit} timer.");
            }

            DateTime today = DateTime.Today;
            DateTime bookingStartDate = booking.StartTime;
            int advanceLimit = boat.AdvanceBookingLimit;

            if (bookingStartDate.Date > today.AddDays(advanceLimit))
            {
                throw new ArgumentException($"Booking kan bare bli gjort {advanceLimit} dager frem i tid.");
            }

            //Adding charging time.
            TimeSpan chargingTime = getChargingTime(booking.StartTime, booking.EndTime, boat.ChargingTime);
            if (booking.isOfficial)
            {
                chargingTime = new TimeSpan(0, 0, 0);
            }

            var _booking = new Booking()
            {
                StartTime = booking.StartTime,
                EndTime = booking.EndTime,
                ChargingDone = booking.EndTime + chargingTime,
                Status = 'P',
                Comment = booking.Comment,
                UserID = booking.UserID,
                BoatID = booking.BoatID,
                isOfficial = booking.isOfficial
            };

            var bookings = GetInternalBookingsTimeSpan(_booking.StartTime - maxTime, _booking.ChargingDone + maxTime, _booking.BoatID);

            foreach (var b in bookings)
            {
                if (b.EndTime <= _booking.StartTime || b.StartTime >= _booking.EndTime)
                {
                    continue;
                }
                else
                {
                    throw new ArgumentException("Booking overlapper med en annen booking.");
                }
            }
            _context.Bookings.Add(_booking);
            _context.SaveChanges();

            bookings = GetInternalBookingsTimeSpan(_booking.StartTime - maxTime, _booking.ChargingDone + maxTime, _booking.BoatID);
            // FixChargingTime call in order to move overlapped charging times to the end of the bookings that are overlapping 
            FixChargingTime(bookings);
            return BuildSingleCalendarView(_booking);
        }

        public List<BookingCalenderViewVM> GetBookingsTimeSpan(DateTime startTime, DateTime endTime, long boatID)
        {
            var tempArray = _context.Bookings.Where(booking => (booking.BoatID == boatID) && (booking.EndTime >= startTime && booking.StartTime <= endTime) && (booking.Status != 'C')).ToList();
            return BuildBookingCalendarViewVM(tempArray);
        }

        private List<Booking> GetInternalBookingsTimeSpan(DateTime startTime, DateTime endTime, long boatID)
        {
            return _context.Bookings
            .Where(booking => (booking.BoatID == boatID) && (booking.EndTime >= startTime && booking.StartTime <= endTime) && (booking.Status != 'C'))
            .OrderBy(booking => booking.StartTime)
            .ToList();
        }


        public List<BookingCalenderViewVM> GetUserBookings(string UserID, long boatID)
        {
            var tempArray = _context.Bookings.Where(booking => (booking.UserID == UserID) && (booking.BoatID == boatID) && (booking.Status != 'C')).ToList();
            return BuildBookingCalendarViewVM(tempArray);
        }

        public BookingReadableVM GetBooking(long BookingID)
        {
            var _bookingReadable = _context.Bookings.Where(b => b.BookingID == BookingID).Select(booking => new BookingReadableVM()
            {
                BookingID = booking.BookingID,
                StartTime = booking.StartTime,
                EndTime = booking.EndTime,
                ChargingDone = booking.ChargingDone,
                KeyTakenTime = booking.KeyTakenTime,
                KeyReturnedTime = booking.KeyReturnedTime,
                Status = booking.Status,
                Comment = booking.Comment,
                BookingOwnerId = booking.User.Id,
                BookingOwner = booking.User.Name,
                BookingOwnerPhone = booking.User.PhoneNumber,
                BookingOwnerEmail = booking.User.Email,
                BookedBoat = booking.Boat.Name,
                isOfficial = booking.isOfficial
            }).FirstOrDefault();

            return _bookingReadable;
        }

        public List<BookingCalenderViewVM> GetBookings()
        {
            var tempArray = _context.Bookings.ToList();
            return BuildBookingCalendarViewVM(tempArray);
        }

        public BookingCalenderViewVM UpdateBooking(long BookingID, BookingVM booking)
        {
            // TODO: move validate into functions, a lot of overlapping validate logic in UpdateBooking and AddBooking

            Boat boat;
            try
            {
                boat = _context.Boats.Single(b => b.BoatID == booking.BoatID);
            }
            catch
            {
                throw new ArgumentException("Båten finnes ikke.");
            }

            if (!validateBookingTime(booking.StartTime, booking.EndTime))
            {
                throw new ArgumentException("Starttiden må være før sluttiden.");
            }

            TimeSpan maxTime = new TimeSpan(boat.MaxBookingLimit, 0, 0);

            if (booking.EndTime - booking.StartTime > maxTime && !booking.isOfficial)
            {
                throw new ArgumentException($"Booking kan ikke være lenger enn {boat.MaxBookingLimit} timer.");
            }

            TimeSpan chargeTime = getChargingTime(booking.StartTime, booking.EndTime, boat.ChargingTime);

            if (booking.isOfficial)
            {
                chargeTime = new TimeSpan(0, 0, 0);
            }

            DateTime today = DateTime.Today;
            DateTime bookingStartDate = booking.StartTime;
            int advanceLimit = boat.AdvanceBookingLimit;

            if (bookingStartDate.Date > today.AddDays(advanceLimit))
            {
                throw new ArgumentException($"Booking kan bare bli gjort {advanceLimit} dager frem i tid.");
            }

            bool isBookingPeriod = false;
            foreach (var BookingPeriod in _context.BookingPeriods)
            {
                if ((booking.StartTime >= BookingPeriod.StartDate && booking.EndTime <= BookingPeriod.EndDate) &&
                    (booking.StartTime.Hour >= BookingPeriod.BookingOpens && ((booking.EndTime.Hour <= BookingPeriod.BookingCloses && booking.EndTime.Minute == 0) || booking.EndTime.Hour < BookingPeriod.BookingCloses)))
                {
                    isBookingPeriod = true;
                }
            }

            if (!isBookingPeriod)
            {
                throw new ArgumentException("Booking må være i en bookingperiode.");
            }

            var _booking = _context.Bookings.FirstOrDefault(b => b.BookingID == BookingID);
            var bookings = GetBookingsTimeSpan(_booking.StartTime - maxTime, _booking.ChargingDone + maxTime, _booking.BoatID);
            if (_booking != null)
            {
                _booking.StartTime = booking.StartTime;
                _booking.EndTime = booking.EndTime;
                _booking.Status = booking.Status;
                _booking.Comment = booking.Comment;
                _booking.ChargingDone = booking.EndTime + chargeTime;
                _booking.UserID = booking.UserID;
                _booking.BoatID = booking.BoatID;
                _booking.isOfficial = booking.isOfficial;
            }

            foreach (var b in bookings)
            {
                if (b.EndTime <= _booking.StartTime || b.StartTime >= _booking.EndTime || b.BookingID == _booking.BookingID)
                {
                    continue;
                }
                else
                {
                    throw new ArgumentException("Booking overlapper med en annen booking.");
                }
            }

            _context.SaveChanges();

            var internalBookings = GetInternalBookingsTimeSpan(_booking.StartTime - maxTime, _booking.ChargingDone + maxTime, _booking.BoatID);
            FixChargingTime(internalBookings);

            return BuildSingleCalendarView(_booking);
        }

        public void DeleteBooking(long BookingID)
        {
            var _booking = _context.Bookings.FirstOrDefault(b => b.BookingID == BookingID);

            if (_booking != null)
            {
                _context.Bookings.Remove(_booking);
                _context.SaveChanges();

                TimeSpan maxTime = new TimeSpan(_booking.Boat.MaxBookingLimit, 0, 0);
                var internalBookings = GetInternalBookingsTimeSpan(_booking.StartTime - maxTime, _booking.ChargingDone + maxTime, _booking.BoatID);
                FixChargingTime(internalBookings);
            }
        }

        public BookingCalenderViewVM GetHasBooked(string UserID)
        {
            DateTime date = DateTime.Now;
            DateTime startTime = date.Add(new TimeSpan(0, -30, 0));
            DateTime endTime = date.Add(new TimeSpan(0, 30, 0));

            var _booking = _context.Bookings.Where(booking => (
                (booking.UserID == UserID) &&
                (booking.EndTime >= startTime && booking.StartTime <= endTime) &&
                (booking.Status != 'C')
            )).FirstOrDefault();

            if (_booking == null) return null;
            return BuildSingleCalendarView(_booking);
        }

        private List<BookingCalenderViewVM> BuildBookingCalendarViewVM(List<Booking> tempArray)
        {
            var BookingArray = new List<BookingCalenderViewVM>();
            foreach (var booking in tempArray)
            {
                BookingArray.Add(BuildSingleCalendarView(booking));
            }
            return BookingArray;
        }

        private BookingCalenderViewVM BuildSingleCalendarView(Booking booking)
        {
            var user = _context.Users.FirstOrDefault(u => u.Id == booking.UserID);
            var tempBookingVM = new BookingCalenderViewVM()
            {
                BookingID = booking.BookingID,
                StartTime = booking.StartTime,
                EndTime = booking.EndTime,
                ChargingDone = booking.ChargingDone,
                Status = booking.Status,
                Comment = booking.Comment,
                BookingOwner = user.Name,
                BookingOwnerNumber = user.PhoneNumber,
                isOfficial = booking.isOfficial,
            };

            return tempBookingVM;
        }

        public string GetBookingOwner(long booking)
        {
            var _booking = _context.Bookings.FirstOrDefault(b => b.BookingID == booking);
            return _booking.UserID;
        }


        private List<Booking> FixChargingTime(List<Booking> bookings)
        {
            // Slow fast pointer used to fix charging times so that overlapped charging times are added to the end of other bookings
            // This ensures that while charging times can be overwritten, the total recommended charging time stays the same

            var length = bookings.Count;
            var slow = 0;
            var fast = 1;

            foreach (var booking in bookings)
            {
                // Skips official bookings as these are bookings where the boat isn't used as normal, but rather used for maintenance or events
                if (!booking.isOfficial)
                {
                    booking.ChargingDone = booking.EndTime + getChargingTime(booking.StartTime, booking.EndTime, booking.Boat.ChargingTime);
                }
            }


            while (fast < length)
            {
                if (bookings[slow].ChargingDone > bookings[fast].StartTime)
                {
                    var timeDiff = bookings[slow].ChargingDone - bookings[fast].StartTime;
                    bookings[slow].ChargingDone = bookings[fast].StartTime;
                    bookings[fast].ChargingDone += timeDiff;
                }
                slow++;
                fast++;
            }
            _context.SaveChanges();
            return bookings;
        }

        public List<BookingCalenderViewVM> GetFutureUserBookings(string userId)
        {
            var bookings = _context.Bookings.Where(booking => booking.UserID == userId && booking.ChargingDone >= DateTime.Now);

            var ids = bookings.Select(el => el.BoatID);
            var boats = _context.Boats.Where(b => ids.Contains(b.BoatID)).ToList();

            var bookingsList = new List<BookingCalenderViewVM>();
            foreach (var booking in bookings)
            {
                if (booking.Status == 'C') continue;

                var boat = boats.Find(el => el.BoatID == booking.BoatID);

                var tempBooking = new BookingCalenderViewVM()
                {
                    BookingID = booking.BookingID,
                    StartTime = booking.StartTime,
                    EndTime = booking.EndTime,
                    BoatName = boat.Name,
                };
                bookingsList.Add(tempBooking);
            }

            return bookingsList.OrderBy(b => b.StartTime).ToList();
        }
    }
}