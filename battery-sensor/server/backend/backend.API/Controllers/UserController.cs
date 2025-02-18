using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using backend.API.Data.ViewModels.Authentication;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    [Authorize(policy: "BasicClaimsPolicy")]
    public class UserController : ControllerBase
    {
        private readonly LockBoxContext _context;
        private readonly UserManager<User> _userManager;
        private readonly RoleManager<IdentityRole> _roleManager;
        private readonly IAuthorizationService _authorizationService;
        public UserController(UserManager<User> userManager, RoleManager<IdentityRole> roleManager, IAuthorizationService authorizationService, LockBoxContext context)
        {
            _context = context;
            _userManager = userManager;
            _roleManager = roleManager;
            _authorizationService = authorizationService;
        }

        [HttpPost]
        [Authorize(policy: "RequireAdminRole")]
        [Authorize(policy: "BasicClaimsPolicy")]
        public async Task<IActionResult> PostUser([FromBody] UserVM user)
        {
            var userExists = await _userManager.FindByEmailAsync(user.Email);
            if (userExists != null)
            {
                return BadRequest("Brukeren finnes allerede.");
            }

            User newUser = new User()
            {
                Name = user.Name,
                UserName = user.Email,
                PhoneNumber = user.Phone,
                Email = user.Email,
                isActive = user.isActive,
            };

            var result = await _userManager.CreateAsync(newUser);

            if (!result.Succeeded)
            {
                return BadRequest("User could not be created.");
            }

            switch (user.Role)
            {
                case "Admin":
                    await _userManager.AddToRoleAsync(newUser, UserRoles.Admin);
                    break;
                case "Member":
                    await _userManager.AddToRoleAsync(newUser, UserRoles.Member);
                    break;
                default:
                    await _userManager.AddToRoleAsync(newUser, UserRoles.Member);
                    break;
            }

            return Created(nameof(PostUser), $"User {user.Email} created");
        }

        [HttpGet]
        [Authorize(policy: "RequireAdminRole")]
        [Authorize(policy: "BasicClaimsPolicy")]
        public async Task<ActionResult<PaginatedObject<UserReadableVM>>> GetUsers(string search, int? pageIndex, int? pageSize)
        {
            var users = from u in _context.Users select u;
            if (!String.IsNullOrEmpty(search))
            {
                users = users.Where(s => s.Name.ToLower().Contains(search.ToLower()));
            }

            users = users.OrderByDescending(u => u.Name);

            var paginatedObject = await PaginatedObject<User>.CreateAsync(users, pageIndex ?? 1, pageSize ?? 20);


            var ReadableArray = new List<UserReadableVM>();
            foreach (var user in paginatedObject.Items)
            {
                var roles = await _userManager.GetRolesAsync(user);
                UserReadableVM temp = new UserReadableVM()
                {
                    ID = user.Id,
                    Name = user.Name,
                    Email = user.Email,
                    Phone = user.PhoneNumber,
                    isActive = user.isActive,
                    Roles = (List<string>)roles,
                    NumberOfBookings = user.Bookings == null ? 0 : user.Bookings.Count
                };
                ReadableArray.Add(temp);
            }

            return paginatedObject.ConvertTo<UserReadableVM>(ReadableArray, users.Count(), pageIndex ?? 1, pageSize ?? 20);
        }

        [HttpGet("{UserID}")]
        [Authorize(policy: "BasicClaimsPolicy")]
        public async Task<IActionResult> GetUser([FromRoute] string UserID)
        {
            var authorizationResult = await _authorizationService.AuthorizeAsync(User, UserID, "EditPolicy");
            // Check that user sending request is also the user to be fetched or admin
            if (authorizationResult.Succeeded)
            {
                var user = await _userManager.FindByIdAsync(UserID);
                if (user != null)
                {
                    return Ok(user);
                }
                else
                {
                    return NotFound("This user does not exist.");
                };
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

        [HttpPut("{UserID}")]
        [Authorize(policy: "BasicClaimsPolicy")]
        public async Task<ActionResult> UpdateUser([FromRoute] string UserID, [FromBody] UserVM user)
        {
            var authorizationResult = await _authorizationService.AuthorizeAsync(User, UserID, "EditPolicy");
            // Check that user sending request is also the user to be updated or admin
            if (authorizationResult.Succeeded)
            {
                var existingUser = await _userManager.FindByIdAsync(UserID);

                if (existingUser != null)
                {
                    existingUser.Name = user.Name;
                    existingUser.Email = user.Email;
                    existingUser.PhoneNumber = user.Phone;
                    existingUser.isActive = user.isActive;

                    var existingRoles = await _userManager.GetRolesAsync(existingUser);
                    await _userManager.RemoveFromRolesAsync(existingUser, existingRoles);

                    switch (user.Role)
                    {
                        case "Admin":
                            await _userManager.AddToRoleAsync(existingUser, UserRoles.Admin);
                            break;
                        case "Member":
                            await _userManager.AddToRoleAsync(existingUser, UserRoles.Member);
                            break;
                        default:
                            await _userManager.AddToRoleAsync(existingUser, UserRoles.Member);
                            break;
                    }
                }
                else
                {
                    return NotFound();
                };

                await _context.SaveChangesAsync();
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

        [HttpDelete("{UserID}")]
        [Authorize(policy: "RequireAdminRole")]
        [Authorize(policy: "BasicClaimsPolicy")]
        public async Task<IActionResult> DeleteUser([FromRoute] string UserID)
        {
            var user = await _userManager.FindByIdAsync(UserID);
            if (user == null)
                return NotFound();

            var result = await _userManager.DeleteAsync(user);
            return Ok();
        }

        [HttpGet("Profile")]
        public async Task<IActionResult> GetUserProfile()
        {
            var user = await _userManager.GetUserAsync(User);
            if (user == null)
                return NotFound("User not found");

            var roles = await _userManager.GetRolesAsync(user);
            UserReadableVM userReadable = new UserReadableVM()
            {
                ID = user.Id,
                Name = user.Name,
                Email = user.Email,
                Phone = user.PhoneNumber,
                Roles = (List<string>)roles,
                isActive = user.isActive,
                NumberOfBookings = user.Bookings == null ? 0 : user.Bookings.Count
            };

            return Ok(userReadable);
        }
    }
}
