using Microsoft.AspNetCore.Mvc;
using backend.API.Data.Services;
using backend.API.Data.ViewModels;
using System;
using Microsoft.AspNetCore.Authorization;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController, Authorize(Policy = "BasicClaimsPolicy")]
    public class BookingsController : ControllerBase
    {
        private BookingsService _bookingsService;
        private readonly IAuthorizationService _authorizationService;

        public BookingsController(BookingsService bookingsService, IAuthorizationService authorizationService)
        {
            _bookingsService = bookingsService;
            _authorizationService = authorizationService;
        }

        [HttpPost]
        public IActionResult PostBooking([FromBody] BookingVM booking)
        {
            try
            {
                var createdBooking = _bookingsService.AddBooking(booking);
                return Ok(createdBooking);
            }
            catch (ArgumentException e)
            {
                return BadRequest(e.Message);
            }
        }

        [HttpGet]
        public IActionResult GetAllBookings()
        {
            var bookings = _bookingsService.GetBookings();
            return Ok(bookings);
        }

        [HttpGet("{BoatID}/span")]
        public IActionResult GetBookingsSpan(DateTime startDate, DateTime endDate, [FromRoute] long BoatID)
        {
            var bookings = _bookingsService.GetBookingsTimeSpan(startDate, endDate, BoatID);
            return Ok(bookings);
        }


        [HttpGet("{BoatID}/byUser/{UserID}/")]
        public IActionResult GetBookingsByUser(string UserID, long BoatID)
        {
            var bookings = _bookingsService.GetUserBookings(UserID, BoatID);
            return Ok(bookings);
        }

        [HttpGet("{BookingID}")]
        public IActionResult GetBooking([FromRoute] long BookingID)
        {
            var booking = _bookingsService.GetBooking(BookingID);
            return Ok(booking);
        }

        [HttpPut("{BookingID}")]
        public async Task<IActionResult> UpdateBooking([FromRoute] long BookingID, [FromBody] BookingVM booking)
        {
            var _bookingOwner = _bookingsService.GetBookingOwner(BookingID);
            // Check that user sending request is also the owner of the booking to be updated or admin
            var authorizationResult = await _authorizationService.AuthorizeAsync(User, _bookingOwner, "EditPolicy");

            if (authorizationResult.Succeeded)
            {
                try
                {
                    var createdBooking = _bookingsService.UpdateBooking(BookingID, booking);
                    return Ok(createdBooking);
                }
                catch (ArgumentException e)
                {
                    return BadRequest(e.Message);
                }
            }
            else if (User.Identity.IsAuthenticated)
            {
                return new ForbidResult();
            }
            else
            {
                return new ChallengeResult();
            }
        }

        [HttpDelete("{BookingID}")]
        public async Task<IActionResult> DeleteBooking([FromRoute] long BookingID)
        {

            var _bookingOwner = _bookingsService.GetBookingOwner(BookingID);
            // Check that user sending request is also the owner of the booking to be updated or admin
            var authorizationResult = await _authorizationService.AuthorizeAsync(User, _bookingOwner, "EditPolicy");

            if (authorizationResult.Succeeded)
            {
                _bookingsService.DeleteBooking(BookingID);
                return Ok();
            }
            else if (User.Identity.IsAuthenticated)
            {
                return new ForbidResult();
            }
            else
            {
                return new ChallengeResult();
            }
        }

        [HttpGet("{UserID}/hasBooked")]
        public ActionResult<BookingCalenderViewVM> GetHasBooked([FromRoute] string UserID)
        {
            var hasBooked = _bookingsService.GetHasBooked(UserID);
            if (hasBooked == null) return Accepted();
            else return Ok(hasBooked);
        }

        [HttpGet("{UserID}/futureBookings")]
        public IActionResult GetFutureUserBookings([FromRoute] string UserID)
        {
            var bookings = _bookingsService.GetFutureUserBookings(UserID);
            return Ok(bookings);
        }
    }
}
