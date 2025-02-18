

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;
using backend.API.Data.ViewModels;

namespace backend.API.Data.Services
{
    public class KeyService
    {
        private LockBoxContext _context;
        private readonly IMQTTClientService _client;
        public KeyService(LockBoxContext context, IMQTTClientService client)
        {
            _context = context;
            _client = client;
        }

        public bool GenerateKeyReport(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null)
                return false;

            var result = _client.IssueMQTTCommand(_lockBox.Topic, IMQTTClientService.MQTTCommands.KEY_STATUS, LockBoxID);

            if (result.Result.ReasonCode.ToString() != "Success")
                return false;

            return true;
        }
        public bool AddNewKeyReport(long lockBoxID, KeyReportStatus status)
        {

            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == lockBoxID);
            if (_lockBox == null)
                return false;

            var _keyReport = new KeyReport()
            {
                LockBox = _lockBox,
                Status = status,
                TimeStamp = DateTime.Now,
            };

            _lockBox.lastResponse = DateTime.Now;

            _context.KeyReports.Add(_keyReport);
            _context.SaveChanges();
            return true;
        }


        public KeyReportVM GetNewestKeyReport(long LockBoxID)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null)
                return null;

            var _keyReport = _context.KeyReports.Where(k => k.LockBox.LockBoxID == LockBoxID)
                .OrderByDescending(k => k.TimeStamp).FirstOrDefault();

            if (_keyReport == null)
                return null;


            var result = new KeyReportVM()
            {
                KeyReportID = _keyReport.KeyReportID,
                LockBoxID = LockBoxID,
                Status = _keyReport.Status,
                TimeStamp = _keyReport.TimeStamp
            };


            return result;
        }

        public bool AddNewKeyEvent(long LockBoxID, KeyEventActions action)
        {
            var _lockBox = _context.LockBoxes.FirstOrDefault(l => l.LockBoxID == LockBoxID);
            if (_lockBox == null)
                return false;

            var _keyEvent = new KeyEvent()
            {
                LockBox = _lockBox,
                Action = action,
                TimeStamp = DateTime.Now,
                UserID = _lockBox.lastInteractionUserID,
            };

            _lockBox.lastResponse = DateTime.Now;

            _context.KeyEvents.Add(_keyEvent);
            _context.SaveChanges();
            return true;
        }

        public async Task<PaginatedObject<KeyEventVM>> GetAllKeyEvents(long LockBoxID, int pageIndex, int pageSize)
        {
            var keyEvents = from k in _context.KeyEvents select k;
            keyEvents = keyEvents.Where(k => k.LockBox.LockBoxID == LockBoxID);
            keyEvents = keyEvents.OrderByDescending(k => k.TimeStamp);
            if (keyEvents == null)
                return null;


            var paginatedObject = await PaginatedObject<KeyEvent>.CreateAsync(keyEvents, pageIndex, pageSize);

            var VMlist = new List<KeyEventVM>();

            foreach (var keyEvent in paginatedObject.Items)
            {
                var username = _context.Users.FirstOrDefault(u => u.Id == keyEvent.UserID).Name;
                VMlist.Add(new KeyEventVM()
                {
                    LockBoxID = LockBoxID,
                    Action = keyEvent.Action,
                    TimeStamp = keyEvent.TimeStamp,
                    UserName = username
                });
            }
            return paginatedObject.ConvertTo<KeyEventVM>(VMlist, keyEvents.Count(), pageIndex, pageSize);
        }
    }



}
