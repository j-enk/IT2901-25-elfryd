using System;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.ViewModels.Authentication;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Identity;
using Microsoft.Extensions.DependencyInjection;

namespace backend.API.Data
{
    public class AppDbInitializer
    {
        public static void Seed(IApplicationBuilder applicationBuilder)
        {
            using (var serviceScope = applicationBuilder.ApplicationServices.CreateScope())
            {

                var context = serviceScope.ServiceProvider.GetService<LockBoxContext>();
                context.Database.EnsureCreated();


                IdentityResult mainadmin;
                if (!context.Users.Any())
                {
                    Console.WriteLine("Triggered.");
                    var userManager = serviceScope.ServiceProvider.GetService<UserManager<User>>();

                    User user_admin = new User()
                    {
                        Name = "Jens Lien",
                        UserName = "0907.lien@gmail.com",
                        PhoneNumber = "93215646",
                        Email = "0907.lien@gmail.com",
                        isActive = true,
                    };

                    mainadmin = userManager.CreateAsync(user_admin).Result;

                    if (mainadmin.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_admin, UserRoles.Admin).Wait();
                    }

                    User user_admin_2 = new User()
                    {
                        Name = "Martin Skatvedt",
                        UserName = "sebheg77@gmail.com",
                        PhoneNumber = "87654321",
                        Email = "sebheg77@gmail.com",
                        isActive = true,
                    };

                    IdentityResult result_2 = userManager.CreateAsync(user_admin_2).Result;

                    if (result_2.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_admin_2, UserRoles.Admin).Wait();
                    }

                    User user_admin_3 = new User()
                    {
                        Name = "Sondre Stokke",
                        UserName = "sondre.stokke@online.ntnu.no",
                        PhoneNumber = "69696969",
                        Email = "sondre.stokke@online.ntnu.no",
                        isActive = true,
                    };

                    IdentityResult result_69 = userManager.CreateAsync(user_admin_3).Result;

                    if (result_69.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_admin_3, UserRoles.Admin).Wait();
                    }

                    User user_admin_4 = new User()
                    {
                        Name = "Haavard",
                        UserName = "haavard.brummenaes@gmail.com",
                        PhoneNumber = "69696968",
                        Email = "haavard.brummenaes@gmail.com",
                        isActive = true,
                    };

                    IdentityResult result_4 = userManager.CreateAsync(user_admin_4).Result;

                    if (result_4.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_admin_4, UserRoles.Admin).Wait();
                    }

                    User admin_test = new User()
                    {
                        Name = "Test Admin",
                        Id = "e57ac49d-f5e5-45e4-841b-7fac77d8d5e3",
                        UserName = "testadmin@test.com",
                        PhoneNumber = "87654321",
                        Email = "testadmin@test.com",
                        isActive = true,
                    };

                    IdentityResult result_3 = userManager.CreateAsync(admin_test).Result;

                    if (result_3.Succeeded)
                    {
                        userManager.AddToRoleAsync(admin_test, UserRoles.Admin).Wait();
                    }

                    User user_member = new User()
                    {
                        Name = "Kari Nordmann",
                        UserName = "haavard.brummenaes@online.ntnu.no",
                        PhoneNumber = "87654321",
                        Email = "haavard.brummenaes@online.ntnu.no",
                        isActive = true,
                    };

                    IdentityResult result1 = userManager.CreateAsync(user_member).Result;

                    if (result1.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_member, UserRoles.Member).Wait();
                    }

                    User user_test = new User()
                    {
                        Name = "Test Testesen",
                        Id = "87766d63-3775-4083-88c0-c13710b1b4d3",
                        UserName = "test@test.com",
                        PhoneNumber = "123",
                        Email = "test@test.com",
                        isActive = true,
                    };

                    IdentityResult result2 = userManager.CreateAsync(user_test).Result;

                    if (result2.Succeeded)
                    {
                        userManager.AddToRoleAsync(user_test, UserRoles.Member).Wait();
                    }

                    context.SaveChangesAsync();
                };

                if (!context.LockBoxes.Any())
                {
                    context.LockBoxes.Add(new LockBox()
                    {
                        Topic = "nrf/command",
                        isOpen = false,
                        lastInteractionUserID = context.Users.FirstOrDefault().Id
                    });
                    context.SaveChanges();
                }

                if (!context.KeyEvents.Any())
                {
                    LockBox lockBox = context.LockBoxes.FirstOrDefault();
                    context.KeyEvents.Add(new KeyEvent()
                    {
                        LockBox = lockBox,
                        TimeStamp = DateTime.Now,
                        Action = KeyEventActions.Removed,
                        UserID = context.Users.FirstOrDefault().Id
                    });
                    context.SaveChanges();
                }

                if (!context.Boats.Any())
                {
                    context.Boats.Add(new Boat()
                    {
                        Name = "Elfryd",
                        ChargingTime = 50,
                        WeatherLimit = 20,
                        AdvanceBookingLimit = 14,
                        MaxBookingLimit = 8,
                        LockBoxID = 1
                    });

                    context.Boats.Add(new Boat()
                    {
                        Name = "Elfryd 2",
                        ChargingTime = 50,
                        WeatherLimit = 20,
                        AdvanceBookingLimit = 90,
                        MaxBookingLimit = 8,
                        LockBoxID = 1
                    });

                    context.SaveChanges();
                };

                if (!context.BookingPeriods.Any())
                {
                    context.BookingPeriods.Add(new BookingPeriod()
                    {
                        Name = "Sommer 2022",
                        StartDate = new DateTime(2022, 05, 01),
                        EndDate = new DateTime(2022, 09, 01),
                        BookingOpens = 8,
                        BookingCloses = 22,
                        BoatID = 1,
                    });
                    context.SaveChanges();
                };
                if (!context.Bookings.Any())
                {
                    var adminUser = context.Users.Where(u => u.Email == "0907.lien@gmail.com").FirstOrDefault();
                    var memberUser = context.Users.Where(u => u.Email == "haavard.brummenaes@online.ntnu.no").FirstOrDefault();
                    context.Bookings.AddRange(new Booking()
                    {
                        StartTime = new DateTime(2022, 4, 22, 12, 0, 0),
                        EndTime = new DateTime(2022, 4, 22, 14, 0, 0),
                        ChargingDone = new DateTime(2022, 4, 22, 15, 0, 0),
                        Status = 'P',
                        UserID = memberUser.Id,
                        BoatID = 1
                    });

                    context.Bookings.AddRange(new Booking()
                    {
                        StartTime = new DateTime(2022, 4, 26, 8, 0, 0),
                        EndTime = new DateTime(2022, 4, 26, 16, 0, 0),
                        ChargingDone = new DateTime(2022, 4, 26, 16, 0, 0),
                        Status = 'P',
                        Comment = "Bytte av motor",
                        UserID = adminUser.Id,
                        BoatID = 1
                    });
                    context.SaveChanges();
                }

            }
        }

        public static async Task SeedRoles(IApplicationBuilder applicationBuilder)
        {
            using (var serviceScope = applicationBuilder.ApplicationServices.CreateScope())
            {

                var context = serviceScope.ServiceProvider.GetService<LockBoxContext>();
                context.Database.EnsureCreated();

                var roleManager = serviceScope.ServiceProvider.GetService<RoleManager<IdentityRole>>();

                if (!await roleManager.RoleExistsAsync(UserRoles.Admin))
                {
                    await roleManager.CreateAsync(new IdentityRole(UserRoles.Admin));
                }
                if (!await roleManager.RoleExistsAsync(UserRoles.Member))
                {
                    await roleManager.CreateAsync(new IdentityRole(UserRoles.Member));
                }
            }
        }
    }
}