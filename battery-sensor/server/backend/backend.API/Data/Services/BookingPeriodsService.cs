using System;
using System.Collections.Generic;
using System.Linq;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;

namespace backend.API.Data.Services
{
    public class BookingPeriodsService
    {
        private LockBoxContext _context;

        public BookingPeriodsService(LockBoxContext context)
        {
            _context = context;
        }

        private Boolean validateBookingPeriodDate(DateTime startDate, DateTime endDate)
        {
            return startDate < endDate;
        }

        private Boolean validateBookingPeriodTime(int BookingOpens, int BookingEnds)
        {
            return BookingOpens <= BookingEnds || BookingEnds == 0;
        }

        public BookingPeriod AddBookingPeriod(BookingPeriodVM bookingPeriod)
        {
            if (!validateBookingPeriodDate(bookingPeriod.StartDate, bookingPeriod.EndDate))
            {
                throw new ArgumentException("Start date must be earlier than end date.");
            }
            if (!validateBookingPeriodTime(bookingPeriod.BookingOpens, bookingPeriod.BookingCloses))
            {
                throw new ArgumentException("Opening time must be earlier than closing time.");
            }

            var _bookingPeriod = new BookingPeriod()
            {
                Name = bookingPeriod.Name,
                StartDate = bookingPeriod.StartDate,
                EndDate = bookingPeriod.EndDate,
                BookingOpens = bookingPeriod.BookingOpens,
                BookingCloses = bookingPeriod.BookingCloses,
                BoatID = bookingPeriod.BoatID
            };

            _context.BookingPeriods.Add(_bookingPeriod);
            _context.SaveChanges();

            return _bookingPeriod;
        }

        public List<BookingPeriod> GetBookingPeriods() => _context.BookingPeriods.ToList();

        public List<BookingPeriod> GetBookingPeriodsByBoat(long BoatID)
        {
            return _context.BookingPeriods.Where(bookingPeriod => bookingPeriod.BoatID == BoatID).ToList();
        }

        public BookingPeriodReadableVM GetBookingPeriod(string Name, long BoatID)
        {
            var _bookingPeriodReadable = _context.BookingPeriods.Where(b => b.Name == Name && b.BoatID == BoatID)
                .Select(bookingPeriod => new BookingPeriodReadableVM()
                {
                    Name = bookingPeriod.Name,
                    StartDate = bookingPeriod.StartDate,
                    EndDate = bookingPeriod.EndDate,
                    BookingOpens = bookingPeriod.BookingOpens,
                    BookingCloses = bookingPeriod.BookingCloses,
                    BoatName = bookingPeriod.Name,
                    BoatID = bookingPeriod.BoatID
                }).FirstOrDefault();
            return _bookingPeriodReadable;
        }

        public BookingPeriod UpdateBookingPeriod(string Name, long BoatID, BookingPeriodVM bookingPeriod)
        {
            var _bookingPeriod = _context.BookingPeriods.FirstOrDefault(bp => bp.Name == Name && bp.BoatID == BoatID);
            if (_bookingPeriod != null)
            {
                _bookingPeriod.Name = bookingPeriod.Name;
                _bookingPeriod.StartDate = bookingPeriod.StartDate;
                _bookingPeriod.EndDate = bookingPeriod.EndDate;
                _bookingPeriod.BookingOpens = bookingPeriod.BookingOpens;
                _bookingPeriod.BookingCloses = bookingPeriod.BookingCloses;
                _bookingPeriod.BoatID = bookingPeriod.BoatID;

                _context.SaveChanges();
            }

            return _bookingPeriod;
        }

        public void DeleteBookingPeriod(string Name, long BoatID)
        {
            var _bookingPeriod = _context.BookingPeriods.FirstOrDefault(bp => bp.Name == Name && bp.BoatID == BoatID);

            if (_bookingPeriod != null)
            {
                _context.BookingPeriods.Remove(_bookingPeriod);
                _context.SaveChanges();
            }
        }
    }
}