using Microsoft.AspNetCore.Mvc;
using backend.API.Data.Services;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using Microsoft.AspNetCore.Authorization;
using System.Collections.Generic;
using System.Threading.Tasks;
using System;
using System.Linq;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    [Authorize(Policy = "BasicClaimsPolicy")]
    public class BookingPeriodsController : ControllerBase
    {
        private readonly LockBoxContext _context;

        private BookingPeriodsService _bookingPeriodsService;

        public BookingPeriodsController(BookingPeriodsService bookingPeriodsService, LockBoxContext context)
        {
            _bookingPeriodsService = bookingPeriodsService;
            _context = context;
        }

        [HttpPost]
        [Authorize(policy: "RequireManagerRole")]
        public IActionResult PostBookingPeriod([FromBody] BookingPeriodVM bookingPeriod)
        {
            try
            {
                var createdBookingPeriod = _bookingPeriodsService.AddBookingPeriod(bookingPeriod);
                return Ok(createdBookingPeriod);
            }
            catch (ArgumentException e)
            {
                return BadRequest(e.Message);
            }
        }

        [HttpGet("{BoatID}")]
        public async Task<ActionResult<PaginatedObject<BookingPeriodReadableVM>>> GetBookingPeriodsByBoat([FromRoute] long BoatID, string search, int? pageIndex, int? pageSize)
        {

            var periods = from p in _context.BookingPeriods select p;
            periods = periods.Where(p => p.BoatID == BoatID);

            if (!String.IsNullOrEmpty(search))
            {
                periods = periods.Where(s => s.Name.ToLower().Contains(search.ToLower()));
            }

            periods = periods.OrderByDescending(p => p.Name);

            var paginatedObject = await PaginatedObject<BookingPeriod>.CreateAsync(periods, pageIndex ?? 1, pageSize ?? 20);

            var ReadableArray = new List<BookingPeriodReadableVM>();
            foreach (var period in paginatedObject.Items)
            {
                BookingPeriodReadableVM temp = new BookingPeriodReadableVM()
                {
                    Name = period.Name,
                    StartDate = period.StartDate,
                    EndDate = period.EndDate,
                    BookingOpens = period.BookingOpens,
                    BookingCloses = period.BookingCloses,
                    BoatID = period.BoatID
                };
                ReadableArray.Add(temp);
            }

            return paginatedObject.ConvertTo<BookingPeriodReadableVM>(ReadableArray, periods.Count(), pageIndex ?? 1, pageSize ?? 20);
        }

        [HttpGet("name={Name}&boat={BoatID}")]
        public IActionResult GetBookingPeriod([FromRoute] string Name, [FromRoute] long BoatID)
        {
            var bookingPeriod = _bookingPeriodsService.GetBookingPeriod(Name, BoatID);
            return Ok(bookingPeriod);
        }

        [HttpPut("name={Name}&boat={BoatID}")]
        [Authorize(policy: "RequireManagerRole")]
        public IActionResult UpdateBookingPeriod([FromRoute] string Name, [FromRoute] long BoatID, [FromBody] BookingPeriodVM bookingPeriod)
        {
            var _bookingPeriod = _bookingPeriodsService.UpdateBookingPeriod(Name, BoatID, bookingPeriod);
            return Ok(_bookingPeriod);
        }

        [HttpDelete("name={Name}&boat={BoatID}")]
        [Authorize(policy: "RequireManagerRole")]
        public IActionResult DeleteBookingPeriod([FromRoute] string Name, [FromRoute] long BoatID)
        {
            _bookingPeriodsService.DeleteBookingPeriod(Name, BoatID);
            return Ok();
        }
    }
}
