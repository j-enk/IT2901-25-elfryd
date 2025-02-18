using System;
using System.Collections.Generic;

namespace backend.API.Data.ViewModels
{
    public class LockBoxVM
    {
        public long LockBoxID { get; set; }
        public string Topic { get; set; }
        public bool isOpen { get; set; }
        public DateTime lastResponse { get; set; }
        public List<long> BoatIDs { get; set; }
    }

    public class LockBoxReadableVM
    {
        public long LockBoxID { get; set; }
        public string Topic { get; set; }
        public bool isOpen { get; set; }
        public DateTime lastResponse { get; set; }
        public List<string> BoatNames { get; set; }
    }

}