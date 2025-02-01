using Microsoft.EntityFrameworkCore;
using System;

public enum KeyEventActions
{
    Removed,
    Returned,
}

namespace backend.API.Data.Models
{
    [Index(nameof(KeyEventID), IsUnique = true)]
    public class KeyEvent
    {
        public long KeyEventID { get; set; }
        public LockBox LockBox { get; set; }
        public DateTime TimeStamp { get; set; }
        public KeyEventActions Action { get; set; }
        public string UserID { get; set; }
    }
}