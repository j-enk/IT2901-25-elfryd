using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;
using Microsoft.AspNetCore.Identity;

namespace backend.API.Data.Services
{
    public class LockBoxesService
    {
        private LockBoxContext _context;
        private readonly IMQTTClientService _client;
        private readonly BookingsService _bookingsService;
        private readonly UserManager<User> _userManager;

        public LockBoxesService(LockBoxContext context, IMQTTClientService client, BookingsService bookingsService, UserManager<User> userManager)
        {
            _context = context;
            _client = client;
            _bookingsService = bookingsService;
            _userManager = userManager;
        }

        public LockBox AddLockBox(LockBoxVM lockBox)
        {
            var _lockBox = new LockBox()
            {
                Topic = lockBox.Topic,
                isOpen = false,
            };

            _context.LockBoxes.Add(_lockBox);
            _context.SaveChanges();

            return _lockBox;
        }

        public List<LockBoxReadableVM> GetLockBoxes()
        {
            var tempArray = _context.LockBoxes.ToList();
            var LockBoxVMArray = new List<LockBoxReadableVM>();
            if (tempArray.Count > 0)
            {
                foreach (var lockBox in tempArray)
                {
                    var temp = new LockBoxReadableVM()
                    {
                        LockBoxID = lockBox.LockBoxID,
                        Topic = lockBox.Topic,
                        isOpen = lockBox.isOpen,
                        lastResponse = lockBox.lastResponse,
                        BoatNames = lockBox.Boats == null ? null : lockBox.Boats.Select(b => b.Name).ToList()
                    };
                    LockBoxVMArray.Add(temp);
                }
            }
            return LockBoxVMArray;
        }

        public LockBoxReadableVM GetLockBox(long LockBoxID)
        {
            var _lockBoxReadable = _context.LockBoxes.Where(b => b.LockBoxID == LockBoxID).Select(lockBox => new LockBoxReadableVM()
            {
                LockBoxID = lockBox.LockBoxID,
                Topic = lockBox.Topic,
                isOpen = lockBox.isOpen,
                lastResponse = lockBox.lastResponse,
                BoatNames = lockBox.Boats.Select(b => b.Name).ToList()
            }).FirstOrDefault();

            return _lockBoxReadable;
        }

        public LockBox UpdateLockBox(long LockBoxID, LockBoxVM lockBox)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(b => b.LockBoxID == LockBoxID);
            if (_lockBox != null)
            {
                _lockBox.Topic = lockBox.Topic;
                _lockBox.isOpen = lockBox.isOpen;

                _context.SaveChanges();
            }

            return _lockBox;
        }

        public void DeleteLockBox(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(b => b.LockBoxID == LockBoxID);

            if (_lockBox != null)
            {
                _context.LockBoxes.Remove(_lockBox);
                _context.SaveChanges();
            }
        }

        public async Task OpenLock(long LockBoxID, string userID)
        {

            var _user = _context.Users.FirstOrDefault(u => u.Id == userID);
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);

            if (_lockBox == null)
            {
                throw new Exception("LockBox not found");
            }
            var hasBooking = _bookingsService.GetHasBooked(userID);

            if (hasBooking == null)
            {
                throw new Exception("User has no active booking");
            }
        
            var _booking = _context.Bookings.FirstOrDefault(b => b.BookingID == hasBooking.BookingID);
            var result = await _client.IssueMQTTCommand(_lockBox.Topic, IMQTTClientService.MQTTCommands.OPEN, LockBoxID);
            var isSuccess = result.ReasonCode.ToString() == "Success";

            if (!isSuccess)
            {
                throw new Exception("Error while opening lockbox");
            }

            _lockBox.lastInteractionUserID = _user.Id;

            if (_booking.Status == 'P') _booking.Status = 'O';
            else if (_booking.Status == 'O') _booking.Status = 'D';

            await _context.SaveChangesAsync();
        }

        public async Task AdminOpenLock (long LockBoxID, string userID){
            var _user = _context.Users.FirstOrDefault(u => u.Id == userID);
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);

            if (_lockBox == null)
            {
                throw new Exception("LockBox not found");
            }

            var result = await _client.IssueMQTTCommand(_lockBox.Topic, IMQTTClientService.MQTTCommands.OPEN, LockBoxID);
            var isSuccess = result.ReasonCode.ToString() == "Success";

            if (!isSuccess)
            {
                throw new Exception("Error while opening lockbox");
            }

            _lockBox.lastInteractionUserID = _user.Id;

            await _context.SaveChangesAsync();
        }

        private bool IsBetweenSpan(DateTime time, DateTime SpanStart, DateTime SpanEnd)
        {
            return (time >= SpanStart && time <= SpanEnd);
        }


        public bool setLockBoxStatus(long LockBoxID, bool isOpen)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null) return false;
            _lockBox.isOpen = isOpen;
            _lockBox.lastResponse = DateTime.Now;

            _context.SaveChanges();
            return true;
        }

        public bool lockBoxNewResponse(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null) return false;

            _lockBox.lastResponse = DateTime.Now;
            _context.SaveChanges();

            return true;
        }

        public bool PingLockBox(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null)
                return false;

            var result = _client.IssueMQTTCommand(_lockBox.Topic, IMQTTClientService.MQTTCommands.PING, LockBoxID);

            if (result.Result.ReasonCode.ToString() != "Success")
                return false;

            return true;
        }

        public bool RequestLockStatus(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null)
                return false;

            var result = _client.IssueMQTTCommand(_lockBox.Topic, IMQTTClientService.MQTTCommands.LOCK_STATUS, LockBoxID);

            if (result.Result.ReasonCode.ToString() != "Success")
                return false;

            return true;
        }
    }
}
