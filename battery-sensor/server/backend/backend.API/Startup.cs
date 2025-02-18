using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.OpenApi.Models;
using Microsoft.EntityFrameworkCore;
using backend.API.Data.Services;
using backend.API.Data;
using Microsoft.AspNetCore.Identity;
using backend.API.Data.Models;
using backend.API.MQTT;
using System;
using Microsoft.AspNetCore.HttpLogging;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.IdentityModel.Tokens;
using System.Text;
using System.Collections.Generic;
using Microsoft.AspNetCore.Authorization;
using System.Security.Claims;

namespace backend.API
{
    public class Startup
    {
        readonly string MyAllowSpecificOrigins = "_myAllowSpecificOrigins";
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        // This method gets called by the runtime. Use this method to add services to the container.
        public void ConfigureServices(IServiceCollection services)
        {
            services.AddCors(options =>
            {
                options.AddPolicy(name: MyAllowSpecificOrigins,
                    builder =>
                    {
                        builder.WithOrigins("http://localhost:3000", "https://elfryd.narverk.no")
                        .AllowAnyMethod()
                        .AllowAnyHeader()
                        .AllowCredentials();
                    });
            });

            services.AddIdentity<User, IdentityRole>()
                .AddEntityFrameworkStores<LockBoxContext>();


            services.AddAuthentication(options =>
            {
                options.DefaultAuthenticateScheme = JwtBearerDefaults.AuthenticationScheme;
                options.DefaultChallengeScheme = JwtBearerDefaults.AuthenticationScheme;
            })
            .AddJwtBearer(options =>
            {
                options.TokenValidationParameters = new TokenValidationParameters
                {
                    ValidateIssuer = true,
                    ValidateAudience = true,
                    ValidateLifetime = true,
                    ValidateIssuerSigningKey = true,
                    ValidIssuer = ConfigurationManager.AppSetting["JWT:ValidIssuer"],
                    ValidAudience = ConfigurationManager.AppSetting["JWT:ValidAudience"],
                    IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(ConfigurationManager.AppSetting["JWT:Secret"])),
                    ClockSkew = TimeSpan.Zero
                };
            })
                .AddGoogle(options =>
                {
                    IConfigurationSection googleAuthNSection =
                           Configuration.GetSection("Authentication:Google");

                    options.ClientId = ConfigurationManager.AppSetting["Google:ClientId"];
                    options.ClientSecret = ConfigurationManager.AppSetting["Google:ClientSecret"];
                    options.SaveTokens = true;
                    options.Scope.Add("profile");
                    options.Scope.Add("openid");
                    options.SignInScheme = Microsoft.AspNetCore.Identity.IdentityConstants.ExternalScheme;
                });

            services.AddAuthorization(options =>
            {
                // Policies used in the controllers. These decide which roles are allowed to hit which endpoints
                options.AddPolicy("RequireAdminRole", policy => policy.RequireRole("Admin"));
                options.AddPolicy("RequireManagerRole", policy => policy.RequireRole("Admin", "Manager"));
                options.AddPolicy("EditPolicy", policy => policy.Requirements.Add(new SameAuthorRequirement()));
                options.AddPolicy("BasicClaimsPolicy", policy =>
                {
                    policy.RequireClaim("isActive", "True");
                    policy.RequireClaim(ClaimTypes.Email);
                });
            });
            services.AddControllers();

            services.AddSwaggerGen(c =>
            {
                c.SwaggerDoc("v1", new OpenApiInfo { Title = "backend.API", Version = "v1" });
                c.AddSecurityDefinition("Bearer", new OpenApiSecurityScheme
                {
                    Scheme = "Bearer",
                    BearerFormat = "JWT",
                    In = ParameterLocation.Header,
                    Name = "Authorization",
                    Description = "Bearer Authentication with JWT Token",
                    Type = SecuritySchemeType.Http
                });

                c.AddSecurityRequirement(new OpenApiSecurityRequirement
                {
                    {
                        new OpenApiSecurityScheme
                    {
                        Reference = new OpenApiReference
                        {
                            Id = "Bearer",
                            Type = ReferenceType.SecurityScheme
                        }
                    },
                    new List<string>()
                     }
             });
            });

            IConfigurationSection dbSection = Configuration.GetSection("SQLiteConnection");
            try
            {
                services.AddDbContext<LockBoxContext>(options =>
                    options
                        .UseSqlite(Configuration.GetConnectionString("SQLiteConnection"))
                        .UseSnakeCaseNamingConvention()
                        //.UseLoggerFactory(LoggerFactory.Create(builder => builder.AddConsole()))
                        .EnableSensitiveDataLogging());
            }

            catch (Exception ex)
            {
            }
            

            services.AddDatabaseDeveloperPageExceptionFilter();
            services.AddControllersWithViews()
                .AddNewtonsoftJson(options =>
                options.SerializerSettings.ReferenceLoopHandling = Newtonsoft.Json.ReferenceLoopHandling.Ignore
                );

            services.AddSingleton<IAuthorizationHandler, DocumentAuthorizationHandler>();


            services.AddSingleton<IMQTTClientService, MQTTClientService>();
            services.AddTransient<BoatsService>();
            services.AddTransient<LockBoxesService>();
            services.AddTransient<BookingPeriodsService>();
            services.AddTransient<BookingsService>();
            services.AddTransient<JWTService>();
            services.AddTransient<KeyService>();

            services.AddHttpLogging(logging =>
            {
                logging.LoggingFields = HttpLoggingFields.All;
                logging.RequestBodyLogLimit = 4096;
                logging.ResponseBodyLogLimit = 4096;

            });
        }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IWebHostEnvironment env)
        {
           // if (env.IsDevelopment())
            {
                app.UseDeveloperExceptionPage();
                app.UseSwagger();
                app.UseSwaggerUI(c =>
                {
                    c.SwaggerEndpoint("/swagger/v1/swagger.json", "backend.API v1");
                    c.RoutePrefix = "doc";
                });

            }

            app.UseHttpLogging();
            app.UseHttpsRedirection();

            app.UseRouting();

            app.UseCors(MyAllowSpecificOrigins);

            app.UseAuthentication();
            app.UseAuthorization();


            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllers();
            });

            AppDbInitializer.SeedRoles(app).Wait();
            AppDbInitializer.Seed(app);
        }
    }
}
