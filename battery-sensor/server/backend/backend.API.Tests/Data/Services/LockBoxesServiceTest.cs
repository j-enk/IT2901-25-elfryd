using backend.API.Data;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using backend.API.Data.Services;
using Microsoft.EntityFrameworkCore;
using System.Collections.Generic;
using System.Linq;
using System;
using Xunit;

namespace backend.API.Tests.Services
{
    public class LockBoxesServiceTests
    {
        private LockBoxContext _context;
        private LockBoxesService _lockBoxesService;

        public LockBoxesServiceTests()
        {
            var dbOptionsBuilder = new DbContextOptionsBuilder<LockBoxContext>().UseInMemoryDatabase(Guid.NewGuid().ToString());
            _context = new LockBoxContext(dbOptionsBuilder.Options);
            _lockBoxesService = new LockBoxesService(_context, null);
        }

        [Fact]
        public void TestAddLockBox()
        {
            var lockBoxVM = new LockBoxVM { Topic = "Test Topic", isOpen = false };

            var lockBox = _lockBoxesService.AddLockBox(lockBoxVM);

            Assert.NotNull(lockBox);
            Assert.Equal(lockBoxVM.Topic, lockBox.Topic);
            Assert.False(lockBox.isOpen);
        }

        [Fact]
        public void TestGetLockBoxes()
        {
            var lockBox1 = new LockBox { Topic = "Test Topic 1", isOpen = false };
            var lockBox2 = new LockBox { Topic = "Test Topic 2", isOpen = true };
            _context.LockBoxes.Add(lockBox1);
            _context.LockBoxes.Add(lockBox2);
            _context.SaveChanges();

            var lockBoxes = _lockBoxesService.GetLockBoxes();

            Assert.NotNull(lockBoxes);
            Assert.Equal(2, lockBoxes.Count);
            Assert.Equal(lockBox1.Topic, lockBoxes[0].Topic);
            Assert.Equal(lockBox1.isOpen, lockBoxes[0].isOpen);
            Assert.Equal(lockBox2.Topic, lockBoxes[1].Topic);
            Assert.Equal(lockBox2.isOpen, lockBoxes[1].isOpen);
        }

        [Fact]
        public void TestGetLockBox()
        {
            var lockBox1 = new LockBox { LockBoxID = 1, Topic = "Test Topic 1", isOpen = false };
            var lockBox2 = new LockBox { LockBoxID = 2, Topic = "Test Topic 2", isOpen = true };
            _context.LockBoxes.Add(lockBox1);
            _context.LockBoxes.Add(lockBox2);
            _context.SaveChanges();

            var lockBox = _lockBoxesService.GetLockBox(lockBox1.LockBoxID);

            Assert.NotNull(lockBox);
            Assert.Equal(lockBox1.Topic, lockBox.Topic);
            Assert.Equal(lockBox1.isOpen, lockBox.isOpen);
        }

        [Fact]
        public void TestUpdateLockBox()
        {
            var lockBoxVM = new LockBoxVM { Topic = "Test Topic", isOpen = false };
            var lockBox = _lockBoxesService.AddLockBox(lockBoxVM);

            var updatedLockBoxVM = new LockBoxVM { Topic = "Updated Test Topic", isOpen = true };

            var updatedLockBox = _lockBoxesService.UpdateLockBox(lockBox.LockBoxID, updatedLockBoxVM);

            Assert.NotNull(updatedLockBox);
            Assert.Equal(lockBox.LockBoxID, updatedLockBox.LockBoxID);
            Assert.Equal(updatedLockBoxVM.Topic, updatedLockBox.Topic);
            Assert.True(updatedLockBox.isOpen);
        }

    }
}