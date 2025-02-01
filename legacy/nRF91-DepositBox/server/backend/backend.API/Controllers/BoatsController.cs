using System.Threading.Tasks;
using backend.API.Data.Services;
using backend.API.Data.ViewModels;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    [Authorize(Policy = "BasicClaimsPolicy")]
    public class BoatsController : ControllerBase
    {
        public BoatsService _boatsService;

        public BoatsController(BoatsService boatsService)
        {
            _boatsService = boatsService;
        }

        [HttpPost]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult PostBook([FromBody] BoatVM boat)
        {
            var createdBoat = _boatsService.AddBoat(boat);
            return Ok(createdBoat);
        }

        [HttpGet]
        public async Task<ActionResult<PaginatedObject<BoatListVM>>> GetBoats(string search, int? pageIndex, int? pageSize)
        {
            var allBoats = await _boatsService.GetBoats(search, pageIndex ?? 1, pageSize ?? 25);
            return allBoats;
        }

        [HttpGet("{id}")]
        public IActionResult GetBoat([FromRoute] long id)
        {
            var boat = _boatsService.GetBoat(id);

            if (boat != null)
            {
                return Ok(boat);
            }
            else
            {
                return NotFound("This boat does not exist.");
            };
        }

        [HttpPut("{id}")]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult UpdateBoat([FromRoute] long id, [FromBody] BoatVM boat)
        {
            var updatedBoat = _boatsService.UpdateBoat(id, boat);

            if (updatedBoat != null)
            {
                return Ok(updatedBoat);
            }
            else
            {
                return BadRequest();
            };
        }

        [HttpDelete("{id}")]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult DeleteBoat([FromRoute] long id)
        {
            _boatsService.DeleteBoat(id);
            return Ok();
        }

    }
}
