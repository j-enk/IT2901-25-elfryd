using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
namespace backend.API.Data.Models
{
    [Index(nameof(LockBoxID), IsUnique = true)]
    public class LockBox
    {
        public long LockBoxID { get; set; }
        public string Topic { get; set; }
        public bool isOpen { get; set; }
        public DateTime lastResponse { get; set; }
        public string lastInteractionUserID { get; set; }
        public List<Boat> Boats { get; set; }
    }
}