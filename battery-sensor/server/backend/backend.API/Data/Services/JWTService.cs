
using System;
using System.Collections.Generic;
using System.IdentityModel.Tokens.Jwt;
using System.Linq;
using System.Security.Claims;
using System.Text;
using backend.API.Data.Models;
using backend.API.Data.ViewModels.Authentication;
using Microsoft.Extensions.Configuration;
using Microsoft.IdentityModel.Tokens;

namespace backend.API.Data.Services
{
    public class JWTService
    {
        private readonly IConfiguration _configuration;

        public JWTService(IConfiguration configuration)
        {
            _configuration = configuration;
        }
        public jwtVM GenerateTokenPair(User user, IList<string> roles)
        {
            var accessToken = GenerateAuthToken(user, roles);
            var refreshToken = GenerateRefreshToken(user);

            return new jwtVM
            {
                AccessToken = accessToken,
                RefreshToken = refreshToken
            };
        }

        public string GenerateAuthToken(User user, IList<string> roles)
        {
            // Set user info as claims on the token, can be decoded at jwt.io
            var claims = new List<Claim>
                {
                    new Claim("email", user.Email),
                    new Claim("name", user.Name),
                    new Claim("sub", user.Id),
                    new Claim("isActive", user.isActive.ToString() ),
                };

            foreach (var role in (List<string>)roles)
            {
                claims.Add(new Claim("roles", role));
            }

            var secretKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(ConfigurationManager.AppSetting["JWT:Secret"]));
            var signinCredentials = new SigningCredentials(secretKey, SecurityAlgorithms.HmacSha256);
            var tokenOptions = new JwtSecurityToken(
                issuer: ConfigurationManager.AppSetting["JWT:ValidIssuer"],
                audience: ConfigurationManager.AppSetting["JWT:ValidAudience"],
                claims: claims.ToArray(),
                expires: DateTime.Now.AddMinutes(15),
                signingCredentials: signinCredentials
            );
            return new JwtSecurityTokenHandler().WriteToken(tokenOptions);
        }

        public string GenerateRefreshToken(User user)
        {
            var claims = new List<Claim>
                {
                    new Claim("sub", user.Id),
                };

            var secretKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(ConfigurationManager.AppSetting["JWT:Secret"]));
            var signinCredentials = new SigningCredentials(secretKey, SecurityAlgorithms.HmacSha256);
            var tokenOptions = new JwtSecurityToken(
                issuer: ConfigurationManager.AppSetting["JWT:ValidIssuer"],
                audience: ConfigurationManager.AppSetting["JWT:ValidAudience"],
                claims: claims.ToArray(),
                expires: DateTime.Now.AddDays(7),
                signingCredentials: signinCredentials
            );
            return new JwtSecurityTokenHandler().WriteToken(tokenOptions);
        }

        public string ValidateAndGetUUID(string token)
        {
            var tokenHandler = new JwtSecurityTokenHandler();

            try
            {

                IConfigurationSection jwtSection = _configuration.GetSection("Authentication:JWT");
                tokenHandler.ValidateToken(token, new TokenValidationParameters
                {
                    ValidateIssuer = true,
                    ValidateAudience = true,
                    ValidateLifetime = true,
                    ValidateIssuerSigningKey = true,
                    ValidIssuer = ConfigurationManager.AppSetting["JWT:ValidIssuer"],
                    ValidAudience = ConfigurationManager.AppSetting["JWT:ValidAudience"],
                    IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(ConfigurationManager.AppSetting["JWT:Secret"])),
                    ClockSkew = TimeSpan.Zero
                }, out SecurityToken validatedToken);

                var jwtToken = tokenHandler.ReadJwtToken(token);

                return jwtToken.Claims.FirstOrDefault(claim => claim.Type == "sub").Value;
            }
            catch
            {
                return null;
            }
        }

        public string GetEmailFromExpiredToken(string token)
        {
            var tokenHandler = new JwtSecurityTokenHandler();
            var jwtToken = tokenHandler.ReadJwtToken(token);

            return jwtToken.Claims.FirstOrDefault(claim => claim.Type == "email").Value;
        }
    }
}