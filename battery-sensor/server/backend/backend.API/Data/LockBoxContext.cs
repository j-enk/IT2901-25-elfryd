using Microsoft.EntityFrameworkCore;
using backend.API.Data.Models;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;

namespace backend.API
{
    public class LockBoxContext : IdentityDbContext<User>
    {

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            modelBuilder.Entity<User>()
            .HasMany(u => u.Bookings)
            .WithOne(b => b.User)
            .OnDelete(DeleteBehavior.Cascade);
            base.OnModelCreating(modelBuilder);
        }
        public LockBoxContext(DbContextOptions<LockBoxContext> options)
            : base(options)
        {
        }
        public DbSet<Boat> Boats { get; set; }
        public DbSet<Booking> Bookings { get; set; }
        public DbSet<BookingPeriod> BookingPeriods { get; set; }
        public DbSet<LockBox> LockBoxes { get; set; }
        public DbSet<User> User { get; set; }
        public DbSet<KeyEvent> KeyEvents { get; set; }
        public DbSet<KeyReport> KeyReports { get; set; }
    }

    public class TestDbContext : DbContext
    {
        public TestDbContext(DbContextOptions<TestDbContext> options)
            : base(options)
        {
        }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            base.OnModelCreating(modelBuilder);
        }
        public DbSet<Boat> Boats { get; set; }
        public DbSet<Booking> Bookings { get; set; }
        public DbSet<BookingPeriod> BookingPeriods { get; set; }
        public DbSet<LockBox> LockBoxes { get; set; }
        public DbSet<User> Users { get; set; }
        public DbSet<KeyReport> KeyEvents { get; set; }
    }
}