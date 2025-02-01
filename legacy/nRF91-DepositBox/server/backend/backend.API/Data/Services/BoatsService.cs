using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;

namespace backend.API.Data.Services
{
    public class BoatsService
    {
        private LockBoxContext _context;

        public BoatsService(LockBoxContext context)
        {
            _context = context;
        }

        public Boat AddBoat(BoatVM boat)
        {
            var _boat = new Boat()
            {
                Name = boat.Name,
                ChargingTime = boat.ChargingTime,
                WeatherLimit = boat.WeatherLimit,
                AdvanceBookingLimit = boat.AdvanceBookingLimit,
                MaxBookingLimit = boat.MaxBookingLimit,
                LockBoxID = boat.LockBoxID,
            };

            _context.Boats.Add(_boat);
            _context.SaveChanges();

            return _boat;
        }

        public async Task<PaginatedObject<BoatListVM>> GetBoats(string search, int pageIndex, int pageSize)
        {

            var boats = from b in _context.Boats select b;
            if (!string.IsNullOrEmpty(search))
            {
                boats = boats.Where(b => b.Name.ToLower().Contains(search.ToLower()));
            }

            boats = boats.OrderBy(b => b.BoatID);

            var paginatedObject = await PaginatedObject<Boat>.CreateAsync(boats, pageIndex, pageSize);
            var BoatVMArray = new List<BoatListVM>();

            foreach (var boat in paginatedObject.Items)
            {
                var tempBoatVM = new BoatListVM()
                {
                    BoatID = boat.BoatID,
                    Name = boat.Name,
                    ChargingTime = boat.ChargingTime,
                    WeatherLimit = boat.WeatherLimit,
                    AdvanceBookingLimit = boat.AdvanceBookingLimit,
                    MaxBookingLimit = boat.MaxBookingLimit,
                    LockBoxID = boat.LockBoxID
                };
                BoatVMArray.Add(tempBoatVM);
            }

            return paginatedObject.ConvertTo<BoatListVM>(BoatVMArray, boats.Count(), pageIndex, pageSize);
        }

        public BoatReadableVM GetBoat(long boatID)
        {
            var _boatReadable = _context.Boats.Where(b => b.BoatID == boatID).Select(boat => new BoatReadableVM()
            {
                BoatID = boat.BoatID,
                Name = boat.Name,
                ChargingTime = boat.ChargingTime,
                WeatherLimit = boat.WeatherLimit,
                AdvanceBookingLimit = boat.AdvanceBookingLimit,
                LockBoxID = boat.LockBoxID,
                BookingsIds = boat.Bookings.Select(booking => booking.BookingID).ToList(),
                BookingPeriodsNames = boat.BookingPeriods.Select(bp => bp.Name).ToList()
            }).FirstOrDefault();

            return _boatReadable;
        }

        public Boat UpdateBoat(long boatID, BoatVM boat)
        {
            var _boat = _context.Boats.FirstOrDefault(b => b.BoatID == boatID);
            if (_boat != null)
            {
                _boat.Name = boat.Name;
                _boat.ChargingTime = boat.ChargingTime;
                _boat.WeatherLimit = boat.WeatherLimit;
                _boat.AdvanceBookingLimit = boat.AdvanceBookingLimit;
                _boat.MaxBookingLimit = boat.MaxBookingLimit;
                _boat.LockBoxID = boat.LockBoxID;

                _context.SaveChanges();
            }

            return _boat;
        }

        public void DeleteBoat(long boatID)
        {
            var _boat = _context.Boats.FirstOrDefault(b => b.BoatID == boatID);

            if (_boat != null)
            {
                _context.Boats.Remove(_boat);
                _context.SaveChanges();
            }
        }
    }
}