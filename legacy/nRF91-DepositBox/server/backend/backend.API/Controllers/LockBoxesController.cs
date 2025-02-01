using Microsoft.AspNetCore.Mvc;
using backend.API.Data.Services;
using backend.API.Data.ViewModels;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authorization;
using System.Linq;
using System.Security.Claims;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    [Authorize(Policy = "BasicClaimsPolicy")]
    public class LockBoxesController : ControllerBase
    {
        private LockBoxesService _lockBoxesService;
        private KeyService _keyService;

        public LockBoxesController(LockBoxesService lockBoxesService, KeyService keyService)
        {
            _lockBoxesService = lockBoxesService;
            _keyService = keyService;
        }

        [HttpPost]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult PostLockBox([FromBody] LockBoxVM lockBox)
        {
            var createdBoat = _lockBoxesService.AddLockBox(lockBox);
            return Ok(createdBoat);
        }

        [HttpGet]
        public IActionResult GetLockBoxes()
        {
            var lockBoxes = _lockBoxesService.GetLockBoxes();
            return Ok(lockBoxes);
        }

        [HttpGet("{LockBoxID}")]
        public IActionResult GetLockBox([FromRoute] long LockBoxID)
        {
            var lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            return Ok(lockBox);
        }

        [HttpPut("{LockBoxID}")]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult UpdateLockBox([FromRoute] long LockBoxID, [FromBody] LockBoxVM lockBox)
        {
            var _lockBox = _lockBoxesService.UpdateLockBox(LockBoxID, lockBox);
            return Ok(_lockBox);
        }

        [HttpDelete("{LockBoxID}")]
        [Authorize(policy: "RequireAdminRole")]
        public IActionResult DeleteLockBox([FromRoute] long LockBoxID)
        {
            _lockBoxesService.DeleteLockBox(LockBoxID);
            return Ok();
        }

        [HttpPost("{LockBoxID}/open")]
        public async Task<IActionResult> OpenLockBox([FromRoute] long LockBoxID)
        {
            var userID = User.Claims.FirstOrDefault(c => c.Type == ClaimTypes.NameIdentifier).Value;
            await _lockBoxesService.OpenLock(LockBoxID, userID);
            return Ok();
        }

        [HttpPost("{LockBoxID}/adminOpen")]
        [Authorize(policy: "RequireAdminRole")]
        public async Task<IActionResult> AdminOpenLockBox([FromRoute] long LockBoxID) 
        {
            var userID = User.Claims.FirstOrDefault(c => c.Type == ClaimTypes.NameIdentifier).Value;
            await _lockBoxesService.AdminOpenLock(LockBoxID, userID);
            return Ok();
        }

        [HttpGet("{LockBoxID}/allKeyStatus")]
        public async Task<ActionResult<PaginatedObject<KeyEventVM>>> GetAllKeyStatus([FromRoute] long LockBoxID, int? pageIndex, int? pageSize)
        {
            var _lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            if (_lockBox == null)
                return NotFound();

            var allEvents = await _keyService.GetAllKeyEvents(LockBoxID, pageIndex ?? 1, pageSize ?? 20);

            if (allEvents == null)
                return NoContent();

            return allEvents;
        }


        [HttpPost("{LockBoxID}/generateKeyReport")]
        public IActionResult GenerateKeyReport([FromRoute] long LockBoxID)
        {

            var _lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            if (_lockBox == null)
                return NotFound();

            var report = _keyService.GenerateKeyReport(LockBoxID);

            return Ok(report);
        }

        [HttpGet("{LockBoxID}/latestKeyReport")]
        public IActionResult GetNewestKeyReport([FromRoute] long LockBoxID)
        {
            var _lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            if (_lockBox == null)
                return NotFound();

            var report = _keyService.GetNewestKeyReport(LockBoxID);

            return Ok(report);
        }

        [HttpGet("{LockBoxID}/pingLockBox")]
        public IActionResult PingLockBox([FromRoute] long LockBoxID)
        {

            var _lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            if (_lockBox == null)
                return NotFound();

            var report = _lockBoxesService.PingLockBox(LockBoxID);

            if (!report) return BadRequest();
            return Ok();
        }

        [HttpGet("{LockBoxID}/requestLockStatus")]
        public IActionResult RequestLockStaus([FromRoute] long LockBoxID)
        {

            var _lockBox = _lockBoxesService.GetLockBox(LockBoxID);
            if (_lockBox == null)
                return NotFound();

            var report = _lockBoxesService.RequestLockStatus(LockBoxID);

            if (!report) return BadRequest();
            return Ok();
        }





    }
}
