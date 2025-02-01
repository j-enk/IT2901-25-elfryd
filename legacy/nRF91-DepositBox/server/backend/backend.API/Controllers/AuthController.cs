using System;
using System.Security.Claims;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.Services;
using backend.API.Data.ViewModels.Authentication;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;

namespace backend.API.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class AuthController : ControllerBase
    {
        private readonly LockBoxContext _context;
        private readonly UserManager<User> _userManager;
        private readonly RoleManager<IdentityRole> _roleManager;
        private readonly SignInManager<User> _signInManager;
        private readonly IConfiguration _configuration;
        private readonly IAuthorizationService _authorizationService;
        private readonly IWebHostEnvironment _env;
        private readonly JWTService _jwtService;
        public AuthController(UserManager<User> userManager,
            RoleManager<IdentityRole> roleManager,
            LockBoxContext context,
            IConfiguration configuration, SignInManager<User> signInManager, IAuthorizationService authorizationService, JWTService jwtService, IWebHostEnvironment env)
        {
            _context = context;
            _userManager = userManager;
            _roleManager = roleManager;
            _configuration = configuration;
            _signInManager = signInManager;
            _authorizationService = authorizationService;
            _jwtService = jwtService;
            _env = env;
        }

        [HttpPost("account/external-login")]
        public IActionResult ExternalLogin(string provider, string returnUrl)
        {
            var baseRedirectURL = ConfigurationManager.AppSetting["AuthRedirectURL"];
            var redirectUrl = $"{baseRedirectURL}?returnUrl={returnUrl}";
            
            var properties = _signInManager.ConfigureExternalAuthenticationProperties(provider, redirectUrl);
            properties.AllowRefresh = true;
            return Challenge(properties, provider);
        }

        [HttpGet]
        [Route("account/external-auth-callback")]
        public async Task<IActionResult> ExternalLoginCallback()
        {
            IConfiguration section = _configuration.GetSection("Cors:Url");
            ExternalLoginInfo info = await _signInManager.GetExternalLoginInfoAsync();

            System.Console.WriteLine();

            var result = await ExternalLoginHandler(info);

            if (result == null)
            {
                //Update this with a method for Redirect to a page with information of why they can not log in.
                return BadRequest();
            }

            // Adds Set-Cookie header to response in order to set cookies in the users browsers
            Response.Cookies.Append("refresh_token", result.RefreshToken, new CookieOptions
            {
                SameSite = SameSiteMode.None,
                Secure = true,
                HttpOnly = false,
                Expires = DateTime.Now.AddDays(7),
                Domain = ConfigurationManager.AppSetting["CookieDomain"] 
            });

            Response.Cookies.Append("token", result.AccessToken, new CookieOptions
            {
                SameSite = SameSiteMode.None,
                Secure = true,
                HttpOnly = false,
                Expires = DateTime.Now.AddMinutes(20),
                Domain = ConfigurationManager.AppSetting["CookieDomain"] 
            });

            return Redirect(ConfigurationManager.AppSetting["FrontendRedirectURL"]);
        }

        private async Task<jwtVM> ExternalLoginHandler(ExternalLoginInfo info)
        {
            if (info.Equals(null))
                return null;

            var signinResult = await _signInManager.ExternalLoginSignInAsync(info.LoginProvider, info.ProviderKey, false);
            var email = info.Principal.FindFirst(ClaimTypes.Email);

            var user = await _userManager.FindByEmailAsync(email.Value);

            if ((user == null) || (email == null))
                return null;

            await _userManager.AddLoginAsync(user, info);
            await _signInManager.SignInAsync(user, false);
            var roles = await _userManager.GetRolesAsync(user);
            var tokenPair = _jwtService.GenerateTokenPair(user, roles);
            return tokenPair;
        }

        [HttpPost("refresh-token")]
        public async Task<IActionResult> RefreshToken()
        {
            var refreshToken = Request.Cookies["refresh_token"].ToString();
            var accessToken = Request.Cookies["token"].ToString();

            if (refreshToken == null || refreshToken == "")
                return BadRequest("Refresh token is not provided");

            if (accessToken == null || accessToken == "")
                return BadRequest("Access token is not provided");


            var email = _jwtService.GetEmailFromExpiredToken(accessToken);
            var user = await _userManager.FindByEmailAsync(email);
            var roles = await _userManager.GetRolesAsync(user);
            
            // if refresh token expired or invalid returns 403 Forbidden
            var refresh_uuid = _jwtService.ValidateAndGetUUID(refreshToken);

            if (refresh_uuid == null)
            {
                return new ForbidResult();
            }

            if (refresh_uuid != user.Id)
            {
                return new ForbidResult();
            }

            var NewAccessToken = _jwtService.GenerateAuthToken(user, roles);

            Response.Cookies.Append("token", NewAccessToken, new CookieOptions
            {
                SameSite = SameSiteMode.None,
                Secure = true,
                HttpOnly = false,
                Expires = DateTime.Now.AddMinutes(20),
                Domain = ConfigurationManager.AppSetting["CookieDomain"] 
            });

            return Ok();
        }

    }
}
